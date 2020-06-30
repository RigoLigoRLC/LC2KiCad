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
        
        virtual void parseSchLibDocument() const;
        virtual void parsePCBLibDocument();

        virtual void parseSchLibComponent(std::vector<std::string>&, EDADocument&) const;
        virtual void parsePCBLibComponent(std::vector<std::string>&, EDADocument&) const;

        PCB_Pad* parsePCBPadString(const std::string&) const;
        PCB_Hole* parsePCBHoleString(const std::string&) const;
        PCB_Via* parsePCBViaString(const std::string&) const;
        PCB_CopperTrack* parsePCBCopperTrackString(const std::string&) const;
        PCB_GraphicalTrack* parsePCBGraphicalTrackString(const std::string&) const;
        PCB_FloodFill* parsePCBFloodFillString(const std::string&) const;
        PCB_CopperCircle* parsePCBCopperCircleString(const std::string&) const;
        PCB_GraphicalCircle* parsePCBGraphicalCircleString(const std::string&) const;
        PCB_Rect* parsePCBRectString(const std::string&) const;
        
        
        Schematic_Pin* parseSchPin(const std::string&) const;
        Schematic_Polyline* parseSchPolyline(const std::string&) const;
        Schematic_Polygon* parseSchPolygon(const std::string&) const;
        Schematic_Text* parseSchText(const std::string&) const;
        Schematic_Rect* parseSchRect(const std::string&) const;
        /*
        void parseSchImage(const std::string&) const;
        */
        
        bool judgeIsOnCopperLayer(const int layerKiCad) const;
      private:
        str_dbl_map internalCompatibilitySwitches;
        EDADocument *workingDocument = nullptr;
        double schematic_unit_coefficient;
    };
  }

#endif
