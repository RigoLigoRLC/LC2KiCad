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

#include "includes.hpp"
#include "consts.hpp"

using std::vector;

namespace lc2kicad
{
  coordinates middlepoint(coordinates coord1, coordinates coord2, double t)
  {
    return (coord1 - coord2) * t + coord1;
  }

  /**
   * Control points are in the order of P1 C1 ... Cn P2.
   * At least 3 iterations are required for a proper rendering.
   */
  coordslist* render_bezier_curve(coordslist& control_points, int iterations)
  {
    if(iterations < 2)
      throw "Invalid iterations for Bezier curve renderer.";

    coordslist cp, *ret = new coordslist;
    for(int i = 0; i < control_points.size(); i++)
      cp[i] = control_points[i];
    
    double t = 0;
    (*ret)[0] = cp[0];
    for(int i = 1; i < iterations; i++)
    {
      t = double (i) / 1.0;
      (*ret)[1] = middlepoint(cp[i], cp[i + 1], t);
    }
    (*ret).push_back(cp[cp.size() - 1]);
    return ret;
  }
}