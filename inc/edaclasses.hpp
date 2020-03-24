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
  
#ifndef LC2KICAD_ELEMENTS
  #define LC2KICAD_ELEMENTS

  #include <vector>
  #include <fstream>
  #include <string>

  #include "includes.hpp"
  #include "consts.hpp"
  #include "rapidjson.hpp"
  
  using std::vector;
  using std::fstream;
  using std::string;

  /**
   * This section is dedicated for class definitions of EDA documents.
   * 
   * Path to the actual file should be specified before thrown into serialibuglist.cgi?quicksearch=plasma notezer.
   * Serializer will have to determine the file type and use the corresponding parser.
   * At the time of development, EasyEDA is still using its old format, but it would
   * change its format in about a year, so it's important to make the serializer a
   * modular design in order to maintain backwards compatibility while not having too
   * much hassle in the processing code.
   * 
   * Objects derived from EDADocument should be fed into the deserializer to get it
   * converted and inserted into the output stream given by the invoking function.
   */
  namespace lc2kicad
  {
    struct EDADocument
    {
      string pathToFile;
      bool module; // When is true, means only convert the first element contained and output as a module.

      string* parseSelf();
      string* parseSelf(str_dbl_pair deserializerSwitch);
      //string* parseSelf(EDAInternalDeserializer& deserializer)
    };

    struct PCBDocument : public EDADocument
    {
      std::vector<PCBElements*> containedElements;
    };
  }

  /**
   * This section is dedicated for elements on the PCBs and footprints.
   */
  namespace lc2kicad
  {
    /**
     * LCEDA flood fill (COPPERAREA) fill style. KiCad does not support grid fill yet,
     * thus a warning should be given to the user.
     */
    enum class floodFillStyle : int { noFill = 1, solidFill = 2, gridFill = 3 };
    enum class PCBPadShape : int { circle = 0, oval = 1, rectangle = 2, polygon = 3 };
    enum class PCBPadType : int { top = 0, bottom = 1, through = 2, noplating = 3 };
    enum class PCBHoleShape : int { circle = 0, slot = 1 };

    struct PCBElements
    {
        bool isVisible;

        bool getVisibility() { return isVisible; };
        void setVisibility(bool visibility) { isVisible = visibility; };
        virtual string* parseSelf() = 0;
        virtual string* parseSelf(str_dbl_pair& deserializerSwitch)
        {//TODO: Split deserializer
          //if (deserializerSwitch.first == "")
            //parseSelf(currentLatestDeserializer)
        }
        //string* parseSelf(EDAInternalDeserializer& deserializer) const = 0;
        //Haven't decided to use abstract data type.
    };

    /**
     * PADs as said. Soldering pads.
     * Fixme: Orphaned PADs on PCBs should be converted inside a MODULE, then placed onto PCB.
     */
    struct PCB_Pad : public PCBElements
    {
      PCBPadShape padShape;
      PCBPadType padType;
      PCBHoleShape holeShape;
      double orientation;
      coordinates padCoordinates;
      sizeXY padSize, holeSize;
      string netName, pinNumber;
      coordslist shapePolygonPoints;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

    //TRACKs on non copper layers.
    struct PCB_GraphicalTrack : public PCBElements
    {
      int layerKiCad;
      double width;
      coordslist trackPoints;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

    //TRACKs on copper layers.
    struct PCB_CopperTrack : public PCB_GraphicalTrack
    {
      string netName;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

    //HOLEs (non-plated through-holes) in LCEDA.
    struct PCB_Hole : public PCBElements
    {
      coordinates holeCoordinates;
      double holeDiameter;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

    /**
     * VIAs in LCEDA.
     * Fixme: LCEDA don't support blind or buried vias now but we need support.
     *        No known implementation, so not adding it for now.
     */
    struct PCB_Via : public PCB_Hole
    {
      coordinates viaCoordinates;
      string netName;
      double viaDiameter; //The outer diameter of the copper ring
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

    /**
     * SOLIDREGIONs on PCBs.
     * Note: for PCBs, solid regions will be converted into solid flood fills if no compatibility switches
     *       are set, with a warning message prompted.
     */
    struct PCB_SolidRegion : public PCBElements
    {
      coordslist fillAreaPolygonPoints;
      string netName;
      int layerKiCad;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

    /**
     * COPPERAREAs in LCEDA.
     * Several features were not implemented or supported by KiCad:
     * Grid fill style, improve fabrication, board border clearance.
     */
    struct PCB_FloodFill : public PCB_SolidRegion
    {
      floodFillStyle fillStyle;
      double spokeWidth, clearanceWidth;
      bool isPreservingIslands, isSpokeConnection;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };
        
    //CIRCLEs on non copper layers.
    struct PCB_GraphicalCircle : public PCBElements
    {
      coordinates center;
      int layerKiCad;
      double width, radius;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

    //CIRCLEs on copper layers.
    struct PCB_CopperCircle : public PCB_GraphicalCircle
    {
      string netName;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

    /**
     * RECTs on footprints are hollow, and on PCBs are solid regions.
     * We decided to treat this specific object differently. Because its behavior doesn't differ
     * when it's on different layers, but rather in different files.
     * 
     * Ahhh, EasyEDA traditions. I h8 it. -Rigo
     */
    struct PCB_Rect : public PCBElements
    {
      coordinates topLeftPos;
      sizeXY size;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

    //ARCs on non copper layers. Derived from PCB_Arc.
    struct PCB_GraphicalArc : public PCBElements
    {
      coordinates center;
      //For default, use right deirection as 0 deg point. Use degrees not radians.
      double beginAngle, endAngle, width;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

    //ARCs on copper layers.
    struct PCB_CopperArc : public PCB_GraphicalArc
    {
      string netName;
      //string* parseSelf(EDAInternalDeserializer& deserializer); const;
    };

  }
#endif
