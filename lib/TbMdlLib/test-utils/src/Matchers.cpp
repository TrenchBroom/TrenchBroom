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

#include "mdl/Matchers.h"

#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/StringMakers.h"
#include "mdl/WorldNode.h"

#include "vm/approx.h"
#include "vm/vec.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>

namespace tb::mdl
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
    [&](const WorldNode& expectedWorldNode) {
      const auto* inWorldNode = dynamic_cast<const WorldNode*>(&rhs);
      return inWorldNode && inWorldNode->entity() == expectedWorldNode.entity()
             && nodesMatch(inWorldNode->children(), expectedWorldNode.children());
    },
    [&](const LayerNode& expectedLayerNode) {
      const auto* inLayerNode = dynamic_cast<const LayerNode*>(&rhs);
      return inLayerNode && inLayerNode->layer() == expectedLayerNode.layer()
             && nodesMatch(inLayerNode->children(), expectedLayerNode.children());
    },
    [&](const GroupNode& expectedGroupNode) {
      const auto* inGroupNode = dynamic_cast<const GroupNode*>(&rhs);
      return inGroupNode && inGroupNode->group() == expectedGroupNode.group()
             && inGroupNode->linkId() == expectedGroupNode.linkId()
             && nodesMatch(inGroupNode->children(), expectedGroupNode.children());
    },
    [&](const EntityNode& expectedEntityNode) {
      const auto* inEntityNode = dynamic_cast<const EntityNode*>(&rhs);
      return inEntityNode && inEntityNode->entity() == expectedEntityNode.entity()
             && inEntityNode->linkId() == expectedEntityNode.linkId()
             && nodesMatch(inEntityNode->children(), expectedEntityNode.children());
    },
    [&](const BrushNode& expectedBrushNode) {
      const auto* inBrushNode = dynamic_cast<const BrushNode*>(&rhs);
      return inBrushNode && inBrushNode->brush() == expectedBrushNode.brush()
             && inBrushNode->linkId() == expectedBrushNode.linkId();
    },
    [&](const PatchNode& expectedPatchNode) {
      const auto* inPatchNode = dynamic_cast<const PatchNode*>(&rhs);
      return inPatchNode && inPatchNode->patch() == expectedPatchNode.patch()
             && inPatchNode->linkId() == expectedPatchNode.linkId();
    }));
}

bool nodesMatch(const std::vector<Node*>& lhs, const std::vector<Node*>& rhs)
{
  return std::ranges::equal(lhs, rhs, [](const auto* lhsChild, const auto* rhsChild) {
    return nodesMatch(*lhsChild, *rhsChild);
  });
}

bool valueOpsMatch(const std::optional<ValueOp>& lhs, const std::optional<ValueOp>& rhs)
{
  if (!lhs.has_value() && !rhs.has_value())
  {
    return true;
  }

  if (lhs.has_value() != rhs.has_value())
  {
    return false;
  }

  return std::visit(
    kdl::overload(
      [](const SetValue& lhsSetValue, const SetValue& rhsSetValue) {
        return lhsSetValue.value == vm::optional_approx{rhsSetValue.value};
      },
      [](const AddValue& lhsAddValue, const AddValue& rhsAddValue) {
        return lhsAddValue.delta == vm::approx{rhsAddValue.delta};
      },
      [](const MultiplyValue& lhsMultiplyValue, const MultiplyValue& rhsMultiplyValue) {
        return lhsMultiplyValue.factor == vm::approx{rhsMultiplyValue.factor};
      },
      [](const auto&, const auto&) { return false; }),
    *lhs,
    *rhs);
}

} // namespace

BrushVertexMatcher::BrushVertexMatcher(const Brush& expected, const double epsilon)
  : m_expected{expected}
  , m_epsilon{epsilon}
{
}

bool BrushVertexMatcher::match(const Brush& in) const
{
  if (in.vertexCount() != m_expected.vertexCount())
  {
    return false;
  }

  auto unmatchedPositions = in.vertexPositions();
  for (const auto& expectedPosition : m_expected.vertexPositions())
  {
    const auto it = std::ranges::find_if(unmatchedPositions, [&](const auto& position) {
      return vm::is_equal(position, expectedPosition, m_epsilon);
    });
    if (it == unmatchedPositions.end())
    {
      return false;
    }
    unmatchedPositions.erase(it);
  }

  return true;
}

std::string BrushVertexMatcher::describe() const
{
  return fmt::format(
    "has the same vertex positions as the expected brush with epsilon {}", m_epsilon);
}

BrushVertexMatcher MatchesBrushVertices(const Brush& expected, const double epsilon)
{
  return BrushVertexMatcher{expected, epsilon};
}

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

BrushFaceAttributesMatcher::BrushFaceAttributesMatcher(BrushFaceAttributes expected)
  : m_expected{std::move(expected)}
{
}

bool BrushFaceAttributesMatcher::match(const BrushFaceAttributes& in) const
{
  return in.materialName() == m_expected.materialName()
         && in.surfaceContents() == m_expected.surfaceContents()
         && in.surfaceFlags() == m_expected.surfaceFlags()
         && in.surfaceValue() == m_expected.surfaceValue()
         && in.color() == m_expected.color();
}

std::string BrushFaceAttributesMatcher::describe() const
{
  return fmt::format("{}", fmt::streamed(m_expected));
}

UVAttributesMatcher::UVAttributesMatcher(const UVAttributes& expected)
  : m_expected{expected}
{
}

bool UVAttributesMatcher::match(const UVAttributes& in) const
{
  return in.offset == vm::approx{m_expected.offset}
         && in.scale == vm::approx{m_expected.scale}
         && in.rotation == vm::approx{m_expected.rotation};
}

std::string UVAttributesMatcher::describe() const
{
  return fmt::format("{}", fmt::streamed(m_expected));
}

UVAttributesMatcher MatchesUVAttributes(const UVAttributes& expected)
{
  return UVAttributesMatcher{expected};
}

BrushFaceAttributesMatcher MatchesBrushFaceAttributes(BrushFaceAttributes expected)
{
  return BrushFaceAttributesMatcher{std::move(expected)};
}

UpdateBrushFaceAttributesMatcher::UpdateBrushFaceAttributesMatcher(
  UpdateBrushFaceAttributes expected)
  : m_expected{std::move(expected)}
{
}

bool UpdateBrushFaceAttributesMatcher::match(const UpdateBrushFaceAttributes& in) const
{
  return in.materialName == m_expected.materialName
         && valueOpsMatch(in.xOffset, m_expected.xOffset)
         && valueOpsMatch(in.yOffset, m_expected.yOffset)
         && valueOpsMatch(in.rotation, m_expected.rotation)
         && valueOpsMatch(in.xScale, m_expected.xScale)
         && valueOpsMatch(in.yScale, m_expected.yScale)
         && in.surfaceFlags == m_expected.surfaceFlags
         && in.surfaceContents == m_expected.surfaceContents
         && in.surfaceValue == m_expected.surfaceValue && in.color == m_expected.color
         && in.axis == m_expected.axis;
}

std::string UpdateBrushFaceAttributesMatcher::describe() const
{
  return fmt::format("{}", fmt::streamed(m_expected));
}

UpdateBrushFaceAttributesMatcher MatchesUpdateBrushFaceAttributes(
  UpdateBrushFaceAttributes expected)
{
  return UpdateBrushFaceAttributesMatcher{std::move(expected)};
}

} // namespace tb::mdl
