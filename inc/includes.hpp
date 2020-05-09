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


#ifndef GLOBAL_FUNCS
  #define GLOBAL_FUNCS
  
  //#define MAKE_CUSTOM_TEST_OF_FUNCS
  
  #include "consts.hpp"
  
  #include <vector>
  #include <string>
  #include <map>
  #include <stdexcept>
  
  namespace lc2kicad
  {
    template <typename T> class RAIIC
    {
      public:
        RAIIC()
          { resource = new T; }
        RAIIC(T* ptr)
          { resource = ptr; }
        ~RAIIC()
          { if(!isProtected) delete resource; }
        RAIIC& operator++()
          { isProtected = true; return *this; }
        T& operator*()
          { return *resource; }
        T* operator->()
          { return resource; }
        T* operator!()
          { return resource; }
      private:
        T* resource;
        bool isProtected = false;
    };
    
    class coordinates
    {
      public:
        double X;
        double Y;
        
        coordinates() {}
        coordinates(double _X, double _Y)
          { X = _X, Y = _Y; }
        coordinates operator+(coordinates coord)
          { coordinates ret; ret.X = this->X + coord.X; ret.Y = this->Y + coord.Y; return ret; }
        coordinates operator-(coordinates coord)
          { coordinates ret; ret.X = this->X - coord.X; ret.Y = this->Y - coord.Y; return ret; }
        coordinates operator*(coordinates coord)
          { coordinates ret; ret.X = this->X * coord.X; ret.Y = this->Y * coord.Y; return ret; }
        coordinates operator/(coordinates coord)
          { coordinates ret; ret.X = this->X / coord.X; ret.Y = this->Y / coord.Y; return ret; }
        coordinates operator+(double n)
          { coordinates ret; ret.X = this->X + n; ret.Y = this->Y + n; return ret; }
        coordinates operator*(double n)
          { coordinates ret; ret.X = this->X * n; ret.Y = this->Y * n; return ret; }
    };

    typedef coordinates sizeXY;
    typedef std::vector<std::string> stringlist;
    typedef std::vector<coordinates> coordslist;
    typedef std::pair<std::string, double> str_dbl_pair;
    typedef std::pair<std::string, std::string> str_str_pair;
    typedef std::map<std::string, double> str_dbl_map;
    //typedef std::vector<str_dbl_pair> str_dbl_pairlist;
    typedef std::map<std::string, std::string> str_str_map;
    
    enum documentTypes {schematic = 1, schematic_lib = 2, pcb = 3, pcb_lib = 4, project = 5, sub_part = 6, spice_symbol = 7};

    struct programArgumentParseResult
    {
      bool invokeHelp = false, invokeVersionInfo = false;
      bool convertAsProject = false;
      bool useCompatibilitySwitches = false;
      str_dbl_map compatibilityOptions;
      stringlist filenames;
    };

    programArgumentParseResult programArgumentParser(const int&, const char**&);
    void errorAndQuit(std::runtime_error *e);
    void assertThrow(const bool statement, const char* message);  
    void assertThrow(const bool statement, const std::string &message);

    int LCLayerToKiCadLayer(const int&);
    std::string LCLayerToKiCadName(const int&);

    coordslist* simpleLCSVGSegmentizer(const std::string&, int);
    stringlist splitString(std::string sourceString, char delimeter);
    std::string base_name(const std::string& path);
    std::string decToHex(const long _decimal);
    void findAndReplaceString(std::string& subject, const std::string& search,const std::string& replace);
    
  }

#endif
