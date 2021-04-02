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

#include "floatint.hpp"

namespace FloatInt
{
  fpi32::fpi32(std::string str, int _digits)
  {
    int decimalPointPos = -1, length = str.length();
    for(unsigned int i = 0; i < length; i++)
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
      for(int i = 0; i < decimalPointPos + _digits + 1 - length; i++)
        str.push_back('0'); // Add zeros if not enough
      for(int i = 0; i < length - decimalPointPos - _digits - 1; i++)
        str.pop_back(); // Trim the end
      str.erase(decimalPointPos, 1);
    }
    else
    {
      for(int i = 0; i < _digits; i++)
        str.push_back('0');
    }

    data = std::stoi(str);
    digits = _digits;
  }

  std::string fpi32::str() const
  {
    std::string ret;

    ret = std::to_string(data);
    if(digits != 0)
    {
      while(ret.length() < digits + 1)
      ret = '0' + ret;
      ret.insert(ret.length() - digits, ".");
    }

    return ret;
  }

  fpi32 fpi32::operator+(fpi32 op)
  {
    int smallerdigit, _data, _ldata,
        largerdigit = (unsigned int)op.getDigits() > digits ?
                      (smallerdigit = digits, _data = data, _ldata = op.getData(), op.getDigits()) :
                      (smallerdigit = op.getDigits(), _data = op.getData(), _ldata = data, digits),
        _digits = largerdigit;

    for(int i = 0; i < largerdigit - smallerdigit; i++)
      _data *= 10;
    _data += _ldata;

    return fpi32(_data, _digits);
  }

  fpi32 fpi32::operator-(fpi32 op)
  {
    int smallerdigit, _data = data, _ldata = op.getData(), *mul,
        largerdigit = (unsigned int)op.getDigits() > digits ?
                      (smallerdigit = digits, mul = &_data, op.getDigits()) :
                      (smallerdigit = op.getDigits(), mul = &_ldata, digits),
        _digits = largerdigit;

    for(int i = 0; i < largerdigit - smallerdigit; i++)
      *mul *= 10;
    _data -= _ldata;

    return fpi32(_data, _digits);
  }

  fpi32 fpi32::operator*(fpi32 op)
  {
    return fpi32(data * op.getData(), digits + op.getDigits());
  }

  fpi32 fpi32::operator/(fpi32 op)
  {
    if(op.getData()==0) throw fpi32_except("FPI32 error: divided by zero.");
    int smallerdigit, _data = data, _ldata = op.data,
        largerdigit = (unsigned int)op.getDigits() > digits ?
                      (smallerdigit = digits, op.getDigits()) :
                      (smallerdigit = op.getDigits(), digits),
        _digits = largerdigit;

    for(int i = 0; i < largerdigit - smallerdigit; i++)
      _data *= 10;
    _data /= _ldata;

    return fpi32(_data, _digits);
  }
}
