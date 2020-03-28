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
  
#include <vector>
#include <fstream>
#include <string>
#include <memory>

#include "includes.hpp"
#include "consts.hpp"
#include "lc2kicadcore.hpp"
#include "rapidjson.hpp"
#include "edaclasses.hpp"

using std::vector;
using std::fstream;
using std::string;

namespace lc2kicad
{
  EDADocument::EDADocument(bool useJSONStorage)
  {
    if(useJSONStorage)
      jsonParseResult = std::make_shared<rapidjson::Document>();
  }

  PCBDocument::PCBDocument(const EDADocument& a)
  {
    pathToFile = a.pathToFile;
    docInfo = a.docInfo;
    jsonParseResult = a.jsonParseResult;
  }

  PCBDocument::~PCBDocument() //Destructor
  {
    if(containedElements.size() != 0) //Free memory taken up by elements contained in the document
      for(auto i : containedElements) //Iterate through the vector
        if(!i) //If it's not a NULL or nullptr
          delete i;
  }

  string* PCB_Pad::deserializeSelf() const { return parent->parent->getDeserializer()->outputPad(*this); };
  string* PCB_Pad::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPad(*this); };

  string* PCB_GraphicalTrack::deserializeSelf() const { return parent->parent->getDeserializer()->outputGraphicalTrack(*this); };
  string* PCB_GraphicalTrack::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputGraphicalTrack(*this); };

  string* PCB_CopperTrack::deserializeSelf() const { return parent->parent->getDeserializer()->outputCopperTrack(*this); };
  string* PCB_CopperTrack::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputCopperTrack(*this); };

  string* PCB_Hole::deserializeSelf() const { return parent->parent->getDeserializer()->outputHole(*this); };
  string* PCB_Hole::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputHole(*this); };

  string* PCB_Via::deserializeSelf() const { return parent->parent->getDeserializer()->outputVia(*this); };
  string* PCB_Via::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputVia(*this); };

  string* PCB_SolidRegion::deserializeSelf() const { return parent->parent->getDeserializer()->outputSolidRegion(*this); };
  string* PCB_SolidRegion::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputSolidRegion(*this); };
  
  string* PCB_FloodFill::deserializeSelf() const { return parent->parent->getDeserializer()->outputFloodFill(*this); };
  string* PCB_FloodFill::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputFloodFill(*this); };

  string* PCB_GraphicalCircle::deserializeSelf() const { return parent->parent->getDeserializer()->outputGraphicalCircle(*this); };
  string* PCB_GraphicalCircle::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputGraphicalCircle(*this); };

  string* PCB_CopperCircle::deserializeSelf() const { return parent->parent->getDeserializer()->outputCopperCircle(*this); };
  string* PCB_CopperCircle::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputCopperCircle(*this); };

  string* PCB_Rect::deserializeSelf() const { return parent->parent->getDeserializer()->outputRect(*this); };
  string* PCB_Rect::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputRect(*this); };

  string* PCB_GraphicalArc::deserializeSelf() const { return parent->parent->getDeserializer()->outputGraphicalArc(*this); };
  string* PCB_GraphicalArc::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputGraphicalArc(*this); };

  string* PCB_CopperArc::deserializeSelf() const { return parent->parent->getDeserializer()->outputCopperArc(*this); };
  string* PCB_CopperArc::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputCopperArc(*this); };
}