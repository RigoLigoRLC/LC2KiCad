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

#include <stdexcept>
#include <string>

namespace FloatInt
{
  class fpi32
  {
    private:
      int data;
      size_t digits;
    public:
      int getData() const { return data; }
      int getDigits() const { return digits; }
      std::string str() const;

      fpi32 operator+(fpi32 op); // Will expand digits to the larger one
      fpi32 operator-(fpi32 op);
      fpi32 operator*(fpi32 op);
      fpi32 operator/(fpi32 op);

      fpi32(int _data, int _digits) { data = _data, digits = _digits; }
      fpi32(std::string, int);
  };

  class fpi32_except : public std::logic_error
  {
    public:
      fpi32_except(const char *message) : std::logic_error(message) {};
      fpi32_except() : std::logic_error("Unspecified FPI32 error.") {};
  };
}
