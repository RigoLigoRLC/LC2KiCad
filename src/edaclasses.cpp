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
  EDADocument::EDADocument()
  {
    jsonParseResult = std::make_shared<rapidjson::Document>();
    module = false;
    docType = documentTypes::invalid;
    gridSize = 2.54;
  }
  
  void EDADocument::addElement(EDAElement*) {} //I wished it to be a pure virtual function but can't do it. UHHH

  EDADocument::~EDADocument()
  {

  }

  EDADocument::EDADocument(const bool useJSONStorage)
  {
    if(useJSONStorage)
      jsonParseResult = std::make_shared<rapidjson::Document>();
    module = false;
    docType = documentTypes::invalid;
    gridSize = 2.54;
  }
  
  EDAElement::~EDAElement()
  {
    
  }

  PCBDocument::PCBDocument(const EDADocument& a)// : EDADocument::EDADocument(true)
  {
    pathToFile = a.pathToFile;
    docInfo = a.docInfo;
    module = a.module;
    jsonParseResult = a.jsonParseResult;
  }
  
  void PCBDocument::addElement(EDAElement* element)
  {
    if(this->module)
      static_cast<PCB_Module*>(this->containedElements.back())->containedElements.push_back(static_cast<PCBElement*>(element));
    else
      this->containedElements.push_back(static_cast<PCBElement*>(element));
  }

  PCBDocument::~PCBDocument() //Destructor
  {
    if(containedElements.size() != 0) //Free memory taken up by elements contained in the document
      for(auto i : containedElements) //Iterate through the vector
        if(i) //If it's not a NULL or nullptr
          delete i, i = nullptr;
  }
  
  SchematicDocument::SchematicDocument(const EDADocument& a)// : EDADocument::EDADocument(true)
  {
    pathToFile = a.pathToFile;
    docInfo = a.docInfo;
    module = a.module;
    jsonParseResult = a.jsonParseResult;
  }

  SchematicDocument::~SchematicDocument() //Destructor
  {
    if(containedElements.size() != 0) //Free memory taken up by elements contained in the document
      for(auto i : containedElements) //Iterate through the vector
        if(i) //If it's not a NULL or nullptr
          delete i, i = nullptr;
  }

  unsigned int PCBNetManager::obtainNetCode(std::string &netName)
  {
    for(auto &it : netNameCodeMap)
      if(it.second == netName)
        return it.first;
    netNameCodeMap[netNameCodeMap.size()] = netName;
    return netNameCodeMap.size() - 1;
  }

  void PCBNetManager::setNet(std::string &netName, PCBNet &net)
  {
    auto cleanNetName = escapeQuotedString(netName);
    net.first = obtainNetCode(cleanNetName);
    net.second = cleanNetName;
  }

  bool PCBNetManager::findNet(std::string &netName)
  {
    for(auto it = netNameCodeMap.begin(); it != netNameCodeMap.end(); it++)
      if(it->second == netName)
        return true;
    return false;
  }

  std::string PCBNetManager::outputPCBNetInfo()
  {
    string ret;
    for(auto &i : netNameCodeMap)
      ret += "  (net " + std::to_string(i.first) + " \"" + i.second + "\")\n";
    return ret;
  }

  PCBNetManager::PCBNetManager()
  {
    netNameCodeMap[0] = "";
  }

  void PCBFloodFillPriorityManager::logPriority(unsigned int easyedaFillPriority)
  {
    if(easyedaFillPriority > maximumPriority) maximumPriority = easyedaFillPriority;
    VERBOSE_INFO(std::string("Logged area priority ") + std::to_string(easyedaFillPriority) +
                 ", current maximum priority " + std::to_string(maximumPriority));
  }

  unsigned int PCBFloodFillPriorityManager::getKiCadPriority(unsigned int easyedaFillPriority)
  {
    return easyedaFillPriority ? maximumPriority + 1 - easyedaFillPriority : 0; // 0 was reserved for solid regions
  }
  
  string* PCB_Module::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBModule(*this); }
  string* PCB_Pad::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBPad(*this); }
  string* PCB_GraphicalTrack::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBGraphicalTrack(*this); }
  string* PCB_CopperTrack::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBCopperTrack(*this); }
  string* PCB_Hole::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBHole(*this); }
  string* PCB_Via::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBVia(*this); }
  string* PCB_CopperSolidRegion::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBCopperSolidRegion(*this); }
  string* PCB_GraphicalSolidRegion::deserializeSelf(KiCad_5_Deserializer &deserializer) const { return deserializer.outputPCBGraphicalSolidRegion(*this); }
  string* PCB_FloodFill::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBFloodFill(*this); }
  string* PCB_KeepoutRegion::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBKeepoutRegion(*this); }
  string* PCB_GraphicalCircle::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBGraphicalCircle(*this); }
  string* PCB_CopperCircle::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBCopperCircle(*this); }
  string* PCB_Rect::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBRect(*this); }
  string* PCB_GraphicalArc::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBGraphicalArc(*this); }

  string* PCB_CopperArc::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBCopperArc(*this); }
  string* PCB_Text::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputPCBText(*this); }
  string* Schematic_Module::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputSchModule(*this); }
  string* Schematic_Pin::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputSchPin(*this); }
  string* Schematic_Polyline::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputSchPolyline(*this); }
  string* Schematic_Rect::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputSchRect(*this); }
  string* Schematic_Polygon::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputSchPolygon(*this); }

  string* Schematic_Arc::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputSchArc(*this); }

  string* Schematic_Text::deserializeSelf(KiCad_5_Deserializer& deserializer) const { return deserializer.outputSchText(*this); }
}
