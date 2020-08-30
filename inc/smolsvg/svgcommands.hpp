/*
    Copyright (c) 2020 RigoLigoRLC.

    This file is part of SmolSVG.

    SmolSVG is distributed under MIT License. You must use the source code of
    SmolSVG under the restrictions of MIT License.

    ABSOLUTE NO WARRANTY was promised.

    You can obtain a copy of MIT License here: https://opensource.org/licenses/MIT
*/
#ifndef SMOLSVG_SVGCOMMANDS_HPP_
#define SMOLSVG_SVGCOMMANDS_HPP_

#include <vector>
#include <cmath>

namespace SmolSVG
{
  struct SmolCoord
  {
    double X, Y;
    SmolCoord operator+(const SmolCoord &o) { return { this->X + o.X, this->Y + o.Y }; }
    SmolCoord operator-(const SmolCoord &o) { return { this->X - o.X, this->Y - o.Y }; }
    SmolCoord operator*(const double &o) { return { this->X * o, this->Y * o }; }
    void operator+=(const double &o) { X += o, Y += o; }
    void operator-=(const double &o) { X -= o; Y -= o; }
    void operator*=(const double &o) { X *= o, Y *= o; }
    double lengthToOrigin() const { return std::sqrt(X * X + Y * Y); }
    lc2kicad::coordinates lc2kicadCoord() { return lc2kicad::coordinates(X, Y); }
    SmolCoord(const double x, const double y) { X = x, Y = y; }
    SmolCoord() { X = Y = 0.0; }
    SmolCoord(lc2kicad::coordinates o) { X = o.X, Y = o.Y; } // Conversion constructor
  };

  class baseCommand
  {
      virtual std::vector<SmolCoord> linearize() = 0;

    public:
      baseCommand() { relative = false; }
      virtual ~baseCommand() = default;
      SmolCoord getConstStartPoint() { return pointFrom; }
      SmolCoord getConstEndPoint() { return pointTo; }
      virtual void scaleToOrigin(const double coeff) { pointFrom *= coeff, pointTo *= coeff; }

    protected:
      SmolCoord pointFrom, pointTo;
      bool relative;
  };

  class commandLineTo : public baseCommand
  {
    public:
      commandLineTo(const double pFromX, const double pFromY, const double pToX, const double pToY)
        { pointFrom.X = pFromX, pointFrom.Y = pFromY, pointTo.X = pToX, pointTo.Y = pToY; }
      commandLineTo(const SmolCoord &pFrom,  const SmolCoord &pTo) { pointFrom = pFrom, pointTo = pTo; }

      std::vector<SmolCoord> linearize()
      {
        std::vector<SmolCoord> ret { pointFrom, pointTo };
        return ret;
      }
  };

  class commandQuadraticBezierTo : public baseCommand
  {
      SmolCoord pointHandle;
    public:
      commandQuadraticBezierTo(const double pFromX, const double pFromY, const double pHandleX, const double pHandleY,
                               const double pToX, const double pToY)
      {
        pointFrom.X = pFromX, pointFrom.Y = pFromY, pointHandle.X = pHandleX, pointHandle.Y = pHandleY,
        pointTo.X = pToX, pointTo.Y = pToY;
      }
      commandQuadraticBezierTo(const SmolCoord &pFrom, const SmolCoord &pHandle, const SmolCoord &pTo)
        { pointFrom = pFrom, pointHandle = pHandle, pointTo = pTo; }
        void scaleToOrigin(const double coeff) override { pointFrom *= coeff, pointTo *= coeff, pointHandle *= coeff; }

      std::vector<SmolCoord> linearize()
      {
        std::vector<SmolCoord> ret; // TODO stub quadratic bezier
        unsigned long segmentCount = (pointHandle - pointFrom).lengthToOrigin() + (pointTo - pointFrom).lengthToOrigin();
        ret.emplace_back(pointFrom);
        for(double t = 1 / segmentCount; t < 1; t += 1 / segmentCount)
        {
          // t*(b-a)+a -> t*b-t*a+a -> t*b+(1-t)*a
          SmolCoord i1 = pointHandle * t + pointFrom * (1 - t), i2 = pointTo * t + pointHandle * (1 - t);
          ret.emplace_back(i1 * t + i2 * (1 - t));
        }
        ret.emplace_back(pointTo);
        return ret;
      }
  };

