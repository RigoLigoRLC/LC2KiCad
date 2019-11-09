#include <include.hpp>


#ifndef lc2kicad_

  #define lc2kicad_

  namespace lc2kicad
  {
    //Definitions here.
    #define ENABLE_EXCEPTION_TESTER

    //Function Prototypes here.
    void displayUsage();
    void displayAbout();
    void errorAndAbort(std::runtime_error*);
    void errorAndQuit(std::runtime_error*);
    void parseDocumentList(int fileCount, char *args[]);
    void parseDocuments(int fileCount, char* args[]);
    void parseDocument(char *filePath, char* bufferField);

    //Variables for global uses here.

  }

#endif