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


#include "includes.hpp"
#include "lc2kicad.hpp"
#include "edaclasses.hpp"
#include "lc2kicadcore.hpp"

#include "floatint.hpp"

#ifdef USE_WINAPI_FOR_TEXT_COLOR
  #include <Windows.h>
#endif

// #define MAKE_CUSTOM_TEST_OF_FUNCS

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::to_string;
using namespace lc2kicad;

namespace lc2kicad
{
  void displayAbout();
  void displayUsage();
  programArgumentParseResult argParseResult;
  long errorCount = 0, warningCount = 0;
  std::ostream* logstream = nullptr;
#ifdef USE_WINAPI_FOR_TEXT_COLOR
  HANDLE hStdOut;
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
  WORD wBackgroundColor;
#endif
}

int main(int argc, const char** argv)
{
#ifdef USE_WINAPI_FOR_TEXT_COLOR
  hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
#endif


  try { argParseResult = programArgumentParser(argc, argv);}
  catch (std::exception &e) { std:cerr << (string("Argument parsing failed with exception: \n") + e.what()); exit(1);};

#ifdef MAKE_CUSTOM_TEST_OF_FUNCS

#endif

  logstream = argParseResult.usePipe ? &std::cerr : &std::cout;

  if(argParseResult.verboseInfo)
    argParseResult.verboseOutputArgParseResult(&argParseResult);
  if(argParseResult.invokeHelp) // Show help or version info then exit
  {
    lc2kicad::displayUsage();
    exit(0);
  }
  else if(argParseResult.invokeVersionInfo)
  {
    lc2kicad::displayAbout();
    exit(0);
  }

  LC2KiCadCore core = LC2KiCadCore(argParseResult.parserArguments); //Initialize Core Program
  list<EDADocument*> documentCacheList;

  string path = "";

  if(argParseResult.convertAsProject)
  {
    Error("converting as project is not supported yet.");
    exit(1);
  }
  
  if(!argParseResult.usePipe) // When using file IO; mostly this case
  {
    for(auto &i : argParseResult.filenames)
      try
      {
        auto docList = core.autoParseLCFile(i);
        for(auto &j : docList)
          if(j)
            documentCacheList.push_back(j);
      }
      catch(std::runtime_error &e)
      {
        Error(string("Parsing for \"") + i + "\" failed with exception: " + e.what());
      }

    for(auto &i : documentCacheList)
      if(i)
        core.deserializeFile(i, &path), delete i;
  }
  else // When using piped IO
  {
    try
    {
      documentCacheList.push_back(core.parseLCFileFromStdin());
    }
    catch(std::runtime_error &e)
    {
      Error(string("Parsing from standard input failed with exception: ") + e.what());
    }

    for(auto &i : documentCacheList)
    if(i)
      core.deserializeFile(i, &path), delete i;
  }

  *logstream << endl;
  if(errorCount | warningCount)
    Warn(string("Error(s): ") + to_string(errorCount) + ", warning(s): " + to_string(warningCount) + ".");
  else
    *logstream << "Error(s): " << errorCount << ", warning(s): " << warningCount << ".\n";

  return 0;
}


namespace lc2kicad
{
  void displayUsage()
  {
  cerr << "Usage: lc2kicad [OPTION] [--] FILENAME\n\n"
          "FILENAME: The EasyEDA JSON Document path. The file should have been exported\n"
          "          via EasyEDA menu \"Document - Export - EasyEDA\".\n\n"
          "  -h, --help:     Display this help message and quit.\n"
          "      --version:  Display about message.\n"
          "  -a [ARGS]:      Specify parser arguments; see documentation for details.\n"
          "  -l:             Export nested libraries from a document.\n";
  }

  void displayAbout()
  {
    cerr  << "LC2KiCad version " << SOFTWARE_VERSION << endl
          //<< "Compiled from " << gitCommitHash << endl
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
}
