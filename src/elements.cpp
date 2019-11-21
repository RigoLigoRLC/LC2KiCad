/*
    Copyright (c) 2020 Harrison Wade, aka "RigoLigo RLC"

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

#include "elements.hpp"
#include "include.hpp"
#include "consts.hpp"
#include "rapidjson.hpp"

using std::vector;
using std::to_string;
using rapidjson::Value;

namespace lc2kicad
{
  

  PCB_Pad::PCB_Pad(vector<string> &paramList, coordinates origin)
  {
    //Resolve pad shape
      switch (paramList[1][0])
      {
      case 'E': //ELLIPSE, ROUND
        padShape = PCBPadShape::circle;
        break;
      case 'O': //OVAL
        padShape = PCBPadShape::oval;
        break;
      case 'R': //RECT
        padShape = PCBPadShape::rectangle;
        break;
      case 'P': //POLYGON
        padShape = PCBPadShape::polygon;
        break;
      default:
        assertRTE(false, (string("Invalid pad shape: ") + paramList[12]).data());
        break;
      }
    //Resolve pad coordinates
    padCoordinates.X = (atof(paramList[2].c_str()) - origin.X) * tenmils_to_mm_coefficient;
    padCoordinates.Y = (atof(paramList[3].c_str()) - origin.Y) * tenmils_to_mm_coefficient;
    orientation = static_cast<int>(atof(paramList[12].c_str()));
    //Resolve hole shape size
    if(holeShape == PCBHoleShape::slot)
    {
      holeSize.X = atof(paramList[13].c_str()) * tenmils_to_mm_coefficient;
      holeSize.Y = atof(paramList[9].c_str()) * 2 * tenmils_to_mm_coefficient;
    }
    else
      holeSize.X = holeSize.Y = atof(paramList[9].c_str()) * 2 * tenmils_to_mm_coefficient;
    //Resolve pad shape and size
    if(padShape == PCBPadShape::oval || padShape == PCBPadShape::rectangle)
    {
      padSize.X = atof(paramList[4].c_str()) * tenmils_to_mm_coefficient;
      padSize.Y = atof(paramList[5].c_str()) * tenmils_to_mm_coefficient;
    }
    else if(padShape == PCBPadShape::circle)
      padSize.X = padSize.Y = atof(paramList[4].c_str()) * tenmils_to_mm_coefficient;
    else //polygon
    {
      padSize.X = padSize.Y = holeSize.Y;
      vector<string> polygonCoordinates = splitString(paramList[10], ' ');
      coordinates polygonPointTemp = { 0.0f, 0.0f };
      for(int i = 0; i < polygonCoordinates.size(); i += 2)
      {
        polygonPointTemp.X = (atof(polygonCoordinates[  i  ].c_str()) - _rawCoords.X) * tenmils_to_mm_coefficient;
        polygonPointTemp.Y = (atof(polygonCoordinates[i + 1].c_str()) - _rawCoords.Y) * tenmils_to_mm_coefficient;
        shapePolygonPoints.push_back(polygonPointTemp);
      }
    }
    //Resolve pad type
    int padTypeTemp = atoi(paramList[6].c_str());
    if(padTypeTemp == 11)
      if(paramList[15] == "Y")
        padType = PCBPadType::through;
      else
        padType = PCBPadType::noplating;
    else 
      if(padTypeTemp == 1)
        padType = PCBPadType::top;
      else if(padTypeTemp == 2)
        padType = PCBPadType::bottom;
    //store net name
    netName = paramList[7];
    pinNumber = paramList[8];
  }

  string PCB_Pad::outputKiCadFormat(string &convArgs)
  {
    std::stringstream returnValue;
    returnValue << "(pad \"" << pinNumber << "\" " << padTypeKiCad[static_cast<int>(padType)] << ' '
                << padShapeKiCad[static_cast<int>(padShape)] << " (at " << to_string(padCoordinates.X)
                << ' ' << to_string(padCoordinates.Y) << ") (size " << to_string(padSize.X) << ' '
                << to_string(padSize.Y);
    if(padType == PCBPadType::through || padType == PCBPadType::noplating)
    {
      returnValue << ") (drill";
      if(holeShape == PCBHoleShape::slot)
      {
        returnValue << " oval " << to_string(holeSize.X) << ' ' << to_string(holeSize.Y);
      }
      else
      {
        returnValue << ' ' << to_string(holeSize.X);
      }
    }
    returnValue << ") (layers ";
    switch(padType)
    {
      case PCBPadType::top:
        returnValue << "F.Cu F.Paste F.Mask)";
        break;
      case PCBPadType::bottom:
        returnValue << "B.Cu B.Paste B.Mask)";
        break;
      default:
        returnValue << "*.Cu *.Mask)";
    }
    if(padShape != PCBPadShape::polygon)
      returnValue << ')';
    else
    {
      returnValue << "\n  (zone_connect 2)\n  (options (clearance outline) (anchor circle))\n  (primitives\n"
                  << "    (gr_poly (pts\n      ";
      for(int i = 0; i < shapePolygonPoints.size(); i++)
        returnValue << " (xy " << to_string(shapePolygonPoints[i].X) << ' ' << to_string(shapePolygonPoints[i].Y) << ')';
      returnValue << ") (width 0))\n  ))";
    }
    return returnValue.str();
  }
    
  PCB_Via::PCB::Via(vector<string> &paramList, coordinates origin)
  {
    //Resolving the via coordinates
    viaCoordinates.X = (atof(paramList[1].c_str()) - origin.X) * ten_mils_to_mm_coefficient;
    viaCoordinates.Y = (atof(paramList[2].c_str()) - origin.Y) * ten_mils_to_mm_coefficient;
    //Resolve via diameter (size)
    viaSize = atof(paramList[3].c_str()) * ten_mils_to_mm_coefficient;
    drillSize = atof(paramList[5].c_str()) * ten_mils_to_mm_coefficient;
    netName = paramList[4];
  }
  
  
}

