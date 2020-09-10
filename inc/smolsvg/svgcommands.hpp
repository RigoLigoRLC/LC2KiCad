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

#include <list>
#include <cmath>

namespace SmolSVG
{
  enum commandType
  { Invalid, MoveTo, ClosePath, LineTo, HorizontalTo, VerticalTo, CurveTo, SmoothTo, QuadTo, SmoothQuadTo, ArcTo };

  struct SmolCoord
  {
    double X, Y;
    SmolCoord operator+(const SmolCoord &o) const { return { this->X + o.X, this->Y + o.Y }; }
    SmolCoord operator-(const SmolCoord &o) const { return { this->X - o.X, this->Y - o.Y }; }
    SmolCoord operator*(const double &o) const { return { this->X * o, this->Y * o }; }
    void operator+=(const double &o) { X += o, Y += o; }
    void operator-=(const double &o) { X -= o; Y -= o; }
    void operator*=(const double &o) { X *= o, Y *= o; }
    operator lc2kicad::coordinates() const { return {X, Y}; }
    double lengthToOrigin() const { return std::sqrt(X * X + Y * Y); }
    lc2kicad::coordinates nativeCoord() const { return lc2kicad::coordinates(X, Y); }
    SmolCoord(const double x, const double y) { X = x, Y = y; }
    SmolCoord() { X = Y = 0.0; }
  };

  class baseCommand
  {
      virtual std::list<SmolCoord> linearize() = 0;

    protected:
      SmolCoord pointFrom, pointTo;
      bool relative;
      commandType cmdType = Invalid;
    public:
      baseCommand() { relative = false; }
      virtual ~baseCommand() = default;
      const SmolCoord& getConstStartPoint() { return pointFrom; }
      const SmolCoord& getConstEndPoint() { return pointTo; }
      virtual void scaleToOrigin(const double coeff) { pointFrom *= coeff, pointTo *= coeff; }
      const commandType type() { return cmdType; }

  };

  class commandLineTo : public baseCommand
  {
    public:
      commandLineTo(const double pFromX, const double pFromY, const double pToX, const double pToY)
        { pointFrom.X = pFromX, pointFrom.Y = pFromY, pointTo.X = pToX, pointTo.Y = pToY; cmdType = LineTo; }
      commandLineTo(const SmolCoord &pFrom, const SmolCoord &pTo)
        { pointFrom = pFrom, pointTo = pTo; cmdType = LineTo; }

      std::list<SmolCoord> linearize()
      {
        std::list<SmolCoord> ret { pointFrom, pointTo };
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
        cmdType = QuadTo;
      }

      commandQuadraticBezierTo(const SmolCoord &pFrom, const SmolCoord &pHandle, const SmolCoord &pTo)
        { pointFrom = pFrom, pointHandle = pHandle, pointTo = pTo; cmdType = QuadTo; }
        void scaleToOrigin(const double coeff) override { pointFrom *= coeff, pointTo *= coeff, pointHandle *= coeff; }

      const SmolCoord getHandle() const { return pointHandle; }

      std::list<SmolCoord> linearize()
      {
        std::list<SmolCoord> ret; // TODO stub quadratic bezier
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
        cmdType = CurveTo;
      }

      commandCubicBezierTo(const SmolCoord &pFrom, const SmolCoord &pHandleA, const SmolCoord &pHandleB,
                           const SmolCoord &pTo)
      { pointFrom = pFrom, pointHandleA = pHandleA, pointHandleB = pHandleB, pointTo = pTo; cmdType = CurveTo; }

      void scaleToOrigin(const double coeff) override
      { pointFrom *= coeff, pointTo *= coeff, pointHandleA *= coeff, pointHandleB *= coeff; }

      const SmolCoord getHandleA() const { return pointHandleA; }
      const SmolCoord getHandleB() const { return pointHandleB; }

      std::list<SmolCoord> linearize()
      {
        std::list<SmolCoord> ret; // TODO stub cubic bezier
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
        cmdType = ArcTo;
      }
      commandEllipticalArcTo(const SmolCoord &pFrom, const SmolCoord pRadii, const double XAxisRot, const bool largeArc,
                             const bool sweep, const SmolCoord pTo)
      {
        pointFrom = pFrom, radii = pRadii, XAxisRotation = XAxisRot, flagLargeArc = largeArc, flagSweep = sweep,
        pointTo = pTo;
        cmdType = ArcTo;
      }

      const SmolCoord getRadii() { return radii; }
      const double getXAxisRotation() { return XAxisRotation; }
      const bool getLargeArc() { return flagLargeArc; }
      const bool getFlagSweep() { return flagSweep; }

      std::list<SmolCoord> linearize()
      {
        std::list<SmolCoord> ret; // TODO stub elliptical arc
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