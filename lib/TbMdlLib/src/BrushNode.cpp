/*
 Copyright (C) 2010 Kristian Duske

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

#include "mdl/BrushNode.h"

#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushRendererBrushCache.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/ModelUtils.h"
#include "mdl/PatchNode.h"
#include "mdl/PickResult.h"
#include "mdl/TagVisitor.h"
#include "mdl/Validator.h"
#include "mdl/WorldNode.h"

#include "kd/const_overload.h"
#include "kd/overload.h"

#include "vm/intersection.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <algorithm>
#include <string>
#include <vector>

namespace tb::mdl
{
const HitType::Type BrushNode::BrushHitType = HitType::freeType();

BrushNode::BrushNode(Brush brush)
  : m_brushRendererBrushCache(std::make_unique<BrushRendererBrushCache>())
  , m_brush(std::move(brush))
{
  clearSelectedFaces();
}

BrushNode::~BrushNode() = default;

const EntityNodeBase* BrushNode::entity() const
{
  return visitParent(
           kdl::overload(
             [](const WorldNode* world) -> const EntityNodeBase* { return world; },
             [](const EntityNode* entity) -> const EntityNodeBase* { return entity; },
             [](auto&& thisLambda, const LayerNode* layer) -> const EntityNodeBase* {
               return layer->visitParent(thisLambda).value_or(nullptr);
             },
             [](auto&& thisLambda, const GroupNode* group) -> const EntityNodeBase* {
               return group->visitParent(thisLambda).value_or(nullptr);
             },
             [](auto&& thisLambda, const BrushNode* brush) -> const EntityNodeBase* {
               return brush->visitParent(thisLambda).value_or(nullptr);
             },
             [](auto&& thisLambda, const PatchNode* patch) -> const EntityNodeBase* {
               return patch->visitParent(thisLambda).value_or(nullptr);
             }))
    .value_or(nullptr);
}

EntityNodeBase* BrushNode::entity()
{
  return KDL_CONST_OVERLOAD(entity());
}

const Brush& BrushNode::brush() const
{
  return m_brush;
}

Brush BrushNode::setBrush(Brush brush)
{
  const auto nodeChange = NotifyNodeChange{*this};
  const auto boundsChange = NotifyPhysicalBoundsChange{*this};

  using std::swap;
  swap(m_brush, brush);

  updateSelectedFaceCount();
  invalidateIssues();
  invalidateVertexCache();

  return brush;
}

bool BrushNode::hasSelectedFaces() const
{
  return m_selectedFaceCount > 0u;
}

void BrushNode::selectFace(const size_t faceIndex)
{
  m_brush.face(faceIndex).select();
  ++m_selectedFaceCount;
}

void BrushNode::deselectFace(const size_t faceIndex)
{
  m_brush.face(faceIndex).deselect();
  --m_selectedFaceCount;
}

void BrushNode::updateFaceTags(const size_t faceIndex, TagManager& tagManager)
{
  m_brush.face(faceIndex).updateTags(tagManager);
}

void BrushNode::setFaceMaterial(const size_t faceIndex, gl::Material* material)
{
  m_brush.face(faceIndex).setMaterial(material);

  invalidateIssues();
  invalidateVertexCache();
}

static bool containsPatch(const Brush& brush, const PatchGrid& grid)
{
  if (!brush.bounds().contains(grid.bounds))
  {
    return false;
  }

  for (const auto& point : grid.points)
  {
    if (!brush.containsPoint(point.position))
    {
      return false;
    }
  }

  return true;
}

bool BrushNode::contains(const Node* node) const
{
  return node->accept(kdl::overload(
    [](const WorldNode*) { return false; },
    [](const LayerNode*) { return false; },
    [&](const GroupNode* group) { return m_brush.contains(group->logicalBounds()); },
    [&](const EntityNode* entity) { return m_brush.contains(entity->logicalBounds()); },
    [&](const BrushNode* brush) { return m_brush.contains(brush->brush()); },
    [&](const PatchNode* patch) { return containsPatch(m_brush, patch->grid()); }));
}

static bool faceIntersectsEdge(
  const BrushFace& face, const vm::vec3d& p0, const vm::vec3d& p1)
{
  const auto ray = vm::ray3d{p0, p1 - p0}; // not normalized
  if (const auto dist = face.intersectWithRay(ray))
  {
    // dist is scaled by inverse of vm::length(p1 - p0)
    return *dist >= 0.0 && *dist <= 1.0;
  }
  return false;
}

static bool intersectsPatch(const Brush& brush, const PatchGrid& grid)
{
  if (!brush.bounds().intersects(grid.bounds))
  {
    return false;
  }

  // if brush contains any grid point, they intersect (or grid is contained, which we
  // count as intersection)
  for (const auto& point : grid.points)
  {
    if (brush.containsPoint(point.position))
    {
      return true;
    }
  }

  // now check if any quad edge of the given grid intersects with any face
  for (const auto& face : brush.faces())
  {
    // check row edges
    for (size_t row = 0u; row < grid.pointRowCount; ++row)
    {
      for (size_t col = 0u; col < grid.pointColumnCount - 1u; ++col)
      {
        const auto& p0 = grid.point(row, col).position;
        const auto& p1 = grid.point(row, col + 1u).position;
        if (faceIntersectsEdge(face, p0, p1))
        {
          return true;
        }
      }
    }
    // check column edges
    for (size_t col = 0u; col < grid.pointColumnCount; ++col)
    {
      for (size_t row = 0u; row < grid.pointRowCount - 1u; ++row)
      {
        const auto& p0 = grid.point(row, col).position;
        const auto& p1 = grid.point(row + 1u, col).position;
        if (faceIntersectsEdge(face, p0, p1))
        {
          return true;
        }
      }
    }
  }

  return false;
}

bool BrushNode::intersects(const Node* node) const
{
  return node->accept(kdl::overload(
    [](const WorldNode*) { return false; },
    [](const LayerNode*) { return false; },
    [&](const GroupNode* group) { return m_brush.intersects(group->logicalBounds()); },
    [&](const EntityNode* entity) { return m_brush.intersects(entity->logicalBounds()); },
    [&](const BrushNode* brush) { return m_brush.intersects(brush->brush()); },
    [&](const PatchNode* patch) { return intersectsPatch(m_brush, patch->grid()); }));
}

void BrushNode::clearSelectedFaces()
{
  for (BrushFace& face : m_brush.faces())
  {
    if (face.selected())
    {
      face.deselect();
    }
  }
  m_selectedFaceCount = 0u;
}

void BrushNode::updateSelectedFaceCount()
{
  m_selectedFaceCount = 0u;
  for (const BrushFace& face : m_brush.faces())
  {
    if (face.selected())
    {
      ++m_selectedFaceCount;
    }
  }
}

const std::string& BrushNode::doGetName() const
{
  static const std::string name("brush");
  return name;
}

const vm::bbox3d& BrushNode::doGetLogicalBounds() const
{
  return m_brush.bounds();
}

const vm::bbox3d& BrushNode::doGetPhysicalBounds() const
{
  return logicalBounds();
}

double BrushNode::doGetProjectedArea(const vm::axis::type axis) const
{
  const auto normal = vm::vec3d::axis(axis);

  auto result = static_cast<double>(0);
  for (const auto& face : m_brush.faces())
  {
    // only consider one side of the brush -- doesn't matter which one!
    if (vm::dot(face.boundary().normal, normal) > 0.0)
    {
      result += face.projectedArea(axis);
    }
  }

  return result;
}

Node* BrushNode::doClone(const vm::bbox3d& /* worldBounds */) const
{
  auto result = std::make_unique<BrushNode>(m_brush);
  cloneLinkId(*result);
  cloneAttributes(*result);
  return result.release();
}

