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

  void LCJSONSerializer::parsePCBLibDocument()
  {
    assertThrow(workingDocument->module, "Internal document type mismatch: Parse an internal document with its module property set to \"false\".");
    workingDocument->docType = documentTypes::pcb_lib;

    fstream outputFile;
    outputFile.open(workingDocument->docInfo["filename"] + ".pretty");
    assertThrow(outputFile.fail(), "Cannot create the output file for file \"" + workingDocument->docInfo["filename"] + "\" .");
    
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
    for(int i = 0; i < shape.Size(); i++)
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
              parsePCBPadString(i);
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
                parsePCBCopperTrackString(i);
              else
                parsePCBGraphicalTrackString(i);
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
                parsePCBCopperCircleString(i);
              else
                parsePCBGraphicalCircleString(i);
              break;
          }
          break;
        case 'R': // Rect
          parsePCBRectString(i);
          break;
        case 'A': // Arc
          break;
        case 'V': // Via
          parsePCBViaString(i);
          break;
        case 'H': // Hole
          parsePCBHoleString(i);
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
    
    std::ofstream writer;
    writer.open(packageName + ".kicad_mod", std::ios::out);
    
    std::ostream *outstream = &cout;
    
    if(writer.fail())
      cout << "Error: Cannot create file. Will print the file content out.\n";
    else
      outstream = &writer;
  }


  /**
   * The below section is for PCB elements serializing.
   */

  void LCJSONSerializer::parsePCBPadString(const string &LCJSONString) const
  {
    PCB_Pad *result = new PCB_Pad();
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

    // Resolve hole shape size
    result->holeSize.X = stod(paramList[13]) * tenmils_to_mm_coefficient;
    result->holeSize.Y = (stod(paramList[9]) * 2 * tenmils_to_mm_coefficient);
    result->holeSize.X == 0.0f ? result->holeSize.X = result->holeSize.Y, result->holeShape = PCBHoleShape::circle : result->holeShape = PCBHoleShape::slot; 
    
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

    workingDocument->containedElements.push_back(result);
  }

  void LCJSONSerializer::parsePCBHoleString(const string &LCJSONString) const
  {
    PCB_Hole *result = new PCB_Hole();
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[4]; // GGE ID.

    result->holeCoordinates.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->holeCoordinates.Y = (stod(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    result->holeDiameter = stod(paramList[3]) * 2 * tenmils_to_mm_coefficient;

    workingDocument->containedElements.push_back(result);
  }

  void LCJSONSerializer::parsePCBViaString(const string &LCJSONString) const 
  {
    PCB_Via *result = new PCB_Via();
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.

    // Resolving the via coordinates
    result->holeCoordinates.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->holeCoordinates.Y = (stod(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    // Resolve via diameter (size)
    result->viaDiameter = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->holeDiameter = stod(paramList[5]) * tenmils_to_mm_coefficient;

    result->netName = paramList[4];

    workingDocument->containedElements.push_back(result);
  }
  
  void LCJSONSerializer::parsePCBCopperTrackString(const string &LCJSONString) const
  {
    PCB_CopperTrack *result = new PCB_CopperTrack();
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

    workingDocument->containedElements.push_back(result);
  }

  void LCJSONSerializer::parsePCBGraphicalTrackString(const string &LCJSONString) const
  {
    PCB_GraphicalTrack *result = new PCB_GraphicalTrack();
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

    workingDocument->containedElements.push_back(result);
  }

  void LCJSONSerializer::parsePCBFloodFillString(const string &LCJSONString) const
  {
    PCB_FloodFill *result = new PCB_FloodFill();
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

    workingDocument->containedElements.push_back(result);
  }
  
  void LCJSONSerializer::parsePCBCopperCircleString(const string &LCJSONString) const
  {
    PCB_CopperCircle *result = new PCB_CopperCircle();
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.
    
    result->center.X = stod(paramList[1]) * tenmils_to_mm_coefficient;
    result->center.Y = stod(paramList[2]) * tenmils_to_mm_coefficient;
    result->radius = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->width = stod(paramList[4]) * tenmils_to_mm_coefficient;
    result->layerKiCad = LCLayerToKiCadLayer(stoi(paramList[5]));
    result->netName = paramList[8];
    
    workingDocument->containedElements.push_back(result);
  }
  
  void LCJSONSerializer::parsePCBGraphicalCircleString(const string &LCJSONString) const
  {
    PCB_GraphicalCircle *result = new PCB_GraphicalCircle();
    stringlist paramList = splitString(LCJSONString, '~');

    result->id = paramList[6]; // GGE ID.
    
    result->center.X = (stod(paramList[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->center.Y = (stod(paramList[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    result->radius = stod(paramList[3]) * tenmils_to_mm_coefficient;
    result->width = stod(paramList[4]) * tenmils_to_mm_coefficient;
    result->layerKiCad = LCLayerToKiCadLayer(stoi(paramList[5]));
    
    workingDocument->containedElements.push_back(result);
  }

  void LCJSONSerializer::parsePCBRectString(const string &LCJSONString) const
  {
    PCB_Rect *result = new PCB_Rect();
    stringlist paramlist = splitString(LCJSONString, '~');

    result->id = paramlist[6]; // GGE ID.

    result->topLeftPos.X = (stod(paramlist[1]) - workingDocument->origin.X) * tenmils_to_mm_coefficient;
    result->topLeftPos.Y = (stof(paramlist[2]) - workingDocument->origin.Y) * tenmils_to_mm_coefficient;
    result->size.X = stod(paramlist[3]) * tenmils_to_mm_coefficient;
    result->size.Y = stod(paramlist[4]) * tenmils_to_mm_coefficient;
    result->layerKiCad = LCtoKiCadLayerLUT[stoi(paramlist[5])];
    result->strokeWidth = stod(paramlist[8]) * tenmils_to_mm_coefficient;

    workingDocument->containedElements.push_back(result);
  }
    
  // Judgement member function of parsers

  bool LCJSONSerializer::judgeIsOnCopperLayer(const int layerKiCad) const
  {
    return layerKiCad >= 0 && layerKiCad <= 31;
  }
  
  /**
   * This part is for schematic elements serializing.
   */
  
  
  
}