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
    along with LC2KiCad. If not, see <https://www.gnu.org/licenses/>.
*/

#include "includes.hpp"
#include "internalsserializer.hpp"
#include "edaclasses.hpp"

#include <string>
#include <vector>

using std::vector;
using std::string;

namespace lc2kicad
{
  
  PCB_Pad*  StandardLCStringParser::parsePadString(const string &LCJSONString, const coordinates &origin) const
  {
    PCB_Pad *ret = new PCB_Pad();
    stringlist paramList = splitString(LCJSONString, '~');

    //Resolve pad shape
    switch (paramList[1][0])
    {
    case 'E': //ELLIPSE, ROUND
      ret->padShape = PCBPadShape::circle;
      break;
    case 'O': //OVAL
      ret->padShape = PCBPadShape::oval;
      break;
    case 'R': //RECT
      ret->padShape = PCBPadShape::rectangle;
      break;
    case 'P': //POLYGON
      ret->padShape = PCBPadShape::polygon;
      break;
    default:
      assertThrow(false, (string("Invalid pad shape: ") + paramList[12]).data());
      break;
    }

    //Resolve pad coordinates
    coordinates _rawCoords = { static_cast<float>(atof(paramList[2].c_str())), static_cast<float>(atof(paramList[3].c_str())) };
    ret->padCoordinates.X = (atof(paramList[2].c_str()) - origin.X) * tenmils_to_mm_coefficient;
    ret->padCoordinates.Y = (atof(paramList[3].c_str()) - origin.Y) * tenmils_to_mm_coefficient;
    ret->orientation = static_cast<int>(atof(paramList[12].c_str()));

    //Resolve hole shape size
    if(ret->holeShape == PCBHoleShape::slot)
    {
      ret->holeSize.X = atof(paramList[13].c_str()) * tenmils_to_mm_coefficient;
      ret->holeSize.Y = atof(paramList[9].c_str()) * 2 * tenmils_to_mm_coefficient;
    }
    else
      ret->holeSize.X = ret->holeSize.Y = atof(paramList[9].c_str()) * 2 * tenmils_to_mm_coefficient;
    
    //Resolve pad shape and size
    if(ret->padShape == PCBPadShape::oval || ret->padShape == PCBPadShape::rectangle)
    {
      ret->padSize.X = atof(paramList[4].c_str()) * tenmils_to_mm_coefficient;
      ret->padSize.Y = atof(paramList[5].c_str()) * tenmils_to_mm_coefficient;
    }
    else if(ret->padShape == PCBPadShape::circle)
      ret->padSize.X = ret->padSize.Y = atof(paramList[4].c_str()) * tenmils_to_mm_coefficient;
    else //polygon
    {
      ret->padSize.X = ret->padSize.Y = ret->holeSize.Y;
      vector<string> polygonCoordinates = splitString(paramList[10], ' ');
      coordinates polygonPointTemp = { 0.0, 0.0 };
      for(int i = 0; i < polygonCoordinates.size(); i += 2)
      {
        polygonPointTemp.X = (atof(polygonCoordinates[  i  ].c_str()) - _rawCoords.X) * tenmils_to_mm_coefficient;
        polygonPointTemp.Y = (atof(polygonCoordinates[i + 1].c_str()) - _rawCoords.Y) * tenmils_to_mm_coefficient;
        ret->shapePolygonPoints.push_back(polygonPointTemp);
      }
    }

    //Resolve pad type
    int padTypeTemp = atoi(paramList[6].c_str());
    if(padTypeTemp == 11)
      if(paramList[15] == "Y")
        ret->padType = PCBPadType::through;
      else
        ret->padType = PCBPadType::noplating;
    else 
      if(padTypeTemp == 1)
        ret->padType = PCBPadType::top;
      else if(padTypeTemp == 2)
        ret->padType = PCBPadType::bottom;
    //store net name
    ret->netName = paramList[7];
    ret->pinNumber = paramList[8];

    return ret;
  }

  PCB_Via* StandardLCStringParser::parseViaString(const string &LCJSONString, const coordinates &origin) const 
  {
    PCB_Via *ret = new PCB_Via();
    stringlist paramList = splitString(LCJSONString, '~');

    //Resolving the via coordinates
    ret->viaCoordinates.X = (atof(paramList[1].c_str()) - origin.X) * tenmils_to_mm_coefficient;
    ret->viaCoordinates.Y = (atof(paramList[2].c_str()) - origin.Y) * tenmils_to_mm_coefficient;
    //Resolve via diameter (size)
    ret->viaSize = atof(paramList[3].c_str()) * tenmils_to_mm_coefficient;
    ret->drillSize = atof(paramList[5].c_str()) * tenmils_to_mm_coefficient;

    ret->netName = paramList[4];

    return ret;
  }
  
