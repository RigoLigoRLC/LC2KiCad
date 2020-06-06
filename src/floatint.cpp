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

#include "floatint.hpp"

namespace FloatInt
{
  fpi32::fpi32(std::string str, int _digits)
  {
    int decimalPointPos = -1;
    for(unsigned int i = 0; i < str.length(); i++)
      if(str[i] == '.')
      {
        if(decimalPointPos == -1)
          decimalPointPos = i;
        else
          throw std::logic_error("FPI32 error: more than one decimal point input.");
      }
    //if(str[0] == '-') // If negative number
    //  digits++;
    if(decimalPointPos != -1) // Decimal point detected?
    {
      // Normalize
      for(unsigned int i = 0; i < decimalPointPos + _digits + 2 - str.length(); i++)
        str.push_back('0'); // Add zeros if not enough
      for(unsigned int i = 0; i < str.length() - decimalPointPos - _digits - 1; i++)
        str.pop_back(); // Trim the end
    }
    else
    {
      for(int i = 0; i < _digits; i++)
        str.push_back('0');
    }

    str.erase(decimalPointPos, 1);

    data = std::stoi(str);
    digits = _digits;
  }
}
