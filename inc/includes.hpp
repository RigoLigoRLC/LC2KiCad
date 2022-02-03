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

#ifndef GLOBAL_FUNCS
  #define GLOBAL_FUNCS

  #ifdef _WIN32
    #define USE_WINAPI_FOR_TEXT_COLOR // Comment this line out to use ANSI escape code coloring for Windows too
  #endif

  #define PI 3.14159265358979323846
  
  #include <vector>
  #include <string>
  #include <map>
  #include <stdexcept>
  #include <cmath>
  #include <functional>

  namespace lc2kicad
  {
  /*
   * Implementation of RAII, for those objects that should be created dynamically.
   *
   * Operators note:
   * operator++ for protecting the contents. RAIIC objects will not destroy internal data
   *            if it's protected when RAIIC object expires. This operation is irreversible.
   *
   * operator-- for increasing the reference count. RAIIC can be copy constructed, but it WILL NOT
   *            increase the reference count for you like a shared_ptr. You must do it manually
   *            when needed.
   *
   * operator! and
   * operator-> for obtaining the pointer to the dereferenced RAII-managed object.
   *
   * operator*  for obtaining a dereferenced RAII-managed object reference.
   *
   * In LC2KiCad, we usually use RAIIC objects like this:
   * When you have to create an object protected by RAIIC, use
   *
   *   RAIIC<CLASS_NAME> obj;
   *
   * When you're absolutely sure that this object can be released, and in LC2KiCad it's usually at
   * the end of a (de)serializer method, do
   *
   *   return !++obj;
   *
   * Thanks to C++ operator priority, you will protect it first, then return the pointer.
   *
   * If you'd like to replace the resource managed by RAIIC, invoke RAIIC::replace.
   * The previously protected resource pointer will be returned in case you need it.
   * This really should only happen when you initialized RAIIC with a nullptr so you can determine
   * the type of data to be put into it after on.
   */
    template <typename T> class RAIIC
    {
      public:
        RAIIC()
          { resource = new T; }
        RAIIC(T* ptr)
          { resource = ptr; }
        ~RAIIC()
          { if(!--refs)if(!isProtected) delete resource; }
        T* replace(T* a)
          { T* r = resource; resource = a; return r; }
        RAIIC& operator++()
          { isProtected = true; return *this; }
        T& operator*()
          { return *resource; }
        T* operator->()
          { return resource; }
        T* operator!()
          { return resource; }
        RAIIC& operator--()
          { refs++; return *this; }
      private:
        T* resource;
        unsigned short int refs = 1;
        bool isProtected = false;
    };
    
    class coordinates
    {
      public:
        double X;
        double Y;
        
        coordinates() { X = 0.0, Y = 0.0; }
        coordinates(double _X, double _Y)
          { X = _X, Y = _Y; }
        coordinates operator+(const coordinates coord)
          { coordinates ret; ret.X = this->X + coord.X; ret.Y = this->Y + coord.Y; return ret; }
        coordinates operator-(const coordinates coord)
          { coordinates ret; ret.X = this->X - coord.X; ret.Y = this->Y - coord.Y; return ret; }
        coordinates operator*(const coordinates coord)
          { coordinates ret; ret.X = this->X * coord.X; ret.Y = this->Y * coord.Y; return ret; }
        coordinates operator/(const coordinates coord)
          { coordinates ret; ret.X = this->X / coord.X; ret.Y = this->Y / coord.Y; return ret; }
        coordinates operator+=(coordinates coord)
          { this->X += coord.X; this->Y += coord.Y; return *this; }
        coordinates operator-=(coordinates coord)
          { this->X -= coord.X; this->Y -= coord.Y; return *this; }
        coordinates operator*=(coordinates coord)
          { this->X *= coord.X; this->Y *= coord.Y; return *this; }
        coordinates operator/=(coordinates coord)
          { this->X /= coord.X; this->Y /= coord.Y; return *this; }
        coordinates operator+(double n)
          { coordinates ret; ret.X = this->X + n; ret.Y = this->Y + n; return ret; }
        coordinates operator*(double n)
          { coordinates ret; ret.X = this->X * n; ret.Y = this->Y * n; return ret; }
        void swapXY() { double t; t = X; X = Y; Y = t; }
    };

    typedef coordinates sizeXY;
    typedef std::vector<std::string> stringlist;
    typedef std::vector<coordinates> coordslist;
    typedef std::pair<std::string, double> str_dbl_pair;
    typedef std::pair<std::string, std::string> str_str_pair;
    typedef std::map<std::string, double> str_dbl_map;
    //typedef std::vector<str_dbl_pair> str_dbl_pairlist;
    typedef std::map<std::string, std::string> str_str_map;

    struct centerArc
    {
      coordinates center;
      sizeXY size;
      double angleStart, angleExtend;
    };
    
    enum documentTypes { invalid = 0, schematic = 1, schematic_lib = 2, pcb = 3, pcb_lib = 4, project = 5, sub_part = 6, spice_symbol = 7 };

    struct programArgumentParseResult
    {
      static void verboseOutputArgParseResult(const programArgumentParseResult *result);
      bool invokeHelp = false, invokeVersionInfo = false;
      bool convertAsProject = false,
           useCompatibilitySwitches = false,
           exportNestedLibs = false,
           verboseInfo = false,
           usePipe = false;
      std::string configFile,
                  outputDirectory;
      str_dbl_map parserArguments;
      stringlist filenames;
    };

    programArgumentParseResult programArgumentParser(const int&, const char**&);
    void checkArgConflict(const programArgumentParseResult *);
    void assertThrow(const bool statement, const char* message);  
    void assertThrow(const bool statement, const std::string &message);

    //int EasyEdaToKiCadLayerMap\[\];
    std::string LCLayerToKiCadName(const int&);

    stringlist splitString(std::string sourceString, char delimeter);
    std::string base_name(const std::string& path);
    void sanitizeFileName(std::string &filename);
    std::string decToHex(const unsigned long long _decimal);
    void findAndReplaceString(std::string& subject, const std::string& search,const std::string& replace);
    std::string loadNthSeparated(std::string &s, char delimiter, unsigned int nth);
    int tolStoi(const std::string &, const int fail = 0);
    double tolStod(const std::string &, const double fail = 0.0);
    inline double toRadians(double degree) { return (degree / 180.0) * PI; }
    inline double toDegrees(double radian) { return (radian / PI) * 180.0; }
    bool fuzzyCompare(const double, const double);
    centerArc svgEllipticalArcComputation(double, double, double, double, double, bool, bool, double, double);
    std::vector<std::string> splitByString(const std::string&, std::string&&);
    std::string escapeQuotedString(const std::string);

    void Error(std::string s);
    void Warn(std::string s);
    void Info(std::string s);
    void InfoVerbose(std::function<std::string()> sf);
  }

  // Utility macros

#define VERBOSE_INFO(X) InfoVerbose([&]() -> std::string { return X; })

#define ASSERT_RETURN(X) if(!(X))return;
#define ASSERT_RETURN_MSG(X,Y) if(!(X)){lc2kicad::Error(Y);return;}
#define ASSERT_RETNULLPTR(X) if(!(X))return;
#define ASSERT_RETNULLPTR_MSG(X,Y) if(!(X)){lc2kicad::Error(Y);return nullptr;}
#define ASSERT_CONT_MSG(X,Y) if(!(X)){lc2kicad::Error(Y);continue;}

#endif