  PCB_Track* StandardLCStringParser::parseTrackString(const string &LCJSONString, const coordinates &origin) const
  {
    PCB_Track *ret = new PCB_Track();
    stringlist paramList = splitString(LCJSONString, '~');

    //Resolve track width
    ret->width = atof(paramList[1].c_str()) * tenmils_to_mm_coefficient;

    //Resolve track layer
    ret->layerKiCad = LCtoKiCadLayerLUT[int (atof(paramList[2].c_str()))];
    assertThrow(ret->layerKiCad != -1, ("Invalid layer for TRACK " + paramList[3]).c_str());

    //Resolve track points
    stringlist pointsStrList = splitString(paramList[4], ' ');
    coordinates tempCoord;
    for(int i = 0; i < pointsStrList.size(); i += 2)
    {
      tempCoord.X = (atof(pointsStrList[i].c_str()) - origin.X) * tenmils_to_mm_coefficient;
      tempCoord.Y = (atof(pointsStrList[i + 1].c_str()) - origin.Y) * tenmils_to_mm_coefficient;
      ret->trackPoints.push_back(tempCoord);
    }

    return ret;
  }

  PCB_GraphicalLine* StandardLCStringParser::parseGraphicalLineString(const string &LCJSONString, const coordinates &origin) const
  {
    PCB_GraphicalLine *ret = new PCB_GraphicalLine();
    stringlist paramList = splitString(LCJSONString, '~');

    //Resolve track width
    ret->width = atof(paramList[1].c_str()) * tenmils_to_mm_coefficient;

    //Resolve track layer
    ret->layerKiCad = LCLayerToKiCadLayer(atoi(paramList[2].c_str()));
    assertThrow(ret->layerKiCad != -1, ("Invalid layer for TRACK " + paramList[3]).c_str());

    //Resolve track points
    stringlist pointsStrList = splitString(paramList[4], ' ');
    coordinates tempCoord;
    for(int i = 0; i < pointsStrList.size(); i += 2)
    {
      tempCoord.X = (atof(pointsStrList[i].c_str()) - origin.X) * tenmils_to_mm_coefficient;
      tempCoord.Y = (atof(pointsStrList[i + 1].c_str()) - origin.Y) * tenmils_to_mm_coefficient;
      ret->trackPoints.push_back(tempCoord);
    }

    return ret;
  }

  PCB_FloodFill* StandardLCStringParser::parseFloodFillString(const string &LCJSONString, const coordinates &origin) const
  {
    PCB_FloodFill *ret = new PCB_FloodFill();
    stringlist paramList = splitString(LCJSONString, '~');

    //Resolve layer ID and net name
    ret->netName = paramList[3].c_str();
    ret->layerKiCad = LCLayerToKiCadLayer(atof(paramList[2].c_str()));
    //Throw error with gge ID if layer is invalid
    assertThrow(ret->layerKiCad != -1, "Invalid layer for COPPERAREA " + paramList[7]);

    //Resolve track points
    stringlist pointsStrList = splitString(paramList[4], ' ');
    coordinates tempCoord;
    for(int i = 0; i < pointsStrList.size(); i += 2)
    {
      tempCoord.X = (atof(pointsStrList[i].c_str()) - origin.X) * tenmils_to_mm_coefficient;
      tempCoord.Y = (atof(pointsStrList[i + 1].c_str()) - origin.Y) * tenmils_to_mm_coefficient;
      ret->fillAreaPolygonPoints.push_back(tempCoord);
    }


    ret->clearanceWidth = atof(paramList[5].c_str()) * tenmils_to_mm_coefficient; //Resolve clearance width
    ret->fillStyle = (paramList[6] == "solid" ? floodFillStyle::solidFill : floodFillStyle::noFill);
      //Resolve fill style
    ret->isSpokeConnection = (paramList[8] == "spoke" ? true : false); //Resolve connection type
    ret->isPreservingIslands = (paramList[9] == "yes" ? true : false); //Resolve island keep

    return ret;
  }
  
  PCB_Circle* StandardLCStringParser::parseCircleString(const string &LCJSONString, const coordinates &origin) const
  {
    PCB_Circle *ret = new PCB_Circle();
    stringlist paramList = splitString(LCJSONString, '~');
    
    ret->center.X = atof(paramList[1].c_str()) * tenmils_to_mm_coefficient;
    ret->center.Y = atof(paramList[2].c_str()) * tenmils_to_mm_coefficient;
    ret->radius = atof(paramList[3].c_str()) * tenmils_to_mm_coefficient;
    ret->width = atof(paramList[4].c_str()) * tenmils_to_mm_coefficient;
    ret->layerKiCad = LCLayerToKiCadLayer(atoi(paramList[5].c_str()));
    ret->netName = paramList[8];
    
    return ret;
  }
  
  PCB_GraphicalCircle* StandardLCStringParser::parseGraphicalCircleString(const string &LCJSONString, const coordinates &origin) const
  {
    PCB_GraphicalCircle *ret = new PCB_GraphicalCircle();
    stringlist paramList = splitString(LCJSONString, '~');
    
    ret->center.X = (atof(paramList[1].c_str()) - origin.X) * tenmils_to_mm_coefficient;
    ret->center.Y = (atof(paramList[2].c_str()) - origin.Y) * tenmils_to_mm_coefficient;
    ret->radius = atof(paramList[3].c_str()) * tenmils_to_mm_coefficient;
    ret->width = atof(paramList[4].c_str()) * tenmils_to_mm_coefficient;
    ret->layerKiCad = LCLayerToKiCadLayer(atoi(paramList[5].c_str()));
    
    return ret;
  }

  //Judgement member function of parsers

  bool StandardLCStringParser::judgeIsOnCopperLayer(const int layerKiCad) const
  {
    return layerKiCad >= 0 && layerKiCad <= 31;
  }

}
