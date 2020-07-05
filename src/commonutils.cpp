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
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "consts.hpp"
#include "includes.hpp"

namespace lc2kicad
{
  
  //void assertThrow(bool statement, const char* message){if(!statement){std::runtime_error e(message); errorAndQuit(&e);}}
  void assertThrow(const bool statement, const char* message) {if(!statement){throw std::runtime_error(message);}}
  void assertThrow(const bool statement, const std::string &message) {if(!statement){throw std::runtime_error(message.c_str());}}

  std::vector<std::string> splitString(std::string sourceString, char delimeter)
  {
    std::stringstream ss(sourceString);
    std::string item;
    std::vector<std::string> rtn;
    while(std::getline(ss, item, delimeter))
      rtn.push_back(item);
    return rtn;
  }

  
  void findAndReplaceString(std::string& subject, const std::string& search,const std::string& replace)
  {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
    }
  }

  std::string base_name(const std::string& path)
  {
    return path.substr(path.find_last_of("/\\") + 1);
  }

  std::string decToHex(long _decimal)
  {
    std::stringstream tmp;
    tmp << std::hex << _decimal;
    return tmp.str();
  }

  std::vector<std::string> splitByString (std::string& s, std::string &&delimiter)
  {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
    {
      token = s.substr(pos_start, pos_end - pos_start);
      pos_start = pos_end + delim_len;
      res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
  }
  
  coordslist* simpleLCSVGSegmentizer(const std::string &SVGPath, int arcResolution)
  {
    RAIIC<coordslist> ret;
    std::string workingPath = SVGPath; //Duplicate the original path
    size_t indexer = 0;
    coordinates subpathHead {0, 0}, penLocation {0, 0};
    bool relative = false;
    
    arcResolution < 2 ? arcResolution = 2 : 0;
    
    //Pre-processing for convenience
    //Add white space for the commands if needed. Also replace commas with white spaces.
    for(std::string::iterator it = workingPath.begin(); it != workingPath.end(); it++)
    {
      if(*it == ',')
        *it = ' ';
      if(std::strchr("AaCcHhLlMmQqSsTtVvZz", *it))
      {
        it++;
        it = workingPath.insert(it, ' ');
      }
    }
    
    std::cout << workingPath << std::endl;
    
    
    while(indexer < workingPath.size())
    {
      switch(workingPath[indexer])
      {
        case ' ': //Blank space, fetch next character
          indexer++;
          continue;

        case 'm': //Move to, relative
          relative = true;
        case 'M': //Move to, absolute
        {
          coordinates moveTo {0, 0};
          size_t xbegin = workingPath.find_first_not_of(' ', indexer), xend = workingPath.find_first_of(' ', xbegin) - 1;
          size_t ybegin = workingPath.find_first_not_of(' ', xend), yend = workingPath.find_first_of(' ', ybegin) - 1;
          indexer = yend + 1;
          moveTo.X = atof(workingPath.substr(xbegin, xend - xbegin + 1).c_str());
          moveTo.Y = atof(workingPath.substr(ybegin, yend - ybegin + 1).c_str());
          subpathHead = penLocation = (relative ? penLocation + moveTo : moveTo);
          break;
        }
        case 'l': //Line to, relative
          relative = true;
        case 'L': //Lint to, absolute
        {
          break;
        }
        default: //Unknown situation
          std::cout << ">>> Warning: error executing SVG path graph conversion, offset " << std::to_string(indexer)
                    << " of path \"" << workingPath << "\". This may cause problem.";
          indexer++;
          break;
        relative = false;
      }
    }
    
    return !++ret;
  }

}
