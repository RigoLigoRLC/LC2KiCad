/*
    Copyright (c) 2020 RigoLigoRLC.

    This file is part of SmolSVG.

    SmolSVG is distributed under MIT License. You must use the source code of
    SmolSVG under the restrictions of MIT License.

    ABSOLUTE NO WARRANTY was promised.

    You can obtain a copy of MIT License here: https://opensource.org/licenses/MIT
*/

#ifndef SMOLSVG_PATHREADER_H
#define SMOLSVG_PATHREADER_H

#include "svgpath.hpp"
#include <string>
#include <vector>

namespace SmolSVG
{
  SVGRawPath *readPathString(std::string &pathStr);

  static SmolCoord realCoord(const bool relative, const SmolCoord &pen, const double X, const double Y)
  {
    return relative ? (SmolCoord(X, Y) + pen) : SmolCoord(X, Y);
  }

  static SmolCoord realCoord(const bool relative, const SmolCoord &pen, const std::vector<double> &argCache,
                             size_t coordPos)
  {
    return relative ?
                      (SmolCoord(argCache[coordPos], argCache[coordPos + 1]) + pen) :
                      SmolCoord(argCache[coordPos], argCache[coordPos + 1]);
  }

  SVGRawPath *readPathString(std::string &pathStr)
  {
    SVGRawPath *retptr = new SVGRawPath;
    SVGRawPath &ret = *retptr;
    SmolCoord PenLocation, CloseAt;
    std::vector<double> argCache;

    std::string::const_iterator cutBegin;
    unsigned int remainingArgCount = 0, commandArgCount = 0;
    bool relativeToken = false, finishAfterGeneration = false;
    enum { SkipSpacer, ReadCommand, ReadArgs, GenerateCommandObject } status = SkipSpacer;
    commandType commandToken;

    auto endByte = pathStr.cend() - 1;

    for(std::string::const_iterator i = pathStr.cbegin(); i < pathStr.cend(); )
    {
      switch(status)
      {
        case SkipSpacer:
          if(*i == ' ' || *i == ',')
            i++;
          else if(*i >= 'A' && *i <= 'z')
          {
            if(!remainingArgCount)
              status = ReadCommand;
            else
              throw std::logic_error("Unexpected command amongst arguments");
          }
          else if(*i >= '-' && *i <= '9')
          {
            if(remainingArgCount == 0)
            {
              remainingArgCount = commandArgCount;
              argCache.clear();
            }
            status = ReadArgs;
            cutBegin = i;
          }
          break;

        case ReadCommand:
          argCache.clear();
          switch(*i)
          {
            case 'm':
              relativeToken = true;
              commandToken = MoveTo;
              remainingArgCount = commandArgCount = 2;
              status = SkipSpacer;
              break;

            case 'M':
              relativeToken = false;
              commandToken = MoveTo;
              remainingArgCount = commandArgCount = 2;
              status = SkipSpacer;
              break;

            case 'z':
              relativeToken = true;
              commandToken = ClosePath;
              remainingArgCount = commandArgCount = 0;
              status = GenerateCommandObject; // Because 'z' token has to be closed immediately
              break;

            case 'Z':
              relativeToken = false;
              commandToken = ClosePath;
              remainingArgCount = commandArgCount = 0;
              status = GenerateCommandObject;
              break;

            case 'l':
              relativeToken = true;
              commandToken = LineTo;
              remainingArgCount = commandArgCount = 2;
              status = SkipSpacer;
              break;

            case 'L':
              relativeToken = false;
              commandToken = LineTo;
              remainingArgCount = commandArgCount = 2;
              status = SkipSpacer;
              break;

            case 'v':
              relativeToken = true;
              commandToken = VerticalTo;
              remainingArgCount = commandArgCount = 1;
              status = SkipSpacer;
              break;

            case 'V':
              relativeToken = false;
              commandToken = VerticalTo;
              remainingArgCount = commandArgCount = 1;
              status = SkipSpacer;
              break;

            case 'h':
              relativeToken = true;
              commandToken = HorizontalTo;
              remainingArgCount = commandArgCount = 1;
              status = SkipSpacer;
              break;

            case 'H':
              relativeToken = false;
              commandToken = HorizontalTo;
              remainingArgCount = commandArgCount = 1;
              status = SkipSpacer;
              break;

            case 'c':
              relativeToken = true;
              commandToken = CurveTo;
              remainingArgCount = commandArgCount = 6;
              status = SkipSpacer;
              break;

            case 'C':
              relativeToken = false;
              commandToken = CurveTo;
              remainingArgCount = commandArgCount = 6;
              status = SkipSpacer;
              break;

            case 's':
              relativeToken = true;
              commandToken = SmoothTo;
              remainingArgCount = commandArgCount = 4;
              status = SkipSpacer;
              break;

            case 'S':
              relativeToken = false;
              commandToken = SmoothTo;
              remainingArgCount = commandArgCount = 4;
              status = SkipSpacer;
              break;

            case 'q':
              relativeToken = true;
              commandToken = QuadTo;
              remainingArgCount = commandArgCount = 4;
              status = SkipSpacer;
              break;

            case 'Q':
              relativeToken = false;
              commandToken = QuadTo;
              remainingArgCount = commandArgCount = 4;
              status = SkipSpacer;
              break;

            case 't':
              relativeToken = true;
              commandToken = SmoothQuadTo;
              remainingArgCount = commandArgCount = 2;
              status = SkipSpacer;
              break;

            case 'T':
              relativeToken = false;
              commandToken = SmoothQuadTo;
              remainingArgCount = commandArgCount = 2;
              status = SkipSpacer;
              break;

            case 'a':
              relativeToken = true;
              commandToken = ArcTo;
              remainingArgCount = commandArgCount = 7;
              status = SkipSpacer;
              break;

            case 'A':
              relativeToken = false;
              commandToken = ArcTo;
              remainingArgCount = commandArgCount = 7;
              status = SkipSpacer;
              break;

            default:
              throw std::logic_error("Unknown or invalid SVG command");
          }

          if(i + 1 == pathStr.cend())
            status = GenerateCommandObject;
          else
            i++;

          break;

        case ReadArgs:
        {
          bool exitSwitch = false;
          while(*i >= '-' && *i <= '9')
            if(i == endByte)
            {
              status = GenerateCommandObject;
              finishAfterGeneration = true;
              break;
            }
            else
              i++;
          argCache.emplace_back(std::stod(std::string(cutBegin, i + 1)));
          remainingArgCount--;
          if(remainingArgCount)
            status = SkipSpacer;
          else
            status = GenerateCommandObject;
          break;
        }

        case GenerateCommandObject:
        {
          baseCommand *lastCommand;

          switch(commandToken)
          {
            case MoveTo:
              CloseAt = PenLocation = { argCache[0], argCache[1] };
              break;
            case ClosePath:
              ret.addRawCommand(new commandLineTo(PenLocation, CloseAt));
              PenLocation = CloseAt;
              i++; // Because it doesn't have args, no one will increment i for it
              break;
            case LineTo:
              ret.addRawCommand(new commandLineTo(PenLocation,
                                          realCoord(relativeToken, PenLocation, argCache, 0)));
              PenLocation = ret.getLastCommand()->getConstEndPoint();
              break;
            case VerticalTo:
              ret.addRawCommand(new commandLineTo(PenLocation,
                                          realCoord(relativeToken, PenLocation, relativeToken ? 0 : PenLocation.X, argCache[0])));
              PenLocation = ret.getLastCommand()->getConstEndPoint();
              break;
            case HorizontalTo:
              ret.addRawCommand(new commandLineTo(PenLocation,
                                          realCoord(relativeToken, PenLocation, argCache[0], relativeToken ? 0 : PenLocation.Y)));
              PenLocation = ret.getLastCommand()->getConstEndPoint();
              break;
            case CurveTo:
              ret.addRawCommand(new commandCubicBezierTo(PenLocation,
                                                 realCoord(relativeToken, PenLocation, argCache, 0),
                                                 realCoord(relativeToken, PenLocation, argCache, 2),
                                                 realCoord(relativeToken, PenLocation, argCache, 4)));
              PenLocation = ret.getLastCommand()->getConstEndPoint();
              break;
            case SmoothTo:
              lastCommand = ret.getLastCommand();
              if(lastCommand->type() == CurveTo)
                ret.addRawCommand(new commandCubicBezierTo(PenLocation,
                                                   PenLocation * 2 - dynamic_cast<commandCubicBezierTo *>(lastCommand)->getHandleB(),
                                                   realCoord(relativeToken, PenLocation, argCache, 0),
                                                   realCoord(relativeToken, PenLocation, argCache, 2)));
              else
                ret.addRawCommand(new commandCubicBezierTo(PenLocation,
                                                   PenLocation * 2 - lastCommand->getConstEndPoint(),
                                                   realCoord(relativeToken, PenLocation, argCache, 0),
                                                   realCoord(relativeToken, PenLocation, argCache, 2)));
              PenLocation = ret.getLastCommand()->getConstEndPoint();
              break;
            case QuadTo:
              ret.addRawCommand(new commandQuadraticBezierTo(PenLocation,
                                                     realCoord(relativeToken, PenLocation, argCache, 0),
                                                     realCoord(relativeToken, PenLocation, argCache, 2)));
              PenLocation = ret.getLastCommand()->getConstEndPoint();
              break;
            case SmoothQuadTo:
              lastCommand = ret.getLastCommand();
              if(lastCommand->type() == QuadTo)
                ret.addRawCommand(new commandQuadraticBezierTo(PenLocation,
                                                       PenLocation * 2 - dynamic_cast<commandQuadraticBezierTo *>(lastCommand)->getHandle(),
                                                       realCoord(relativeToken, PenLocation, argCache, 0)));
              else
                ret.addRawCommand(new commandQuadraticBezierTo(PenLocation,
                                                       PenLocation * 2 - lastCommand->getConstEndPoint(),
                                                       realCoord(relativeToken, PenLocation, argCache, 0)));
              PenLocation = ret.getLastCommand()->getConstEndPoint();
              break;
            case ArcTo:
              ret.addRawCommand(new commandEllipticalArcTo(PenLocation,
                                                   { argCache[0], argCache[1] },
                                                   argCache[2], argCache[3] == 1.0, argCache[4] == 1.0,
                                                   realCoord(relativeToken, PenLocation, argCache, 5)));
              PenLocation = ret.getLastCommand()->getConstEndPoint();
              break;
            default:
              throw std::logic_error("Unknown token enumeration");
          }
          status = SkipSpacer;
          if(finishAfterGeneration)
            i++; // Args at end
          break;
        }
        default:;
      }

    }

    return retptr;
  }
}

#endif //INC_0005_SMOLSVG_PATHREADER_H
