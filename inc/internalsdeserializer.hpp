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

        std::string* outputPCBNetclassRules(const vector<PCBNetClass>&);
  
        std::string* outputPCBModule(const PCB_Module&);
        std::string* outputPCBPad(const PCB_Pad&) const;
        std::string* outputPCBVia(const PCB_Via&) const;
        std::string* outputPCBGraphicalTrack(const PCB_GraphicalTrack&) const;
        std::string* outputPCBCopperTrack(const PCB_CopperTrack&) const;
        std::string* outputPCBHole(const PCB_Hole&) const;
        std::string* outputPCBCopperSolidRegion(const PCB_CopperSolidRegion&) const;
        std::string* outputPCBGraphicalSolidRegion(const PCB_GraphicalSolidRegion&) const;
        std::string* outputPCBKeepoutRegion(const PCB_KeepoutRegion& target) const;
        std::string* outputPCBFloodFill(const PCB_FloodFill&) const;
        std::string* outputPCBGraphicalCircle(const PCB_GraphicalCircle&) const;
        std::string* outputPCBCopperCircle(const PCB_CopperCircle&) const;
        std::string* outputPCBGraphicalArc(const PCB_GraphicalArc&) const;
        std::string* outputPCBCopperArc(const PCB_CopperArc&) const;
        std::string* outputPCBRect(const PCB_Rect&) const;
        std::string* outputPCBText(const PCB_Text&) const;
        
        std::string* outputSchModule(const Schematic_Module& target);
        std::string* outputSchPin(const Schematic_Pin&) const;
        std::string* outputSchPolyline(const Schematic_Polyline&) const;
        std::string* outputSchText(const Schematic_Text&) const;
        std::string* outputSchRect(const Schematic_Rect&) const;
        std::string* outputSchPolygon(const Schematic_Polygon&) const;
        std::string* outputSchArc(const Schematic_Arc&) const;
        /*
        std::string* outputSchImage(const Schematic_Image) const;
        */

      private:
        EDADocument *workingDocument = nullptr;
        str_dbl_map internalCompatibilitySwitches; //3-Character version copy of compatibility switches.
        std::string indent;
        inline bool isProcessingModules() const { return workingDocument->module | processingModule; };
        bool        processingModule, currentPackageOnTopLayer;
    };
  }
#endif
