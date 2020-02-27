/*
    Copyright (c) 2020 RigoLigoRLC, BadLuckW and all other LC2KiCad contributers.

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

  void errorAndQuit(std::runtime_error *e)
  #ifdef ERROR_EXIT
    { std::cout << "Error running the program: " << e->what() << std::endl << std::endl << "The intended operation cannot be done. The application will quit.\n"; exit(1); }
  #else
    #ifdef ERROR_ABORT
      { std::cout << "Runtime error: " << e->what() << std::endl << std::endl << "The intended operation cannot be done. The application will quit.\n"; abort(); }
    #endif
  #endif
  
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
  
  coordslist* simpleLCSVGSegmentizer(const std::string &SVGPath, int arcResolution)
  {
    coordslist *ret = new coordslist;
    stringlist pathPartials = splitString(SVGPath, ' ');
    
    arcResolution < 2 ? arcResolution = 2 : 0;
    
    return ret;
  }

}
