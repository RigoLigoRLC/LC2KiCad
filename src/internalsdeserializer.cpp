/*
    Copyright (c) 2020 RigoLigoRLC.

    This file is part of LC2KiCad.

    LC2KiCad is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as 
    published by the Free Software Foundation, version 2, or version 3
    of the License.

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
#include <cmath>

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
using std::sin;
using std::cos;

namespace lc2kicad
{
  void KiCad_5_Deserializer::initWorkingDocument(EDADocument *_workingDocument) 
  { 
    indent = "  "; 
    workingDocument = _workingDocument;
  }
  void KiCad_5_Deserializer::deinitWorkingDocument() { indent = ""; workingDocument = nullptr; }
  void KiCad_5_Deserializer::setCompatibilitySwitches(const str_dbl_map &_compatibSw) { internalCompatibilitySwitches = _compatibSw; }
  
  KiCad_5_Deserializer::~KiCad_5_Deserializer() { };

  string* KiCad_5_Deserializer::outputFileHeader()
  {
    RAIIC<string> ret;
    string timestamp = decToHex(time(nullptr));
    str_str_map &docInfo = workingDocument->docInfo;
    docInfo["timestamp"] = timestamp;

    // File "header" is everything comes before the actual components.
    // It is document type dependent.

    switch(workingDocument->docType)
    {
      case documentTypes::schematic_lib:
        *ret += "EESchema-LIBRARY Version 2.4\n"
                "#encoding utf-8\n"
                "#\n"
                "# " + docInfo["documentname"] + "\n"
                "#\n"
                "DEF " + docInfo["documentname"] + " " + docInfo["prefix"] + " 0 40 Y Y 1 F N\n"
                "F0 \"" + docInfo["prefix"] + "\" 0 50 50 H V C CNN\n"
                "F1 \"" + docInfo["documentname"] + "\" 0 -50 50 H V C CNN\n"
                "F2 \"\" 0 0 50 H I C CNN\n"
                "F3 \"\" 0 0 50 H I C CNN\n"
                "DRAW\n";
        break;
      case documentTypes::pcb:
      {
        *ret += "(kicad_pcb (version 20171130) (host pcbnew \"(5.1.4-0-10_14)\")\n";
        *ret += static_cast<PCBDocument*>(workingDocument)->netManager.outputPCBNetInfo();

        auto netclassStr = outputPCBNetclassRules(static_cast<PCBDocument*>(workingDocument)->netClasses);
        *ret += *netclassStr;
        delete netclassStr;

        break;
      }
      case documentTypes::pcb_lib:
        *ret += "(module \"" + docInfo["documentname"] + "\" (tedit " + timestamp + ")\n"
              + "  (fp_text reference REF*** (at 0 10) (layer F.SilkS)"
                " (effects (font (size 1 1) (thickness 0.15))))\n"
                "  (fp_text value \"" + docInfo["documentname"] + "\" (at 0 0) (layer F.Fab)"
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
      case documentTypes::schematic_lib:
        *ret += "ENDDRAW\n"
                "ENDDEF\n"
                "#\n"
                "#End Library\n";
        break;
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

  std::string *KiCad_5_Deserializer::outputPCBNetclassRules(const vector<PCBNetClass>& target)
  {
    RAIIC<string> ret;

    for(auto &i : target)
    {
      *ret += "(net_class \"" + i.name + "\" \"Default net class.\"\n";
      for(auto &j : i.rules)
        *ret += "  (" + j.first + " " + to_string(j.second) + ")\n";

      if(i.netClassMembers.size() > 0)
        for(auto &j : i.netClassMembers)
          *ret += "  (add_net \"" + j + "\")\n";

      *ret += ")\n\n";
    }

    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBModule(const PCB_Module& target)
  {
    RAIIC<string> ret;
    string* elementOutput;
    
    if(!workingDocument->module) // Do not output when dealing with PCB module file, but do it for PCB nested modules
    {
      indent = "  ";
      *ret += "(module \"LC2KICAD:" + target.name + "\" (layer " + KiCadLayerName[target.layer] + ") (at "
           + to_string(target.moduleCoords.X) + ' ' + to_string(target.moduleCoords.Y) /*+ ' ' + to_string(target.orientation)*/ + ")\n"
           + indent + "  (fp_text reference REF*** (at 0 10) (layer F.SilkS)"
           "  (effects (font (size 1 1) (thickness 0.15))))\n"
           "   (fp_text value \"" + target.name + "\" (at 0 0) (layer F.Fab)"
           "  (effects (font (size 1 1) (thickness 0.15))))\n\n";
    }

    processingModule = true;
    currentPackageOnTopLayer = target.topLayer;
    for(auto &i : target.containedElements)
    {
      if(!i) continue;
      elementOutput = i->deserializeSelf(*this);
      if(elementOutput)
      {
        *ret += *elementOutput + "\n";
        delete elementOutput;
      }
    }
    processingModule = false; // TODO: RAII

    if(!workingDocument->module)
      *ret += ")\n";

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

    if(isProcessingModules())
    {
      if(target.net.second != "")
        *ret += " (net " + to_string(target.net.first) + " \"" + target.net.second + "\")";
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

    if(!isProcessingModules())
    { // Ordinary Vias on PCBs
      *ret += indent;
      *ret += "(via (at " + to_string(target.holeCoordinates.X) + ' ' + to_string(target.holeCoordinates.Y) + ") (size "
              + to_string(target.viaDiameter)
              + ") (drill " + to_string(target.holeDiameter) + ") (layers ";

      // LCEDA currently doesn't support buried or blind vias. If this function is implemented later, we'll have to update
      // the layer section.
      *ret += "F.Cu B.Cu";

      *ret += ") (net " + to_string(target.net.first) + "))";
    }
    else
    { // Vias got converted to pads inside footprints
      VERBOSE_INFO(target.id + ": this via is in a footprint and is output as a pad.");
      *ret += indent
            + "(pad 0 thru_hole circle (at " + to_string(target.holeCoordinates.X)
            + ' ' + to_string(target.holeCoordinates.Y) + ") (size " + to_string(target.viaDiameter)
            + ' '+ to_string(target.viaDiameter) + ") (drill " + to_string(target.holeDiameter) + ") (layers *.Cu)"
            + (workingDocument->module ? ")" : " (net " + to_string(target.net.first) + " \"" + target.net.second + "\"))");
    }
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBCopperTrack(const PCB_CopperTrack& target) const
  {
    RAIIC<string> ret;
    bool isInFootprint = isProcessingModules(); // If not in a footprint, use gr_line. Else, use fp_line

    if(isInFootprint)
      Warn(target.id + ": Copper track on footprint. This can cause DRC violations.");

    for(unsigned int i = 0; i < target.trackPoints.size() - 1; i++)
    {
      *ret += indent + string(isInFootprint ? "(fp_line (start " : "(segment (start ") + to_string(target.trackPoints[i].X) + ' '
           + to_string(target.trackPoints[i].Y) + ") (end " + to_string(target.trackPoints[i + 1].X) + ' '
           + to_string(target.trackPoints[i + 1].Y) + ") (width " + to_string(target.width) + ") (layer "
           + KiCadLayerName[target.layerKiCad] + ")";
      if(!isInFootprint)
        *ret += "(net " + to_string(target.net.first) + ")";
      *ret += ")\n";
    }

    ret->pop_back(); // Remove the last '\n' because no end-of-line is needed at the end right there
    
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBGraphicalTrack(const PCB_GraphicalTrack& target) const
  {
    RAIIC<string> ret;
    bool isInFootprint = isProcessingModules(); // If not in a footprint, use gr_line. Else, use fp_line

    for(unsigned int i = 0; i < target.trackPoints.size() - 1; i++)
      *ret += indent + string(isInFootprint ? "(fp_line (start " : "(gr_line (start ") + to_string(target.trackPoints[i].X)
           + ' ' + to_string(target.trackPoints[i].Y) + ") (end " + to_string(target.trackPoints[i + 1].X) + ' '
           + to_string(target.trackPoints[i + 1].Y) + ") (layer " + KiCadLayerName[target.layerKiCad] + ") (width "
           + to_string(target.width) + "))\n";

    ret->pop_back(); // Remove the last '\n' because no end-of-line is needed at the end right there
    
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBFloodFill(const PCB_FloodFill& target) const
  {
    RAIIC<string> ret;

    if(isProcessingModules())
    {
      Warn(target.id + ": Fill areas are not allowed within footprint. This area was discarded.");
      return nullptr;
    }

    *ret += indent + string("(zone (net ") + to_string(target.net.first) + ") (net_name \"" + target.net.second
        + "\") (layer " + KiCadLayerName[target.layerKiCad] + ") (tstamp 0) (hatch edge 0.508)\n"

        + indent + "  (priority " + to_string(static_cast<PCBDocument*>(workingDocument)->fillPriorityManager
                                              .getKiCadPriority(target.EasyEDAPriority)) + ")"

        + indent + "  (connect_pads " + (target.isSpokeConnection ? "" : "yes") + " (clearance "
        + to_string(target.clearanceWidth) + "))\n"

        + indent + "  (min_thickness " + to_string(target.minimumWidth) + ")\n"

        + indent + "  (fill " + (target.fillStyle == floodFillStyle::noFill ? "no" : "yes")
        + " (arc_segments 32) (thermal_gap " + to_string(target.clearanceWidth) + ") (thermal_bridge_width "
        + to_string(target.spokeWidth) + "))\n"

        + indent + "  (polygon\n"
        + indent + "    (pts\n"
        + indent + "      ";
    
    for(coordinates i : target.fillAreaPolygonPoints)
      *ret += "(xy " + to_string(i.X) + ' ' + to_string(i.Y) + ") ";

    *ret += indent + string("    )\n") + indent + "  )\n" + indent + ")";

    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBKeepoutRegion(const PCB_KeepoutRegion& target) const
  {
    RAIIC<string> ret;

    // TODO: disallow only if you specify KiCad 5
    if(isProcessingModules())
    {
      Warn(target.id + ": Keepout areas are not allowed within footprint in KiCad 5. This area was discarded.");
      return nullptr;
    }

    *ret += indent + "(zone (net 0) (net_name \"\") (layer " + KiCadLayerName[target.layerKiCad] + ") (tstamp 0) (hatch edge 0.508)\n"
          + indent + "  (connect_pads (clearance 0.508))\n"
          + indent + "  (min_thickness 0.254)\n"
          + indent + "  (keepout (tracks " + (target.allowRouting ? "allowed" : "not_allowed")
                   + ") (vias " + (target.allowVias ? "allowed" : "not_allowed")
                   + ") (copperpour " + (target.allowFloodFill ? "allowed" : "not_allowed") + "))\n"
          + indent + "  (fill (arc_segments 32) (thermal_gap 0.508) (thermal_bridge_width 0.508))\n"
          + indent + "  (polygon\n"
          + indent + "    (pts\n"
          + indent + "      ";
    for(coordinates i : target.fillAreaPolygonPoints)
      *ret += "(xy " + to_string(i.X) + ' ' + to_string(i.Y) + ") ";
    *ret += indent + "    )\n"
          + indent + "  )\n"
          + indent + ")";

    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputPCBCopperCircle(const PCB_CopperCircle& target) const
  {
    RAIIC<string> ret;

    // Warn the user about this
    if(isProcessingModules())
      Warn(target.id + ": Copper track on footprint. This can cause DRC violations.");

    *ret += indent + string(isProcessingModules() ? "(fp_circle " : "(gr_circle ")
          + "(center " + to_string(target.center.X) + ' ' + to_string(target.center.Y) + ") "
            "(end " + to_string(target.center.X) + ' ' + to_string(target.center.Y + target.radius) + ") "
            "(layer " + KiCadLayerName[target.layerKiCad]
          + ") (width " + to_string(target.width) + "))\n";
    
    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputPCBGraphicalCircle(const PCB_GraphicalCircle& target) const
  {
    RAIIC<string> ret;

    *ret += indent + string(isProcessingModules() ? "(fp_circle (center " : "(gr_circle (center ") + to_string(target.center.X)
          + ' ' + to_string(target.center.Y) + ") (end " + to_string(target.center.X) + ' ' + to_string(target.center.Y + target.radius)
          + ") (layer " + KiCadLayerName[target.layerKiCad] + ") (width " + to_string(target.width) + "))";
    
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBHole(const PCB_Hole& target) const
  {
    RAIIC<string> ret;

    if(isProcessingModules())
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

    if(isProcessingModules())
    {
      
      *ret += indent + "(fp_line (start " + x1 + " " + y1 + ") (end " + x2 + " " + y1 + ") (layer " + KiCadLayerName[target.layerKiCad]
                     + ") (width " + w + "))\n";
      *ret += indent + "(fp_line (start " + x2 + " " + y1 + ") (end " + x2 + " " + y2 + ") (layer " + KiCadLayerName[target.layerKiCad]
                     + ") (width " + w + "))\n";
      *ret += indent + "(fp_line (start " + x2 + " " + y2 + ") (end " + x1 + " " + y2 + ") (layer " + KiCadLayerName[target.layerKiCad]
                     + ") (width " + w + "))\n";
      *ret += indent + "(fp_line (start " + x1 + " " + y2 + ") (end " + x1 + " " + y1 + ") (layer " + KiCadLayerName[target.layerKiCad]
                     + ") (width " + w + "))\n";
    }
    else
    {
      *ret += "  (gr_poly (pts (xy " + x1 + " " + y1 + ") (xy " + x1 + " " + y2 + ") (xy " + x2 + ' ' + y2 + ") (xy " + x1 + ' ' + y2 + ")) "
              "(layer " + KiCadLayerName[target.layerKiCad] + ") (width " + w + "))";
    }

    return !++ret;
  }

  string *KiCad_5_Deserializer::outputPCBText(const PCB_Text& target) const
  {
    //if(target.type == PCBTextTypes::StandardText && (processingModule | isProcessingModules()))
    //  return nullptr;

    RAIIC<string> ret;
    *ret += indent + ((isProcessingModules()) ? "(fp_text " : "(gr_text ");
    switch(target.type)
    {
      case PCBTextTypes::StandardText: if(isProcessingModules()) *ret += "user"; break;
      case PCBTextTypes::PackageReference: *ret += "reference"; break;
      case PCBTextTypes::PackageValue: *ret += "value"; break;
      case PCBTextTypes::PackageName: return nullptr; break;
      default:
        break;
    }
    *ret += indent
          + " \"" + target.text + "\" (at " + to_string(target.midLeftPos.X) + ' ' + to_string(target.midLeftPos.Y)
          + (target.orientation ? " " + to_string(target.orientation) + ") " : ") ") + "(layer "
          + (target.type == PCBTextTypes::PackageValue ?
               target.layerKiCad == F_SilkS ? KiCadLayerName[F_Fab] : KiCadLayerName[B_Fab]
                                              : KiCadLayerName[target.layerKiCad]);
    if((workingDocument->module | processingModule))
      if(!target.visibility)
        *ret += ") hide\n";
      else
        *ret += ")\n";
    else
      *ret += ")\n";

    *ret += indent + "  (effects (font (size " + to_string(target.height) + ' ' + to_string(target.height) + ") (thickness "
          + to_string(target.width) + ")) (justify left";
    if(target.mirrored)
      *ret += " mirror";
    *ret += "))\n"

          + indent + ")\n";

    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBCopperSolidRegion(const PCB_CopperSolidRegion& target) const
  {
    RAIIC<string> ret;
    Warn("KiCad_5_Deserializer::outputPCBSolidRegion stub. " + target.id + "is ignored.");
    return !++ret;
  }

  string *KiCad_5_Deserializer::outputPCBGraphicalSolidRegion(const PCB_GraphicalSolidRegion& target) const
  {
    RAIIC<string> ret;
    KiCadLayerIndex realLayer;
    // NOTE: Although the hacks of courtyard doesn't cover all cases, but till now, all instances of
    //       EasyEDA courtyards are solid fills. so this should work
    if(target.layerKiCad == F_CrtYd && !currentPackageOnTopLayer)
      realLayer = B_CrtYd;
    else
      realLayer = target.layerKiCad;
    *ret += (isProcessingModules() ? "(fp_poly (pts " : "(gr_poly (pts ");
    for(auto &i : target.fillAreaPolygonPoints)
      *ret += "(xy " + to_string(i.X) + ' ' + to_string(i.Y) + ") ";
    *ret += ") (layer " + KiCadLayerName[realLayer] + ") (fill ";

    if(realLayer == F_CrtYd || realLayer == B_CrtYd)
      *ret += "none) (width 0.508))";
    else
      *ret += "solid) (width 0))";

    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBGraphicalArc(const PCB_GraphicalArc& target) const
  {
    RAIIC<string> ret;
    *ret += ((isProcessingModules()) ? "(fp_arc (start " : "(gr_arc (start ") + to_string(target.center.X)
          + ' ' + to_string(target.center.Y) + ") (end " + to_string(target.endPoint.X) + ' ' + to_string(target.endPoint.Y)
          + ") (angle " + to_string(target.angle) + ") (layer " + KiCadLayerName[target.layerKiCad]
          + ") (width " + to_string(target.width) + "))";
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputPCBCopperArc(const PCB_CopperArc& target) const
  {
    RAIIC<string> ret;
    *ret += ((isProcessingModules()) ? "(fp_arc (start " : "(gr_arc (start ") + to_string(target.center.X)
          + ' ' + to_string(target.center.Y) + ") (end " + to_string(target.endPoint.X) + ' ' + to_string(target.endPoint.Y)
          + ") (angle " + to_string(target.angle) + ") (layer " + KiCadLayerName[target.layerKiCad]
          + ") (width " + to_string(target.width) + "))";
    Warn(target.id + ": KiCad 5 doesn't support copper layer arc with nets. Net info of this copper arc is discarded.");
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputSchModule(const Schematic_Module& target)
  {
    RAIIC<string> ret;
    if(isProcessingModules())
    {
      string* elementOutput;

      for(auto &i : target.containedElements)
      {
        elementOutput = i->deserializeSelf(*this);
        *ret += *elementOutput + "\n";
        delete elementOutput;
      }
    }
    return !++ret;
  }
  
  // KiCad schematic pin output, legacy format.
  string* KiCad_5_Deserializer::outputSchPin(const Schematic_Pin& target) const
  {
    RAIIC<string> ret;
    *ret += "X " + target.pinName + " " + target.pinNumber + " " + to_string(static_cast<int>(target.pinCoord.X)) + " "
          + to_string(static_cast<int>(target.pinCoord.Y)) + " " + to_string(static_cast<int>(target.pinLength)) + " ";
    switch(target.pinRotation)
    {
      default:
      case SchematicRotations::Deg0:
        *ret += "L "; break;
      case SchematicRotations::Deg90:
        *ret += "D "; break;
      case SchematicRotations::Deg180:
        *ret += "R "; break;
      case SchematicRotations::Deg270:
        *ret += "U "; break;
    }

    *ret += to_string(target.fontSize) + " " + to_string(target.fontSize) + " ";
    *ret += "1 0 ";

    //Electrical property. EasyEDA didn't split power in and power out, so power would become passive to avoid ERC violations.
    switch (target.electricProperty)
    {
      case SchPinElectricProperty::Unspecified:
        *ret += "U "; break;
      case SchPinElectricProperty::Input:
        *ret += "I "; break;
      case SchPinElectricProperty::Output:
        *ret += "O "; break;
      case SchPinElectricProperty::Bidirectional:
        *ret += "B "; break;
      case SchPinElectricProperty::Power:
      default:
        *ret += "P "; break;
    }
    *ret += target.clock ? target.inverted ? "IC" : "C" : target.inverted ? "I" : ""; //Either clock, or target. Or both, or none.
    
    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputSchPolyline(const Schematic_Polyline& target) const
  {
    RAIIC<string> ret;
    *ret += "P " + to_string(target.polylinePoints.size()) + " 0 0 " + to_string(target.lineWidth) + " ";
    for(auto &i : target.polylinePoints)
      *ret += to_string(static_cast<int>(i.X)) + " " + to_string(static_cast<int>(i.Y)) + " ";
    *ret += (target.isFilled ? "f" : "N");
    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputSchRect(const Schematic_Rect& target) const
  {
    RAIIC<string> ret;
    *ret += "S " + to_string(static_cast<int>(target.position.X)) + " " + to_string(static_cast<int>(target.position.Y)) + " "
         + to_string(static_cast<int>(target.size.X + target.position.X)) + " "          // KiCad uses the top-left and bottom-right
         + to_string(static_cast<int>(target.size.Y * -1 + target.position.Y)) + " 0 0 " // corner coordinates to determine a rectangle
         + to_string(static_cast<int>(target.width)) + (target.isFilled ? " f" : " N");
    return !++ret;
  }
  
  string* KiCad_5_Deserializer::outputSchPolygon(const Schematic_Polygon& target) const
  {
    RAIIC<string> ret;
    *ret += "P " + to_string(target.polylinePoints.size() + 1) + " 0 0 " + to_string(target.lineWidth) + " ";
    for(auto &i : target.polylinePoints)
      *ret += to_string(static_cast<int>(i.X)) + " " + to_string(static_cast<int>(i.Y)) + " ";
    // For polygons they are represented in "P" as well but the last point is the same as the first point.
    // Point count is also incremented by 1.
    *ret += to_string(static_cast<int>(target.polylinePoints[0].X)) + " " + to_string(static_cast<int>(target.polylinePoints[0].Y));
    *ret += (target.isFilled ? " f" : " N");
    return !++ret;
  }

  string *KiCad_5_Deserializer::outputSchArc(const Schematic_Arc& target) const
  {
    RAIIC<string> ret;
    *ret += "A " + to_string(static_cast<int>(target.center.X)) + " " + to_string(static_cast<int>(target.center.Y)) + " "
          + to_string(static_cast<int>(target.size.X)) + " "
          + to_string(static_cast<int>(target.startAngle * 10)) + " " + to_string(static_cast<int>(target.endAngle * 10)) + " "
            "0 1 "
          + to_string(static_cast<int>(target.width)) + " " + (target.isFilled ? " f" : " N") + " "
          + to_string(static_cast<int>(target.startPoint.X)) + " " + to_string(static_cast<int>(target.startPoint.Y)) + " "
          + to_string(static_cast<int>(target.endPoint.X)) + " " + to_string(static_cast<int>(target.endPoint.Y)) + " ";
    return !++ret;
  }

  string* KiCad_5_Deserializer::outputSchText(const Schematic_Text& target) const
  {
    RAIIC<string> ret;
    Warn("KiCad_5_Deserializer::outputSchText stub. " + target.id + "is ignored.");
    return !++ret;
  }
}
