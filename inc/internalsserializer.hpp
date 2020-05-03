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
        
        virtual ~LCJSONSerializer();
        
        virtual void parsePCBLibDocument();

        virtual void parsePCBPadString(const std::string&) const;
        virtual void parsePCBHoleString(const std::string&) const;
        virtual void parsePCBViaString(const std::string&) const;
        virtual void parsePCBCopperTrackString(const std::string&) const;
        virtual void parsePCBGraphicalTrackString(const std::string&) const;
        virtual void parsePCBFloodFillString(const std::string&) const;
        virtual void parsePCBCopperCircleString(const std::string&) const;
        virtual void parsePCBGraphicalCircleString(const std::string&) const;
        virtual void parsePCBRectString(const std::string&) const;
        
        /*
        virtual void parseSchPin(const std::string&) const;
        virtual void parseSchPolyline(const std::string&) const;
        virtual void parseSchText(const std::string&) const;
        virtual void parseSchRect(const std::string&) const;
        virtual void parseSchPolygon(const std::string&) const;
        virtual void parseSchImage(const std::string&) const;
        */
        
        bool judgeIsOnCopperLayer(const int layerKiCad) const;
      private:
        str_dbl_map internalCompatibilitySwitches;
        EDADocument *workingDocument = nullptr;
    };
  }

#endif
