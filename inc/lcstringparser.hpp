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

#ifndef LC2KICAD_LCSTRINGPARSER
  #define LC2KICAD_LCSTRINGPARSER

  #include <string>
  #include "elements.hpp"

  namespace lc2kicad
  {
    class LCStringParserContainer
    {
      public:
        virtual std::string getParserType() const { return "LC String Parser Container"; };
        virtual PCB_Pad* parsePadString(const string&, const coordinates&) const { return new PCB_Pad(); };
        virtual PCB_Via* parseViaString(const string&, const coordinates&) const { return new PCB_Via(); };
        virtual PCB_Track* parseTrackString(const string&, const coordinates&) const { return new PCB_Track(); };
        virtual PCB_GraphicalLine* parseGraphicalLineString(const string&, const coordinates&) const { return new PCB_GraphicalLine(); };
        virtual PCB_FloodFill* parseFloodFillString(const string&, const coordinates&) const { return new PCB_FloodFill(); };
        virtual PCB_Circle* parseCircleString(const string&, const coordinates&) const { return new PCB_Circle(); };
        virtual PCB_GraphicalCircle* parseGraphicalCircleString(const string&, const coordinates&) const { return new PCB_GraphicalCircle(); };
        virtual bool judgeIsOnCopperLayer(const int layerKiCad) const { return true; };
    };

    class StandardLCStringParser : public LCStringParserContainer
    {
      public:
        std::string getParserType() const;
        PCB_Pad* parsePadString(const string&, const coordinates&) const;
        PCB_Via* parseViaString(const string&, const coordinates&) const;
        PCB_Track* parseTrackString(const string&, const coordinates&) const;
        PCB_GraphicalLine* parseGraphicalLineString(const string&, const coordinates&) const;
        PCB_FloodFill* parseFloodFillString(const string&, const coordinates&) const;
        PCB_Circle* parseCircleString(const string&, const coordinates&) const;
        PCB_GraphicalCircle* parseGraphicalCircleString(const string&, const coordinates&) const;
        bool judgeIsOnCopperLayer(const int layerKiCad) const;
    };
  }

#endif
