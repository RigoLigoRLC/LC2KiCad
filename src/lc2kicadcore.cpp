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

#include <iostream>
#include <vector>
#include <fstream>
#include <ctime>

#include "consts.hpp"
#include "includes.hpp"
#include "rapidjson.hpp"
#include "edaclasses.hpp"
#include "lc2kicadcore.hpp"
#include "internalsserializer.hpp"
#include "internalsdeserializer.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::fstream;
using std::string;
using std::vector;
using std::stof;
using std::stoi;
using std::to_string;
using std::runtime_error;
using rapidjson::FileReadStream;
using rapidjson::Document;
using rapidjson::Value;

namespace lc2kicad
{
  LC2KiCadCore::LC2KiCadCore(str_dbl_map &setCompatibSw)
  {
    switch(static_cast<int>(setCompatibSw["SSV"])) // SelectSerializerVersion
    {
      case 0: // Default
      case 1: // EasyEDA 6
      default:
        internalSerializer = new LCJSONSerializer();
    }

    switch(static_cast<int>(setCompatibSw["SDV"])) // SelectDeserializerVersion
    {
      case 0: // Default
      case 1: // KiCad 5
      default:
        internalDeserializer = new KiCad_5_Deserializer();
    }

    internalSerializer->setCompatibilitySwitches(setCompatibSw);
    internalDeserializer->setCompatibilitySwitches(setCompatibSw);
    coreParserArguments = setCompatibSw;
  }

  LC2KiCadCore::~LC2KiCadCore()
  {
    delete internalSerializer;
    delete internalDeserializer;
  }

  vector<EDADocument*> LC2KiCadCore::autoParseLCFile(string& filePath)
  {
    // First, the program has to identify what the file type EasyEDA document is.
    // Determine if it's JSON file. So use RapidJSON read the file first.

    // Create an internal class object.
    vector<EDADocument*> ret;
    EDADocument targetInternalDoc(true);
    targetInternalDoc.pathToFile = filePath; //Just for storage; not being used now.
    targetInternalDoc.parent = this; // Set parent. Currently used for deserializer referencing.

    char readBuffer[BUFSIZ]; // Create the buffer for RapidJSON to read the file
    std::FILE *parseTarget = std::fopen(filePath.c_str(), "r");
    assertThrow(parseTarget != 0, "Error: File \"" + filePath + "\" couldn't be opened. Parse of this file is aborted.");
    FileReadStream fileReader(parseTarget, readBuffer, BUFSIZ);
    targetInternalDoc.jsonParseResult->ParseStream(fileReader); // Let RapidJSON parse JSON file
    std::fclose(parseTarget);

    // Create a reference to the JSON parse result for convenience
    // (Actually also cause I don't want to change the code structure)
    Document& parseTargetDoc = *targetInternalDoc.jsonParseResult;
    
    // EasyEDA files are now only in JSON. If fail to detect a valid JSON file, throw an exception.
    assertThrow(!parseTargetDoc.HasParseError(),
                string("Error occured while parsing a file:\n") + "Error when parsing file \"" + filePath + "\":\n" +
                "\tError code " + rapidjsonErrorMsg[parseTargetDoc.GetParseError()] + " at offset " +
                to_string(parseTargetDoc.GetErrorOffset()) + ".\n"
                );

    // If this file is a valid JSON file, continue parsing.
    int documentType = -1;
    string filename = base_name(string(filePath)), editorVer = "";

    // Judge the document type and do tasks accordingly.
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
    assertThrow((documentType >= 1 && documentType <= 7), "Not supported document type.");
    cout << "This document is a " << documentTypeName[documentType] << " file";
    if(editorVer != "")
      cout << ", exported by EasyEDA Editor " << editorVer << ".\n";
    else
      cout << ". Unknown EasyEDA Editor version.\n";

    targetInternalDoc.docInfo["filename"] = filename;
    targetInternalDoc.docInfo["documentname"] = filename;
    targetInternalDoc.docInfo["editorversion"] = editorVer;
    
    // Now decide what are we going to parse, whether schematics or PCB, anything else.
    PCBDocument* targetDoc = new PCBDocument(targetInternalDoc);
    RAIIC<PCBDocument> targetDocument(targetDoc);
    switch(documentType)
    {
      case 2:
      {
        targetDocument->module = true;
        targetDocument->containedElements.push_back(new Schematic_Module);

        internalSerializer->initWorkingDocument(!targetDocument);
        internalSerializer->parseSchLibDocument();
        internalSerializer->deinitWorkingDocument();

        ret.push_back(!++targetDocument);
        break;
      }
      case 3:
      {
        if(coreParserArguments.count("ENL"))
        {
          internalSerializer->initWorkingDocument(!targetDocument);
          ret = internalSerializer->parsePCBNestedLibs();
          internalSerializer->deinitWorkingDocument();
          break;
        }
      }
      case 4:
      {
        targetDocument->module = true;
        targetDocument->containedElements.push_back(new PCB_Module);

        internalSerializer->initWorkingDocument(!targetDocument);
        internalSerializer->parsePCBLibDocument(); //Exceptions will be thrown out of the function. Dynamic memory will be released by RAIIC
        internalSerializer->deinitWorkingDocument();

        ret.push_back(!++targetDocument); // Remember to manage dynamic memory and check if it's valid!
        break;
      }
      default:
        cerr << "Error: Cannot process file \"" << filename << "\".\n"
                "       This kind of document type is not supported yet.";
    }
    return ret;
  }

  /*
   * Provide an internal document object and parse it as a EasyEDA 6 document file.
   * Note that the jsonParseResult member should be a valid one. Or else, the program will read and parse JSON
   * from the path provided by pathToFile member.
   *
   * This function will push the parsed document into the ret vector reference that the user provides.
   * Therefore a valid reference to a corresponding vector must be provided.
   */
  void LC2KiCadCore::parseAsEasyEDA6File(EDADocument &targetInternalDoc, vector<EDADocument *> &ret)
  {

  }

  void LC2KiCadCore::deserializeFile(EDADocument* target, string* path)
  {
    cout  << "Create output file for " << target->docInfo["documentname"] << documentExtensionName[target->docType]
          << " for output.\n";

    std::ofstream outputfile;
    std::ostream *outputStream = &cout;
    string* tempResult;
    outputfile.open(*path + target->docInfo["documentname"] + documentExtensionName[target->docType], std::ios::out);

    if(outputfile.fail())
      cerr << "Error: Cannot create file for this document. File content would be written into"
              "the standard output stream.";
    else
      outputStream = &outputfile;
    
    internalDeserializer->initWorkingDocument(target);

    *outputStream << *internalDeserializer->outputFileHeader();
    for(auto &i : target->containedElements)
    {
      tempResult = i->deserializeSelf(*internalDeserializer);
      *outputStream << *tempResult << endl;
      delete tempResult;
    }
    *outputStream << *internalDeserializer->outputFileEnding();

    if(!outputfile.fail())
      outputfile.close();
  }
}
