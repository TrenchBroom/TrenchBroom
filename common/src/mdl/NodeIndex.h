/*
 Copyright (C) 2025 Kristian Duske

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

#include "kdl/compact_trie_forward.h"

#include <memory>
#include <string_view>
#include <vector>

namespace tb::mdl
{
class Node;

using NodeStringIndex = kdl::compact_trie<Node*>;

class NodeIndex
{
private:
  std::unique_ptr<NodeStringIndex> m_index;

public:
  NodeIndex();
  ~NodeIndex();

  void addNode(Node& node);
  void removeNode(Node& node);

  void clear();

  template <typename NodeType = Node>
  std::vector<NodeType*> findNodes(const std::string_view pattern) const
  {
    if constexpr (std::is_same_v<NodeType, Node>)
    {
      return doFindNodes(pattern);
    }
    else
    {
      auto result = std::vector<NodeType*>{};
      for (auto* node : doFindNodes(pattern))
      {
        if (auto* nodeWithType = dynamic_cast<NodeType*>(node))
        {
          result.push_back(nodeWithType);
        }
      }
      return result;
    }
  }

private:
  std::vector<Node*> doFindNodes(std::string_view pattern) const;
};

} // namespace tb::mdl
