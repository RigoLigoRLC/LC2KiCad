/*
    Copyright (c) 2020 RigoLigoRLC, BadLuckW and all other LC2KiCad contributers.

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
    #define READ_BUFFER_SIZE 65536
    #define SOFTWARE_VERSION "version alpha-0.1.2"

    #define ERROR_EXIT
  //#define ERROR_ABORT

    const double tenmils_to_mm_coefficient = 0.254;

    extern const char *documentTypeName[8];
    extern const int LCtoKiCadLayerLUT[];
    extern const char *KiCadLayerNameLUT[];
    extern const char *padTypeKiCad[];
    extern const char *padShapeKiCad[];
    
  }

#endif // !LC2KICAD_CONSTS