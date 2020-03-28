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

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "consts.hpp"
#include "includes.hpp"

namespace lc2kicad
{
  
  //void assertThrow(bool statement, const char* message){if(!statement){std::runtime_error e(message); errorAndQuit(&e);}}
  void assertThrow(const bool statement, const char* message) {if(!statement){throw std::runtime_error(message);}}
  void assertThrow(const bool statement, const std::string &message) {if(!statement){throw std::runtime_error(message.c_str());}}

  const char gitCommitHash[] = "@GIT_SHA1@";

  std::vector<std::string> splitString(std::string sourceString, char delimeter)
  {
    std::stringstream ss(sourceString);
    std::string item;
    std::vector<std::string> rtn;
    while(std::getline(ss, item, delimeter))
      rtn.push_back(item);
    return rtn;
  }

  /**
   * LCLayerToKiCadLayer fucntion
   * 
   * @param LCLayer Layer ID in LCEDA
   * @return KiCad Layer ID.
   */
  int LCLayerToKiCadLayer(const int &LCLayer)
  {
    assertThrow(LCLayer <= 51, std::string("LCLayerToKiCadLayer: Invalid LC Layer number ") + std::to_string(LCLayer));
    return LCtoKiCadLayerLUT[LCLayer];
  }

  std::string LCLayerToKiCadName(const int &LCLayer)
  {
    return KiCadLayerNameLUT[LCLayerToKiCadLayer(LCLayer)];
  }
  
  void findAndReplaceString(std::string& subject, const std::string& search,const std::string& replace) {
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
  
  coordslist* simpleLCSVGSegmentizer(const std::string &SVGPath, int arcResolution)
  {
    coordslist *ret = new coordslist;
    size_t indexer = 0;
    coordinates subpathHead {0, 0}, penLocation {0, 0};
    bool relative = false;
    
    arcResolution < 2 ? arcResolution = 2 : 0;
    
    while(indexer < SVGPath.size())
    {
      switch(SVGPath[indexer])
      {
        case ' ': //Blank space, fetch next character
          indexer++;
          continue;

        case 'm': //Move to, relative
          relative = true;
        case 'M': //Move to, absolute
        {
          coordinates moveTo {0, 0};
          size_t xbegin = SVGPath.find_first_not_of(' ', indexer), xend = SVGPath.find_first_of(' ', xbegin) - 1;
          size_t ybegin = SVGPath.find_first_not_of(' ', xend), yend = SVGPath.find_first_of(' ', ybegin) - 1;
          indexer = yend + 1;
          moveTo.X = atof(SVGPath.substr(xbegin, xend - xbegin + 1).c_str());
          moveTo.Y = atof(SVGPath.substr(ybegin, yend - ybegin + 1).c_str());
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
                    << " of path \"" << SVGPath << "\". This may cause problem.";
          indexer++;
          break;
        relative = false;
      }
    }
    
    return ret;
  }

}
