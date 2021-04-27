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
    bool noDoubleDash = true;
    char currentShortSwitch = 0;
    programArgumentParseResult ret;
    enum { none, configFile, outputDirectory, parserArgument } status = none;

    if(argc == 1)
    {
      ret.invokeHelp = true;
      return ret;
    }

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
      if(strlen(argv[i]) == 2 && argv[i][0] == '-') // Short argument gets detected.
      {
        if(remainingArgs != 0) // If system expects arugments for a switch parsed earlier, throw exception.
          assertThrow(false, string("Error: too few arguments for switch \"") + argv[i - 1] + "\"");

        currentShortSwitch = argv[i][1];

        switch (argv[i][1])
        {
          case 'h':
            ret.invokeHelp = true;
            break;
          case 'v':
            ret.verboseInfo = true;
            break;
          case 'p':
            ret.usePipe = true;
            break;
          case 'P':
            ret.convertAsProject = true;
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
          case 'l': // Export nested libraries
            ret.exportNestedLibs = true;
            break;
          default:
            assertThrow(false, string("Error: unrecognized switch \"-") + currentShortSwitch + "\"");
            break;
        }
      }
      else // Long arguments get detected.

        if(!strcmp(argv[i], "--help")) // Long switches detection.
          ret.invokeHelp = true;
        else if(!strcmp(argv[i], "--version"))
          ret.invokeVersionInfo = true;
        else if(!strcmp(argv[i], "--pipe"))
          ret.usePipe = true;

        else if(remainingArgs > 0) // Not long switches, then it could only be arguments for a switch.
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
              discreteArgs = splitString(parserArgumentCache, ',');
              for(auto i : discreteArgs)
              {
                splitArgKeyCache = splitString(i, ':');

                try { parserArguments[splitArgKeyCache[0]] = std::stod(splitArgKeyCache[1]); }
                catch(...) { assertThrow(false, "Error: Failed to parse parser argument \"" + i + "\""); }
              }
              ret.parserArguments = parserArguments;
              break;
            default:
              break;
          }
          remainingArgs--;
          if(remainingArgs == 0) // After processing all required arguments, reset status.
            status = none;
        }
        else if(remainingArgs == 0) // The one argument being read now is not related to switches and their arguments.
        {
          if(noDoubleDash)
            ret.filenames.push_back(argv[i]); // If no double dash, it is a filename.
          else // If has double dash, then we're parsing switches, and met an invalid one.
            assertThrow(false, string("Error: unrecognized switch \"") + argv[i] + "\"");
        }
    }

    // If even after parsing all the arguments the system still expects argument, then error out.
    assertThrow(!remainingArgs, string("Error: too few arguments for switch \"-") + currentShortSwitch + "\"");

    if(!noDoubleDash)
      for(int i = endpos + 1; i < argc; i++)
        ret.filenames.push_back(argv[i]); // If we got double dash, parse all args after the double dash as filenames.

    // Check ENL in advance
    if(ret.exportNestedLibs)
      ret.parserArguments["ENL"] = 1;

    // Check if things are valid
    checkArgConflict(&ret);

    return ret;
  }

  void programArgumentParseResult::verboseOutputArgParseResult(const programArgumentParseResult *result)
  {
    VERBOSE_INFO(string("Invoke help: ") + (result->invokeHelp ? "true" : "false"));
    VERBOSE_INFO(string("Invoke version: ") + (result->invokeVersionInfo ? "true" : "false"));
    VERBOSE_INFO(string("Convert as project: ") + (result->convertAsProject ? "true" : "false"));
    if(result->configFile.size())
      VERBOSE_INFO(string("Explicitly specified config file: ") + result->configFile);
    if(result->outputDirectory.size())
      VERBOSE_INFO(string("Specified output directory: ") + result->outputDirectory);
    if(result->parserArguments.size())
    {
      string parserArgumentsList("Specified parser arguments:\n");
      for(auto &i : result->parserArguments)
        parserArgumentsList += "Arg \"" + i.first + "\" = " + std::to_string(i.second) + "\n";
      parserArgumentsList.pop_back(); // Remove excess '\n'. Same for next one.
      VERBOSE_INFO(parserArgumentsList);
    }
    if(result->filenames.size())
    {
      string filenameList("Input files: \n");
      for(auto &i : result->filenames)
        filenameList += i + "\n";
      filenameList.pop_back();
      VERBOSE_INFO(filenameList);
    }
  }

  void checkArgConflict(const programArgumentParseResult *result)
  {
    // Piped operation has multiple restrictions
    if(result->usePipe)
    {
      // Piped operation cannot live with output directory
      assertThrow(result->outputDirectory == "",
                  "Setting an output directory is not accepted when using piped operation!");

      // And is also likely to cause problems when extracting nested libraries
      if(result->parserArguments.count("ENL"))
        if(result->parserArguments.at("ENL") == 1)
          Info("Extract nested libraries when using piped operation may cause problems.");
    }
  }
}
