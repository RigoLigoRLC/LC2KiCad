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

namespace lc2kicad
{
  programArgumentParseResult programArgumentParser(const int &argc, const char** &argv)
  {
    int endpos = -1;
    bool warnCompatibility = false;
    programArgumentParseResult ret;

    if(argc == 1) //If no arguments passed, then go out and display help.
    {
      ret.invokeHelp = true;
      return ret;
    }

    //Look for "--" symbol, determine the end of arguments.
    //The endpos value is the index of "--" plus 1.
    for(int i = 1; i < argc; i++)
      if(strcmp(argv[i], "--"))
      {
        endpos = i + 1;
        break;
      }

    //When "--" not found, endpos == -1; when "--" is the second argument
    //(no arguments), endpos == 2.
    if(endpos != -1 || endpos != 2) 
      for(int i = 1; i < endpos; i++)
      {
        if(strcmp(argv[i], "-h") || strcmp(argv[i], "--help"))
        {
          ret.invokeHelp = true;
          break;
        }
        else if(strcmp(argv[i], "-v") || strcmp(argv[i], "--version"))
        {
          ret.invokeVersionInfo = true;
          break;
        }

        if(strncmp(argv[i], "-c", 2) && strlen(argv[i]) > 2)
        {
          if(ret.compatibilityOptions.size() == 0)
          {
            ret.useCompatibilitySwitches = true;
            
            std::string compatbStr(argv[i] + 2);
            std::cout << "Using compatibility options: <<" << compatbStr << ">>\n";
            stringlist compatbCache = splitString(compatbStr, ','),
                       pairStrTemp;

            findAndReplaceString(compatbStr, ":", "="); //Replace colons with equal signs

            for(std::string i : compatbCache)
            {
              pairStrTemp = splitString(i, '=');
              for (auto & c : pairStrTemp[0]) c = toupper(c); //Convert switch name string to upper case
              if(pairStrTemp.size() == 1)
                ret.compatibilityOptions.insert(str_dbl_pair(pairStrTemp[0], -1.0f));
              else
                ret.compatibilityOptions.insert(str_dbl_pair(pairStrTemp[0], std::stod(pairStrTemp[1])));
            }
          }
          else if(!warnCompatibility)
          {
            std::cout << "[Warning] More than one occurance of -c option was found. "
                         "Only the first occurance of compatibility options will be accepted.";
            warnCompatibility = true;
          }
          continue;
        }

        if(endpos == -1) //Not matching -h -v -c and there's no "--", then treat as filename
          ret.filenames.push_back(argv[i]);
      }
    
    if(endpos != -1) //If there is "--" to split args then do following else just return ret
      if(endpos != argc) //If endpos doesn't equal to argc then there are files, just push them back
        for(int i = endpos + 1; i < argc; i++)
          ret.filenames.push_back(argv[i]);
      else
      {
        std::cout << "[Warning] No files specified. Program will exit.\n";
        exit(0);
      }
    
    return ret;
  }
}