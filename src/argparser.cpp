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

#include <cstring>
#include <string>
#include <iostream>
#include "includes.hpp"

using std::string;

namespace lc2kicad
{
  programArgumentParseResult programArgumentParser(const int &argc, const char** &argv)
  {
    int endpos = -1, remainingArgs = 0;
    bool warnCompatibility = false, noDoubleDash = true;
    programArgumentParseResult ret;
    enum { none, configFile, outputDirectory, parserArgument } status = none;

    for(int i = 0; i < argc; i++)
    {
      if(noDoubleDash && !strcmp(argv[i], "--")) // Double dash splits arguments and filenames.
      {
        noDoubleDash = false;
        endpos = i;
      }
    }
    if(noDoubleDash)
      endpos = argc;
    
    for(int i = 1; i < endpos; i++)
    {
      if(strlen(argv[i]) == 2 && argv[i][0] == '-') // Short argument
      {
        if(remainingArgs != 0)
          assertThrow(false, string("Error: too few arguments for switch \"") + argv[i - 1] + "\"");

        switch (argv[i][1])
        {
        case 'h':
          ret.invokeHelp = true;
          break;
        case 'v':
          ret.invokeVersionInfo = true;
          break;
        case 'a': // Parser argument (formerly Compatibility switches)
          status = parserArgument;
          remainingArgs = 1;
          break;
        case 'f': // Specify configuration file
          status = configFile;
          remainingArgs = 1;
          break;
        case 'o': // Specify output directory
          status = outputDirectory;
          remainingArgs = 1;
          break;
        default:
          assertThrow(false, string("Error: unrecognized switch \"") + argv[i] + "\"");
          break;
        }
      }
      else
        if(!strcmp(argv[i], "--help"))
          ret.invokeHelp = true;
        else if(!strcmp(argv[i], "--version"))
          ret.invokeVersionInfo = true;
      else if(remainingArgs > 0)
      {
        std::string parserArgumentCache;
        stringlist  discreteArgs, splitArgKeyCache;
        str_dbl_map parserArguments;

        switch (status)
        {
        case configFile:
          ret.configFile = argv[i];
          break;
        case outputDirectory:
          ret.outputDirectory = argv[i];
          break;
        case parserArgument:
          parserArgumentCache = argv[i];
          for(auto i : discreteArgs)
          {
            discreteArgs = splitString(parserArgumentCache, ',');
            splitArgKeyCache = splitString(i, ':');

            try { parserArguments[splitArgKeyCache[0]] = std::stod(splitArgKeyCache[1]); }
            catch(...) { assertThrow(false, "Error: Failed to parse parser argument \"" + i + "\""); }
          }
          ret.parserArguments = parserArguments;
          break;
        default:
          break;
        }

        if(remainingArgs == 1)
          status = none;
      }
      else if(remainingArgs == 0)
        if(noDoubleDash)
          ret.filenames.push_back(argv[i]);
        else
          assertThrow(false, string("Error: unrecognized switch \"") + argv[i] + "\"");
    }

    return ret;
  }
}