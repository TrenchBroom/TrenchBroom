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

#pragma once

#include "base/Macros.h"
#include "el/VariableStore.h"

#include <string>
#include <vector>

namespace tb::mdl
{
class Map;
class Node;

/**
 * Binds the search/filter query language's node schema (type, name, classname,
 * properties, entity, materials, bounds, center, layerName, groupName, tags, visible,
 * locked, selected, linked) for a single node. Fields that don't apply to the wrapped
 * node's concrete kind evaluate to `el::Value::Undefined` rather than being omitted.
 */
class NodeVariableStore : public el::VariableStore
{
private:
  Map& m_map;
  const Node& m_node;

public:
  NodeVariableStore(Map& map, const Node& node);

  VariableStore* clone() const override;
  size_t size() const override;
  el::Value value(const std::string& name) const override;
  std::vector<std::string> names() const override;
  void set(std::string name, el::Value value) override;

  deleteCopyAndMove(NodeVariableStore);
};

} // namespace tb::mdl
