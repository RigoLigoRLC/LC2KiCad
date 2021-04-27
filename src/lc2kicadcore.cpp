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

#include <iostream>
#include <vector>
#include <fstream>
#include <ctime>

#include "consts.hpp"
#include "includes.hpp"
#include "rapidjson.hpp"
#include "rapidjson/istreamwrapper.h"
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
  extern programArgumentParseResult argParseResult;

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

  list<EDADocument*> LC2KiCadCore::autoParseLCFile(string& filePath)
  {
    // First, the program has to identify what the file type EasyEDA document is.
    // Determine if it's JSON file. So use RapidJSON read the file first.

    // Create an internal class object.
    list<EDADocument*> ret;
    EDADocument tempTargetDoc(true); // This is used because we don't know what type we're dealing with at first.
                                     // Will be constructing a new one in the switch case.
    tempTargetDoc.pathToFile = filePath; // Just for storage so the document will know who he is.
    tempTargetDoc.parent = this; // Set parent. Currently used for deserializer referencing.

    char readBuffer[BUFSIZ]; // Create the buffer for RapidJSON to read the file
    std::FILE *parseTarget = std::fopen(filePath.c_str(), "r");
    assertThrow(parseTarget != 0, "File \"" + filePath + "\" couldn't be opened. Parse of this file is aborted.");
    FileReadStream fileReader(parseTarget, readBuffer, BUFSIZ);
    tempTargetDoc.jsonParseResult->ParseStream(fileReader); // Let RapidJSON parse JSON file
    std::fclose(parseTarget);

    // Create a reference to the JSON parse result for convenience
    // (Actually also cause I don't want to change the code structure)
    Document& parseTargetDoc = *tempTargetDoc.jsonParseResult;
    
    // EasyEDA files are now only in JSON. If fail to detect a valid JSON file, throw an exception.
    assertThrow(!parseTargetDoc.HasParseError(),
                string("RapidJSON reported error when parsing the file. Error code: ") +
                rapidjsonErrorMsg[parseTargetDoc.GetParseError()] + ", offset " +
                to_string(parseTargetDoc.GetErrorOffset()) + ".\n"
                );

    // If this file is a valid JSON file, continue parsing.
    parseJsonAsEasyEDA6File(tempTargetDoc, ret);


    return ret;
  }

  EDADocument* LC2KiCadCore::parseLCFileFromStdin()
  {
    list<EDADocument*> ret;
    EDADocument tempTargetDoc(true);

    rapidjson::IStreamWrapper cinwrapper(std::cin);
    tempTargetDoc.jsonParseResult->ParseStream(cinwrapper); // Parse from stdin

    Document& parseTargetDoc = *tempTargetDoc.jsonParseResult;
    
    assertThrow(!parseTargetDoc.HasParseError(),
                string("RapidJSON reported error when parsing the file. Error code: ") +
                rapidjsonErrorMsg[parseTargetDoc.GetParseError()] + ", offset " +
                to_string(parseTargetDoc.GetErrorOffset()) + ".\n"
                );

    parseJsonAsEasyEDA6File(tempTargetDoc, ret);

    // Piped conversion can only handle single input and print single output file,
    // So we'll fail if we have multiple ones.
    if(ret.size() != 1)
    {
      for(auto &i : ret)
        delete i; // Dont forget to cleanup
      assertThrow(false, "Piped processing only supports one file being output!");
    }
    
    return ret.front();
  }

  /*
   * Provide an internal document object and parse it as a EasyEDA 6 document file.
   * Note that the jsonParseResult member should be a valid one. Or else, the program will read and parse JSON
   * from the path provided by pathToFile member.
   *
   * This function will push the parsed document into the ret vector reference that the user provides.
   * Therefore a valid reference to a corresponding vector must be provided.
   */
  void LC2KiCadCore::parseJsonAsEasyEDA6File(EDADocument &aTargetDoc, list<EDADocument *> &ret)
  {
    /*
    cerr << "Auto Parse Processor: Document " << aTargetDoc.pathToFile <<  " is a " << documentTypeName[documentType]
         << " file";
    if(editorVer != "")
      cerr << ", exported by EasyEDA Editor " << editorVer << ".\n";
    else
      cerr << ". EasyEDA Editor version unknown.\n";
      */
    cerr << "[Auto Parser] Read input document \"" << aTargetDoc.pathToFile << "\" as EasyEDA 6 document...\n";

    string filename = base_name(string(aTargetDoc.pathToFile));

    aTargetDoc.docInfo["filename"] = filename;
    aTargetDoc.docInfo["documentname"] = filename;
    //aTargetDoc.docInfo["editorversion"] = editorVer;

    processEasyEDA6DocumentObject(*aTargetDoc.jsonParseResult, &aTargetDoc, ret);
  }

  // We process document objects here.
  void LC2KiCadCore::processEasyEDA6DocumentObject(rapidjson::Value &aDocObject,
                                                   EDADocument* aBasicDocument,
                                                   list<EDADocument *> &ret)
  {
    ASSERT_RETURN_MSG(aDocObject.IsObject(), "Invalid document object");


    int documentType = -1;
    // Judge the document type and do tasks accordingly.
    if(aDocObject.HasMember("head"))
    {
      Value& head = aDocObject["head"];
      assertThrow(head.IsObject(), "Invalid \"head\" type.");
      assertThrow(head.HasMember("docType"), "\"docType\" not found.");
      assertThrow(head["docType"].IsString(), "Invalid \"docType\" type: not string.");
      documentType = stoi(head["docType"].GetString());
      //if(head.HasMember("editorVersion") && head["editorVersion"].IsString())
      //  editorVer = head["editorVersion"].GetString();
    }
    else
    {
      assertThrow(aDocObject.HasMember("docType"), "\"docType\" not found.");
      assertThrow(aDocObject["docType"].IsString(), "Invalid \"docType\" type: not string.");
      documentType = stoi(aDocObject["docType"].GetString());
      //if(parseTargetDoc.HasMember("editorVersion") && parseTargetDoc["editorVersion"].IsString())
      //  editorVer = parseTargetDoc["editorVersion"].GetString();
    }
    assertThrow((documentType >= 1 && documentType <= 7),
                string("Unsupported document type ID ") + to_string(documentType) + ".");




    // Now decide what are we going to parse, whether schematics or PCB, anything else.
    //PCBDocument* targetDoc = new PCBDocument(targetInternalDoc); // Deprecated
    RAIIC<EDADocument> targetDocument(nullptr);
    switch(documentType)
    {
      case 1:
      {
        targetDocument.replace(new SchematicDocument(*aBasicDocument));
        Document *realDocument = new Document;
        realDocument->SetObject() = aDocObject;
        targetDocument->jsonParseResult = shared_ptr<Document>(realDocument);
        if(coreParserArguments.count("ENL"))
        {
          internalSerializer->initWorkingDocument(!targetDocument);
          ret.splice(ret.end(), internalSerializer->parseSchNestedLibs());
          internalSerializer->deinitWorkingDocument();
          break;
        }
        else
        {
          Error("Schematics cannot be converted now. Only supports library extraction.");
        }
        break;
      }
      case 2:
      {
        targetDocument.replace(new SchematicDocument(*aBasicDocument)); // FIXME: Do not copy JSON result around?
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
        targetDocument.replace(new PCBDocument(*aBasicDocument));
        if(coreParserArguments.count("ENL"))
        {
          internalSerializer->initWorkingDocument(!targetDocument);
          ret.splice(ret.end(), internalSerializer->parsePCBNestedLibs());
          internalSerializer->deinitWorkingDocument();
          break;
        }
        else
        {
          internalSerializer->initWorkingDocument(!targetDocument);
          internalSerializer->parsePCBDocument();
          internalSerializer->deinitWorkingDocument();

          ret.push_back(!++targetDocument);
        }
        break;
      }
      case 4:
      {
        targetDocument.replace(new PCBDocument(*aBasicDocument));
        targetDocument->module = true;
        targetDocument->containedElements.push_back(new PCB_Module);

        internalSerializer->initWorkingDocument(!targetDocument);
        internalSerializer->parsePCBLibDocument(); //Exceptions will be thrown out of the function. Dynamic memory will be released by RAIIC
        internalSerializer->deinitWorkingDocument();

        ret.push_back(!++targetDocument); // Remember to manage dynamic memory and check if it's valid!
        break;
      }
      case 5: // A collection of schematics, known as "project"
      {
        if(coreParserArguments.count("ENL")) // TODO: when we have proper schematics convert, this judgement is no longer needed
        {
          // Iterate through each page and feed them into the parser
          assertThrow(aDocObject.HasMember("schematics"), "\"schematics\" not found.");
          assertThrow(aDocObject["schematics"].IsArray(), "Invalid \"schematics\" type: not array.");
          for(auto &i : aDocObject["schematics"].GetArray())
          {
            if(!i.IsObject()) continue;
            ASSERT_CONT_MSG(i.HasMember("dataStr"), "\"dataStr\" not found.");
            ASSERT_CONT_MSG(i["dataStr"].IsObject(), "Invalid \"dataStr\" type: not object.")
            processEasyEDA6DocumentObject(i["dataStr"], aBasicDocument, ret);
          }
        }
        else
        {
          Error("Schematics cannot be converted now. Only supports library extraction.");
        }
        break;
      }
      default:
        Error(string("The document type \"") + documentTypeName[documentType] + "\" is not supported yet.");
        //ret.push_back(nullptr); // Not really needed to do this?
    }
  }

  void LC2KiCadCore::deserializeFile(EDADocument* target, string* path)
  {
    std::ofstream outputfile;
    std::ostream *outputStream = &cout;
    string* tempResult, outputFileName;
        
    if(!argParseResult.usePipe)
    {
      outputFileName = *path + target->docInfo["documentname"] + documentExtensionName[target->docType];
      sanitizeFileName(outputFileName);
      cerr << "[Deserializer] Write file \"" << outputFileName << "\"...\n";
      outputfile.open(outputFileName, std::ios::out);
      if(!outputfile) // Dont error with pipe IO
        Error("[Deserializer] Cannot create file for this document. File content would be written into"
              "the standard output stream.");
      else
        outputStream = &outputfile;
    }
    
    internalDeserializer->initWorkingDocument(target);

    // Deserializer output are pointers to dynamic memory. They must be freed manually.

    // Headers
    tempResult = internalDeserializer->outputFileHeader();
    *outputStream << *tempResult << endl;
    delete tempResult;

    for(auto &i : target->containedElements)
    {
      if(!i) continue;
      try { tempResult = i->deserializeSelf(*internalDeserializer); }
      catch(std::runtime_error &e)
      {
        Error(string("[Deserializer] Unexpected error outputting a component: ") + e.what());
      }

      *outputStream << *tempResult << endl;
      delete tempResult;
    }

    tempResult = internalDeserializer->outputFileEnding();
    *outputStream << *tempResult << endl;
    delete tempResult;

    if(!outputfile)
      outputfile.close();
  }
}
