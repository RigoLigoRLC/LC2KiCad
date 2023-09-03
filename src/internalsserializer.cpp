/*
  Copyright (c) 2020 RigoLigoRLC.
  Copyright 2019-2020 Wokwi (for arc serialization).

  This file is part of LC2KiCad.

  LC2KiCad is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as 
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  LC2KiCad is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with LC2KiCad. If not, see <https:// www.gnu.org/licenses/>.

  Further notice on arc serialization code: this part of code is ported
  from wokwi/easyeda2kicad project, licensed under MIT license.
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <ctime>

#include "includes.hpp"
#include "rapidjson.hpp"
#include "edaclasses.hpp"
#include "smolsvg/pathreader.hpp"
#include "internalsserializer.hpp"

using std::cout;
using std::endl;
using std::fstream;
using std::string;
using std::vector;
using std::stod;
using std::stoi;
using std::to_string;
using rapidjson::FileReadStream;
using rapidjson::Document;
using rapidjson::Value;

namespace lc2kicad
{
  void LCJSONSerializer::initWorkingDocument(EDADocument *_workingDocument) { workingDocument = _workingDocument; }
  void LCJSONSerializer::deinitWorkingDocument() { workingDocument = nullptr; }

  void LCJSONSerializer::setCompatibilitySwitches(const str_dbl_map &aSwitches)
  {
    internalCompatibilitySwitches = aSwitches;

    // Set schematics coefficient
    switch(static_cast<int>(internalCompatibilitySwitches["SDV"]))
    {
      case 0: // Default
      case 1: // KiCad 5, imperial
        schematic_unit_coefficient = 10; break;
      //case 2: // KiCad 6, metric
      //  schematic_unit_coefficient = 25.4; break;
      // temporarily disabled, not implemented.
    }

    if(internalCompatibilitySwitches.count("ENL"))
     exportNestedLibs = true;
  }

  LCJSONSerializer::~LCJSONSerializer() { };

  void LCJSONSerializer::parseSchLibDocument()
  {
    assertThrow(workingDocument->module, "Internal document type mismatch: Parse an internal document as symbol with its module property set to \"false\".");
    workingDocument->docType = documentTypes::schematic_lib;

    Document &parseTarget = *workingDocument->jsonParseResult; // Create a reference for convenience.
    vector<string> canvasPropertyList, shapesList;
    string symbolName, contributor, prefix;
    str_str_map &docInfo = workingDocument->docInfo;
    Value shape, head;

    parseCommonDoucmentStructure(parseTarget, canvasPropertyList, shape, head);

    // Write canvas properties like origin and gridsize
    workingDocument->origin.X = stod(canvasPropertyList[13]);
    workingDocument->origin.Y = stod(canvasPropertyList[14]);
    workingDocument->gridSize = stod(canvasPropertyList[6]);
    coordinates origin = workingDocument->origin;

    VERBOSE_INFO(string("Document origin X") + to_string(origin.X) + " Y" + to_string(origin.Y) + \
          ", grid size " + to_string(workingDocument->gridSize));

    // Write Prefix and contributor
    assertThrow(head.HasMember("c_para"), "\"c_para\" not found.");
    assertThrow(head["c_para"].IsObject(), "Invalid \"c_para\" type: not object.");
    Value &headlist = head["c_para"];
    symbolName = headlist.HasMember("name") ? headlist["name"].IsString() ? headlist["name"].GetString() : "" : "";\

    for(unsigned int i = 0; i < shape.Size(); i++)
      shapesList.push_back(shape[i].GetString());

    if(symbolName.size() != 0)
      docInfo["documentname"] = symbolName;
    prefix = headlist.HasMember("pre") ? headlist["pre"].IsString() ? headlist["pre"].GetString() : "U" : "U";
    prefix.pop_back();
    if(prefix.size()) docInfo["prefix"] = prefix;
    else
    {
      Warn("This symbol library has an empty prefix. It's replaced with UNK now.");
      docInfo["prefix"] = "UNK"; // Prefix could be empty after popping trailing '?', therefore we add UNK for those.
    }
    docInfo["contributor"] = headlist.HasMember("Contributor") ? headlist["Contributor"].IsString() ?
                                   headlist["Contributor"].GetString() : "" : "" ;

    parseSchLibComponent(shapesList, static_cast<Schematic_Module*>(workingDocument->containedElements.back())->containedElements);
  }

  void LCJSONSerializer::parseSchLibComponent(vector<string> &shapesList, vector<Schematic_Element*> &containedElements)
  {
    for(auto &i : shapesList)
    {
      // vector<string> parameters = splitString(shapesList[i], '~');
      switch(i[0])
      {
        case 'P':
          switch(i[1])
          {
            case 'G': // Polygon
              containedElements.push_back(parseSchPolygon(i));
              break;
            case 'I': // Pie
              break;
            case 'L': // Polyline
              containedElements.push_back(parseSchPolyline(i));
              break;
            case 'T': // Path
              break;
            case 'i': // ImageInTheGrid
              break;
            default: // Pin
              containedElements.push_back(parseSchPin(i));
              break;
          }
          break;
        case 'R': // Rectangle
          containedElements.push_back(parseSchRect(i));
          break;
        case 'A':
          switch(i[1])
          {
            case 'R': // Arrowhead
              break;
            default: // Arc
              containedElements.push_back(parseSchArc(i));
              break;
          }
          break;
        case 'B':
          switch(i[1])
          {
            case 'E': // Bus Entry
              break;
            default: // Bus
              break;
          }
          break;
        case 'I': // Image
          break;
        case 'L': // Line
          break;
        case 'C': // Circle
          break;
        case 'E': // Ellipse
          break;
        case 'T': // Annotations
          break;
        case 'N': // Netlabels
          break;
        case 'F': // Netflags (Netports)
          break;
        case 'W': // Wire
          break;
        case 'J': // Junction
          break;
        case 'O': // No Connect Flag
          break;
        default:
          assertThrow(false, "Invalid element string <<<" + i + ">>>.");
      }
    }
  }

  void LCJSONSerializer::parsePCBDocument()
  {
    assertThrow(!workingDocument->module, "Internal document type mismatch: Parse an internal document as PCB with its module property set to \"true\".");
    workingDocument->docType = documentTypes::pcb;

    Document &parseTarget = *workingDocument->jsonParseResult; // Create a reference for convenience.
    vector<string> canvasPropertyList, shapesList;
    vector<int> layerMapper;
    string footprintName, contributor;
    str_str_map &docInfo = workingDocument->docInfo;
    Value shape, head;

    (void)docInfo; // Supress warning

    parseCommonDoucmentStructure(parseTarget, canvasPropertyList, shape, head);

    // Write canvas properties like origin and gridsize
    workingDocument->origin.X = stod(canvasPropertyList[16]);
    workingDocument->origin.Y = stod(canvasPropertyList[17]);
    workingDocument->gridSize = stod(canvasPropertyList[6]);
    coordinates origin = workingDocument->origin;

    VERBOSE_INFO(string("Document origin X") + to_string(origin.X) + " Y" + to_string(origin.Y) + \
          ", grid size " + to_string(workingDocument->gridSize));

    for(unsigned int i = 0; i < shape.Size(); i++)
      shapesList.push_back(shape[i].GetString());

    parsePCBLibComponent(shapesList, workingDocument->containedElements);


  }

  void LCJSONSerializer::parsePCBLibDocument()
  {
    assertThrow(workingDocument->module, "Internal document type mismatch: Parse an internal document as footprint with its module property set to \"false\".");
    workingDocument->docType = documentTypes::pcb_lib;

    Document &parseTarget = *workingDocument->jsonParseResult; // Create a reference for convenience.
    vector<string> canvasPropertyList, shapesList;
    vector<int> layerMapper;
    string footprintName, contributor;
    str_str_map &docInfo = workingDocument->docInfo;
    Value shape, head;

    parseCommonDoucmentStructure(parseTarget, canvasPropertyList, shape, head);

    // Write canvas properties like origin and gridsize
    workingDocument->origin.X = stod(canvasPropertyList[16]);
    workingDocument->origin.Y = stod(canvasPropertyList[17]);
    workingDocument->gridSize = stod(canvasPropertyList[6]);
    coordinates origin = workingDocument->origin;

    VERBOSE_INFO(string("Document origin X") + to_string(origin.X) + " Y" + to_string(origin.Y) + \
          ", grid size " + to_string(workingDocument->gridSize));

    // Write Prefix and contributor
    assertThrow(head.HasMember("c_para"), "\"c_para\" not found.");
    assertThrow(head["c_para"].IsObject(), "Invalid \"c_para\" type: not object.");
    Value &headlist = head["c_para"];
    footprintName = headlist.HasMember("package") ?
                      headlist["package"].IsString() ?
                        escapeQuotedString(headlist["package"].GetString()) :
                      "" :
                    "";

    for(unsigned int i = 0; i < shape.Size(); i++)
      shapesList.push_back(shape[i].GetString());

    if(footprintName.size() != 0)
      docInfo["documentname"] = footprintName;
    docInfo["contributor"] = headlist.HasMember("Contributor") ? headlist["Contributor"].IsString() ?
                                   headlist["Contributor"].GetString() : "" : "";

    processingModule = true;

    parsePCBLibComponent(shapesList, static_cast<PCB_Module*>(workingDocument->containedElements.back())->containedElements);

    processingModule = false;
  }

  list<EDADocument*> LCJSONSerializer::parseSchNestedLibs()
  {
    assertThrow(!workingDocument->module, "Internal document type mismatch: Parse an internal document as schematics with its module property set to \"true\".");
    workingDocument->docType = schematic;
    map<string, RAIIC<EDADocument>> prepareList; // UUID<->Component pair
    list<EDADocument*> ret;
    stringlist canvasPropertyList;
    Value shape, head;

    Document &parseTarget = *workingDocument->jsonParseResult; // Create a reference for convenience.

    parseCommonDoucmentStructure(parseTarget, canvasPropertyList, shape, head);

    // Write canvas properties like origin and gridsize
    workingDocument->origin.X = stod(canvasPropertyList[13]);
    workingDocument->origin.Y = stod(canvasPropertyList[14]);
    workingDocument->gridSize = stod(canvasPropertyList[10]);
    coordinates origin = workingDocument->origin;

    VERBOSE_INFO(string("SchSheet origin X") + to_string(origin.X) + " Y" + to_string(origin.Y) + \
          ", grid size " + to_string(workingDocument->gridSize));

    stringlist shapesList;
    string shapeStringTmp;
    for(unsigned int i = 0; i < shape.Size(); i++)
    {
      shapeStringTmp = shape[i].GetString();
      if(shapeStringTmp.find("LIB~", 0, 4) == 0) // Get all shapes begin with "LIB~"
        shapesList.push_back(shapeStringTmp);
    }

    for(auto &i : shapesList)
    {
      RAIIC<EDADocument> t;
      t->origin = origin;
      t->module = true;
      RAIIC<Schematic_Module> m = parseSchModuleString(i, !t, &prepareList); // Try parse module without knowing if identical ones were processed
      if(!m) // If there is an identical one then m is nullptr and both RAIIC managed memory blocks will be cleared out.
      {
        VERBOSE_INFO("SchNestedLib ID=" + m->uuid);
        t->containedElements.push_back(!++m); // Protect the module and push it back into Document
        prepareList[static_cast<Schematic_Module*>((!t)->containedElements.back())->uuid] = --t; // operator-- on RAIIC means one destruction will be ignored.
      }
    }

    for(auto &i : prepareList)
    {
      ret.push_back(!++(i.second));
      EDADocument *doc = ret.back();
      map<string, string> &cpara = static_cast<Schematic_Module*>(doc->containedElements.back())->cparaContent;
      doc->docInfo["documentname"] = static_cast<Schematic_Module*>(doc->containedElements.back())->name;
      doc->docInfo["contributor"] = cpara["contributor"];
      doc->docInfo["prefix"] = cpara["spicePre"]; // TODO: Proper prefix? But we would assume spicePre is identical with normal prefix
      doc->pathToFile = workingDocument->pathToFile + "__" + doc->docInfo["documentname"];
      doc->docType = schematic_lib;
    }

    return ret;
  }

  list<EDADocument*> LCJSONSerializer::parsePCBNestedLibs()
  {
    assertThrow(!workingDocument->module, "Internal document type mismatch: Parse an internal document as PCB with its module property set to \"true\".");
    workingDocument->docType = pcb;
    map<string, RAIIC<EDADocument>> prepareList; // UUID<->Component pair
    list<EDADocument*> ret;
    stringlist canvasPropertyList;
    Value shape, head;

    Document &parseTarget = *workingDocument->jsonParseResult; // Create a reference for convenience.

    parseCommonDoucmentStructure(parseTarget, canvasPropertyList, shape, head);

    // Write canvas properties like origin and gridsize
    workingDocument->origin.X = stod(canvasPropertyList[16]);
    workingDocument->origin.Y = stod(canvasPropertyList[17]);
    workingDocument->gridSize = stod(canvasPropertyList[6]);
    coordinates origin = workingDocument->origin;

    VERBOSE_INFO(string("Document origin X") + to_string(origin.X) + " Y" + to_string(origin.Y) + \
          ", grid size " + to_string(workingDocument->gridSize));

    stringlist shapesList;
    string shapeStringTmp;
    for(unsigned int i = 0; i < shape.Size(); i++)
    {
      shapeStringTmp = shape[i].GetString();
      if(shapeStringTmp.find("LIB~", 0, 4) == 0) // Get all shapes begin with "LIB~"
        shapesList.push_back(shapeStringTmp);
    }

    for(auto &i : shapesList)
    {
      RAIIC<EDADocument> t;
      t->origin = origin;
      t->module = true;
      RAIIC<PCB_Module> m = parsePCBModuleString(i, !t, &prepareList); // Try parse module without knowing if identical ones were processed
      if(!m) // If there is an identical one then m is nullptr and both RAIIC managed memory blocks will be cleared out.
      {
        t->containedElements.push_back(!++m); // Protect the module and push it back into Document
        prepareList[static_cast<PCB_Module*>((!t)->containedElements.back())->uuid] = --t; // operator-- on RAIIC means one destruction will be ignored.
      }
    }

    // When we got errors of any kind, RAIIC will handle the dynamic memory. Now we're not errored out,
    // so we move everything into a retval vector and process misc stuff.
    for(auto &i : prepareList)
    {
      ret.push_back(!++(i.second));
      EDADocument *doc = ret.back();
      map<string, string> &cpara = static_cast<PCB_Module*>(doc->containedElements.back())->cparaContent;
      doc->docInfo["documentname"] = static_cast<PCB_Module*>(doc->containedElements.back())->name;
      doc->docInfo["contributor"] = cpara["contributor"];
      doc->pathToFile = workingDocument->pathToFile + "__" + doc->docInfo["documentname"];
      doc->docType = pcb_lib;
    }

    return ret;
  }

  void LCJSONSerializer::parsePCBLibComponent(vector<string> &shapesList, vector<EDAElement*> &containedElements)
  {
    for(auto &i : shapesList)
    {
      // vector<string> parameters = splitString(shapesList[i], '~');
      switch(i[0])
      {
        case 'P':
          switch(i[1])
          {
            case 'A': // Pad
              if(processingModule)
                containedElements.push_back(parsePCBPadString(i));
              else
                containedElements.push_back(parsePCBDiscretePadString(i));
              break;
            case 'R': // Protractor
              break;
            case 'L': // PlanarZone (negative)
              containedElements.push_back(parsePCBPlaneZoneString(i));
              break;
            default:
              Error("Invalid element string <<<" + i + ">>>.");
          }
          break;
        case 'T':
          switch(i[1])
          {
            case 'E': // Text
              containedElements.push_back(parsePCBTextString(i));
              break;
            case 'R': // Track
            {
              stringlist tmp = splitString(i, '~');
              if(judgeIsOnCopperLayer(EasyEdaToKiCadLayerMap[stoi(loadNthSeparated(i, '~', 2))]))
                containedElements.push_back(parsePCBCopperTrackString(i));
              else
                containedElements.push_back(parsePCBGraphicalTrackString(i));
              break;
            }
            default:
              Error("Invalid element string <<<" + i + ">>>.");
          }
          break;
        case 'C':
          switch(i[1])
          {
            case 'O': // CopperArea
              containedElements.push_back(parsePCBFloodFillString(i));
              break;
            case 'I': // Circle
              if(judgeIsOnCopperLayer(EasyEdaToKiCadLayerMap[stoi(loadNthSeparated(i, '~', 5))]))
                containedElements.push_back(parsePCBCopperCircleString(i));
              else
                containedElements.push_back(parsePCBGraphicalCircleString(i));
              break;
            default:
              Error("Invalid element string <<<" + i + ">>>.");
          }
          break;
        case 'R': // Rect
          containedElements.push_back(parsePCBRectString(i));
          break;
        case 'A': // Arc
        {
          if(judgeIsOnCopperLayer(EasyEdaToKiCadLayerMap[stoi(loadNthSeparated(i, '~', 2))]))
            containedElements.push_back(parsePCBCopperArcString(i));
          else
            containedElements.push_back(parsePCBGraphicalArcString(i));
          break;
        }
        case 'V': // Via
          containedElements.push_back(parsePCBViaString(i));
          break;
        case 'H': // Hole
          containedElements.push_back(parsePCBHoleString(i));
          break;
        case 'D': // Dimension
          break;
        case 'S':
        {
          switch(i[1])
          {
            case 'V': // SVGNODE
              // Discarding SVGNODE objects usually doesn't result in broken boards,
              // therefore I decided to move it into verbose info.
              VERBOSE_INFO("An SVGNODE object has been discarded.");
              break;
            case 'O': // Solidregion
              if(!processingModule)
              {
                auto type = loadNthSeparated(i, '~', 4);
                if(type == "solid")
                  if(judgeIsOnCopperLayer(EasyEdaToKiCadLayerMap[stoi(loadNthSeparated(i, '~', 1))]))
                    containedElements.push_back(parsePCBCopperSolidRegionString(i));
                  else
                    containedElements.push_back(parsePCBGraphicalSolidRegionString(i));
                else if(type == "npth")
                  containedElements.push_back(parsePCBNpthRegionString(i));
                else if(type == "cutout")
                  containedElements.push_back(parsePCBKeepoutRegionString(i));
              }
              else
              {

                auto type = loadNthSeparated(i, '~', 4);
                if(type == "solid")
                  if(!judgeIsOnCopperLayer(EasyEdaToKiCadLayerMap[stoi(loadNthSeparated(i, '~', 1))]))
                    containedElements.push_back(parsePCBGraphicalSolidRegionString(i));
                  else
                    Warn(loadNthSeparated(i, '~', 5) +
                         ": A copper region was found inside a footprint, which is not allowed in KiCad. "
                         "This region is discarded!");
                else if(type == "npth")
                  containedElements.push_back(parsePCBNpthRegionString(i));
                else if(type == "cutout")
                  containedElements.push_back(parsePCBKeepoutRegionString(i));
                // Can we move the region into main board? Probably not, cause we can't.
                // That's how LC2KiCad was constructed. You can't put an element into board,
                // because we can only see the containedElements of the footprint in this function.
              }
              break;
            default:
              Error("Invalid element string <<<" + i + ">>>.");
          }
          break;
        }
        case 'L': // Footprint
          containedElements.push_back(parsePCBModuleString(i));
          break;
        default:
          assertThrow(false, "Invalid element string <<<" + i + ">>>.");
      }
    }
  }

  void LCJSONSerializer::parseCommonDoucmentStructure(rapidjson::Document &parseTarget,
                            std::vector<std::string> &canvasPropertyList,
                            rapidjson::Value &shapesArray,
                            rapidjson::Value &headObject)
  {
    assertThrow(parseTarget.HasMember("head"), "\"head\" not found.");
    assertThrow(parseTarget["head"].IsObject(), "Invalid \"head\" type: not object.");
    headObject = parseTarget["head"].GetObject();

    assertThrow(parseTarget.HasMember("canvas"), "\"canvas\" not found.");
    assertThrow(parseTarget["canvas"].IsString(), "Invalid \"canvas\" type: not string.");
    string canvasPropertyString = parseTarget["canvas"].GetString();
    canvasPropertyList = splitString(canvasPropertyString, '~');

    assertThrow(parseTarget.HasMember("shape"), "\"shape\" not found.");
    assertThrow(parseTarget["shape"].IsArray(), "Invalid \"shape\" type: not array.");
    shapesArray = parseTarget["shape"].GetArray();
  }

  void LCJSONSerializer::parsePCBDRCRules(rapidjson::Value &drcRules)
  {
    ASSERT_RETURN_MSG(drcRules.IsObject(), "Invalid DRC entry.");

    auto drcRulesObject = drcRules.GetObject();
    vector<PCBNetClass> netClassStorage;
    for(auto &i : drcRulesObject)
    {
      if(i.value.GetType() != rapidjson::kObjectType) continue;
      auto drcNetclass = i.value.GetObject();
      PCBNetClass netClass;
      for(auto &j : drcNetclass)
      {
        if(j.name == "nets")
        {
          ASSERT_RETURN_MSG(j.value.IsArray(), "Invalid DRC nets entry.");
          auto drcNets = j.value.GetArray();
          for(auto &k : drcNets)
          {
            ASSERT_RETURN_MSG(k.IsString(), "Invalid DRC netclass member found.");
            netClass.netClassMembers.emplace_back(k.GetString());
          }
        }
        else
        {
          ASSERT_RETURN_MSG(j.value.IsDouble(), string("Invalid DRC rule entry ") + j.name.GetString());
          if(j.name == "trackWidth")
            netClass.rules["trace_width"] = j.value.GetDouble() * tenmils_to_mm_coefficient;
          else if(j.name == "clearance")
            netClass.rules["clearance"] = j.value.GetDouble() * tenmils_to_mm_coefficient;
          else if(j.name == "viaHoleDiameter")
            netClass.rules["via_dia"] = j.value.GetDouble() * tenmils_to_mm_coefficient;
          else if(j.name == "viaHoleD")
            netClass.rules["via_drill"] = j.value.GetDouble() * tenmils_to_mm_coefficient;
          else
            Warn(string("Unrecognized DRC rule entry ") + j.name.GetString());
        }
        ASSERT_RETURN_MSG(i.name.IsString(), "Invalid DRC netclass name found.");
        netClass.name = i.name.GetString();
        netClassStorage.emplace_back(netClass);
      }
      // No asserts has been triggered. Transfer to PCB Document.
      static_cast<PCBDocument*>(workingDocument)->netClasses = netClassStorage;
    }
  }

  /**
   * The below section is for PCB elements serializing.
   */

  PCB_Pad* LCJSONSerializer::parsePCBPadString(const string &LCJSONString)
  {
    RAIIC<PCB_Pad> result;
    stringlist paramList = splitString(LCJSONString, '~');
    stringlist polygonDrillCoordsString;

    result->id = paramList[12]; // GGE ID.

    // Resolve pad shape
    switch (paramList[1][0])
    {
      case 'E': // ELLIPSE, ROUND
        result->padShape = PCBPadShape::circle;
        break;
      case 'O': // OVAL
        result->padShape = PCBPadShape::oval;
        break;
      case 'R': // RECT
        result->padShape = PCBPadShape::rectangle;
        break;
      case 'P': // POLYGON
        result->padShape = PCBPadShape::polygon;
        break;
      default:
        assertThrow(false, result->id + string(": Invalid pad shape: ") + paramList[12]);
        break;
    }

    // Resolve pad coordinates
    if(result->padShape == PCBPadShape::polygon)
    {
      polygonDrillCoordsString = splitString(paramList[19], ',');
      result->padCoordinates.X = (stod(polygonDrillCoordsString[0]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
      result->padCoordinates.Y = (stod(polygonDrillCoordsString[1]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
      result->orientation = 0;
    }
    else
    {
      result->padCoordinates.X = (stod(paramList[2]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
      result->padCoordinates.Y = (stod(paramList[3]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
      result->orientation = (stod(paramList[11]));
    }

    // Resolve pad shape and size
    if(result->padShape == PCBPadShape::oval || result->padShape == PCBPadShape::rectangle)
    {
      result->padSize.X = stod(paramList[4]) * tenmils_to_mm_coefficient;
      result->padSize.Y = stod(paramList[5]) * tenmils_to_mm_coefficient;
    }
    else if(result->padShape == PCBPadShape::circle)
      result->padSize.X = result->padSize.Y = stod(paramList[4]) * tenmils_to_mm_coefficient;
    else // polygon
    {
      result->padSize.X = result->padSize.Y = result->holeSize.Y;
      vector<string> polygonCoordinates = splitString(paramList[10], ' ');
      coordinates polygonPointTemp = { 0.0, 0.0 };
      /**
       * EasyEDA rotate their polygon pads at the "pad center" where the drill has its own coordinates,
       * while KiCad rotate the polygon pads at the drill center since the drill is the pad origin.
       * Here we need to manipulate the pad a little bit so it would not cause problems.
       */
      for(unsigned int i = 0; i < polygonCoordinates.size(); i += 2)
      {
        polygonPointTemp.X = (stod(polygonCoordinates[  i  ]) - stod(polygonDrillCoordsString[0])) * tenmils_to_mm_coefficient;
        polygonPointTemp.Y = (stod(polygonCoordinates[i + 1]) - stod(polygonDrillCoordsString[1])) * tenmils_to_mm_coefficient;
        result->shapePolygonPoints.push_back(polygonPointTemp);
      }
    }

    // Resolve pad type
    int padTypeTemp = stoi(paramList[6]);
    if(padTypeTemp == 11)
      if(paramList[15] == "Y")
        result->padType = PCBPadType::through;
      else
        result->padType = PCBPadType::noplating;
    else
    if(padTypeTemp == 1)
      result->padType = PCBPadType::top;
    else if(padTypeTemp == 2)
      result->padType = PCBPadType::bottom;
    if(padTypeTemp == 11) // Fix: Only parse hole size when the pad is a through-hole pad.
    {
      // Resolve hole shape size
      result->holeSize.X = stod(paramList[13]) * tenmils_to_mm_coefficient;
      result->holeSize.Y = (stod(paramList[9]) * 2 * tenmils_to_mm_coefficient);
      result->holeSize.X == 0.0f ? result->holeSize.X = result->holeSize.Y, result->holeShape = PCBHoleShape::circle : result->holeShape = PCBHoleShape::slot;
      /**
       * Fix: EasyEDA determines the slot direction by pad size.
       * When rotation is 0 degrees and the pad has a circular/polygonal shape, slot drill is horizontal.
       * When the pad is rectangular or oval shape, when X is smaller than Y, slot drill will become vertical;
       * when X is greater or equal to Y, slot drill will remain horizontal. Great design!
       */
      if(result->padSize.X < result->padSize.Y)
        result->holeSize.swapXY();
    }
    // store net name
    static_cast<PCBDocument*>(workingDocument)->netManager.setNet(paramList[7], result->net);
    result->pinNumber = paramList[8];

    //workingDocument->containedElements.push_back(!++result);
    return !++result;
  }

  PCB_Module *LCJSONSerializer::parsePCBDiscretePadString(const string &LCJSONString)
  {
    RAIIC<PCB_Module> result;
    RAIIC<PCB_Pad> pad = parsePCBPadString(LCJSONString);
    RAIIC<PCB_Text> ref;

    result->id = pad->id;
    result->name = "DiscretePad" + pad->id;
    result->moduleCoords = pad->padCoordinates;
    result->orientation = pad->orientation;
    pad->padCoordinates = { 0, 0 };
    pad->orientation = 0;
    ref->type = PCBTextTypes::PackageReference;
    ref->layerKiCad = KiCadLayerIndex::F_Fab;
    ref->text = "Pad" + pad->id;
    ref->height = 4 * tenmils_to_mm_coefficient;
    ref->width = 0.4 * tenmils_to_mm_coefficient;
    ref->midLeftPos = { -5, -5 };
    ref->orientation = 0.0;
    ref->visibility = false;

    switch(pad->padType)
    {
      default:
      case PCBPadType::top:
        result->layer = KiCadLayerIndex::F_Cu;
        result->topLayer = true;
        break;
      case PCBPadType::bottom:
        result->layer = KiCadLayerIndex::B_Cu;
        result->topLayer = false;
        break;
    }

    result->containedElements.push_back(!++pad);
    result->containedElements.push_back(!++ref);

    result->updateTime = time(nullptr);

    return !++result;
  }

  PCB_Hole* LCJSONSerializer::parsePCBHoleString(const string &LCJSONString)
  {
    RAIIC<PCB_Hole> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[4]; // GGE ID.

    result->holeCoordinates.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->holeCoordinates.Y = (stod(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    result->holeDiameter = stod(paramList[3]) * 2 * tenmils_to_mm_coefficient;

    return !++result;
  }

  PCB_Via* LCJSONSerializer::parsePCBViaString(const string &LCJSONString)
  {
    RAIIC<PCB_Via> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.

    // Resolving the via coordinates
    result->holeCoordinates.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->holeCoordinates.Y = (stod(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    // Resolve via diameter (size)
    result->viaDiameter = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->holeDiameter = stod(paramList[5]) * tenmils_to_mm_coefficient * 2; // Hole "holeR" is radius.

    static_cast<PCBDocument*>(workingDocument)->netManager.setNet(paramList[4], result->net);

    return !++result;
  }

  PCB_CopperTrack* LCJSONSerializer::parsePCBCopperTrackString(const string &LCJSONString)
  {
    RAIIC<PCB_CopperTrack> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[5];

    // Resolve track width
    result->width = stod(paramList[1]) * tenmils_to_mm_coefficient;

    // Resolve track layer
    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[2])];
    assertThrow(result->layerKiCad != KiCadLayerIndex::Invalid, result->id + (": Invalid layer for TRACK " + paramList[3]));
    static_cast<PCBDocument*>(workingDocument)->netManager.setNet(paramList[3], result->net);

    // Resolve track points
    stringlist pointsStrList = splitString(paramList[4], ' ');
    coordinates tempCoord;
    for(unsigned int i = 0; i < pointsStrList.size(); i += 2)
    {
      tempCoord.X = (stod(pointsStrList[i]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
      tempCoord.Y = (stod(pointsStrList[i + 1]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
      result->trackPoints.push_back(tempCoord);
    }

    return !++result;
  }

  PCB_GraphicalTrack* LCJSONSerializer::parsePCBGraphicalTrackString(const string &LCJSONString)
  {
    RAIIC<PCB_GraphicalTrack> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[5]; // GGE ID.

    // Resolve track width
    result->width = stod(paramList[1]) * tenmils_to_mm_coefficient;

    // Resolve track layer
    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[2])];
    if(result->layerKiCad == KiCadLayerIndex::Invalid)
    {
      VERBOSE_INFO(result->id + ": Invalid layer " + paramList[2] + " for graphical TRACK");
      return nullptr;
    }

    // Resolve track points
    stringlist pointsStrList = splitString(paramList[4], ' ');
    coordinates tempCoord;
    for(unsigned int i = 0; i < pointsStrList.size(); i += 2)
    {
      tempCoord.X = (stod(pointsStrList[i]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
      tempCoord.Y = (stod(pointsStrList[i + 1]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
      result->trackPoints.push_back(tempCoord);
    }

    return !++result;
  }

  PCB_FloodFill* LCJSONSerializer::parsePCBFloodFillString(const string &LCJSONString)
  {
    RAIIC<PCB_FloodFill> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[7]; // GGE ID.

    // Resolve layer ID and net name
    static_cast<PCBDocument*>(workingDocument)->netManager.setNet(paramList[3], result->net);

    // Old EasyEDA file omits the priority. Send a warning and set that to highest if this happened.
    if(!paramList[13].size())
    {
      Warn(result->id + ": Empty flood fill priority. Will be set to highest.");
      paramList[13] = "1";
    }
    static_cast<PCBDocument*>(workingDocument)->fillPriorityManager.logPriority(result->EasyEDAPriority = stoi(paramList[13]));
    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[2])];
    // Throw error with gge ID if layer is invalid
    assertThrow(result->layerKiCad != -1, result->id + ": Invalid layer for COPPERAREA " + paramList[7]);

    // Resolve track points
    auto path = SmolSVG::readPathString(paramList[4]);

    for(auto &i : *path)
      result->fillAreaPolygonPoints.emplace_back(
              (i->getConstStartPoint().nativeCoord() - workingDocument->origin) * tenmils_to_mm_coefficient);
    delete path;

    result->clearanceWidth = stod(paramList[5]) * tenmils_to_mm_coefficient; // Resolve clearance width
    result->fillStyle = (paramList[6] == "solid" ? floodFillStyle::solidFill : floodFillStyle::noFill);
    // Resolve fill style
    result->isSpokeConnection = (paramList[8] == "spoke" ? true : false); // Resolve connection type
    result->isPreservingIslands = (paramList[9] == "yes" ? true : false); // Resolve island keep
    result->minimumWidth = 0.254; // 20 mils; KiCad default.
    result->spokeWidth = stod(paramList[18]) * tenmils_to_mm_coefficient;
    if(result->spokeWidth <= 0)
    {
      result->spokeWidth = 0.508;
      Info(result->id + ": Flood fill area spoke width was not set; it is now 0.508mm.");
    }
    if(result->spokeWidth <= 0.254)
    {
      result->minimumWidth = result->spokeWidth;
      Info(result->id + ": Flood fill area spoke width seems low (" + to_string(result->spokeWidth) + "mm). "
                                                      "Minimum width was set to the spoke width automatically from 0.254mm.");
    }

    return !++result;
  }

  PCB_KeepoutRegion *LCJSONSerializer::parsePCBKeepoutRegionString(const string &LCJSONString)
  {
    RAIIC<PCB_KeepoutRegion> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[5];

    result->allowRouting = result->allowVias = true;
    result->allowFloodFill = false;
    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[1])];

    auto path = SmolSVG::readPathString(paramList[3]);
    for(auto &i : *path)
      result->fillAreaPolygonPoints.emplace_back(
              (i->getConstStartPoint().nativeCoord() - workingDocument->origin) * tenmils_to_mm_coefficient);
    delete path;

    Warn(result->id + ": Flood fill keepout regions will prevent all fills rather than just flood fills with "
                      "lower priority. This is a behavior difference. You've been warned.");
    return !++result;
  }

  PCB_GraphicalTrack *LCJSONSerializer::parsePCBNpthRegionString(const string &LCJSONString)
  {
    RAIIC<PCB_GraphicalTrack> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[5];
    result->layerKiCad = Edge_Cuts;

    // Resolve track points
    auto path = SmolSVG::readPathString(paramList[3]);

    for(auto &i : *path)
      result->trackPoints.emplace_back(
              (i->getConstStartPoint().nativeCoord() - workingDocument->origin) * tenmils_to_mm_coefficient);
    result->trackPoints.emplace_back(
            (path->getLastCommand()->getConstEndPoint().nativeCoord() - workingDocument->origin) * tenmils_to_mm_coefficient);
    delete path;

    result->width = 0.1;

    return !++result;
  }

  PCB_GraphicalSolidRegion *LCJSONSerializer::parsePCBGraphicalSolidRegionString(const std::string &LCJSONString)
  {
    RAIIC<PCB_GraphicalSolidRegion> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[5];

    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[1])];

    // Fail gracefully if you got an area on an invalid layer
    if(result->layerKiCad == Invalid)
    {
      VERBOSE_INFO(result->id + ": Invalid layer " + paramList[1] + " for graphical SOLIDREGION");
      return nullptr;
    }

    // Resolve track points
    auto path = SmolSVG::readPathString(paramList[3]);

    for(auto &i : *path)
      result->fillAreaPolygonPoints.emplace_back(
              (i->getConstStartPoint().nativeCoord() - workingDocument->origin) * tenmils_to_mm_coefficient);
    delete path;

    return !++result;

    return !++result;
  }

  PCB_FloodFill* LCJSONSerializer::parsePCBCopperSolidRegionString(const string &LCJSONString)
  {
    RAIIC<PCB_FloodFill> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[5];
    // Resolve layer ID and net name
    static_cast<PCBDocument*>(workingDocument)->netManager.setNet(paramList[2], result->net);
    result->EasyEDAPriority = 0;
    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[1])];
    // Throw error with gge ID if layer is invalid
    assertThrow(result->layerKiCad != -1, result->id + ": Invalid layer for copper SOLIDREGION");

    // Resolve track points
    auto path = SmolSVG::readPathString(paramList[3]);

    for(auto &i : *path)
      result->fillAreaPolygonPoints.emplace_back(
                  (i->getConstStartPoint().nativeCoord() - workingDocument->origin) * tenmils_to_mm_coefficient);
    delete path;

    return !++result;
  }

  PCB_FloodFill *LCJSONSerializer::parsePCBPlaneZoneString(const string &LCJSONString)
  {
    RAIIC<PCB_FloodFill> result;
    stringlist parts = splitByString(LCJSONString, "#@$");

    ASSERT_RETNULLPTR_MSG(parts.size() == 2, "Invalid PLANEZONE <<<" + LCJSONString + ">>>.");

    stringlist paramList = splitString(parts[0], '~'),
               pathList = splitString(parts[1], '~');

    result->id = paramList[4]; // We use the GGE ID of zone instead of the path.

    Info(result->id + ": Plane zone has been converted to a flood fill zone. "
                        "You'll need to delete the tracks used to separate the zones.");

    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[1])];
    static_cast<PCBDocument*>(workingDocument)->netManager.setNet(paramList[2], result->net);

    pathList[1].pop_back(); // Remove trailing Z
    pathList[1][0] = '0'; // Remove leading M
    stringlist pointsList = splitString(pathList[1], ' '); // Split point data

    for(auto &i : pointsList)
    {
      stringlist pointCoord = splitString(i, ',');
      result->fillAreaPolygonPoints.emplace_back(
            (coordinates(stoi(pointCoord[0]), stoi(pointCoord[1])) - workingDocument->origin)
            * tenmils_to_mm_coefficient
          );
    }

    return !++result;
  }

  PCB_CopperCircle* LCJSONSerializer::parsePCBCopperCircleString(const string &LCJSONString)
  {
    RAIIC<PCB_CopperCircle> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.

    result->center.X = stod(paramList[1]) * tenmils_to_mm_coefficient;
    result->center.Y = stod(paramList[2]) * tenmils_to_mm_coefficient;
    result->radius = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->width = stod(paramList[4]) * tenmils_to_mm_coefficient;
    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[5])];
    static_cast<PCBDocument*>(workingDocument)->netManager.setNet(paramList[8], result->net);

    return !++result;
  }

  PCB_GraphicalCircle* LCJSONSerializer::parsePCBGraphicalCircleString(const string &LCJSONString)
  {
    RAIIC<PCB_GraphicalCircle> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.
    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[5])];
    if(result->layerKiCad == Invalid)
    {
      VERBOSE_INFO(result->id + ": Invalid layer " + paramList[5] + " for graphical ARC");
      return nullptr;
    }

    result->center.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->center.Y = (stod(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    result->radius = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->width = stod(paramList[4]) * tenmils_to_mm_coefficient;

    return !++result;
  }

  /*
    The arc serialization code is ported from wokwi/easyeda2kicad.
    Original: https://github.com/wokwi/easyeda2kicad/blob/master/src/board.ts
  */

  PCB_CopperArc *LCJSONSerializer::parsePCBCopperArcString(const string &LCJSONString)
  {
    RAIIC<PCB_CopperArc> result;
    stringlist paramList = splitString(LCJSONString, '~'), arcCmdParams, movetoCmdParams;

    result->id = paramList[6]; // GGE ID

    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[2])];
    result->width = stod(paramList[1]) * tenmils_to_mm_coefficient;
    static_cast<PCBDocument*>(workingDocument)->netManager.setNet(paramList[3], result->net);

    // Resolve track points
    auto path = SmolSVG::readPathString(paramList[4]);
    coordinates startpoint = path->getLastCommand()->getConstStartPoint(),
            endpoint = path->getLastCommand()->getConstEndPoint();
    auto &smolArcCmd = *(static_cast<SmolSVG::commandEllipticalArcTo*>(path->getLastCommand()));

    centerArc resultArc = svgEllipticalArcComputation(startpoint.X, startpoint.Y, smolArcCmd.getRadii().X,
                                                      smolArcCmd.getRadii().Y, smolArcCmd.getXAxisRotation(), smolArcCmd.getLargeArc(),
                                                      smolArcCmd.getFlagSweep(), endpoint.X, endpoint.Y);

    result->center = (resultArc.center - workingDocument->origin) * tenmils_to_mm_coefficient;
    result->angle = std::abs(resultArc.angleExtend);
    result->endPoint = ((smolArcCmd.getFlagSweep() ? startpoint : endpoint) - workingDocument->origin) * tenmils_to_mm_coefficient;

    return !++result;

  }

  PCB_GraphicalArc *LCJSONSerializer::parsePCBGraphicalArcString(const string &LCJSONString)
  {
    RAIIC<PCB_GraphicalArc> result;
    stringlist paramList = splitString(LCJSONString, '~'), arcCmdParams, movetoCmdParams;

    result->id = paramList[6]; // GGE ID

    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[2])];
    if(result->layerKiCad == Invalid)
    {
      VERBOSE_INFO(result->id + ": Invalid layer " + paramList[2] + " for ARC");
      return nullptr;
    }

    result->width = stod(paramList[1]) * tenmils_to_mm_coefficient;

    // Resolve track points
    auto path = SmolSVG::readPathString(paramList[4]);
    coordinates startpoint = path->getLastCommand()->getConstStartPoint(),
                endpoint = path->getLastCommand()->getConstEndPoint();
    auto &smolArcCmd = *(static_cast<SmolSVG::commandEllipticalArcTo*>(path->getLastCommand()));

    centerArc resultArc = svgEllipticalArcComputation(startpoint.X, startpoint.Y, smolArcCmd.getRadii().X,
                            smolArcCmd.getRadii().Y, smolArcCmd.getXAxisRotation(), smolArcCmd.getLargeArc(),
                            smolArcCmd.getFlagSweep(), endpoint.X, endpoint.Y);

    result->center = (resultArc.center - workingDocument->origin) * tenmils_to_mm_coefficient;
    result->angle = std::abs(resultArc.angleExtend);
    result->endPoint = ((smolArcCmd.getFlagSweep() ? startpoint : endpoint) - workingDocument->origin) * tenmils_to_mm_coefficient;

    return !++result;
  }

  PCB_Rect* LCJSONSerializer::parsePCBRectString(const string &LCJSONString)
  {
    RAIIC<PCB_Rect> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.

    result->topLeftPos.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->topLeftPos.Y = (stof(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    result->size.X = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->size.Y = stod(paramList[4]) * tenmils_to_mm_coefficient;
    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[5])];
    result->strokeWidth = stod(paramList[8]) * tenmils_to_mm_coefficient;

    return !++result;
  }

  PCB_Text *LCJSONSerializer::parsePCBTextString(const string &LCJSONString)
  {
    RAIIC<PCB_Text> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[13];

    // Know what type this text is.
    result->text = escapeQuotedString(paramList[10]);
    if(paramList[1].length() == 1)
      switch(paramList[1][0])
      {
        default:
        case 'L': result->type = PCBTextTypes::StandardText; break;
        case 'N': result->type = PCBTextTypes::PackageValue; break;
        case 'P': result->type = PCBTextTypes::PackageReference; break;
      }
    else if(paramList[1] == "PK")
      result->type = PCBTextTypes::PackageName;
    else
      result->type = PCBTextTypes::StandardText;

    // If we're reading a package that isn't going to end up in PCB, ignore the
    // references, values since they'll be added by deserializer.
    if(exportNestedLibs && result->type != PCBTextTypes::StandardText)
      return nullptr;

    result->height = stod(paramList[9]) * tenmils_to_mm_coefficient;
    result->orientation = stod(paramList[5]);
    result->midLeftPos = (coordinates(stod(paramList[2]) - (result->height - 2) * cos(toRadians(result->orientation + 90)),
                    stod(paramList[3]) - (result->height + 2) * sin(toRadians(result->orientation + 90)))
              - workingDocument->origin) * tenmils_to_mm_coefficient;

    // Crude fix for shift down issue
    //result->midLeftPos.Y -= 0.5;

    result->width = stod(paramList[4]) * tenmils_to_mm_coefficient;
    result->mirrored = tolStoi(paramList[6]);
    result->layerKiCad = EasyEdaToKiCadLayerMap[stoi(paramList[7])];

    if(paramList[14] != "")
    {
      result->height *= 0.75;
      result->width = 0.1;
    }

    if(paramList[12] == "none")
      result->visibility = false;
    else
      result->visibility = true;

    return !++result;
  }

  PCB_Module* LCJSONSerializer::parsePCBModuleString(const string &LCJSONString, EDADocument *parent,
                           map<string, RAIIC<EDADocument>> *exportedList)
  {
    RAIIC<PCB_Module> result;
    stringlist shapesList = splitByString(LCJSONString, string("#@$")),
        moduleHeader = splitString(shapesList[0], '~'),
        cparaTmp = splitString(moduleHeader[3], '`');

    shapesList.erase(shapesList.begin()); // Purge the header string

    result->id = moduleHeader[6]; // GGE ID.
    result->uuid = moduleHeader[8]; // UUID; only for modules.

    for(unsigned int i = 0; i < cparaTmp.size(); i += 2)
      result->cparaContent[cparaTmp[i]] = cparaTmp[i + 1]; // Transfer c_para content

    result->name = result->cparaContent["package"]; // Set package(aka footprint) name

    // Only for nested library use. If you pass an std::map here, UUID will be checked and make sure
    // extra efforts were not wasted on an already-parsed component.
    if(exportedList) // Only do following when we *are* actually dealing with export nested libs
      for(auto i = exportedList->begin(); i != exportedList->end(); i++) // Iterate through all the parsed footprints
      {
        if(i->first == result->uuid) // If found one with exact UUID then go out
          return nullptr;
        else
        if(static_cast<PCB_Module*>(i->second->containedElements.back())->name == result->name)
        { // If found that there's a footprint with the same name but they aren't actually the same one (which is tested possible)
          Info("More than one footprint on this board was found called <<<" + result->name + ">>>(" +
             result->id + "), gID will be added to the name.");
          result->name += ("__" + result->id); // Modify the name for clarification
        }
      }

    result->moduleCoords =
        (coordinates{stod(moduleHeader[1]), stod(moduleHeader[2])} - workingDocument->origin) * tenmils_to_mm_coefficient;
    result->orientation = stod(moduleHeader[4] == "" ? "0" : moduleHeader[4]);
    result->topLayer = stoi(moduleHeader[7]) == 1;
    result->layer = result->topLayer ? KiCadLayerIndex::F_Cu : KiCadLayerIndex::B_Cu;
    result->updateTime = (time_t)tolStoi(moduleHeader[9]);

    processingModule = true;

    if((workingDocument->docType == documentTypes::pcb) || (parent != nullptr))
    {
      coordinates originalOrigin = coordinates(workingDocument->origin);
      workingDocument->origin = coordinates{stod(moduleHeader[1]), stod(moduleHeader[2])};
      parsePCBLibComponent(shapesList, result->containedElements);
      workingDocument->origin = originalOrigin;
    }
    else
      parsePCBLibComponent(shapesList, result->containedElements);

    processingModule = false;

    return !++result;
  }

  // Judgement member function of parsers

  bool LCJSONSerializer::judgeIsOnCopperLayer(const KiCadLayerIndex layerKiCad)
  {
    return layerKiCad >= F_Cu && layerKiCad <= B_Cu;
  }

  /**
   * This part is for schematic elements serializing.
   */

  Schematic_Pin* LCJSONSerializer::parseSchPin(const string &LCJSONString) const
  {
    RAIIC<Schematic_Pin> result;
    string pinString = LCJSONString;
    findAndReplaceString(pinString, "^^", "~"); //Double circumflex is bad design for us. We simply replace them
    stringlist paramList = splitString(pinString, '~');

    result->id = paramList[7]; //GGE ID.

    //KiCad schematics uses mils for now. S-expression versions might take metric units.
    //EasyEDA uses the inversed direction in schematics against KiCad.
    result->pinCoord = { (stod(paramList[4]) - workingDocument->origin.X) * schematic_unit_coefficient,
               (stod(paramList[5]) - workingDocument->origin.Y) * -1 * schematic_unit_coefficient };

    // Pin electric property on EasyEDA didn't split power in and power out, so power would be treated as passive.
    result->electricProperty = SchPinElectricProperty(tolStoi(paramList[2]));

    //Resolve pin rotation
    if(paramList[6] == "")
      result->pinRotation = SchematicRotations::Deg0;
    else
      switch(paramList[6][0])
      {
        case '1':
          result->pinRotation = SchematicRotations::Deg180; break;
        case '2':
          result->pinRotation = SchematicRotations::Deg270; break;
        case '9':
          result->pinRotation = SchematicRotations::Deg90; break;
        default:
        case '0':
          result->pinRotation = SchematicRotations::Deg0; break;
      }

    for(auto i = paramList[17].begin(); i < paramList[17].end(); i++)
      if(*i == ' ')
        *i = '_'; // KiCad schematics lib won't recognize space, even if you use semicolons.

    result->pinName = paramList[17];
    result->pinNumber = paramList[26];

    result->clock = paramList[34][0] == '1' ? true : false ;
    result->inverted = paramList[31][0] == '1' ? true : false ;

    if(paramList[20] != "")
      result->fontSize = 50;
    else
    {
      findAndReplaceString(paramList[20], "pt", "");
      result->fontSize = paramList[20] == "" ? 50 : int (stod(paramList[20]) * (50.0f / 7.0f));
    }

    /*
    auto pinLengthTemp = splitString(paramList[11], 'h'); // h or v? I have to reimplement this later.
    if(pinLengthTemp.size() == 1)
      pinLengthTemp = splitString(paramList[11], 'v');

    while(pinLengthTemp[1][0] == ' ') // Get rid of potential spaces ("v -10" or "v-10" variation)
      pinLengthTemp[1].erase(0, 1);

    if(pinLengthTemp[1][0] == '-') // Get rid of potential negative signs
      pinLengthTemp[1][0] = ' ';
    */

    double pinLength = 0.0;

    auto pinPath = SmolSVG::readPathString(paramList[11]);
    const auto &drawPath = pinPath->getLastCommand();
    SmolSVG::SmolCoord lengthVec = drawPath->getConstEndPoint() - drawPath->getConstStartPoint();
    if(fuzzyCompare(lengthVec.X, 0.0)) // X direction difference is 0
      pinLength = std::abs(lengthVec.Y);
    else
      pinLength = std::abs(lengthVec.X);

    result->pinLength = (pinLength + (result->inverted ? 6 : 0)) * sch_convert_coefficient;

    return !++result;
  }

  Schematic_Polyline* LCJSONSerializer::parseSchPolyline(const string &LCJSONString) const
  {
    RAIIC<Schematic_Polyline> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6];

    auto pointTemp = splitString(paramList[1], ' ');
    for(unsigned int i = 0; i < pointTemp.size(); i += 2)
      result->polylinePoints.push_back(
          coordinates((stod(pointTemp[i]) - workingDocument->origin.X) * schematic_unit_coefficient,
                (stod(pointTemp[i + 1]) - workingDocument->origin.Y) * schematic_unit_coefficient * -1));

    result->isFilled = paramList[5] == "none" ? false : true;
    result->lineWidth = int (stoi(paramList[3]) * schematic_unit_coefficient);

    return !++result;
  }

  Schematic_Polygon* LCJSONSerializer::parseSchPolygon(const string &LCJSONString) const
  {
    RAIIC<Schematic_Polygon> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6];

    auto pointTemp = splitString(paramList[1], ' ');
    for(unsigned int i = 0; i < pointTemp.size(); i += 2)
      result->polylinePoints.push_back(
          coordinates((stod(pointTemp[i]) - workingDocument->origin.X) * schematic_unit_coefficient,
                (stod(pointTemp[i + 1]) - workingDocument->origin.Y) * schematic_unit_coefficient * -1));

    result->isFilled = paramList[5] == "none" ? false : true;
    result->lineWidth = int (stoi(paramList[3]) * schematic_unit_coefficient);

    return !++result;
  }

  Schematic_Text* LCJSONSerializer::parseSchText(const string &LCJSONString) const
  {
    RAIIC<Schematic_Text> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[15];

    result->text = paramList[12];
    result->bold = (paramList[9] == "normal" | paramList[9] == "") ? false : true;
    result->italic = (paramList[10] == "normal" | paramList[10] == "") ? false : true;

    paramList[7].erase(paramList[7].end() - 2); //Remove "pt" characters
    result->fontSize = stoi(paramList[7]);

    result->position = { stod(paramList[2]) * schematic_unit_coefficient, //We output the file as left justified, so this is fine.
               (stod(paramList[3]) - 0.5 * result->fontSize) * -1 * schematic_unit_coefficient };

    return !++result;
  }

  Schematic_Rect* LCJSONSerializer::parseSchRect(const string &LCJSONString) const
  {
    RAIIC<Schematic_Rect> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[11];
    result->position = { (stoi(paramList[1]) - static_cast<int>(workingDocument->origin.X)) * schematic_unit_coefficient,
               (stoi(paramList[2]) - static_cast<int>(workingDocument->origin.Y)) * schematic_unit_coefficient * -1 };
    result->size = { stoi(paramList[5]) * schematic_unit_coefficient, stoi(paramList[6]) * schematic_unit_coefficient };
    result->isFilled = paramList[10] == "none" ? false : true;
    result->width = int (stoi(paramList[8]) * schematic_unit_coefficient);

    return !++result;
  }


  Schematic_Arc *LCJSONSerializer::parseSchArc(const string &LCJSONString) const
  {
    RAIIC<Schematic_Arc> result;
    stringlist paramList = splitString(LCJSONString, '~'), arcCmdParams, movetoCmdParams;

    result->id = paramList[7];
    result->isFilled = paramList[6] == "none" ? false : true;
    result->width = int (stoi(paramList[4]) * schematic_unit_coefficient);

    auto path = SmolSVG::readPathString(paramList[1]);
    coordinates startpoint = path->getLastCommand()->getConstStartPoint(),
            endpoint = path->getLastCommand()->getConstEndPoint();
    auto &smolArcCmd = *(static_cast<SmolSVG::commandEllipticalArcTo*>(path->getLastCommand()));
    auto &size = smolArcCmd.getRadii();

    centerArc resultArc = svgEllipticalArcComputation(startpoint.X, startpoint.Y, smolArcCmd.getRadii().X,
                                                      smolArcCmd.getRadii().Y, smolArcCmd.getXAxisRotation(), smolArcCmd.getLargeArc(),
                                                      smolArcCmd.getFlagSweep(), endpoint.X, endpoint.Y);

    double angle1 = fmod(resultArc.angleStart, 360.0), angle2 = fmod(resultArc.angleStart + resultArc.angleExtend, 360.0);

    result->elliptical = fuzzyCompare(size.X, size.Y);
    result->center = (resultArc.center - workingDocument->origin) * schematic_unit_coefficient;
    result->startAngle = angle1 > angle2 ? angle2 : angle1;
    result->endAngle = angle1 > angle2 ? angle1 : angle2;
    result->startPoint = (startpoint - workingDocument->origin) * schematic_unit_coefficient;
    result->endPoint = (endpoint - workingDocument->origin) * schematic_unit_coefficient;
    result->size = size * schematic_unit_coefficient;

    if(!result->elliptical)
      Warn(result->id + ": Arc is not perfectly elliptical. This could cause problem.");

    result->center.Y *= -1, result->startPoint.Y *= -1, result->endPoint.Y *= -1;

    return !++result;
  }

  Schematic_Module *LCJSONSerializer::parseSchModuleString(const std::string &LCJSONString, EDADocument *parent,
                                                           map<std::string, RAIIC<EDADocument> > *exportedList)
  {
    RAIIC<Schematic_Module> result;
    stringlist shapesList = splitByString(LCJSONString, string("#@$")),
        moduleHeader = splitString(shapesList[0], '~'),
        cparaTmp = splitString(moduleHeader[3], '`');

    shapesList.erase(shapesList.begin()); // Purge the header string

    result->id = moduleHeader[6];
    result->uuid = moduleHeader[8];

    for(unsigned int i = 0; i < cparaTmp.size(); i += 2)
      result->cparaContent[cparaTmp[i]] = cparaTmp[i + 1]; // Transfer c_para content

    result->name = result->cparaContent["Manufacturer Part"]; // Set symbol name

    // Only for nested library use. If you pass an std::map here, UUID will be checked and make sure
    // extra efforts were not wasted on an already-parsed component.
    if(exportedList) // Only do following when we *are* actually dealing with export nested libs
      for(auto i = exportedList->begin(); i != exportedList->end(); i++) // Iterate through all the parsed symbols
      {
        if(i->first == result->uuid) // If found one with exact UUID then go out
          return nullptr;
        else
        if(static_cast<Schematic_Module*>(i->second->containedElements.back())->name == result->name)
        { // If found that there's a symbol with the same name but they aren't actually the same one (which is tested possible)
          Info("More than one footprint on this board was found called <<<" + result->name + ">>>(" +
             result->id + "), gID will be added to the name.");
          result->name += ("__" + result->id); // Modify the name for clarification
        }
      }

    result->moduleCoords =
        (coordinates{stod(moduleHeader[1]), stod(moduleHeader[2])} - workingDocument->origin) * tenmils_to_mm_coefficient;
    result->orientation = stod(moduleHeader[4] == "" ? "0" : moduleHeader[4]);
    result->updateTime = (time_t)tolStoi(moduleHeader[9]);


    processingModule = true;

    if((workingDocument->docType == documentTypes::schematic) || (parent != nullptr))
    {
      coordinates originalOrigin = coordinates(workingDocument->origin);
      workingDocument->origin = coordinates{stod(moduleHeader[1]), stod(moduleHeader[2])};
      parseSchLibComponent(shapesList, result->containedElements);
      workingDocument->origin = originalOrigin;
    }
    else
      parseSchLibComponent(shapesList, result->containedElements);

    processingModule = false;

    return !++result;
  }
}
