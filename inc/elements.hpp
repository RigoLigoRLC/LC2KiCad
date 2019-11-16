/*
    Copyright (c) 2020 Harrison Wade, aka "RigoLigo RLC"

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

  #include <include.hpp>
  #include <consts.hpp>
  #include <rapidjson.hpp>

  using std::vector;
  using std::fstream;
  using std::string;
  //using rapidjson::Value;

  namespace lc2kicad
  {
    struct coordinates
    {
      float X;
      float Y;
    };
    typedef coordinates sizeXY;

    class PCBElements
    {
      private:
        bool isVisible;
      public:
        bool getVisibility() { return isVisible; };
        bool setVisibility(bool visibility) { isVisible = visibility; };
        int  outputToKiCadFile(fstream &kicadFile);
    };

    enum class PCBPadShape : int { circle = 1, oval = 2, rectangle = 3, polygon = 4 };
    enum class PCBPadType : int { top = 1, bottom = 2, through = 3, noplating = 4 };
    enum class PCBHoleShape : int { circle = 1, slot = 2 };

    class PCB_Pad : public PCBElements
    {
      private:
        PCBPadShape padShape;
        PCBPadType padType;
        PCBHoleShape holeShape;
        int orientation;
        coordinates padCoordinates;
        sizeXY padSize, holeSize;
        string netName;
      public:
        PCB_Pad();
        PCB_Pad(vector<string> &paramList, coordinates origin);
        ~PCB_Pad();
        vector<coordinates> shapePolygonPoints;
        /*void setPadCoordinate(float X, float Y) { padCoordinates.X = X; padCoordinates.Y = Y; };
          void setPadSize(float X, float Y) { padSize.X = X; padSize.Y = Y; };
          void setHoleSize(float X, float Y) { holeSize.X = X; holeSize.Y = Y; };
          void setPadRotation(float rotation) { orientation = rotation; };
          void setPadShape(int shape) { padShape = shape; };
          void setPadType(int type) { padType = type; };
          void setHoleShape(int shape) { holeShape = shape; };

          const coordinates &getPadCoordinate() { return padCoordinates; };
          const sizeXY &setPadSize() { return padSize; };
          const sizeXY &setHoleSize() { return holeSize; };
          float getPadRotation() { return orientation; };
          int getPadShape() { return padShape; };
          int getPadType() { return padType; };
          int getHoleShape() { return holeShape; };*/
    };

    class PCB_Track : public PCBElements
    {
      private:
        int  width, layerKiCad;
        vector<coordinates> trackPoints;
        string netName;
      public:
        PCB_Track();
        PCB_Track(vector<string> &paramList);
        ~PCB_Track();
    };

    class PCB_Via : public PCBElements
    {
      private:
        int diameter, drillDiameter;
        coordinates viaCoordinates;
        string netName;
      public:
        PCB_Via();
        PCB_Via(vector<string> &paramList);
        ~PCB_Via();
    };
  }
#endif