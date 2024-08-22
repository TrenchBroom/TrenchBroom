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

#include "Matchers.h"

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
#include "StringMakers.h"

#include "kdl/string_compare.h"

namespace TrenchBroom
{

GlobMatcher::GlobMatcher(std::string glob)
  : m_glob{std::move(glob)}
{
}

bool GlobMatcher::match(const std::string& value) const
{
  return kdl::cs::str_matches_glob(value, m_glob);
}

std::string GlobMatcher::describe() const
{
  auto ss = std::stringstream{};
  ss << "matches glob \"" << m_glob << "\"";
  return ss.str();
}

GlobMatcher MatchesGlob(std::string glob)
{
  return GlobMatcher{std::move(glob)};
}

} // namespace TrenchBroom

namespace TrenchBroom::Model
{

namespace
{
bool nodesMatch(const std::vector<Node*>& lhs, const std::vector<Node*>& rhs);

bool nodesMatch(const Node& lhs, const Node& rhs)
{
  if (&lhs == &rhs)
  {
    return true;
  }

  return lhs.accept(kdl::overload(
    [&](const WorldNode* expectedWorldNode) {
      const auto* inWorldNode = dynamic_cast<const WorldNode*>(&rhs);
      return inWorldNode && inWorldNode->entity() == expectedWorldNode->entity()
             && nodesMatch(inWorldNode->children(), expectedWorldNode->children());
    },
    [&](const LayerNode* expectedLayerNode) {
      const auto* inLayerNode = dynamic_cast<const LayerNode*>(&rhs);
      return inLayerNode && inLayerNode->layer() == expectedLayerNode->layer()
             && nodesMatch(inLayerNode->children(), expectedLayerNode->children());
    },
    [&](const GroupNode* expectedGroupNode) {
      const auto* inGroupNode = dynamic_cast<const GroupNode*>(&rhs);
      return inGroupNode && inGroupNode->group() == expectedGroupNode->group()
             && inGroupNode->linkId() == expectedGroupNode->linkId()
             && nodesMatch(inGroupNode->children(), expectedGroupNode->children());
    },
    [&](const EntityNode* expectedEntityNode) {
      const auto* inEntityNode = dynamic_cast<const EntityNode*>(&rhs);
      return inEntityNode && inEntityNode->entity() == expectedEntityNode->entity()
             && inEntityNode->linkId() == expectedEntityNode->linkId()
             && nodesMatch(inEntityNode->children(), expectedEntityNode->children());
    },
    [&](const BrushNode* expectedBrushNode) {
      const auto* inBrushNode = dynamic_cast<const BrushNode*>(&rhs);
      return inBrushNode && inBrushNode->brush() == expectedBrushNode->brush()
             && inBrushNode->linkId() == expectedBrushNode->linkId();
    },
    [&](const PatchNode* expectedPatchNode) {
      const auto* inPatchNode = dynamic_cast<const PatchNode*>(&rhs);
      return inPatchNode && inPatchNode->patch() == expectedPatchNode->patch()
             && inPatchNode->linkId() == expectedPatchNode->linkId();
    }));
}

bool nodesMatch(const std::vector<Node*>& lhs, const std::vector<Node*>& rhs)
{
  return std::equal(
    lhs.begin(),
    lhs.end(),
    rhs.begin(),
    rhs.end(),
    [](const auto* lhsChild, const auto* rhsChild) {
      return nodesMatch(*lhsChild, *rhsChild);
    });
}
} // namespace

NodeMatcher::NodeMatcher(const Node& expected)
  : m_expected{expected}
{
}

bool NodeMatcher::match(const Node& in) const
{
  return nodesMatch(m_expected, in);
}

std::string NodeMatcher::describe() const
{
  auto str = std::stringstream{};
  str << "matches " << convertToString(m_expected);
  return str.str();
}

NodeMatcher MatchesNode(const Node& expected)
{
  return NodeMatcher{expected};
}

NodeVectorMatcher::NodeVectorMatcher(std::vector<Node*> expected)
  : m_expected{std::move(expected)}
{
}

bool NodeVectorMatcher::match(const std::vector<Node*>& in) const
{
  return std::equal(
    in.begin(),
    in.end(),
    m_expected.begin(),
    m_expected.end(),
    [](const auto& lhs, const auto& rhs) { return nodesMatch(*lhs, *rhs); });
}

std::string NodeVectorMatcher::describe() const
{
  auto str = std::stringstream{};
  str << "matches " << kdl::make_streamable(m_expected);
  return str.str();
}

} // namespace TrenchBroom::Model
