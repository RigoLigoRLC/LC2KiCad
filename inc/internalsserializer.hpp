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

#ifndef LC2KICAD_LCSTRINGPARSER
  #define LC2KICAD_LCSTRINGPARSER

  #include <string>
  #include "includes.hpp"
  #include "rapidjson.hpp"
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
        
        virtual void parseSchLibDocument();
        virtual void parsePCBDocument();
        virtual void parsePCBLibDocument();
        virtual list<EDADocument *> parseSchNestedLibs();
        virtual list<EDADocument *> parsePCBNestedLibs();

        virtual void parseSchLibComponent(std::vector<std::string>&, vector<Schematic_Element*> &containedElements);
        virtual void parsePCBLibComponent(std::vector<std::string>&, vector<EDAElement*> &containedElements);

        virtual void parsePCBDRCRules(rapidjson::Value &drcRules);

        void parseCommonDoucmentStructure(rapidjson::Document &parseTarget,
                                          std::vector<std::string> &canvasPropertyList,
                                          rapidjson::Value &shapesArray,
                                          rapidjson::Value &headObject);

        PCB_Pad* parsePCBPadString(const std::string&);
        PCB_Module* parsePCBDiscretePadString(const std::string&);
        PCB_Hole* parsePCBHoleString(const std::string&);
        PCB_Via* parsePCBViaString(const std::string&);
        PCB_CopperTrack* parsePCBCopperTrackString(const std::string&);
        PCB_GraphicalTrack* parsePCBGraphicalTrackString(const std::string&);
        PCB_FloodFill* parsePCBFloodFillString(const std::string&);
        PCB_GraphicalSolidRegion* parsePCBGraphicalSolidRegionString(const std::string&);
        PCB_GraphicalTrack* parsePCBNpthRegionString(const std::string&);
        PCB_FloodFill* parsePCBCopperSolidRegionString(const std::string&);
        PCB_FloodFill* parsePCBPlaneZoneString(const std::string&);
        PCB_KeepoutRegion* parsePCBKeepoutRegionString(const std::string&);
        PCB_CopperCircle* parsePCBCopperCircleString(const std::string&);
        PCB_GraphicalCircle* parsePCBGraphicalCircleString(const std::string&);
        PCB_CopperArc* parsePCBCopperArcString(const std::string&);
        PCB_GraphicalArc* parsePCBGraphicalArcString(const std::string&);
        PCB_Rect* parsePCBRectString(const std::string&);
        PCB_Text* parsePCBTextString(const std::string&);
        PCB_Module* parsePCBModuleString(const std::string& LCJSONString, EDADocument* parent = nullptr,
                                         map<string, RAIIC<EDADocument>>* exportedList = nullptr);

        bool judgeIsOnCopperLayer(const KiCadLayerIndex layerKiCad);
        
        Schematic_Pin* parseSchPin(const std::string&) const;
        Schematic_Polyline* parseSchPolyline(const std::string&) const;
        Schematic_Polygon* parseSchPolygon(const std::string&) const;
        Schematic_Text* parseSchText(const std::string&) const;
        Schematic_Rect* parseSchRect(const std::string&) const;
        Schematic_Arc* parseSchArc(const std::string&) const;
        Schematic_Module* parseSchModuleString(const std::string& LCJSONString, EDADocument* parent = nullptr,
                                         map<string, RAIIC<EDADocument>>* exportedList = nullptr);
        /*
        void parseSchImage(const std::string&) const;
        */
        std::map<int, KiCadLayerIndex> EasyEdaToKiCadLayerMap
        {
          {0, Invalid},         // Placeholder.
          {1, F_Cu},
          {2, B_Cu},
          {3, F_SilkS},
          {4, B_SilkS},
          {5, F_Paste},
          {6, B_Paste},
          {7, F_Mask},
          {8, B_Mask},
          {9, Invalid},
          {10, Edge_Cuts},
          {11, Invalid},        // Multilayer, for through holes, No such layer for KiCad
          {12, Cmts_User},
          {13, F_Fab},
          {14, B_Fab},
          {15, Eco1_User},      // Mechanical layer.
          {16, Invalid},        // 3D Model layer.
          {17, Invalid},        // Component outline layer.
          {18, Invalid},        // Pin outline layer.
          {19, Invalid},        // Through hole layer(graphical only).
          {20, Invalid},        // Violation marker layer.
          {21, In1_Cu},
          {22, In2_Cu},
          {23, In3_Cu},
          {24, In4_Cu},
          {25, In5_Cu},
          {26, In6_Cu},
          {27, In7_Cu},
          {28, In8_Cu},
          {29, In9_Cu},
          {30, In10_Cu},
          {31, In11_Cu},
          {32, In12_Cu},
          {33, In13_Cu},
          {34, In14_Cu},
          {35, In15_Cu},
          {36, In16_Cu},
          {37, In17_Cu},
          {38, In18_Cu},
          {39, In19_Cu},
          {40, In20_Cu},
          {41, In21_Cu},
          {42, In22_Cu},
          {43, In23_Cu},
          {44, In24_Cu},
          {45, In25_Cu},
          {46, In26_Cu},
          {47, In27_Cu},
          {48, In28_Cu},
          {49, In29_Cu},
          {50, In30_Cu},
          {99, F_CrtYd}, // Package Shape, equivalent to KiCad Courtyard but needs to take care of flipping
          {100, Invalid}, // Pin Shape
          {101, Invalid}, // Component marking
        };
      private:
        str_dbl_map internalCompatibilitySwitches;
        EDADocument *workingDocument = nullptr;
        double schematic_unit_coefficient;
        bool processingModule, exportNestedLibs;
    };
  }

#endif