  class commandCubicBezierTo : public baseCommand
  {
      SmolCoord pointHandleA, pointHandleB;
    public:
      commandCubicBezierTo(const double pFromX, const double pFromY, const double pHandleAX, const double pHandleAY,
                           const double pHandleBX, const double pHandleBY, const double pToX, const double pToY)
      {
        pointFrom.X = pFromX, pointFrom.Y = pFromY, pointHandleA.X = pHandleAX, pointHandleA.Y = pHandleAY,
        pointHandleB.X = pHandleBX, pointHandleB.Y = pHandleBY, pointTo.X = pToX, pointTo.Y = pToY;
      }
      commandCubicBezierTo(const SmolCoord &pFrom, const SmolCoord &pHandleA, const SmolCoord &pHandleB,
                           const SmolCoord &pTo)
      { pointFrom = pFrom, pointHandleA = pHandleA, pointHandleB = pHandleB, pointTo = pTo; }
      void scaleToOrigin(const double coeff) override
      { pointFrom *= coeff, pointTo *= coeff, pointHandleA *= coeff, pointHandleB *= coeff; }

      std::vector<SmolCoord> linearize()
      {
        std::vector<SmolCoord> ret; // TODO stub cubic bezier
        unsigned long segmentCount =  (pointHandleA - pointFrom).lengthToOrigin() + // Begin-A distance
                                      (pointHandleB - pointHandleA).lengthToOrigin() + // A-B distance
                                      (pointTo - pointHandleB).lengthToOrigin(); // B-End distance
        ret.emplace_back(pointFrom);
        for(double t = 1 / segmentCount; t < 1; t += 1 / segmentCount)
        {
          SmolCoord i1 = pointHandleA * t + pointFrom * (1 - t), i2 = pointHandleB * t + pointHandleA * (1 - t),
                  i3 = pointTo * t + pointHandleB * (1 - t);
          i1 = i2 * t + i1 * (1 - t), i2 = i3 * t + i2 * (1 - t);
          ret.emplace_back(i1 * t + i2 * (1 - t));
        }
        ret.emplace_back(pointTo);
        return ret;
      }
  };

  class commandEllipticalArcTo : public baseCommand
  {
      SmolCoord radii;
      double XAxisRotation;
      bool flagLargeArc, flagSweep;
    public:
      commandEllipticalArcTo(const double pFromX, const double pFromY, const double pRadiiX, const double pRadiiY,
                             const double XAxisRot, const bool largeArc, const bool sweep, const double pToX,
                             const double pToY)
      {
        pointFrom.X = pFromX, pointFrom.Y = pFromY, radii.X = pRadiiX, radii.Y = pRadiiY, XAxisRotation = XAxisRot,
        flagLargeArc = largeArc, flagSweep = sweep, pointTo.X = pToX, pointTo.Y = pToY;
      }
      commandEllipticalArcTo(const SmolCoord &pFrom, const SmolCoord pRadii, const double XAxisRot, const bool largeArc,
                             const bool sweep, const SmolCoord pTo)
      {
        pointFrom = pFrom, radii = pRadii, XAxisRotation = XAxisRot, flagLargeArc = largeArc, flagSweep = sweep,
        pointTo = pTo;
      }

      const SmolCoord getRadii() { return radii; }
      const double getXAxisRotation() { return XAxisRotation; }
      const bool getLargeArc() { return flagLargeArc; }
      const bool getFlagSweep() { return flagSweep; }

      std::vector<SmolCoord> linearize()
      {
        std::vector<SmolCoord> ret; // TODO stub elliptical arc
        unsigned long segmentCount = radii.lengthToOrigin();

        return ret;
      }
  };

  class commandCentralArc : public baseCommand
  {
      SmolCoord pointCenter, size;
      double rotation;
  };
}

#endif