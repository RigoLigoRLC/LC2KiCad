/*
    Copyright (c) 2020 RigoLigoRLC.

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
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>

#include "includes.hpp"
#include "rapidjson.hpp"
#include "edaclasses.hpp"
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
  void LCJSONSerializer::setCompatibilitySwitches(const str_dbl_map &_compatibSw) { internalCompatibilitySwitches = _compatibSw; }
  
  LCJSONSerializer::~LCJSONSerializer() { };

  void LCJSONSerializer::parseSchLibDocument()
  {
    assertThrow(workingDocument->module, "Internal document type mismatch: Parse an internal document with its module property set to \"false\".");
    workingDocument->docType = documentTypes::schematic_lib;

    Document &parseTarget = *workingDocument->jsonParseResult; // Create a reference for convenience.
    string canvasPropertyString;
    vector<string> canvasPropertyList;
    vector<int> layerMapper;
    string packageName, contributor, prefix;
    str_str_map &docInfo = workingDocument->docInfo;

    // Parse canvas properties
    canvasPropertyString = parseTarget["canvas"].GetString();
    canvasPropertyList = splitString(canvasPropertyString, '~');
    // Write canvas properties like origin and gridsize
    coordinates origin = workingDocument->origin;
    workingDocument->origin.X = stod(canvasPropertyList[13]);
    workingDocument->origin.Y = stod(canvasPropertyList[14]);
    workingDocument->gridSize = stod(canvasPropertyList[6]);

    // Write Prefix and contributor
    Value &head = parseTarget["head"];
    Value &headlist = head.GetObject()["c_para"];
    packageName = headlist["name"].GetString();


    // TODO: Make layer info useful information
    // Value layer = parseTarget["layers"].GetArray();

    assertThrow(parseTarget["shape"].IsArray(), "Not an array.");
    Value shape = parseTarget["shape"].GetArray();
    vector<string> shapesList;
    vector<PCBElement*> elementsList;
    for(unsigned int i = 0; i < shape.Size(); i++)
      shapesList.push_back(shape[i].GetString());

    if(packageName.size() != 0)
      docInfo["documentname"] = packageName;
    prefix = headlist["pre"].GetString();
    prefix.pop_back();
    docInfo["prefix"] = prefix;
    docInfo["contributor"] = headlist.HasMember("Contributor") ? headlist["Contributor"].GetString() : "" ;

    for(auto &i : shapesList)
    {
      // vector<string> parameters = splitString(shapesList[i], '~');
      switch(i[0])
      {
        case 'P':
          switch(i[1])
          {
            case 'G': // Polygon
              break;
            case 'I': // Pie
              break;
            case 'L': // Polyline
              break;
            case 'T': // Path
              break;
            default: // Pin
              workingDocument->addElement(parseSchPin(i));
              break;
          }
          break;
        case 'R': // Rectangle
          break;
        case 'A':
          switch(i[1])
          {
            case 'R': // Arrowhead
              break;
            default: // Arc
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
          assertThrow(false, "Invalid element of <<<" + i + ">>>.");
      }
      workingDocument->containedElements.back()->parent = workingDocument;
    }
  }


  void LCJSONSerializer::parsePCBLibDocument()
  {
    assertThrow(workingDocument->module, "Internal document type mismatch: Parse an internal document with its module property set to \"false\".");
    workingDocument->docType = documentTypes::pcb_lib;
    
    Document &parseTarget = *workingDocument->jsonParseResult; // Create a reference for convenience.
    string canvasPropertyString;
    vector<string> canvasPropertyList;
    vector<int> layerMapper;
    string packageName, contributor;
    
    // Parse canvas properties
    canvasPropertyString = parseTarget["canvas"].GetString();
    canvasPropertyList = splitString(canvasPropertyString, '~');
    // Write canvas properties like origin and gridsize
    coordinates origin = workingDocument->origin;
    workingDocument->origin.X = stod(canvasPropertyList[16]);
    workingDocument->origin.Y = stod(canvasPropertyList[17]);
    workingDocument->gridSize = stod(canvasPropertyList[6]);

    // Write Prefix and contributor
    Value &head = parseTarget["head"];
    Value &headlist = head.GetObject()["c_para"];
    packageName = headlist["package"].GetString();
    
    contributor = headlist.HasMember("Contributor") ? headlist["Contributor"].GetString() : "" ;

    // TODO: Make layer info useful information
    // Value layer = parseTarget["layers"].GetArray();

    assertThrow(parseTarget["shape"].IsArray(), "Not an array.");
    Value shape = parseTarget["shape"].GetArray();
    vector<string> shapesList;
    vector<PCBElement*> elementsList;
    for(unsigned int i = 0; i < shape.Size(); i++)
      shapesList.push_back(shape[i].GetString());
    if(packageName.size() != 0)
      workingDocument->docInfo["documentname"] = packageName;
    workingDocument->docInfo["contributor"] = contributor;

    for(auto &i : shapesList)
    {
      // vector<string> parameters = splitString(shapesList[i], '~');
      switch(i[0])
      {
        case 'P':
          switch(i[1])
          {
            case 'A': // Pad
              workingDocument->addElement(parsePCBPadString(i));
              break;
            case 'R': // Protractor
              break;
          }
        case 'T':
          switch(i[1])
          {
            case 'E': // Text
              break;
            case 'R': // Track
              stringlist tmp = splitString(i, '~');
              if(judgeIsOnCopperLayer(LCtoKiCadLayerLUT[stoi(tmp[2])]))
                workingDocument->addElement(parsePCBCopperTrackString(i));
              else
                workingDocument->addElement(parsePCBGraphicalTrackString(i));
              break;
          }
          break;
        case 'C':
          switch(i[1])
          {
            case 'O': // CopperArea
              break;
            case 'I': // Circle
              stringlist tmp = splitString(i, '~');
              if(judgeIsOnCopperLayer(LCtoKiCadLayerLUT[stoi(tmp[5])]))
                workingDocument->addElement(parsePCBCopperCircleString(i));
              else
                workingDocument->addElement(parsePCBGraphicalCircleString(i));
              break;
          }
          break;
        case 'R': // Rect
          workingDocument->addElement(parsePCBRectString(i));
          break;
        case 'A': // Arc
          break;
        case 'V': // Via
          workingDocument->addElement(parsePCBViaString(i));
          break;
        case 'H': // Hole
          workingDocument->addElement(parsePCBHoleString(i));
          break; 
        case 'D': // Dimension
          break; 
        case 'S': // Solidregion
          break;
        default:
          assertThrow(false, "Invalid element of <<<" + i + ">>>.");
      }
      workingDocument->containedElements.back()->parent = workingDocument;
    }
  }


  /**
   * The below section is for PCB elements serializing.
   */

  PCB_Pad* LCJSONSerializer::parsePCBPadString(const string &LCJSONString) const
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
      assertThrow(false, (string("Invalid pad shape: ") + paramList[12]).data());
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
      for(int i = 0; i < polygonCoordinates.size(); i += 2)
      {
        polygonPointTemp.X = (stod(polygonCoordinates[  i  ]) - stod(polygonDrillCoordsString[0])) * tenmils_to_mm_coefficient;
        polygonPointTemp.Y = (stod(polygonCoordinates[i + 1]) - stod(polygonDrillCoordsString[1])) * tenmils_to_mm_coefficient;
        result->shapePolygonPoints.push_back(polygonPointTemp);
      }
    }

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
    // store net name
    result->netName = paramList[7];
    result->pinNumber = paramList[8];

    //workingDocument->containedElements.push_back(!++result);
    return !++result;
  }

  PCB_Hole* LCJSONSerializer::parsePCBHoleString(const string &LCJSONString) const
  {
    RAIIC<PCB_Hole> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[4]; // GGE ID.

    result->holeCoordinates.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->holeCoordinates.Y = (stod(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    result->holeDiameter = stod(paramList[3]) * 2 * tenmils_to_mm_coefficient;

    return !++result;
  }

  PCB_Via* LCJSONSerializer::parsePCBViaString(const string &LCJSONString) const 
  {
    RAIIC<PCB_Via> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.

    // Resolving the via coordinates
    result->holeCoordinates.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->holeCoordinates.Y = (stod(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    // Resolve via diameter (size)
    result->viaDiameter = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->holeDiameter = stod(paramList[5]) * tenmils_to_mm_coefficient;

    result->netName = paramList[4];

    return !++result;
  }
  
  PCB_CopperTrack* LCJSONSerializer::parsePCBCopperTrackString(const string &LCJSONString) const
  {
    RAIIC<PCB_CopperTrack> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[5];

    // Resolve track width
    result->width = stod(paramList[1]) * tenmils_to_mm_coefficient;

    // Resolve track layer
    result->layerKiCad = LCtoKiCadLayerLUT[int (stod(paramList[2]))];
    assertThrow(result->layerKiCad != -1, ("Invalid layer for TRACK " + paramList[3]));

    // Resolve track points
    stringlist pointsStrList = splitString(paramList[4], ' ');
    coordinates tempCoord;
    for(int i = 0; i < pointsStrList.size(); i += 2)
    {
      tempCoord.X = (stod(pointsStrList[i]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
      tempCoord.Y = (stod(pointsStrList[i + 1]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
      result->trackPoints.push_back(tempCoord);
    }

    return !++result;
  }

  PCB_GraphicalTrack* LCJSONSerializer::parsePCBGraphicalTrackString(const string &LCJSONString) const
  {
    RAIIC<PCB_GraphicalTrack> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[5]; // GGE ID.

    // Resolve track width
    result->width = stod(paramList[1]) * tenmils_to_mm_coefficient;

    // Resolve track layer
    result->layerKiCad = LCLayerToKiCadLayer(stoi(paramList[2]));
    assertThrow(result->layerKiCad != -1, ("Invalid layer for TRACK " + result->id));

    // Resolve track points
    stringlist pointsStrList = splitString(paramList[4], ' ');
    coordinates tempCoord;
    for(int i = 0; i < pointsStrList.size(); i += 2)
    {
      tempCoord.X = (stod(pointsStrList[i]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
      tempCoord.Y = (stod(pointsStrList[i + 1]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
      result->trackPoints.push_back(tempCoord);
    }

    return !++result;
  }

  PCB_FloodFill* LCJSONSerializer::parsePCBFloodFillString(const string &LCJSONString) const
  {
    RAIIC<PCB_FloodFill> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[7]; // GGE ID.

    // Resolve layer ID and net name
    result->netName = paramList[3];
    result->layerKiCad = LCLayerToKiCadLayer(stod(paramList[2]));
    // Throw error with gge ID if layer is invalid
    assertThrow(result->layerKiCad != -1, "Invalid layer for COPPERAREA " + paramList[7]);

    // Resolve track points
    stringlist pointsStrList = splitString(paramList[4], ' ');
    coordinates tempCoord;
    for(int i = 0; i < pointsStrList.size(); i += 2)
    {
      tempCoord.X = (stod(pointsStrList[i]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
      tempCoord.Y = (stod(pointsStrList[i + 1]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
      result->fillAreaPolygonPoints.push_back(tempCoord);
    }


    result->clearanceWidth = stod(paramList[5]) * tenmils_to_mm_coefficient; // Resolve clearance width
    result->fillStyle = (paramList[6] == "solid" ? floodFillStyle::solidFill : floodFillStyle::noFill);
      // Resolve fill style
    result->isSpokeConnection = (paramList[8] == "spoke" ? true : false); // Resolve connection type
    result->isPreservingIslands = (paramList[9] == "yes" ? true : false); // Resolve island keep

    return !++result;
  }
  
  PCB_CopperCircle* LCJSONSerializer::parsePCBCopperCircleString(const string &LCJSONString) const
  {
    RAIIC<PCB_CopperCircle> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.
    
    result->center.X = stod(paramList[1]) * tenmils_to_mm_coefficient;
    result->center.Y = stod(paramList[2]) * tenmils_to_mm_coefficient;
    result->radius = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->width = stod(paramList[4]) * tenmils_to_mm_coefficient;
    result->layerKiCad = LCLayerToKiCadLayer(stoi(paramList[5]));
    result->netName = paramList[8];
    
    return !++result;
  }
  
  PCB_GraphicalCircle* LCJSONSerializer::parsePCBGraphicalCircleString(const string &LCJSONString) const
  {
    RAIIC<PCB_GraphicalCircle> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.
    
    result->center.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->center.Y = (stod(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    result->radius = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->width = stod(paramList[4]) * tenmils_to_mm_coefficient;
    result->layerKiCad = LCLayerToKiCadLayer(stoi(paramList[5]));
    
    return !++result;
  }

  PCB_Rect* LCJSONSerializer::parsePCBRectString(const string &LCJSONString) const
  {
    RAIIC<PCB_Rect> result;
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.

    result->topLeftPos.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->topLeftPos.Y = (stof(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    result->size.X = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->size.Y = stod(paramList[4]) * tenmils_to_mm_coefficient;
    result->layerKiCad = LCtoKiCadLayerLUT[stoi(paramList[5])];
    result->strokeWidth = stod(paramList[8]) * tenmils_to_mm_coefficient;

    return !++result;
  }
    
  // Judgement member function of parsers

  bool LCJSONSerializer::judgeIsOnCopperLayer(const int layerKiCad) const
  {
    return layerKiCad >= 0 && layerKiCad <= 31;
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
    result->pinCoord = { (stod(paramList[4]) - workingDocument->origin.X) * sch_convert_coefficient,
                         (stod(paramList[5]) - workingDocument->origin.Y) * -1 * sch_convert_coefficient };
    
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
    
    result->pinName = paramList[17];
    result->pinNumber = paramList[26];
    
    result->clock = paramList[34][0] == '1' ? true : false ;
    result->inverted = paramList[31][0] == '1' ? true : false ;
    
    if(paramList[20] != "")
      result->fontSize = 50;
    else
    {
      findAndReplaceString(paramList[20], "pt", "");
      result->fontSize = paramList[20] == "" ? 50 : (stod(paramList[20]) * (50.0f / 7.0f));
    }
    
    auto pinLengthTemp = splitString(paramList[11], 'h');
    if(pinLengthTemp[1][0] == '-')
      pinLengthTemp[1][0] = ' ';
    result->pinLength = stod(pinLengthTemp[1]) * 10;
    
    return !++result;
  }
  
  Schematic_Polyline* LCJSONSerializer::parseSchPolyline(const string &LCJSONString) const
  {
    RAIIC<Schematic_Polyline> result;
    stringlist paramList = splitString(LCJSONString, '~');
    
    result->id = paramList[6];
    
    auto pointTemp = splitString(paramList[1], ' ');
    for(int i = 0; i < pointTemp.size(); i += 2)
      result->polylinePoints.push_back(coordinates(stod(pointTemp[i]), stod(pointTemp[i + 1])));
    
    result->isFilled = paramList[5] == "none" ? false : true;
    result->lineWidth = stod(paramList[3]) * 2 * sch_convert_coefficient;
    
    return !++result;
  }
  
  Schematic_Polygon* LCJSONSerializer::parseSchPolygon(const string &LCJSONString) const
  {
    RAIIC<Schematic_Polygon> result;
    stringlist paramList = splitString(LCJSONString, '~');
    
    result->id = paramList[6];
    
    auto pointTemp = splitString(paramList[1], ' ');
    for(int i = 0; i < pointTemp.size(); i += 2)
      result->polylinePoints.push_back(coordinates(stod(pointTemp[i]), stod(pointTemp[i + 1])));
    
    result->isFilled = paramList[5] == "none" ? false : true;
    result->lineWidth = stod(paramList[3]) * 2 * sch_convert_coefficient;
    
    return !++result;
  }
  
  Schematic_Text* LCJSONSerializer::parseSchText(const string &LCJSONString) const
  {
    RAIIC<Schematic_Text> result;
    stringlist paramList = splitString(LCJSONString, '~');
    
    result->id = paramList[15];
    
    result->text = paramList[12];
    result->bold = paramList[9] == "normal" | paramList[9] == "" ? false : true;
    result->italic = paramList[10] == "normal" | paramList[10] == "" ? false : true;
    
    paramList[7].erase(paramList[7].end() - 2); //Remove "pt" characters
    result->fontSize = stod(paramList[7]);
    
    result->position = { stod(paramList[2]) * sch_convert_coefficient, //We output the file as left justified, so this is fine.
                        (stod(paramList[3]) - 0.5 * result->fontSize) * -1 * sch_convert_coefficient };

    return !++result;
  }
  
  Schematic_Rect* LCJSONSerializer::parseSchRect(const string &LCJSONString) const
  {
    RAIIC<Schematic_Rect> result;
    stringlist paramList = splitString(LCJSONString, '~');
    
    result->position = { stod(paramList[1]) * sch_convert_coefficient, stod(paramList[2]) * sch_convert_coefficient * -1 };
    result->size = { stod(paramList[3]) * sch_convert_coefficient, stod(paramList[4]) * sch_convert_coefficient };
    result->isFilled = paramList[10] == "none" ? false : true;
    result->width = stod(paramList[8]) * 2 * sch_convert_coefficient;
    
    return !++result;
  }
}
