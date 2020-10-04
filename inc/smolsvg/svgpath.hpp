/*
    Copyright (c) 2020 RigoLigoRLC.

    This file is part of SmolSVG.

    SmolSVG is distributed under MIT License. You must use the source code of
    SmolSVG under the restrictions of MIT License.

    ABSOLUTE NO WARRANTY was promised.

    You can obtain a copy of MIT License here: https://opensource.org/licenses/MIT
*/

#ifndef SMOLSVG_SVGPATH_HPP
#define SMOLSVG_SVGPATH_HPP

#include "svgcommands.hpp"

namespace SmolSVG
{
  class SVGRawPath
  {
      std::list<baseCommand*> commandStorage;
    public:
      std::list<baseCommand *>::const_iterator begin() const { return commandStorage.cbegin(); }
      std::list<baseCommand *>::const_iterator end() const { return commandStorage.cend(); }
      std::list<baseCommand *>::iterator mbegin() { return commandStorage.begin();  } // Mutable iterator
      std::list<baseCommand *>::iterator mend() { return commandStorage.end(); }
      void addRawCommand(baseCommand* cmd) { commandStorage.emplace_back(cmd); }
      baseCommand* getLastCommand() { return commandStorage.back(); };
      void purgeLastCommand() { commandStorage.pop_back(); }
      void purgeDestroyLastCommand()
      {
        if(commandStorage.size())
          delete commandStorage.back(), purgeLastCommand();
      }
      SVGRawPath()
      {
        commandStorage.clear();
      }
      ~SVGRawPath()
      {
        for (auto &i : commandStorage)
        {
          if (i != nullptr)
            delete i, i = nullptr;
        }
      }
  };
}

#endif //INC_0005_SMOLSVG_SVGPATH_H