bool BrushNode::doCanAddChild(const Node* /* child */) const
{
  return false;
}

bool BrushNode::doCanRemoveChild(const Node* /* child */) const
{
  return false;
}

bool BrushNode::doRemoveIfEmpty() const
{
  return false;
}

bool BrushNode::doShouldAddToSpacialIndex() const
{
  return true;
}

bool BrushNode::doSelectable() const
{
  return true;
}

void BrushNode::doAccept(NodeVisitor& visitor)
{
  visitor.visit(this);
}

void BrushNode::doAccept(ConstNodeVisitor& visitor) const
{
  visitor.visit(this);
}

void BrushNode::doPick(
  const EditorContext& editorContext, const vm::ray3d& ray, PickResult& pickResult)
{
  if (editorContext.visible(*this))
  {
    if (const auto hit = findFaceHit(ray))
    {
      const auto [distance, faceIndex] = *hit;
      const auto hitPoint = vm::point_at_distance(ray, distance);
      pickResult.addHit(
        Hit(BrushHitType, distance, hitPoint, BrushFaceHandle(this, faceIndex)));
    }
  }
}

void BrushNode::doFindNodesContaining(const vm::vec3d& point, std::vector<Node*>& result)
{
  if (m_brush.containsPoint(point))
  {
    result.push_back(this);
  }
}

