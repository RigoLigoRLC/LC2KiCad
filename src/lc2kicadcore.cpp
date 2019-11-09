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

#include <include.hpp>
#include <rapidjson.hpp>

using std::cout;
using std::endl;
using std::fstream;
using std::string;
using std::vector;
using std::stof;
using std::stoi;
using rapidjson::FileReadStream;
using rapidjson::Document;
using rapidjson::Value;

namespace lc2kicad
{
  const char softwareVersion[] = "0.1-beta";

  enum documentTypes {schematic = 1, schematic_lib = 2, pcb = 3, pcb_lib = 4, project = 5, sub_part = 6, spice_symbol = 7};
  const char *documentTypeName[8] = {"", "schematics", "schematic library", "PCB", "PCB library", "project", "sub-part", "SPICE symbol"};

  void errorAndQuit(std::runtime_error *e)
#ifdef ERROR_EXIT
  {
    cout << "Error running the program: " << e->what() << endl << endl
         << "The intended operation cannot be done. The application will quit.\n";
    exit(1);
  }
#else
  #ifdef ERROR_ABORT
    {
      cout << "Runtime error: " << e->what() << endl << endl
          << "The intended operation cannot be done. The application will quit.\n";
      abort();
    }
  #endif
#endif

  void assertRTE(bool statement, const char* message)
  {
    if(!statement)
    {
      std::runtime_error e(message);
      errorAndQuit(&e);
    }
  }

  void displayUsage()
  {
    cout  << "Usage: lc2kicad FILENAME...\n"
          << "  or:  lc2kicad [OPTION]\n\n"
          << "FILENAME: The EasyEDA JSON Document path. THe file should have been exported\n"
          << "          vis EasyEDA menu \"Document - Export - EasyEDA\".\n\n"
          << "  -h, --help:     Display this help message and quit.\n"
          << "  -v, --version:  Display about message.\n";
  }

  void displayAbout()
  {
    cout  << "LC2KiCad version " << SOFTWARE_VERSION << endl << endl
          << "This program is an utility that allows you to convert your EasyEDA documents\n"
          << "into the KiCad 5 version document, so that you will be able to move your\n"
          << "designs in EasyEDA to KiCad for any legit purpose.\n\n"
          << "Note that you should not convert any document that is not owned by you without\n"
          << "written permission from the document author. You should NOT use this software\n"
          << "if you don't accept the EasyEDA Terms of Service.\n\n"
          << "LC2KiCad is a free software, distributed under the terms of GNU Lesser General\n"
          << "Public License as published by Free Software Foundation, either version 3, or\n"
          << "(at your option) any later version.\n\n"
          << "This software comes with ABSOLUTE NO WARRANTY, without even the implied\n"
          << "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n"
          << "You should have received a copy of the GNU Lesser General Public License\n"
          << "along with LC2KiCad. If not, see <https://www.gnu.org/licenses/>.\n";
  }

  string base_name(string const& path)
  {
    return path.substr(path.find_last_of("/\\") + 1);
  }

  vector<string> splitString(string sourceString, char delimeter)
  {
    std::stringstream ss(sourceString);
    string item;
    vector<string> rtn;
    while(std::getline(ss, item, delimeter))
      rtn.push_back(item);
    return rtn;
  }

  void docPCBLibParser(Document &parseTarget, string &filename)
  {
    cout << "\tPCB Library file parser function has been called. Starting PCB file parsing.\n";
    
    string propertyStr;
    vector<string> propList;
    float originX, originY, gridSize, traceWidth;
    string packageName, prefix, contributor;
    
    //Parse canvas properties
    propertyStr = parseTarget["canvas"].GetString();
    propList = splitString(propertyStr, '~');
    //Read canvas properties
    originX = stof(propList[16]);
    originY = stof(propList[17]);
    gridSize = stof(propList[6]);
    traceWidth  =stof(propList[12]);
      //cout << originX << ',' << originY << endl;
    propertyStr.clear();
    propList.clear();

    //Parse Prefix and contributor
    Value& cp = parseTarget["head"];
    cp = cp.GetObject()["c_para"];
    packageName = cp["package"].GetString();
    prefix = cp["pre"].GetString();
    contributor = cp["Contributor"].GetString();

    
  }

  void parseDocument(char *filePath, char *bufferField)
  {
    std::FILE *parseTarget = std::fopen(filePath, "r");

    FileReadStream fileReader(parseTarget, bufferField, READ_BUFFER_SIZE);
    Document parseTargetDoc;
    parseTargetDoc.ParseStream(fileReader);

    std::fclose(parseTarget);

    if(parseTargetDoc.HasParseError())
    {
      cout << "\tError when parsing file \"" << filePath << "\":\n"
           << "\tError code " << parseTargetDoc.GetParseError() << " at offset " << parseTargetDoc.GetErrorOffset() << ".\n";
      assertRTE(false, "Error occured while parsing a file.");
    }

    //If this file is a valid JSON file, continue parsing.

    int documentType = -1;
    string filename = base_name(string(filePath));

    //Judge the document type and do tasks accordingly.
    if(parseTargetDoc.HasMember("head"))
    {
      Value& head = parseTargetDoc["head"];
      assertRTE(head.IsObject(), "Invalid \"head\" type.");
      assertRTE(head.HasMember("docType"), "\"docType\" not found.");
      documentType = stoi(head["docType"].GetString());
    }
    else
    {
      assertRTE(parseTargetDoc.HasMember("docType"), "\"docType\" not found.");
      assertRTE(parseTargetDoc["docType"].IsString(), "Invalid \"docType\" type.");
      documentType = stoi(parseTargetDoc["docType"].GetString());
    }
    if(documentType >= 1 && documentType <= 7)
      cout << "\tThis document is a " << documentTypeName[documentType] << " file.\n";
    else
      assertRTE(false, "Not supported document type.");
      
    
    //Now decide what are we going to parse, whether schematics or PCB, anything else.
    switch(documentType)
    {
      case 4:
        docPCBLibParser(parseTargetDoc, filename);
        break;
      default:
        assertRTE(false, "This kind of document type is not supported yet.");
    }

  }

  void parseDocuments(int fileCount, char *args[])
  {
    fstream opener;
    char *readBufferField = new char[65536];

    for(int i = 1; i <= fileCount; i++)
    {
      cout << "Parsing of file \"" << args[i] << "\" has started...\n";
      try
      {
        parseDocument(args[i], readBufferField);
      }
      catch (std::runtime_error& e)
      {
        errorAndQuit(&e);
      }
    }
  }

  void parseDocumentList(int fileCount, char *args[])
  {
    fstream opener;
    bool iferrored = false;
    for(int i = 1; i <= fileCount; i++)
    {
      opener.open(args[i], std::_Ios_Openmode::_S_in);
      cout << "(" << i << ") ";
      if(opener.fail())
      {
        iferrored = true;
        cout << "Failed to locate file \"" << args[i] << "\".\n";
      }
      else
      {
        cout << "File \"" << args[i] << "\" was located.\n";
        opener.close();
      }
      cout << endl;
      assertRTE(!iferrored, "Missing specified file(s).");
    }
  }
}