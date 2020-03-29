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

#ifndef LC2KICADCORE_HPP_
  #define LC2KICADCORE_HPP_

  #include "includes.hpp"
  #include "internalsserializer.hpp"
  #include "internalsdeserializer.hpp"
  #include "edaclasses.hpp"

  namespace lc2kicad
  {
    class LC2KiCadCore
    {
      public:
        LC2KiCadCore(str_dbl_map&);
        ~LC2KiCadCore();

        EDADocument* autoParseLCFile(string& filePath);

        void deserializeFile(EDADocument*, std::string*);

        KiCad_5_Deserializer* getDeserializer() { return internalDeserializer; };
        LCJSONSerializer* getSerializer() { return internalSerializer; };
        
      private:
        KiCad_5_Deserializer* internalDeserializer;
        LCJSONSerializer* internalSerializer;
    };
  }

#endif
