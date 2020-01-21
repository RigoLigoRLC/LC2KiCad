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
#include "includes.hpp"
#include "consts.hpp"
#include "rapidjson.hpp"

using std::vector;
using std::to_string;
using rapidjson::Value;

namespace lc2kicad
{
  string PCB_Pad::outputKiCadFormat(string &convArgs, char* &indent)
  {
    string ret;
    ret += indent;
    ret += "(pad \"" + pinNumber + "\" " + padTypeKiCad[static_cast<int>(padType)] + ' '
                + padShapeKiCad[static_cast<int>(padShape)] + " (at " + to_string(padCoordinates.X)
                + ' ' + to_string(padCoordinates.Y) + ") (size " + to_string(padSize.X) + ' '
                + to_string(padSize.Y);
    if(padType == PCBPadType::through || padType == PCBPadType::noplating)
    {
      ret += ") (drill";
      if(holeShape == PCBHoleShape::slot)
      {
        ret += " oval " + to_string(holeSize.X) + ' ' + to_string(holeSize.Y);
      }
      else
      {
        ret += ' ' + to_string(holeSize.X);
      }
    }
    ret += ") (layers ";
    switch(padType)
    {
      case PCBPadType::top:
        ret += "F.Cu F.Paste F.Mask)";
        break;
      case PCBPadType::bottom:
        ret += "B.Cu B.Paste B.Mask)";
        break;
      default:
        ret += "*.Cu *.Mask)";
    }
    if(padShape != PCBPadShape::polygon)
      ret += ')';
    else
    {
      ret += '\n' + indent + string("  (zone_connect 2)") + '\n' + indent + "  (options (clearance outline) (anchor circle))\n"
                  + indent + "  (primitives\n" + indent + "    (gr_poly (pts\n      " + indent;
      for(int i = 0; i < shapePolygonPoints.size(); i++)
        ret += " (xy " + to_string(shapePolygonPoints[i].X) + ' ' + to_string(shapePolygonPoints[i].Y) + ')';
      ret += string(") (width 0))\n") + indent + "  ))";
    }
    return ret;
  }

  string PCB_Via::outputKiCadFormat(string &convArgs, char* &indent)
  {
    string ret;

    ret += indent;
    ret += "(via (at " + to_string(viaCoordinates.X) + ' ' + to_string(viaCoordinates.Y) + ") (size " + to_string(viaSize)
                + ") (drill " + to_string(drillSize) + ") (layers ";
    
    //LCEDA currently doesn't support buried or blind vias. If this function is implemented later, we'll have to update
    //the layer section.
    ret += "F.Cu B.Cu";

    ret += ") (net " + netName + "))";
    return ret;
  }

  string PCB_Track::outputKiCadFormat(string &convArgs, char* &indent)
  {
    string ret;

    for(int i = 0; i < trackPoints.size() - 1; i++)
      ret += indent + string("(segment (start ") + to_string(trackPoints[i].X) + ' '
           + to_string(trackPoints[i].Y) + ") (end " + to_string(trackPoints[i + 1].X) + ' '
           + to_string(trackPoints[i + 1].Y) + ") (width " + to_string(width) + ") (layer "
           + KiCadLayerNameLUT[layerKiCad] + ") (net " + netName + "))\n";

    ret[ret.size()] = '\0'; //Remove the last '\n' because no end-of-line is needed at the end right there
    
    return ret;
  }

  string PCB_GraphicalLine::outputKiCadFormat(string &convArgs, char* &indent)
  {
    string ret;

    for(int i = 0; i < trackPoints.size() - 1; i++)
      ret += indent + string("(gr_line (start ") + to_string(trackPoints[i].X) + ' '
           + to_string(trackPoints[i].Y) + ") (end " + to_string(trackPoints[i + 1].X) + ' '
           + to_string(trackPoints[i + 1].Y) + ") (layer " + KiCadLayerNameLUT[layerKiCad] + ") (width "
           + to_string(width) + "))\n";

    ret[ret.size()] = '\0'; //Remove the last '\n' because no end-of-line is needed at the end right there
    
    return ret;
  }
}

