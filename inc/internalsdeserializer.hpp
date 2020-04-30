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

#ifndef INTN_DESERIALIZER_HPP_
  #define INTN_DESERIALIZER_HPP_

  #include <string>
  #include "includes.hpp"
  #include "edaclasses.hpp"

  using namespace lc2kicad;
  
  namespace lc2kicad
  {
    class KiCad_5_Deserializer
    {
      public:
        void initWorkingDocument(EDADocument* targetDoc);
        void deinitWorkingDocument();
        void setCompatibilitySwitches(const str_dbl_map &_compatibSw);
        
        virtual ~KiCad_5_Deserializer();

        virtual std::string* outputFileHeader();
        virtual std::string* outputFileEnding();
  
        virtual std::string* outputPCBModule(const PCB_Module&);
        virtual std::string* outputPCBPad(const PCB_Pad&) const;
        virtual std::string* outputPCBVia(const PCB_Via&) const;
        virtual std::string* outputPCBGraphicalTrack(const PCB_GraphicalTrack&) const;
        virtual std::string* outputPCBCopperTrack(const PCB_CopperTrack&) const;
        virtual std::string* outputPCBHole(const PCB_Hole&) const;
        virtual std::string* outputPCBSolidRegion(const PCB_SolidRegion&) const;
        virtual std::string* outputPCBFloodFill(const PCB_FloodFill&) const;
        virtual std::string* outputPCBGraphicalCircle(const PCB_GraphicalCircle&) const;
        virtual std::string* outputPCBCopperCircle(const PCB_CopperCircle&) const;
        virtual std::string* outputPCBGraphicalArc(const PCB_GraphicalArc&) const;
        virtual std::string* outputPCBCopperArc(const PCB_CopperArc&) const;
        virtual std::string* outputPCBRect(const PCB_Rect&) const;
        
        /*
        virtual std::string* outputSchPin(const Schematic_Pin&) const;
        virtual std::string* outputSchPolyline(const Schematic_Polyline&) const;
        virtual std::string* outputSchText(const Schematic_Text&) const;
        virtual std::string* outputSchRect(const Schematic_Rect&) const;
        virtual std::string* outputSchPolygon(const Schematic_Polygon&) const;
        virtual std::string* outputSchImage(const Schematic_Image) const;
        */

      private:
        EDADocument *workingDocument = nullptr;
        str_dbl_map internalCompatibilitySwitches; //3-Character version copy of compatibility switches.
        std::string indent;
    };
  }
#endif
