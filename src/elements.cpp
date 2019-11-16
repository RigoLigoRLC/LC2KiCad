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
using rapidjson::Value;

namespace lc2kicad
{
  

  PCB_Pad::PCB_Pad(vector<string> &paramList, coordinates origin)
  {
    //Resolve pad shape
      switch (paramList[1][1])
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
    //Resolve slot drill length and pad size
    if(padShape == PCBPadShape::oval || padShape == PCBPadShape::rectangle)
    {
      holeSize.X = atof(paramList[13].c_str()) * tenmils_to_mm_coefficient;
      holeSize.Y = atof(paramList[9].c_str()) * 2 * tenmils_to_mm_coefficient;
      padSize.X = atof(paramList[4].c_str()) * tenmils_to_mm_coefficient;
      padSize.Y = atof(paramList[5].c_str()) * tenmils_to_mm_coefficient;
    }
    else if(padShape == PCBPadShape::circle)
    {
      holeSize.X = padSize.Y = atof(paramList[9].c_str()) * 2 * tenmils_to_mm_coefficient;
      padSize.X = padSize.Y = atof(paramList[4].c_str()) * tenmils_to_mm_coefficient;
    }
    else //polygon
    {
      padSize.X = padSize.Y = 0.0f;
      vector<string> polygonCoordinates = splitString(paramList[10], ' ');
      coordinates polygonPointTemp = { 0.0f, 0.0f };
      for(int i = 0; i < polygonCoordinates.size(); i += 2)
      {
        polygonPointTemp.X = (atof(polygonCoordinates[  i  ].c_str()) - origin.X) * tenmils_to_mm_coefficient;
        polygonPointTemp.Y = (atof(polygonCoordinates[i + 1].c_str()) - origin.Y) * tenmils_to_mm_coefficient;
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
  }
}

