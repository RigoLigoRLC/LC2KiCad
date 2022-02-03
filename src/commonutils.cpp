/*
    Copyright (c) 2020 RigoLigoRLC.
    Copyright 2001-2003  The Apache Software Foundation,
              2019-2020 Wokwi (for lc2kicad::svgEllipticalArcComputation).

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

    Further notice on lc2kicad::svgEllipticalArcComputation function:
    This function is ported from Wokwi's easyeda2kicad project, which is
    based on Apache Batik code. See function implementation for details.
    This part of code is licensed under Apache 2.0 license.
*/
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "consts.hpp"
#include "includes.hpp"

#ifdef USE_WINAPI_FOR_TEXT_COLOR
#include <windows.h>
#endif

namespace lc2kicad
{
  extern programArgumentParseResult argParseResult;
  extern long errorCount, warningCount;
  extern std::ostream *logstream;
#ifdef USE_WINAPI_FOR_TEXT_COLOR
  extern HANDLE hStdOut;
  extern CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
  extern WORD wBackgroundColor;
#endif
  
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

  std::string decToHex(const unsigned long long _decimal)
  {
    std::stringstream tmp;
    tmp << std::hex << _decimal;
    return tmp.str();
  }

  std::vector<std::string> splitByString (const std::string& s, std::string &&delimiter)
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

  std::string loadNthSeparated(std::string &s, char delimiter, unsigned int nth)
  {
    std::stringstream ss(s);
    std::string ret;
    for(unsigned int i = 0; i <= nth; i++)
      std::getline(ss, ret, delimiter);
    return ret;
  }

  void sanitizeFileName(std::string &filename)
  {
    for(std::string::iterator it = filename.begin(); it < filename.end(); it++)
    {
        bool found = illegalCharsOfFilenames.find(*it) != std::string::npos;
        if(found)
            *it = '_';
    }
  }

  std::string escapeQuotedString(const std::string s)
  {
    std::string ret;
    ret.reserve(s.size() + 5); // Reserve memory for the possible escape characters
    for(auto it = s.cbegin(); it < s.cend(); it++)
    {
      if(*it == '"')
        ret += '\\';
      ret += *it;
    }
    return ret;
  }

  int tolStoi(const std::string &c1, const int fail)
  {
    if(c1.size())
      return std::stoi(c1);
    else
      return fail;
  }

  double tolStod(const std::string &c1, const double fail)
  {
    if(c1.size())
      return std::stod(c1);
    else
      return fail;
  }

  bool fuzzyCompare(const double a, const double b)
  {
    return std::abs(a - b) < 1e-6;
  }

