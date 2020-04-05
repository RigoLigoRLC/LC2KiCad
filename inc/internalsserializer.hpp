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
  #include "includes.hpp"
  #include "edaclasses.hpp"

  using namespace lc2kicad;

  namespace lc2kicad
  {
    class LCJSONSerializer
    {
      public:
        void setCompatibilitySwitches(const str_dbl_map&);
        void initWorkingDocument(EDADocument*);
        void deinitWorkingDocument();
        
        virtual void parsePCBLibDocument();

        virtual void parsePadString(const std::string&) const;
        virtual void parseHoleString(const std::string&) const;
        virtual void parseViaString(const std::string&) const;
        virtual void parseCopperTrackString(const std::string&) const;
        virtual void parseGraphicalTrackString(const std::string&) const;
        virtual void parseFloodFillString(const std::string&) const;
        virtual void parseCopperCircleString(const std::string&) const;
        virtual void parseGraphicalCircleString(const std::string&) const;
        virtual void parseRectString(const std::string&) const;

        bool judgeIsOnCopperLayer(const int layerKiCad) const;
      private:
        str_dbl_map internalCompatibilitySwitches;
        EDADocument *workingDocument = nullptr;
    };
  }

#endif
