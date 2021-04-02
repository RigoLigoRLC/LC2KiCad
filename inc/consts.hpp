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
    along with LC2KiCad. If not, see <https://www.gnu.org/licenses/>.
*/
  
#ifndef LC2KICAD_CONSTS
  #define LC2KICAD_CONSTS

  #include <map>
  #include <string>

  namespace lc2kicad
  {
    #define SOFTWARE_VERSION "alpha-0.1.6"

    enum KiCadLayerIndex
    {
      Invalid = -1,
      F_Cu = 0, In1_Cu, In2_Cu, In3_Cu, In4_Cu, In5_Cu, In6_Cu, In7_Cu, In8_Cu, In9_Cu, In10_Cu, In11_Cu, In12_Cu, In13_Cu,
      In14_Cu, In15_Cu, In16_Cu, In17_Cu, In18_Cu, In19_Cu, In20_Cu, In21_Cu, In22_Cu, In23_Cu, In24_Cu, In25_Cu, In26_Cu,
      In27_Cu, In28_Cu, In29_Cu, In30_Cu, B_Cu, B_Adhes, F_Adhes, B_Paste, F_Paste, B_SilkS, F_SilkS, B_Mask, F_Mask,
      Dwgs_User, Cmts_User, Eco1_User, Eco2_User, Edge_Cuts, Margin, B_CrtYd, F_CrtYd, B_Fab, F_Fab
    };

    static const char *rapidjsonErrorMsg[] = {"No error", "Empty document", "Document root is not singular", "Invalid value", "Object missing its name", "Object missing colon", "Object missing ',' or '}'", "Array missing ',' or ']'", "Incorrect digits after \\u escape", "Surrogate pair in the string is invalid", "Invalid escape character in the string", "String missing '\"'", "Invalid encoding of the string", "Number is too big", "Number missing its fraction part", "Number missing its exponent part", "Parsing is terminated", "Unspecificd syntax error"};

    static const double tenmils_to_mm_coefficient = 0.254;
    static const double sch_convert_coefficient = 10;

    static const char *documentTypeName[8] = {"", "schematics", "schematic library", "PCB", "PCB library", "project", "sub-part", "SPICE symbol"};
    static const char *documentExtensionName[8] = {"", "", ".lib", ".kicad_pcb", ".kicad_mod", "prj", "", ""};
    //Layer mapper. Input EasyEDA, ouput KiCad.
    static const int EasyEdaToKiCadLayerMap[] = {-1, 0, 31, 37, 36, 35, 34, 39, 38, -1, 44, -1, 41, 49, 48, -1, -1, -1, -1, -1, -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
    static const char *padTypeKiCad[] = {"smd", "smd", "thru_hole", "np_thru_hole"};
    static const char *padShapeKiCad[] = {"circle", "oval", "rect", "custom"};

    // Definiton from https://en.wikipedia.org/wiki/Filename#Reserved_characters_and_words .
    static const std::string illegalCharsOfFilenames = "\\/:%*?\"<>|,;=";

    enum elementType
    {
      Base,
      
      PCBBase,
      PCBModule, PCBPad, PCBGraphicalTrack, PCBCopperTrack, PCBHole, PCBVia, PCBSolidRegion, PCBFloodFill,
      PCBGraphicalCircle, PCBCopperCircle, PCBRect, PCBGraphicalArc, PCBCopperArc,

      SchBase,
      SchModule, SchPin, SchPolyline, SchText, SchRect, SchPolygon, SchImage
    };

    static std::map<KiCadLayerIndex, std::string> KiCadLayerName
    {
      {F_Cu, "F.Cu"},
      {In1_Cu, "In1.Cu"},
      {In2_Cu, "In2.Cu"},
      {In3_Cu, "In3.Cu"},
      {In4_Cu, "In4.Cu"},
      {In5_Cu, "In5.Cu"},
      {In6_Cu, "In6.Cu"},
      {In7_Cu, "In7.Cu"},
      {In8_Cu, "In8.Cu"},
      {In9_Cu, "In9.Cu,"},
      {In10_Cu, "In10.Cu"},
      {In11_Cu, "In11.Cu"},
      {In12_Cu, "In12.Cu"},
      {In13_Cu, "In13.Cu"},
      {In14_Cu, "In14.Cu"},
      {In15_Cu, "In15.Cu"},
      {In16_Cu, "In16.Cu"},
      {In17_Cu, "In17.Cu"},
      {In18_Cu, "In18.Cu"},
      {In19_Cu, "In19.Cu"},
      {In20_Cu, "In20.Cu"},
      {In21_Cu, "In21.Cu"},
      {In22_Cu, "In22.Cu"},
      {In23_Cu, "In23.Cu"},
      {In24_Cu, "In24.Cu"},
      {In25_Cu, "In25.Cu"},
      {In26_Cu, "In26.Cu"},
      {In27_Cu, "In27.Cu"},
      {In28_Cu, "In28.Cu"},
      {In29_Cu, "In29.Cu"},
      {In30_Cu, "In30.Cu"},
      {B_Cu, "B.Cu"},
      {B_Adhes, "B.Adhes"},
      {F_Adhes, "F.Adhes"},
      {B_Paste, "B.Paste"},
      {F_Paste, "F.Paste"},
      {B_SilkS, "B.SilkS"},
      {F_SilkS, "F.SilkS"},
      {B_Mask, "B.Mask"},
      {F_Mask, "F.Mask"},
      {Dwgs_User, "Dwgs.User"},
      {Cmts_User, "Cmts.User"},
      {Eco1_User, "Eco1.User"},
      {Eco2_User, "Eco2.User"},
      {Edge_Cuts, "Edge.Cuts"},
      {Margin, "Margin"},
      {B_CrtYd, "B.CrtYd"},
      {F_CrtYd, "F.CrtYd"},
      {B_Fab, "B.Fab"},
      {F_Fab, "F.Fab"},
    };

  }

#endif // !LC2KICAD_CONSTS
