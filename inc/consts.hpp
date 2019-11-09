#ifndef LC2KICAD_CONSTS
  #define LC2KICAD_CONSTS

  namespace lc2kicad
  {
    #define READ_BUFFER_SIZE 65536
    #define SOFTWARE_VERSION "version 0.1-b"

    #define ERROR_EXIT
  //#define ERROR_ABORT

    extern const char *documentTypeName[8];
    extern const int layerMapperLUT[];
    extern const char *layerNameLUT[];
  }

#endif // !LC2KICAD_CONSTS