  centerArc svgEllipticalArcComputation(double x0, double y0, double rx, double ry, double angle,
                                         bool largeArcFlag, bool sweepFlag, double x, double y)
  {
    //
    // Elliptical arc implementation based on the SVG specification notes
    //
    // This function is ported from wokwi/easyeda2kicad project.
    // Original: https://github.com/wokwi/easyeda2kicad/blob/master/src/svg-arc.ts

    if(fuzzyCompare(x0, x) && fuzzyCompare(y0, y))
      return {{ x0, y0 }, { 0, 0 }, 0, 360 };
    // Compute the half distance between the current and the final point
    double dx2 = (x0 - x) / 2.0,
           dy2 = (y0 - y) / 2.0;
    // Convert angle from degrees to radians
    angle = toRadians(std::fmod(angle, 360.0));
    double cosAngle = std::cos(angle),
           sinAngle = std::sin(angle);

    //
    // Step 1 : Compute (x1, y1)
    //
    double x1 = cosAngle * dx2 + sinAngle * dy2,
           y1 = -sinAngle * dx2 + cosAngle * dy2;
    // Ensure radii are large enough
    rx = std::abs(rx);
    ry = std::abs(ry);
    double Prx = rx * rx,
           Pry = ry * ry,
           Px1 = x1 * x1,
           Py1 = y1 * y1,
           radiiCheck = Px1 / Prx + Py1 / Pry;
    // check that radii are large enough
    if (radiiCheck > 1) {
      rx = std::sqrt(radiiCheck) * rx;
      ry = std::sqrt(radiiCheck) * ry;
      Prx = rx * rx;
      Pry = ry * ry;
    }

    //
    // Step 2 : Compute (cx1, cy1)
    //
    short sign = largeArcFlag == sweepFlag ? -1 : 1;
    double sq = (Prx * Pry - Prx * Py1 - Pry * Px1) / (Prx * Py1 + Pry * Px1);
    sq = sq < 0 ? 0 : sq;
    double coef = sign * std::sqrt(sq),
           cx1 = coef * ((rx * y1) / ry),
           cy1 = coef * -((ry * x1) / rx);

    //
    // Step 3 : Compute (cx, cy) from (cx1, cy1)
    //
    double sx2 = (x0 + x) / 2.0,
           sy2 = (y0 + y) / 2.0,
           cx = sx2 + (cosAngle * cx1 - sinAngle * cy1),
           cy = sy2 + (sinAngle * cx1 + cosAngle * cy1);

    //
    // Step 4 : Compute the angleStart (angle1) and the angleExtent (dangle)
    //
    double ux = (x1 - cx1) / rx,
           uy = (y1 - cy1) / ry,
           vx = (-x1 - cx1) / rx,
           vy = (-y1 - cy1) / ry;
    // Compute the angle start
    double n = std::sqrt(ux * ux + uy * uy),
           p = ux; // (1 * ux) + (0 * uy)
    sign = uy < 0 ? -1 : 1;
    double angleStart = toDegrees(sign * std::acos(p / n));

    // Compute the angle extent
    n = std::sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
    p = ux * vx + uy * vy;
    sign = ux * vy - uy * vx < 0 ? -1 : 1;
    double angleExtent = toDegrees(sign * std::acos(p / n));
    if (!sweepFlag && angleExtent > 0) {
      angleExtent -= 360;
    } else if (sweepFlag && angleExtent < 0) {
      angleExtent += 360;
    }
    angleExtent = std::fmod(angleExtent, 360.0);
    angleStart = std::fmod(angleStart, 360.0);

    //
    // We can now build the resulting Arc2D in double precision
    //
    return { { cx, cy }, { rx * 2.0, ry * 2.0 }, angleStart, angleExtent };
  }
  
  void Error(std::string s)
  {
#ifdef USE_WINAPI_FOR_TEXT_COLOR
    GetConsoleScreenBufferInfo(hStdOut, &consoleInfo);
    wBackgroundColor = consoleInfo.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
    SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_INTENSITY | wBackgroundColor);
    *logstream << "Error: " << s << std::endl;
    SetConsoleTextAttribute(hStdOut, consoleInfo.wAttributes);
#else
    *logstream << "\033[1;31mError: " << s << "\033[39m\n";
#endif
    errorCount++;
  }

  void Warn(std::string s)
  {
#ifdef USE_WINAPI_FOR_TEXT_COLOR
    GetConsoleScreenBufferInfo(hStdOut, &consoleInfo);
    wBackgroundColor = consoleInfo.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
    SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | wBackgroundColor);
    *logstream << "Warning: " << s << std::endl;
    SetConsoleTextAttribute(hStdOut, consoleInfo.wAttributes);
#else
    *logstream << "\033[1;93mWarning: " << s << "\033[39m\n";
#endif
    warningCount++;
  }

  void Info(std::string s)
  {
#ifdef USE_WINAPI_FOR_TEXT_COLOR
    GetConsoleScreenBufferInfo(hStdOut, &consoleInfo);
    wBackgroundColor = consoleInfo.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY );
    SetConsoleTextAttribute(hStdOut, FOREGROUND_BLUE | FOREGROUND_INTENSITY | wBackgroundColor);
    *logstream << "Info: " << s << std::endl;
    SetConsoleTextAttribute(hStdOut, consoleInfo.wAttributes);
#else
    *logstream << "\033[1;96mInfo: " << s << "\033[39m\n";
#endif
  }

  void InfoVerbose(std::function<std::string()> sf)
  {
    if(!argParseResult.verboseInfo) return;
    Info(sf());
  }
}
