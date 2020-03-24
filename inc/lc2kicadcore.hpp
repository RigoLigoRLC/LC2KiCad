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

#ifndef LC2KICADCORE_HPP_
  #define LC2KICADCORE_HPP_

  #include <iostream>
  #include <string>
  #include <vector>
  
  #include "rapidjson.hpp"

  #include "includes.hpp"
  #include "edaclasses.hpp"

  using std::cout;
  using std::endl;
  using std::fstream;
  using std::string;
  using std::vector;
  using std::stof;
  using std::stoi;
  using std::to_string;
  using rapidjson::FileReadStream;
  using rapidjson::Document;
  using rapidjson::Value;

  namespace lc2kicad
  {
    class LC2KiCadCore
    {
      public:
        EDADocument* autoParseLCFile(string& filePath)
        {
          //First, the program has to identify what the file type EasyEDA document is.
          //Determine if it's JSON file. So use RapidJSON read the file first.
          char readBuffer[BUFSIZ]; //Create the buffer for RapidJSON to read the file
          std::FILE *parseTarget = std::fopen(filePath.c_str(), "r");
          FileReadStream fileReader(parseTarget, readBuffer, BUFSIZ);
          Document parseTargetDoc;
          parseTargetDoc.ParseStream(fileReader);
          std::fclose(parseTarget);
          
          //EasyEDA files are now only in JSON. If fail to detect a valid JSON file, throw an exception.
          assertThrow(!parseTargetDoc.HasParseError(),
                      string("Error occured while parsing a file:\n") + "Error when parsing file \"" + filePath + "\":\n" +
                      "\tError code " + to_string(parseTargetDoc.GetParseError()) + " at offset " + 
                      to_string(parseTargetDoc.GetErrorOffset()) + ".\n"
                      );

          //If this file is a valid JSON file, continue parsing.
          int documentType = -1;
          string filename = base_name(string(filePath)), editorVer = "";

          //Judge the document type and do tasks accordingly.
          if(parseTargetDoc.HasMember("head"))
          {
            Value& head = parseTargetDoc["head"];
            assertThrow(head.IsObject(), "Invalid \"head\" type.");
            assertThrow(head.HasMember("docType"), "\"docType\" not found.");
            documentType = stoi(head["docType"].GetString());
            if(head.HasMember("editorVersion") && head["editorVersion"].IsString())
              editorVer = head["editorVersion"].GetString();
          }
          else
          {
            assertThrow(parseTargetDoc.HasMember("docType"), "\"docType\" not found.");
            assertThrow(parseTargetDoc["docType"].IsString(), "Invalid \"docType\" type.");
            documentType = stoi(parseTargetDoc["docType"].GetString());
            if(parseTargetDoc.HasMember("editorVersion") && parseTargetDoc["editorVersion"].IsString())
              editorVer = parseTargetDoc["editorVersion"].GetString();
          }
          assertThrow(!(documentType >= 1 && documentType <= 7), "Not supported document type.");
          cout << "\tThis document is a " << documentTypeName[documentType] << " file";
          if(editorVer != "")
            cout << ", exported by EasyEDA Editor " << editorVer << ".\n";
          else
            cout << ". Unknown EasyEDA Editor version.\n";
          
          //Now decide what are we going to parse, whether schematics or PCB, anything else.
          switch(documentType)
          {
            case 4:
              doPCBLibParseJob(parseTargetDoc, filename, 1);
              break;
            default:
              assertThrow(false, "This kind of document type is not supported yet.");
          }
        }
    };
  }

#endif