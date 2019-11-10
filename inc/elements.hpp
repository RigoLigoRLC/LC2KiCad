#ifndef LC2KICAD_ELEMENTS
  #define LC2KICAD_ELEMENTS

  #include <consts.hpp>
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

    class PCB_Pad : public PCBElements
    {
      private:
        int  padShape, holeShape, padType, orientation;
        coordinates padCoordinates;
        sizeXY padSize, holeSize;
      public:
        vector<coordinates> shapePolygonPoints;
        void setPadCoordinate(float X, float Y) { padCoordinates.X = X; padCoordinates.Y = Y; };
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
        int getHoleShape() { return holeShape; };
    };
  }
#endif