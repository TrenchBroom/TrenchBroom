/*
 Copyright (C) 2026 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mdl/FaceVariableStore.h"

#include "el/Value.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/QueryVariableValues.h"

namespace tb::mdl
{

FaceVariableStore::FaceVariableStore(
  Map& map, const BrushNode& brushNode, const BrushFace& brushFace)
  : m_map{map}
  , m_brushNode{brushNode}
  , m_brushFace{brushFace}
{
}

el::VariableStore* FaceVariableStore::clone() const
{
  return new FaceVariableStore{m_map, m_brushNode, m_brushFace};
}

size_t FaceVariableStore::size() const
{
  return names().size();
}

el::Value FaceVariableStore::value(const std::string& name) const
{
  if (name == "material")
  {
    return el::Value{m_brushFace.materialName()};
  }
  if (name == "normal")
  {
    return el::Value{m_brushFace.normal()};
  }
  if (name == "bounds")
  {
    return el::Value{m_brushFace.bounds()};
  }
  if (name == "center")
  {
    return el::Value{m_brushFace.center()};
  }
  if (name == "entity")
  {
    return entityMapValue(m_brushNode.entity()->entity());
  }
  if (name == "layerName")
  {
    return layerNameValue(m_brushNode);
  }
  if (name == "groupName")
  {
    return groupNameValue(m_brushNode);
  }
  if (name == "tags")
  {
    return tagsArrayValue(m_map, m_brushFace);
  }
  if (name == "visible")
  {
    return el::Value{m_brushNode.visible()};
  }
  if (name == "locked")
  {
    return el::Value{m_brushNode.locked()};
  }
  if (name == "selected")
  {
    return el::Value{m_brushFace.selected()};
  }
  return el::Value::Undefined;
}

std::vector<std::string> FaceVariableStore::names() const
{
  return {
    "material",
    "normal",
    "bounds",
    "center",
    "entity",
    "layerName",
    "groupName",
    "tags",
    "visible",
    "locked",
    "selected",
  };
}

void FaceVariableStore::set(std::string, el::Value) {}

} // namespace tb::mdl
