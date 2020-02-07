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

#include "includes.hpp"
#include "rapidjson.hpp"
#include "lcstringparser.hpp"

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
  //Layer mapper. Input EasyEDA, ouput KiCad.
  const int LCtoKiCadLayerLUT[] = {-1, 0, 31, 37, 36, 35, 34, 39, 38, -1, 44, -1, 41, 49, 48, -1, -1, -1, -1, -1, -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
  const char *KiCadLayerNameLUT[] = {"F.Cu", "In1.Cu", "In2.Cu", "In3.Cu", "In4.Cu", "In5.Cu", "In6.Cu", "In7.Cu", "In8.Cu", "In9.Cu","In10.Cu", "In11.Cu", "In12.Cu", "In13.Cu", "In14.Cu", "In15.Cu", "In16.Cu", "In17.Cu", "In18.Cu", "In19.Cu", "In20.Cu", "In21.Cu", "In22.Cu", "In23.Cu", "In24.Cu", "In25.Cu", "In26.Cu", "In27.Cu", "In28.Cu", "In29.Cu", "In30.Cu", "B.Cu", "B.Adhes", "F.Adhes", "B.Paste", "F.Paste", "B.SilkS", "F.SilkS", "B.Mask", "F.Mask", "Dwgs.User", "Cmts.User", "Eco1.User", "Eco2.User", "Edge.Cuts", "Margin", "B.CrtYd", "F.CrtYd", "B.Fab", "F.Fab"};
  const char *padTypeKiCad[] = {"smd", "smd", "thru_hole", "np_thru_hole"};
  const char *padShapeKiCad[] = {"circle", "oval", "rect", "custom"};

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



  int layerMap(string &layerString)
  {
    vector<string> layerProp = splitString(layerString, '~');
        //int layerNum;
        //cout << layerProp[0] << ": " << layerProp[1] << " / " << (layerNum < 0 || layerNum > 50 ? "Invalid" : KiCadLayerNameLUT[layerNum]) << endl;
    if(layerProp[5] == "true")
      try
      {
        return LCtoKiCadLayerLUT[stoi(layerProp[0])];
      }
      catch(std::invalid_argument)
      {
       return -1;
      }
    else    vector<string> layerProp = splitString(layerString, '~');
      return -1;
  }

  void docPCBLibParser(Document &parseTarget, string &filename, int iteration, LCStringParserContainer *parser)
  {
    cout << "\tPCB library file parser function has been called. Starting PCB library file parsing.\n";

    fstream outputFile;
    outputFile.open(filename + ".pretty");
    assertThrow(outputFile.fail(), "Cannot create the output file.");
    
    string propertyStr;
    vector<string> propList;
    vector<int> layerMapper;
    double originX, originY, gridSize, traceWidth;
    int layerCount = 0, layerBuffer = 0;
    string packageName, prefix, contributor;
    
    //Parse canvas properties
    propertyStr = parseTarget["canvas"].GetString();
    propList = splitString(propertyStr, '~');
    //Read canvas properties
    coordinates origin;
    origin.X = stof(propList[16]);
    origin.Y = stof(propList[17]);
    gridSize = stof(propList[6]);
    traceWidth = stof(propList[12]);
      //cout << originX << ',' << originY << endl;
    propertyStr.clear();
    propList.clear();

    //Parse Prefix and contributor
    Value &head = parseTarget["head"];
    Value &headlist = head.GetObject()["c_para"];
    packageName = headlist["package"].GetString();
    prefix = headlist["pre"].GetString();
    contributor = headlist["Contributor"].GetString();

    //Parse Layer information
    Value layer = parseTarget["layers"].GetArray();
    string currentlayer;
    layerMapper.push_back(-1);
    for(int i = 0; i < layer.Size(); i++)
    {
      currentlayer = layer[i].GetString();
      layerBuffer = layerMap(currentlayer);
      if(layerBuffer != -1)
        layerCount++;
      layerMapper.push_back(layerBuffer);
    }
    layer.~GenericValue();
    cout << "\tFinished parsing layers. " << layerCount << " layers were(was) used in total.\n";

    assertThrow(parseTarget["shape"].IsArray(), "Not an array.");
    Value shape = parseTarget["shape"].GetArray();
    vector<string> shapesList;
    vector<PCBElements*> elementsList;
    for(int i = 0; i < shape.Size(); i++)
      shapesList.push_back(shape[i].GetString());

    for(int i = 0; i < shapesList.size(); i++)
    {
      //vector<string> parameters = splitString(shapesList[i], '~');
      switch(shapesList[i].c_str()[0])
      {
        case 'P': //Pad
          elementsList.push_back(parser->parsePadString(shapesList[i], origin));
          break;
        case 'T':
          switch(shapesList[i].c_str()[1])
          {
            case 'E': //Text
              break;
            case 'R': //Track
              stringlist tmp = splitString(shapesList[i], '~');
              if(parser->judgeIsOnCopperLayer(LCtoKiCadLayerLUT[atoi(tmp[2].c_str())]))
                elementsList.push_back(parser->parseTrackString(shapesList[i], origin));
              else
                elementsList.push_back(parser->parseGraphicalLineString(shapesList[i], origin));
              break;
          }
          break;
        case 'C':
          switch(shapesList[i][1])
          {
            case 'O': //CopperArea
              break;
            case 'I': //Circle
              break;
          }
          break;
        case 'R': //Rect
          break;
        case 'A': //Arc
          break;
        case 'V': //Via
          elementsList.push_back(parser->parseViaString(shapesList[i], origin));
          break;
        case 'H': //Hole
          break;
        case 'D': //Dimension
          break;
        default:
          string e = "Invalid element of " + shapesList[i];
          assertThrow(false, e.c_str());
      }
    }
    string a = "  ";
    char* A = (char*) a.c_str();
    for(int i = 0; i < elementsList.size(); i++)
    {
      cout << (elementsList[i]->outputKiCadFormat(a, A)) << endl;
    }
  }

  void parseDocument(char *filePath, char *bufferField)
  {
    std::FILE *parseTarget = std::fopen(filePath, "r");

    FileReadStream fileReader(parseTarget, bufferField, READ_BUFFER_SIZE);
    Document parseTargetDoc;
    parseTargetDoc.ParseStream(fileReader);

    LCStringParserContainer *LCParser;

    std::fclose(parseTarget);

    if(parseTargetDoc.HasParseError())
    {
      cout << "\tError when parsing file \"" << filePath << "\":\n"
           << "\tError code " << parseTargetDoc.GetParseError() << " at offset " << parseTargetDoc.GetErrorOffset() << ".\n";
      assertThrow(false, "Error occured while parsing a file.");
    }

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
    if(documentType >= 1 && documentType <= 7)
    {
      cout << "\tThis document is a " << documentTypeName[documentType] << " file";
      if(editorVer != "")
        cout << ", exported by EasyEDA Editor " << editorVer << ".\n";
      else
        cout << ". Unknown EasyEDA Editor version.\n";
    }
    else
      assertThrow(false, "Not supported document type.");

    LCParser = new StandardLCStringParser();
      
    
    //Now decide what are we going to parse, whether schematics or PCB, anything else.
    switch(documentType)
    {
      case 4:
        docPCBLibParser(parseTargetDoc, filename, 1, LCParser);
        break;
      default:
        assertThrow(false, "This kind of document type is not supported yet.");
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
      assertThrow(!iferrored, "Missing specified file(s).");
    }
  }
}