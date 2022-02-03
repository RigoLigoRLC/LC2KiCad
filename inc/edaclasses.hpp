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

#pragma once

  #include <vector>
  #include <list>
  #include <fstream>
  #include <string>
  #include <memory>
  #include <map>

  #include "includes.hpp"
  #include "consts.hpp"
  #include "rapidjson.hpp"
  
  using std::list;
  using std::vector;
  using std::fstream;
  using std::string;
  using std::shared_ptr;
  using std::map;

  /**
   * This section is dedicated for class definitions of EDA documents.
   * 
   * Path to the actual file should be specified before thrown into serializer.
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

    typedef std::pair<unsigned int, string> PCBNet;

    class PCBNetManager
    {
      private:
        map<unsigned int, string> netNameCodeMap;
      public:
        unsigned int obtainNetCode(string &netName); // Get netcode if present, or else would create new one.
        void setNet(string& netName, PCBNet &net);
        bool findNet(string &netName); // Return true if a net is present, vice-versa.
        string outputPCBNetInfo(); // For deserializer calls.
        PCBNetManager();
    };

    /*
     * EasyEDA currently uses "ordering" for fill priority. That means if you have two fill areas
     * of different nets, overlapping with each other, the one on the higher position of EasyEDA
     * fill manager gets filled last (with highest priority), like how paper overlaps.
     * This works completely different from KiCad; KiCad has a priority number for fill areas; an
     * area with higher priority gets filled last, and cull out its shape on already filled areas.
     *
     * This class will keep track of EasyEDA fill "order" number, and MUST NOT be used before the
     * entire document gets resolved by Core, since the maximumPriority can change at any time.
     */
    class PCBFloodFillPriorityManager
    {
      private:
        unsigned int maximumPriority = 0; //< The greatest EasyEDA fill order number
      public:
        void logPriority(unsigned int); //< Use this when you need to add a fill to the beloging document
        unsigned int getKiCadPriority(unsigned int); //< Use this when obtaining KiCad priority on output
    };

    struct PCBNetClass
    {
      string name;
      stringlist netClassMembers;
      str_dbl_map rules;
      // We do not hard code what rules we'll have in the structure;
      // we leave it for the parser implementation to determine.
    };

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
      PCBNetManager netManager;
      PCBFloodFillPriorityManager fillPriorityManager;
      vector<PCBNetClass> netClasses;
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
      string id;

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
    enum class PCBTextTypes : int { StandardText = 0, PackageValue = 1, PackageReference = 2, PackageName = 3 };

    struct PCBElement : public EDAElement { };

    struct PCB_Module : public PCBElement
    {
      vector<EDAElement*> containedElements;
      coordinates moduleCoords;
      double orientation;
      bool topLayer; // TODO: DEPRECATE THIS.
      time_t updateTime;
      KiCadLayerIndex layer;
      map<string, string> cparaContent;
      string reference, name, uuid;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    /**
     * PADs as said. Soldering pads.
     * Orphaned PADs on PCBs would be converted as a nested PAD in MODULE, then placed onto PCB.
     */
    struct PCB_Pad : public PCBElement
    {
      PCBPadShape padShape;
      PCBPadType padType;
      PCBHoleShape holeShape;
      double orientation;
      coordinates padCoordinates;
      sizeXY padSize, holeSize;
      string pinNumber;
      PCBNet net;
      coordslist shapePolygonPoints;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    //TRACKs on non copper layers.
    struct PCB_GraphicalTrack : public PCBElement
    {
      enum KiCadLayerIndex layerKiCad;
      double width;
      coordslist trackPoints;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    //TRACKs on copper layers.
    struct PCB_CopperTrack : public PCB_GraphicalTrack
    {
      PCBNet net;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    //HOLEs (non-plated through-holes) in LCEDA.
    struct PCB_Hole : public PCBElement
    {
      coordinates holeCoordinates;
      double holeDiameter;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    /**
     * VIAs in LCEDA.
     * Fixme: LCEDA don't support blind or buried vias now but we need support.
     *        No known implementation, so not adding it for now.
     */
    struct PCB_Via : public PCB_Hole
    {
      PCBNet net;
      double viaDiameter; //The outer diameter of the copper ring
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };
    
    struct PCB_GraphicalSolidRegion : public PCBElement
    {
      coordslist fillAreaPolygonPoints;
      enum KiCadLayerIndex layerKiCad;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    /**
     * SOLIDREGIONs on PCBs.
     * Note: for PCBs, solid regions will be converted into solid flood fills if no compatibility switches
     *       are set, with a warning message prompted.
     */
    struct PCB_CopperSolidRegion : public PCB_GraphicalSolidRegion
    {
      PCBNet net;
      enum KiCadLayerIndex layerKiCad;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    /**
     * COPPERAREAs in LCEDA.
     * Several features were not implemented or supported by KiCad:
     * Grid fill style, improve fabrication, board border clearance.
     */
    struct PCB_FloodFill : public PCB_CopperSolidRegion
    {
      floodFillStyle fillStyle;
      double spokeWidth, clearanceWidth, minimumWidth;
      bool isPreservingIslands, isSpokeConnection;
      int EasyEDAPriority;
      PCBNet net;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    struct PCB_KeepoutRegion : public PCB_GraphicalSolidRegion
    {
      bool allowRouting, allowVias, allowFloodFill;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };
        
    //CIRCLEs on non copper layers.
    struct PCB_GraphicalCircle : public PCBElement
    {
      coordinates center;
      enum KiCadLayerIndex layerKiCad;
      double width, radius;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    //CIRCLEs on copper layers.
    struct PCB_CopperCircle : public PCB_GraphicalCircle
    {
      PCBNet net;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    /**
     * NOTE: RECTs on footprints are hollow, and on PCBs are solid regions.
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
    };

    //ARCs on non copper layers. Derived from PCB_Arc.
    struct PCB_GraphicalArc : public PCBElement
    {
      coordinates center, endPoint;
      //For default, use right direction as 0 deg point. Use degrees not radians.
      double angle, width;
      enum KiCadLayerIndex layerKiCad;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    //ARCs on copper layers.
    struct PCB_CopperArc : public PCB_GraphicalArc
    {
      PCBNet net;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    //TEXTs on PCBs.
    struct PCB_Text : public PCBElement
    {
      string text;
      coordinates midLeftPos; // EasyEDA takes text bottom-left corner for its anchor point
      bool mirrored;
      double height, orientation, width;
      enum PCBTextTypes type;
      enum KiCadLayerIndex layerKiCad;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    //PROTRACTORs.

    //SHEETs.
    
    /**
     * Schematic elements part
     */
    enum class SchematicRotations : int { Deg0 = 0, Deg90 = 1, Deg180 = 2, Deg270 = 3 };
    enum class SchPinElectricProperty : int { Unspecified = 0, Input = 1, Output = 2, Bidirectional = 3, Power = 4 };

    struct Schematic_Element : public EDAElement { } ;

    struct Schematic_Module : public Schematic_Element
    {
      vector<Schematic_Element*> containedElements;
      coordinates moduleCoords;
      double orientation;
      int subpart = -1;
      string reference, value, uuid, name;
      map<string, string> cparaContent;
      time_t updateTime;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };
    
    struct Schematic_Pin : public Schematic_Element
    {
      string pinName, pinNumber;
      int pinLength;
      int fontSize; //Font size is a fixed-point number, divided by 10 before use
      bool inverted, clock; //In EasyEDA a pin has a property "Dot" which means "Inverted" in KiCad
      SchematicRotations pinRotation;
      SchPinElectricProperty electricProperty;
      /**
       * Coordinates should be inverted in the serialization process:
       * EasyEDA use up and right as positive, while KiCad use down and left.
       */
      coordinates pinCoord;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };
    
    struct Schematic_Polyline : public Schematic_Element
    {
      vector<coordinates> polylinePoints;
      bool isFilled; //Fill color is not supported, but if EasyEDA document has a non-white fill color, then fill it
      int lineWidth;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };
    
    //struct SchematicArc : public SchematicElement
    
    struct Schematic_Text : public Schematic_Element
    {
      string text;
      int fontSize; //Font size is a fixed-point number, divide by 10 before use
      bool italic, bold;
      coordinates position; //Text coordinate defined as the bottom left corner (when 0 deg rotation)
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };
    
    //Schematic rectangle. KiCad doesn't support round corner rectangles.
    struct Schematic_Rect : public Schematic_Element
    {
      coordinates position;
      sizeXY size;
      int width;
      bool isFilled;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };
    
    struct Schematic_Polygon : public Schematic_Polyline
    {
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };

    struct Schematic_Arc : public Schematic_Element
    {
      coordinates center, startPoint, endPoint;
      sizeXY size;
      double startAngle, endAngle;
      int width;
      bool isFilled,
           elliptical; // KiCad doesn't support elliptical arcs, those would require linearization
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };
    
    struct Schematic_Image : public Schematic_Element
    {
      coordinates position;
      string content;
      bool isBase64Image;
      string* deserializeSelf(KiCad_5_Deserializer&) const;
    };
  }
