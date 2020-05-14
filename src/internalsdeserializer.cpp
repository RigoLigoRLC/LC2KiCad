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
#include <fstream>
#include <ctime>

#include "includes.hpp"
#include "internalsdeserializer.hpp"
#include "edaclasses.hpp"

using std::cout;
using std::endl;
using std::fstream;
using std::string;
using std::vector;
using std::stof;
using std::stoi;
using std::to_string;

namespace lc2kicad
{
  void KiCad_5_Deserializer::initWorkingDocument(EDADocument *_workingDocument) 
  { 
    indent = "  "; 
    workingDocument = _workingDocument;
    //TODO: Evaluate the flood fill region priorities
  }
  void KiCad_5_Deserializer::deinitWorkingDocument() { indent = ""; workingDocument = nullptr; }
  void KiCad_5_Deserializer::setCompatibilitySwitches(const str_dbl_map &_compatibSw) { internalCompatibilitySwitches = _compatibSw; }
  
  KiCad_5_Deserializer::~KiCad_5_Deserializer() { };

  string* KiCad_5_Deserializer::outputFileHeader()
  {
    RAIIC<string> ret;
    string timestamp = decToHex(time(nullptr));
    workingDocument->docInfo["timestamp"] = timestamp;
    switch(workingDocument->docType)
    {
      case documentTypes::pcb_lib:
        *ret += "(module " + workingDocument->docInfo["documentname"] + " (tedit " + timestamp + ")\n"
              + "  (fp_text reference REF*** (at 0 10) (layer F.SilkS)"
                " (effects (font (size 1 1) (thickness 0.15))))\n"
                "  (fp_text value " + workingDocument->docInfo["documentname"] + " (at 0 0) (layer F.Fab)"
                " (effects (font (size 1 1) (thickness 0.15))))\n\n";
        indent += "";
        break;
      default:
        assertThrow(false, "Not implemented function: PCB deserializing not supported.");
    }
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputFileEnding()
  {
    RAIIC<string> ret;
    switch(workingDocument->docType)
    {
      case documentTypes::pcb:
      case documentTypes::pcb_lib:
        *ret += ")";
        break;
      default:
        break;
    }
    indent = "";
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBModule(const PCB_Module& target)
  {
    RAIIC<string> ret;
    
    if(!target.parent->module)
      indent = "  ";

    for(auto &i : target.containedElements)
      *ret += *i->deserializeSelf(*this) + "\n";

    indent = "";
    
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBPad(const PCB_Pad& target) const
  {
    RAIIC<string> ret;
    *ret += indent;
    *ret += "(pad \"" + target.pinNumber + "\" " + padTypeKiCad[static_cast<int>(target.padType)] + ' '
          + padShapeKiCad[static_cast<int>(target.padShape)] + " (at " + to_string(target.padCoordinates.X)
          + ' ' + to_string(target.padCoordinates.Y) + (target.orientation ? " " + to_string(target.orientation) : "")
          + ") (size " + to_string(target.padSize.X) + ' '+ to_string(target.padSize.Y);
    if(target.padType == PCBPadType::through || target.padType == PCBPadType::noplating)
    {
      *ret += ") (drill";
      if(target.holeShape == PCBHoleShape::slot)
      {
        *ret += " oval " + to_string(target.holeSize.X) + ' ' + to_string(target.holeSize.Y);
      }
      else
      {
        *ret += ' ' + to_string(target.holeSize.X);
      }
    }
    *ret += ") (layers ";
    switch(target.padType)
    {
      case PCBPadType::top:
        *ret += "F.Cu F.Paste F.Mask)";
        break;
      case PCBPadType::bottom:
        *ret += "B.Cu B.Paste B.Mask)";
        break;
      default:
        *ret += "*.Cu *.Mask)";
    }
    if(target.padShape != PCBPadShape::polygon)
      *ret += ')';
    else
    {
      *ret += string("\n") + indent + string("  (zone_connect 2)") + '\n' + indent +
              "  (options (clearance outline) (anchor circle))\n"
            + indent + "  (primitives\n" + indent + "    (gr_poly (pts\n      " + indent;
      for(coordinates i : target.shapePolygonPoints)
        *ret += " (xy " + to_string(i.X) + ' ' + to_string(i.Y) + ')';
      *ret += string(") (width 0))\n") + indent + "  ))";
    }
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBVia(const PCB_Via& target) const
  {
    RAIIC<string> ret;

    *ret += indent;
    *ret += "(via (at " + to_string(target.holeCoordinates.X) + ' ' + to_string(target.holeCoordinates.Y) + ") (size "
         + to_string(target.viaDiameter)
         + ") (drill " + to_string(target.holeDiameter) + ") (layers ";
    
    // LCEDA currently doesn't support buried or blind vias. If this function is implemented later, we'll have to update
    // the layer section.
    *ret += "F.Cu B.Cu";

    *ret += ") (net " + target.netName + "))";
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBCopperTrack(const PCB_CopperTrack& target) const
  {
    RAIIC<string> ret;
    bool isInFootprint = target.parent->module; // If not in a footprint, use gr_line. Else, use fp_line

    if(isInFootprint)
      std::cerr << "Warning: Copper track " << target.id << " on footprint. This is not recommended.\n";

    for(int i = 0; i < target.trackPoints.size() - 1; i++)
      *ret += indent + string(isInFootprint? "(segment (start " : "(gr_line  (start ") + to_string(target.trackPoints[i].X) + ' '
           + to_string(target.trackPoints[i].Y) + ") (end " + to_string(target.trackPoints[i + 1].X) + ' '
           + to_string(target.trackPoints[i + 1].Y) + ") (width " + to_string(target.width) + ") (layer "
           + KiCadLayerNameLUT[target.layerKiCad] + ") (net " + target.netName + "))\n";

    (!ret)[ret->size()] = '\0'; // Remove the last '\n' because no end-of-line is needed at the end right there
    
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBGraphicalTrack(const PCB_GraphicalTrack& target) const
  {
    RAIIC<string> ret;
    bool isInFootprint = target.parent->module; // If not in a footprint, use gr_line. Else, use fp_line
      

    for(int i = 0; i < target.trackPoints.size() - 1; i++)
      *ret += indent + string(isInFootprint ? "(fp_line (start " : "(gr_line (start ") + to_string(target.trackPoints[i].X)
           + ' ' + to_string(target.trackPoints[i].Y) + ") (end " + to_string(target.trackPoints[i + 1].X) + ' '
           + to_string(target.trackPoints[i + 1].Y) + ") (layer " + KiCadLayerNameLUT[target.layerKiCad] + ") (width "
           + to_string(target.width) + "))\n";

    (*ret)[ret->size()] = '\0'; // Remove the last '\n' because no end-of-line is needed at the end right there
    
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBFloodFill(const PCB_FloodFill& target) const
  {
    // untested
    RAIIC<string> ret;

    *ret += indent + string("(zone (net ") + target.netName + ") (layer " + KiCadLayerNameLUT[target.layerKiCad]
        + ") (tstamp 0) (hatch edge 0.508)\n" + indent + "  (connect_pads " + (target.isSpokeConnection ? "" : "yes")
        + " (clearance " + to_string(target.clearanceWidth) + "))\n" + indent + "  (min_thickness 0.254)\n" + indent
        + "  (fill " + (target.fillStyle == floodFillStyle::noFill ? "no" : "yes") + " (arc_segments 32) (thermal_gap "
        + to_string(target.clearanceWidth) + ") (thermal_bridge_width " + to_string(target.spokeWidth) + "))\n" + indent
        + "  (polygon\n" + indent + "    (pts\n" + indent + "      ";
    
    for(coordinates i : target.fillAreaPolygonPoints)
      *ret += "(xy " + to_string(i.X) + ' ' + to_string(i.Y) + ") ";

    *ret += indent + string("    )\n") + indent + "  )\n" + indent + ")";

    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputPCBCopperCircle(const PCB_CopperCircle& target) const
  {
    RAIIC<string> ret;
    
    // Notice: PCB circle involves compatibility issues. We need to implement compatibility settings first
    //         before we can start working on PCB circle outputPCB.
    
    /*
    bool isInFootprint;

    *ret += indent + string("(fp_circle (start ") + to_string(center.X) + ' ' + to_string(center.Y) + ") (end "
        + to_string(center.X) + ' ' + to_string(center.Y + radius) + ") (layer " + KiCadLayerNameLUT[target.layerKiCad]
        + ") (width " + to_string(width) + "))\n";

    ret[ret.size()] = '\0'; // Remove the last '\n' because no end-of-line is needed at the end right there
    */
    
    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputPCBGraphicalCircle(const PCB_GraphicalCircle& target) const
  {
    RAIIC<string> ret;
    bool isInFootprint;
    
    if(target.parent->module) // Determine if this graphical line is used in footprint
      isInFootprint = true;
    else
      isInFootprint = false;               // If not in a footprint, use gr_circle. Else, use fp_circle
      
    *ret += indent + string(isInFootprint ? "(fp_circle (center " : "(gr_circle (center ") + to_string(target.center.X)
          + ' ' + to_string(target.center.Y) + ") (end " + to_string(target.center.X) + ' ' + to_string(target.center.Y + target.radius)
          + ") (layer " + KiCadLayerNameLUT[target.layerKiCad] + ") (width " + to_string(target.width) + "))";
    
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBHole(const PCB_Hole& target) const
  {
    RAIIC<string> ret;

    if(target.parent->module)
      *ret += indent + 
              "(pad \"\" np_thru_hole circle (at " + to_string(target.holeCoordinates.X) + " " + to_string(target.holeCoordinates.Y) + ") "
              "(size " + to_string(target.holeDiameter) + " " + to_string(target.holeDiameter) + ")"
              "(drill " + to_string(target.holeDiameter) + ") (layers *.Cu *.Mask))";
    else
      *ret += "  (module MountingHole_NonPlated_Converted (layer F.Cu)\n"
              "    (at " + to_string(target.holeCoordinates.X) + " " + to_string(target.holeCoordinates.Y) + ")\n"
              "    (descr MountingHold_NonPlated_Converted)\n"
              "    (pad \"\" np_thru_hole circle (at 0 0) (size " + to_string(target.holeDiameter) + " " + to_string(target.holeDiameter) + ")"
              "   (drill " + to_string(target.holeDiameter) + ") (layers *.Cu *.Mask))\n"
              "  )";
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBRect(const PCB_Rect& target) const
  {
    RAIIC<string> ret;
    string x1 = to_string(target.topLeftPos.X),
           y1 = to_string(target.topLeftPos.Y),
           x2 = to_string(target.topLeftPos.X + target.size.X),
           y2 = to_string(target.topLeftPos.Y + target.size.Y),
           w = to_string(target.strokeWidth);

    if(target.parent->module)
    {
      
      *ret += indent + "(fp_line (start " + x1 + " " + y1 + ") (end " + x2 + " " + y1 + ") (layer " + KiCadLayerNameLUT[target.layerKiCad]
                     + ") (width " + w + "))\n";
      *ret += indent + "(fp_line (start " + x2 + " " + y1 + ") (end " + x2 + " " + y2 + ") (layer " + KiCadLayerNameLUT[target.layerKiCad]
                     + ") (width " + w + "))\n";
      *ret += indent + "(fp_line (start " + x2 + " " + y2 + ") (end " + x1 + " " + y2 + ") (layer " + KiCadLayerNameLUT[target.layerKiCad]
                     + ") (width " + w + "))\n";
      *ret += indent + "(fp_line (start " + x1 + " " + y2 + ") (end " + x1 + " " + y1 + ") (layer " + KiCadLayerNameLUT[target.layerKiCad]
                     + ") (width " + w + "))\n";
    }
    else
    {
      *ret += "  (gr_poly (pts (xy " + x1 + " " + y1 + ") (xy " + x1 + " " + y2 + ") (xy " + x2 + ' ' + y2 + ") (xy " + x1 + ' ' + y2 + ")) "
              "(layer " + KiCadLayerNameLUT[target.layerKiCad] + ") (width " + w + "))";
    }

    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBSolidRegion(const PCB_SolidRegion& target) const
  {
    RAIIC<string> ret;
    std::cerr << "KiCad_5_Deserializer::outputPCBSolidRegion stub. " << target.id << "is ignored.\n";
    return !++ret;
  }
  string* KiCad_5_Deserializer::outputPCBGraphicalArc(const PCB_GraphicalArc& target) const
  {
    RAIIC<string> ret;
    std::cerr << "KiCad_5_Deserializer::outputPCBGraphicalArc stub. " << target.id << "is ignored.\n";
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBCopperArc(const PCB_CopperArc& target) const
  {
    RAIIC<string> ret;
    std::cerr << "KiCad_5_Deserializer::outputPCBCopperArc stub. " << target.id << "is ignored.\n";
    return !++ret;
  }
  
  // KiCad schematic pin output, legacy format.
  string* KiCad_5_Deserializer::outputSchPin(const Schematic_Pin& target) const
  {
    RAIIC<string> ret;
    *ret += string("X ") + target.pinName + " " + target.pinNumber + " " + to_string(target.pinCoord.X) + " " + to_string(target.pinCoord.Y)
          + " " + to_string(target.pinLength) + " ";
    switch(target.pinRotation)
    {
      default:
      case Schematic_Pin::pinRotations::Deg0:
        *ret += "L "; break;
      case Schematic_Pin::pinRotations::Deg90:
        *ret += "D "; break;
      case Schematic_Pin::pinRotations::Deg180:
        *ret += "R "; break;
      case Schematic_Pin::pinRotations::Deg270:
        *ret += "U "; break;
    }
    *ret += to_string(target.fontSize) + " " + to_string(target.fontSize) + " ";
    *ret += "U "; //Electrical property. Not implemented yet
    *ret += target.clock ? target.inverted ? "IC" : "C" : target.inverted ? "I" : ""; //Either clock, or target. Or both, or none.
    
    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputSchPolyline(const Schematic_Polyline& target) const
  {
    RAIIC<string> ret;
    std::cerr << "KiCad_5_Deserializer::outputSchPolyline stub. " << target.id << "is ignored.\n";
    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputSchRect(const Schematic_Rect& target) const
  {
    RAIIC<string> ret;
    std::cerr << "KiCad_5_Deserializer::outputSchRect stub. " << target.id << "is ignored.\n";
    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputSchPolygon(const Schematic_Polygon& target) const
  {
    RAIIC<string> ret;
    std::cerr << "KiCad_5_Deserializer::outputSchPolygon stub. " << target.id << "is ignored.\n";
    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputSchText(const Schematic_Text& target) const
  {
    RAIIC<string> ret;
    std::cerr << "KiCad_5_Deserializer::outputSchText stub. " << target.id << "is ignored.\n";
    return !++ret;
  }
}
