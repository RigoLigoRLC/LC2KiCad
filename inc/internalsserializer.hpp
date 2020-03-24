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

#ifndef LC2KICAD_LCSTRINGPARSER
  #define LC2KICAD_LCSTRINGPARSER

  #include <string>
  #include "edaclasses.hpp"

  namespace lc2kicad
  {
    class LCJSONSerializerMk1
    {
      public:
        int setCompatibilitySwitches(const str_dbl_pairlist) const;
        PCB_Pad* parsePadString(const string&, const coordinates&) const;
        PCB_Via* parseViaString(const string&, const coordinates&) const;
        PCB_CopperTrack* parseTrackString(const string&, const coordinates&) const;
        PCB_GraphicalTrack* parseGraphicalLineString(const string&, const coordinates&) const;
        PCB_FloodFill* parseFloodFillString(const string&, const coordinates&) const;
        PCB_CopperCircle* parseCircleString(const string&, const coordinates&) const;
        PCB_GraphicalCircle* parseGraphicalCircleString(const string&, const coordinates&) const;
        bool judgeIsOnCopperLayer(const int layerKiCad) const;
      private:
        str_dbl_pairlist internalCompatibilitySwitches;
    };
  }

#endif
