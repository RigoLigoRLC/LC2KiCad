#ifndef LC2KICAD_ELEMENTS
  #define LC2KICAD_ELEMENTS

  #include <consts.hpp>
  namespace lc2kicad
  {
    class PCBElements
    {
      private:
        int LCEDA_Layer;

      public:
        void setLCLayer(int LCLayer) {LCEDA_Layer = LCLayer;};
    };
  }
#endif