std::optional<std::tuple<double, size_t>> BrushNode::findFaceHit(
  const vm::ray3d& ray) const
{
  if (vm::intersect_ray_bbox(ray, logicalBounds()))
  {
    for (size_t i = 0u; i < m_brush.faceCount(); ++i)
    {
      const auto& face = m_brush.face(i);
      if (const auto distance = face.intersectWithRay(ray))
      {
        return std::tuple{*distance, i};
      }
    }
  }
  return std::nullopt;
}

Node* BrushNode::doGetContainer()
{
  return parent();
}

LayerNode* BrushNode::doGetContainingLayer()
{
  return findContainingLayer(this);
}

GroupNode* BrushNode::doGetContainingGroup()
{
  return findContainingGroup(this);
}

void BrushNode::invalidateVertexCache()
{
  m_brushRendererBrushCache->invalidateVertexCache();
}

BrushRendererBrushCache& BrushNode::brushRendererBrushCache() const
{
  return *m_brushRendererBrushCache;
}

void BrushNode::initializeTags(TagManager& tagManager)
{
  Taggable::initializeTags(tagManager);
  for (auto& face : m_brush.faces())
  {
    face.initializeTags(tagManager);
  }
}

void BrushNode::clearTags()
{
  for (auto& face : m_brush.faces())
  {
    face.clearTags();
  }
  Taggable::clearTags();
}

void BrushNode::updateTags(TagManager& tagManager)
{
  for (auto& face : m_brush.faces())
  {
    face.updateTags(tagManager);
  }
  Taggable::updateTags(tagManager);
}

bool BrushNode::allFacesHaveAnyTagInMask(TagType::Type tagMask) const
{
  // Possible optimization: Store the shared face tag mask in the brush and updated it
  // when a face changes.

  TagType::Type sharedFaceTags = TagType::AnyType; // set all bits to 1
  for (const auto& face : m_brush.faces())
  {
    sharedFaceTags &= face.tagMask();
  }
  return (sharedFaceTags & tagMask) != 0;
}

bool BrushNode::anyFaceHasAnyTag() const
{
  for (const auto& face : m_brush.faces())
  {
    if (face.hasAnyTag())
    {
      return true;
    }
  }
  return false;
}

bool BrushNode::anyFacesHaveAnyTagInMask(TagType::Type tagMask) const
{
  // Possible optimization: Store the shared face tag mask in the brush and updated it
  // when a face changes.

  for (const auto& face : m_brush.faces())
  {
    if (face.hasTag(tagMask))
    {
      return true;
    }
  }
  return false;
}

void BrushNode::doAcceptTagVisitor(TagVisitor& visitor)
{
  visitor.visit(*this);
}

void BrushNode::doAcceptTagVisitor(ConstTagVisitor& visitor) const
{
  visitor.visit(*this);
}

bool operator==(const BrushNode& lhs, const BrushNode& rhs)
{
  return lhs.brush() == rhs.brush();
}

bool operator!=(const BrushNode& lhs, const BrushNode& rhs)
{
  return !(lhs == rhs);
}

} // namespace tb::mdl
