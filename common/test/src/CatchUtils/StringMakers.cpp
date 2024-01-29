/*
 Copyright (C) 2023 Kristian Duske

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

#include "StringMakers.h"

#include "Model/BezierPatch.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/Layer.h"
#include "Model/LayerNode.h"
#include "Model/Node.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>

#include <sstream>

namespace TrenchBroom::Model
{
namespace
{
void printNodes(
  const std::vector<Node*>& nodes, const std::string& indent, std::ostream& str);

void printChildren(const Node& node, const std::string& indent, std::ostream& str)
{
  str << indent << "m_children: ";
  printNodes(node.children(), indent, str);
}

void printWorldNode(
  const WorldNode& worldNode, const std::string& indent, std::ostream& str)
{
  const auto childIndent = indent + "  ";
  str << indent << "WorldNode{\n";
  str << childIndent << "m_entityPropertyConfig: " << worldNode.entityPropertyConfig()
      << ",\n";
  str << childIndent << "m_mapFormat: " << worldNode.mapFormat() << ",\n";
  str << childIndent << "m_entity: " << worldNode.entity() << ",\n";
  printChildren(worldNode, childIndent, str);
  str << ",\n";
  str << indent << "}";
}

void printLayerNode(
  const LayerNode& layerNode, const std::string& indent, std::ostream& str)
{
  const auto childIndent = indent + "  ";
  str << indent << "LayerNode{\n";
  str << childIndent << "m_layer: " << layerNode.layer() << ",\n";
  printChildren(layerNode, childIndent, str);
  str << ",\n";
  str << indent << "}";
}

void printGroupNode(
  const GroupNode& groupNode, const std::string& indent, std::ostream& str)
{
  const auto childIndent = indent + "  ";
  str << indent << "GroupNode{\n";
  str << childIndent << "m_group: " << groupNode.group() << ",\n";
  str << childIndent << "m_linkId: " << groupNode.linkId() << ",\n";
  printChildren(groupNode, childIndent, str);
  str << ",\n";
  str << indent << "}";
}

void printEntityNode(
  const EntityNode& entityNode, const std::string& indent, std::ostream& str)
{
  const auto childIndent = indent + "  ";
  str << indent << "EntityNode{\n";
  str << childIndent << "m_entity: " << entityNode.entity() << ",\n";
  str << childIndent << "m_linkId: " << entityNode.linkId() << ",\n";
  printChildren(entityNode, childIndent, str);
  str << ",\n";
  str << indent << "}";
}

void printBrushNode(
  const BrushNode& brushNode, const std::string& indent, std::ostream& str)
{
  const auto childIndent = indent + "  ";
  str << indent << "BrushNode{\n";
  str << childIndent << "m_brush: " << brushNode.brush() << ",\n";
  str << childIndent << "m_linkId: " << brushNode.linkId() << ",\n";
  printChildren(brushNode, childIndent, str);
  str << ",\n";
  str << indent << "}";
}

void printPatchNode(
  const PatchNode& patchNode, const std::string& indent, std::ostream& str)
{
  const auto childIndent = indent + "  ";
  str << indent << "PatchNode{\n";
  str << childIndent << "m_patch: " << patchNode.patch() << ",\n";
  str << childIndent << "m_linkId: " << patchNode.linkId() << ",\n";
  printChildren(patchNode, childIndent, str);
  str << ",\n";
  str << indent << "}";
}

void printNode(const Node& node, const std::string& indent, std::ostream& str)
{
  node.accept(kdl::overload(
    [&](const WorldNode* worldNode) { printWorldNode(*worldNode, indent, str); },
    [&](const LayerNode* layerNode) { printLayerNode(*layerNode, indent, str); },
    [&](const GroupNode* groupNode) { printGroupNode(*groupNode, indent, str); },
    [&](const EntityNode* entityNode) { printEntityNode(*entityNode, indent, str); },
    [&](const BrushNode* brushNode) { printBrushNode(*brushNode, indent, str); },
    [&](const PatchNode* patchNode) { printPatchNode(*patchNode, indent, str); }));
}

void printNodes(
  const std::vector<Node*>& nodes, const std::string& indent, std::ostream& str)
{
  if (nodes.empty())
  {
    str << "[]";
  }
  else
  {
    str << "[\n";
    for (size_t i = 0; i < nodes.size(); ++i)
    {
      printNode(*nodes[i], indent + "  ", str);
      if (i < nodes.size() - 1)
      {
        str << ",";
      }
      str << "\n";
    }
    str << indent << "]";
  }
}
} // namespace

std::string convertToString(const Node& node)
{
  auto str = std::stringstream{};
  printNode(node, "", str);
  return str.str();
}

std::string convertToString(const Node* node)
{
  return node ? convertToString(*node) : "nullptr";
}

} // namespace TrenchBroom::Model
