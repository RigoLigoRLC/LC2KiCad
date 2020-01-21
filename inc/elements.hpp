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

  #include <includes.hpp>
  #include <consts.hpp>
  #include <rapidjson.hpp>

  using std::vector;
  using std::fstream;
  using std::string;
  //using rapidjson::Value;

  namespace lc2kicad
  {

    struct PCBElements
    {

        bool isVisible;

        bool getVisibility() { return isVisible; };
        void setVisibility(bool visibility) { isVisible = visibility; };
        virtual string outputKiCadFormat(string &convArgs, char* &indent) { assertThrow(false, "This is PCBElement base class. This class should not be used in the program."); return "";};
        //string outputKiCadFormat();
    };

    enum class PCBPadShape : int { circle = 0, oval = 1, rectangle = 2, polygon = 3 };
    enum class PCBPadType : int { top = 0, bottom = 1, through = 2, noplating = 3 };
    enum class PCBHoleShape : int { circle = 0, slot = 1 };

    struct PCB_Pad : public PCBElements
    {
      PCBPadShape padShape;
      PCBPadType padType;
      PCBHoleShape holeShape;
      int orientation;
      coordinates padCoordinates;
      sizeXY padSize, holeSize;
      string netName, pinNumber;

      string outputKiCadFormat(string &convArgs, char* &indent);
      coordslist shapePolygonPoints;
      /*void setPadCoordinate(double X, double Y) { padCoordinates.X = X; padCoordinates.Y = Y; };
          void setPadSize(double X, double Y) { padSize.X = X; padSize.Y = Y; };
          void setHoleSize(double X, double Y) { holeSize.X = X; holeSize.Y = Y; };
          void setPadRotation(double rotation) { orientation = rotation; };
          void setPadShape(int shape) { padShape = shape; };
          void setPadType(int type) { padType = type; };
          void setHoleShape(int shape) { holeShape = shape; };

          const coordinates &getPadCoordinate() { return padCoordinates; };
          const sizeXY &setPadSize() { return padSize; };
          const sizeXY &setHoleSize() { return holeSize; };
          double getPadRotation() { return orientation; };
          int getPadShape() { return padShape; };
          int getPadType() { return padType; };
          int getHoleShape() { return holeShape; };*/
    };

    struct PCB_Track : public PCBElements
    {
      int layerKiCad;
      double width;
      coordslist trackPoints;
      string netName;

      string outputKiCadFormat(string &convArgs, char* &indent);
    };

    struct PCB_GraphicalLine : public PCB_Track
    {
      string outputKiCadFormat(string &convArgs, char* &indent);
    };

    struct PCB_Via : public PCBElements
    {
      int viaSize, drillSize;
      coordinates viaCoordinates;
      string netName;

      string outputKiCadFormat(string &convArgs, char* &indent);
    };
  }
#endif
