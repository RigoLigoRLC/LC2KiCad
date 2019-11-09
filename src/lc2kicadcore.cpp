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
using rapidjson::FileReadStream;
using rapidjson::Document;

namespace lc2kicad
{
  const char softwareVersion[] = "0.1-beta";

  void errorAndQuit(std::runtime_error *e)
  {
    cout << "Error running the program: " << e->what() << endl << endl
         << "The intended operation cannot be done. The application will quit.\n";
    exit(1);
  }

  void errorAndAbort(std::runtime_error *e)
  {
    cout << "Runtime error: " << e->what() << endl << endl
         << "The intended operation cannot be done. The application will quit.\n";
    abort();
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

  void parseDocument(char *filePath, char *bufferField)
  {
    std::FILE *parseTarget = std::fopen(filePath, "r");

    FileReadStream fileReader(parseTarget, bufferField, READ_BUFFER_SIZE);
    Document parseTargetDoc;
    parseTargetDoc.ParseStream(fileReader);

    std::fclose(parseTarget);

    if(parseTargetDoc.HasParseError())
    {
      cout << "Error when parsing file \"" << filePath << "\":\n"
           << "Error code " << parseTargetDoc.GetParseError() << " at offset " << parseTargetDoc.GetErrorOffset() << ".\n";
      std::runtime_error e("Error occured while parsing a file.");
      throw e;
    }
    else
    {
      cout << "Successfully parsed file \"" << filePath << "\".\n";
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
      if(iferrored)
      {
        std::runtime_error e("Missing specified file(s).");
        throw e;
      }
    }
  }
}