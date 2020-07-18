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

#pragma once

  #include <vector>
  #include <fstream>
  #include <string>
  #include <memory>
  #include <map>

  #include "includes.hpp"
  #include "consts.hpp"
  #include "rapidjson.hpp"
  
  using std::vector;
  using std::fstream;
  using std::string;
  using std::shared_ptr;
  using std::map;

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
    class LC2KiCadCore;
    class KiCad_5_Deserializer;

    struct EDAElement;
    struct PCBElement;
    //Referencing each other, must declare one first.

    struct EDADocument
    {
      string pathToFile;
      bool module; // When is true, means only convert the first element contained and output as a module.
      shared_ptr<rapidjson::Document> jsonParseResult; // For convenience. This is only one pointer and isn't gonna take much RAM
      str_str_map docInfo; // Due to compatibility concerns, use a map to store temporary info for use
      documentTypes docType;
      coordinates origin {0, 0};
      double gridSize;

      std::vector<EDAElement*> containedElements;

      LC2KiCadCore *parent = nullptr;
      
      virtual void addElement(EDAElement*);
      virtual string* deserializeSelf() const ;
      //virtual string* deserializeSelf(str_dbl_pair deserializerSwitch);
      //string* deserializeSelf(KiCad_5_Deserializer&)

      EDADocument();
      EDADocument(const bool useJSONStorage);
      virtual ~EDADocument();
    };

    struct PCBDocument : public EDADocument
    {
      PCBDocument(const EDADocument&);
      void addElement(EDAElement*) override;
      ~PCBDocument();
    };
    
    struct SchematicDocument : public EDADocument
    {
      SchematicDocument(const EDADocument&);
      //void addElement(const EDAElement*) override;
      ~SchematicDocument();
    };
    
    struct EDAElement
    {
      bool visibility = true, locked = false;
      EDADocument *parent = nullptr;
      string id;

      virtual string* deserializeSelf() const = 0;
      virtual string* deserializeSelf(KiCad_5_Deserializer&) const = 0;
      
      virtual ~EDAElement();
    };
    /**
     * This section is dedicated for elements on the PCBs and footprints.
     */
    /**
     * LCEDA flood fill (COPPERAREA) fill style. KiCad does not support grid fill yet,
     * thus a warning should be given to the user.
     */
    enum class floodFillStyle : int { noFill = 1, solidFill = 2, gridFill = 3 };
    enum class PCBPadShape : int { circle = 0, oval = 1, rectangle = 2, polygon = 3 };
    enum class PCBPadType : int { top = 0, bottom = 1, through = 2, noplating = 3 };
    enum class PCBHoleShape : int { circle = 0, slot = 1 };

    struct PCBElement : public EDAElement { };

    struct PCB_Module : public PCBElement
    {
      vector<PCBElement*> containedElements;
      coordinates moduleCoords;
      double orientation;
      bool topLayer = true;
      time_t updateTime;
      KiCadLayerIndex layer;
      map<string, string> cparaContent;
      string reference, name, uuid;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    /**
     * PADs as said. Soldering pads.
     * Fixme: Orphaned PADs on PCBs should be converted as a nested PAD in MODULE, then placed onto PCB.
     */
    struct PCB_Pad : public PCBElement
    {
      PCBPadShape padShape;
      PCBPadType padType;
      PCBHoleShape holeShape;
      double orientation;
      coordinates padCoordinates;
      sizeXY padSize, holeSize;
      string netName, pinNumber;
      coordslist shapePolygonPoints;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    //TRACKs on non copper layers.
    struct PCB_GraphicalTrack : public PCBElement
    {
      enum KiCadLayerIndex layerKiCad;
      double width;
      coordslist trackPoints;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    //TRACKs on copper layers.
    struct PCB_CopperTrack : public PCB_GraphicalTrack
    {
      string netName;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    //HOLEs (non-plated through-holes) in LCEDA.
    struct PCB_Hole : public PCBElement
    {
      coordinates holeCoordinates;
      double holeDiameter;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    /**
     * VIAs in LCEDA.
     * Fixme: LCEDA don't support blind or buried vias now but we need support.
     *        No known implementation, so not adding it for now.
     */
    struct PCB_Via : public PCB_Hole
    {
      string netName;
      double viaDiameter; //The outer diameter of the copper ring
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    /**
     * SOLIDREGIONs on PCBs.
     * Note: for PCBs, solid regions will be converted into solid flood fills if no compatibility switches
     *       are set, with a warning message prompted.
     */
    struct PCB_SolidRegion : public PCBElement
    {
      coordslist fillAreaPolygonPoints;
      string netName;
      enum KiCadLayerIndex layerKiCad;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    /**
     * COPPERAREAs in LCEDA.
     * Several features were not implemented or supported by KiCad:
     * Grid fill style, improve fabrication, board border clearance.
     */
    struct PCB_FloodFill : public PCB_SolidRegion
    {
      floodFillStyle fillStyle;
      enum KiCadLayerIndex layerKiCad;
      double spokeWidth, clearanceWidth;
      bool isPreservingIslands, isSpokeConnection;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };
        
    //CIRCLEs on non copper layers.
    struct PCB_GraphicalCircle : public PCBElement
    {
      coordinates center;
      enum KiCadLayerIndex layerKiCad;
      double width, radius;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    //CIRCLEs on copper layers.
    struct PCB_CopperCircle : public PCB_GraphicalCircle
    {
      string netName;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    /**
     * RECTs on footprints are hollow, and on PCBs are solid regions.
     * We decided to treat this specific object differently. Because its behavior doesn't differ
     * when it's on different layers, but rather in different files.
     * 
     * Ahhh, EasyEDA traditions. I h8 it. -Rigo
     */
    struct PCB_Rect : public PCBElement
    {
      coordinates topLeftPos;
      sizeXY size;
      enum KiCadLayerIndex layerKiCad;
      double strokeWidth;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    //ARCs on non copper layers. Derived from PCB_Arc.
    struct PCB_GraphicalArc : public PCBElement
    {
      coordinates center, endPoint;
      //For default, use right deirection as 0 deg point. Use degrees not radians.
      double angle, width;
      enum KiCadLayerIndex layerKiCad;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    //ARCs on copper layers.
    struct PCB_CopperArc : public PCB_GraphicalArc
    {
      string netName;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };

    //PROTRACTORs.

    //SHEETs.
    
    /**
     * Schematic elements part
     */
    enum class SchematicRotations : int { Deg0 = 0, Deg90 = 1, Deg180 = 2, Deg270 = 3 };

    struct Schematic_Element : public EDAElement { } ;

    struct Schematic_Module : public Schematic_Element
    {
      vector<Schematic_Element*> containedElements;
      double orientation;
      string reference, value;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };
    
    struct Schematic_Pin : public Schematic_Element
    {
      string pinName, pinNumber;
      int pinLength;
      int fontSize; //Font size is a fixed-point number, divided by 10 before use
      bool inverted, clock; //In EasyEDA a pin has a property "Dot" which means "Inverted" in KiCad
      SchematicRotations pinRotation;
      /**
       * Coordinates should be inverted in the serialization process:
       * EasyEDA use up and right as positive, while KiCad use down and left.
       */
      coordinates pinCoord;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };
    
    struct Schematic_Polyline : public Schematic_Element
    {
      vector<coordinates> polylinePoints;
      bool isFilled; //Fill color is not supported, but if EasyEDA document has a non-white fill color, then fill it
      int lineWidth;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };
    
    //struct SchematicArc : public SchematicElement
    
    struct Schematic_Text : public Schematic_Element
    {
      string text;
      int fontSize; //Font size is a fixed-point number, divide by 10 before use
      bool italic, bold;
      coordinates position; //Text coordinate defined as the bottom left corner (when 0 deg rotation)
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };
    
    //Schematic rectangle. KiCad doesn't support round corner rectangles.
    struct Schematic_Rect : public Schematic_Element
    {
      coordinates position;
      sizeXY size;
      int width;
      bool isFilled;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };
    
    struct Schematic_Polygon : public Schematic_Polyline
    {
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };
    
    struct Schematic_Image : public Schematic_Element
    {
      coordinates position;
      string content;
      bool isBase64Image;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
      string* deserializeSelf() const;
    };
  }
