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

#include "includes.hpp"
#include "lc2kicad.hpp"
#include "edaclasses.hpp"
#include "lc2kicadcore.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using namespace lc2kicad;

namespace lc2kicad
{
  void displayAbout();
  void displayUsage();
}

int main(int argc, const char** argv)
{
  
#ifdef MAKE_CUSTOM_TEST_OF_FUNCS
  //Macro above is defined in includes.cpp.
  //test anything here
  
  
  //test end here
  return 0;
#endif
  
  auto argParseResult = programArgumentParser(argc, argv);

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
  vector<EDADocument*> documentCacheList;

  string path = "";

  if(argParseResult.convertAsProject)
  {
    cerr << "Error: converting as project is not supported yet.";
    exit(1);
  }
  
  for(auto &i : argParseResult.filenames)
    try
    {
      documentCacheList.push_back(core.autoParseLCFile(i));
    }
    catch(std::runtime_error &e)
    {
      std::cerr << e.what() << std::endl;
    }

  for(auto &i : documentCacheList)
    core.deserializeFile(i, &path);


  return 0;
}


namespace lc2kicad
{
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
    cout  << "LC2KiCad version " << SOFTWARE_VERSION << endl 
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
