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
  string PCB_Pad::outputKiCadFormat(string &convArgs, char* &indent)
  {
    std::stringstream returnValue;
    returnValue << indent;
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
      returnValue << '\n' << indent << "  (zone_connect 2)" << '\n' << indent << "  (options (clearance outline) (anchor circle))\n"
                  << indent << "  (primitives\n" << indent << "    (gr_poly (pts\n      " << indent;
      for(int i = 0; i < shapePolygonPoints.size(); i++)
        returnValue << " (xy " << to_string(shapePolygonPoints[i].X) << ' ' << to_string(shapePolygonPoints[i].Y) << ')';
      returnValue << ") (width 0))\n" << indent << "  ))";
    }
    return returnValue.str();
  }

  string PCB_Via::outputKiCadFormat(string &convArgs, char* &indent)
  {
    std::stringstream returnValue;

    returnValue << indent;
    returnValue << "(via (at " << to_string(viaCoordinates.X) << ' ' << to_string(viaCoordinates.Y) << ") (size " << viaSize
                << ") (drill " << to_string(drillSize) << ") (layers ";
    
    //LCEDA currently doesn't support buried or blind vias. If this function is implemented later, we'll have to update
    //the layer section.
    returnValue << "F.Cu B.Cu";

    returnValue << ") (net " << netName << "))";
    return returnValue.str();
  }

  string PCB_Track::outputKiCadFormat(string &convArgs, char* &indent)
  {

  }
}

