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
  
#ifndef LC2KICAD_CONSTS
  #define LC2KICAD_CONSTS

  namespace lc2kicad
  {
    #define SOFTWARE_VERSION "version alpha-0.1.99"
    static const char *rapidjsonErrorMsg[] = {"No error", "Empty document", "Document root is not singular", "Invalid value", "Object missing its name", "Object missing colon", "Object missing ‘,’ or ‘}’", "Array missing ‘,’ or ‘]’", "Incorrect digits after \\u escape", "Surrogate pair in the string is invalid", "Invalid escape character in the string", "String missing ‘\\”’", "Invalid encoding of the string", "Number is too big", "Number missing its fraction part", "Number missing its exponent part", "Parsing is terminated", "Unspecificd syntax error"};

    static const double tenmils_to_mm_coefficient = 0.254;

    static const char *documentTypeName[8] = {"", "schematics", "schematic library", "PCB", "PCB library", "project", "sub-part", "SPICE symbol"};
    static const char *documentExtensionName[8] = {"", "", "", "", ".kicad_mod", "prj", "", ""};
    //Layer mapper. Input EasyEDA, ouput KiCad.
    static const int LCtoKiCadLayerLUT[] = {-1, 0, 31, 37, 36, 35, 34, 39, 38, -1, 44, -1, 41, 49, 48, -1, -1, -1, -1, -1, -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
    static const char *KiCadLayerNameLUT[] = {"F.Cu", "In1.Cu", "In2.Cu", "In3.Cu", "In4.Cu", "In5.Cu", "In6.Cu", "In7.Cu", "In8.Cu", "In9.Cu","In10.Cu", "In11.Cu", "In12.Cu", "In13.Cu", "In14.Cu", "In15.Cu", "In16.Cu", "In17.Cu", "In18.Cu", "In19.Cu", "In20.Cu", "In21.Cu", "In22.Cu", "In23.Cu", "In24.Cu", "In25.Cu", "In26.Cu", "In27.Cu", "In28.Cu", "In29.Cu", "In30.Cu", "B.Cu", "B.Adhes", "F.Adhes", "B.Paste", "F.Paste", "B.SilkS", "F.SilkS", "B.Mask", "F.Mask", "Dwgs.User", "Cmts.User", "Eco1.User", "Eco2.User", "Edge.Cuts", "Margin", "B.CrtYd", "F.CrtYd", "B.Fab", "F.Fab"};
    static const char *padTypeKiCad[] = {"smd", "smd", "thru_hole", "np_thru_hole"};
    static const char *padShapeKiCad[] = {"circle", "oval", "rect", "custom"};
  }

#endif // !LC2KICAD_CONSTS
