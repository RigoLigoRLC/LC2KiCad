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

#include <includes.hpp>
#include <rapidjson.hpp>

#ifndef lc2kicad_

  #define lc2kicad_

  namespace lc2kicad
  {
    //Definitions here.
    #define ENABLE_EXCEPTION_TESTER

    //Function Prototypes here.
    void parseDocumentList(int fileCount, char *args[]);
    void parseDocuments(int fileCount, char *args[]);
    void parseDocument(char *filePath, char *bufferField);

    void assertThrow(bool statement, const char *message);

    int  layerMap(std::string &layerString);
    void docPCBLibParser(rapidjson::Document &parseTarget, std::string &filename);

    //Variables for global uses here.

    //Global functions here.
    
  }

#endif