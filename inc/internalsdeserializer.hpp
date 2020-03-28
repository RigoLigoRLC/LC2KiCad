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

        virtual std::string* outputFileHeader();
        virtual std::string* outputFileEnding();
  
        virtual std::string* outputModule(const PCB_Module&);
        virtual std::string* outputPad(const PCB_Pad&) const;
        virtual std::string* outputVia(const PCB_Via&) const;
        virtual std::string* outputGraphicalTrack(const PCB_GraphicalTrack&) const;
        virtual std::string* outputCopperTrack(const PCB_CopperTrack&) const;
        virtual std::string* outputHole(const PCB_Hole&) const;
        virtual std::string* outputSolidRegion(const PCB_SolidRegion&) const;
        virtual std::string* outputFloodFill(const PCB_FloodFill&) const;
        virtual std::string* outputGraphicalCircle(const PCB_GraphicalCircle&) const;
        virtual std::string* outputCopperCircle(const PCB_CopperCircle&) const;
        virtual std::string* outputGraphicalArc(const PCB_GraphicalArc&) const;
        virtual std::string* outputCopperArc(const PCB_CopperArc&) const;
        virtual std::string* outputRect(const PCB_Rect&) const;

      private:
        EDADocument *workingDocument = nullptr;
        str_dbl_map internalCompatibilitySwitches; //3-Character version copy of compatibility switches.
        std::string indent;
    };
  }
#endif
