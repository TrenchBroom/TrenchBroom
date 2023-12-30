/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "GroupNode.h"

#include "Ensure.h"
#include "Error.h"
#include "FloatType.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/LayerNode.h"
#include "Model/LinkedGroupUtils.h"
#include "Model/ModelUtils.h"
#include "Model/NodeContents.h"
#include "Model/PatchNode.h"
#include "Model/PickResult.h"
#include "Model/TagVisitor.h"
#include "Model/Validator.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/parallel.h>
#include <kdl/result.h>
#include <kdl/result_fold.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/ray.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace TrenchBroom::Model
{

GroupNode::GroupNode(Group group)
  : m_group{std::move(group)}
{
}

const Group& GroupNode::group() const
{
  return m_group;
}

Group GroupNode::setGroup(Group group)
{
  using std::swap;
  swap(m_group, group);
  return group;
}

bool GroupNode::opened() const
{
  return m_editState == EditState::Open;
}

bool GroupNode::hasOpenedDescendant() const
{
  return m_editState == EditState::DescendantOpen;
}

bool GroupNode::closed() const
{
  return m_editState == EditState::Closed;
}

void GroupNode::open()
{
  assert(m_editState == EditState::Closed);
  setEditState(EditState::Open);
  openAncestors();
}

void GroupNode::close()
{
  assert(m_editState == EditState::Open);
  setEditState(EditState::Closed);
  closeAncestors();
}

const std::optional<IdType>& GroupNode::persistentId() const
{
  return m_persistentId;
}

void GroupNode::setPersistentId(const IdType persistentId)
{
  m_persistentId = persistentId;
}

void GroupNode::resetPersistentId()
{
  m_persistentId = std::nullopt;
}

bool GroupNode::hasPendingChanges() const
{
  return m_hasPendingChanges;
}

void GroupNode::setHasPendingChanges(const bool hasPendingChanges)
{
  m_hasPendingChanges = hasPendingChanges;
}

void GroupNode::setEditState(const EditState editState)
{
  m_editState = editState;
}

void GroupNode::setAncestorEditState(const EditState editState)
{
  visitParent(kdl::overload(
    [=](auto&& thisLambda, WorldNode* world) -> void { world->visitParent(thisLambda); },
    [=](auto&& thisLambda, LayerNode* layer) -> void { layer->visitParent(thisLambda); },
    [=](auto&& thisLambda, GroupNode* group) -> void {
      group->setEditState(editState);
      group->visitParent(thisLambda);
    },
    [=](
      auto&& thisLambda, EntityNode* entity) -> void { entity->visitParent(thisLambda); },
    [=](auto&& thisLambda, BrushNode* brush) -> void { brush->visitParent(thisLambda); },
    [=](
      auto&& thisLambda, PatchNode* patch) -> void { patch->visitParent(thisLambda); }));
}

void GroupNode::openAncestors()
{
  setAncestorEditState(EditState::DescendantOpen);
}

void GroupNode::closeAncestors()
{
  setAncestorEditState(EditState::Closed);
}

const std::string& GroupNode::doGetName() const
{
  return m_group.name();
}

const vm::bbox3& GroupNode::doGetLogicalBounds() const
{
  if (!m_boundsValid)
  {
    validateBounds();
  }
  return m_logicalBounds;
}

const vm::bbox3& GroupNode::doGetPhysicalBounds() const
{
  if (!m_boundsValid)
  {
    validateBounds();
  }
  return m_physicalBounds;
}

FloatType GroupNode::doGetProjectedArea(const vm::axis::type) const
{
  return static_cast<FloatType>(0);
}

Node* GroupNode::doClone(
  const vm::bbox3& /* worldBounds */, const SetLinkId setLinkIds) const
{
  auto groupNode = std::make_unique<GroupNode>(
    setNewLinkIdIf(m_group, setLinkIds == SetLinkId::generate));
  cloneAttributes(groupNode.get());
  return groupNode.release();
}

/** Check whether the given parent node or any of its ancestors and the given group node
 *  or any of its descendants have the same linked group id.
 */
static bool checkRecursiveLinkedGroups(
  const Node& parentNode, const GroupNode& groupNodeToAdd)
{
  const auto ancestorLinkedGroupIds = [&]() {
    auto result = std::vector<std::string>{};
    const auto* node = &parentNode;
    while (node)
    {
      const auto* groupNode = dynamic_cast<const Model::GroupNode*>(node);
      const auto linkedGroupId =
        groupNode ? groupNode->group().linkedGroupId() : std::nullopt;
      if (linkedGroupId)
      {
        result.push_back(*linkedGroupId);
      }
      node = node->parent();
    }
    return kdl::vec_sort_and_remove_duplicates(std::move(result));
  }();

  const auto linkedGroupIdsToAdd = [&]() {
    auto result = std::vector<std::string>{};
    groupNodeToAdd.accept(kdl::overload(
      [](const WorldNode*) {},
      [](const LayerNode*) {},
      [&](auto&& thisLambda, const GroupNode* groupNode) {
        if (const auto linkedGroupId = groupNode->group().linkedGroupId())
        {
          result.push_back(*linkedGroupId);
        }
        Node::visitAll(groupNode->children(), thisLambda);
      },
      [](const EntityNode*) {},
      [](const BrushNode*) {},
      [](const PatchNode*) {}));
    return kdl::vec_sort_and_remove_duplicates(std::move(result));
  }();

  return kdl::set_has_shared_element(ancestorLinkedGroupIds, linkedGroupIdsToAdd);
}

bool GroupNode::doCanAddChild(const Node* child) const
{
  return child->accept(kdl::overload(
    [](const WorldNode*) { return false; },
    [](const LayerNode*) { return false; },
    [&](const GroupNode* groupNode) {
      return !checkRecursiveLinkedGroups(*this, *groupNode);
    },
    [](const EntityNode*) { return true; },
    [](const BrushNode*) { return true; },
    [](const PatchNode*) { return true; }));
}

bool GroupNode::doCanRemoveChild(const Node* /* child */) const
{
  return true;
}

bool GroupNode::doRemoveIfEmpty() const
{
  return true;
}

bool GroupNode::doShouldAddToSpacialIndex() const
{
  return false;
}

void GroupNode::doChildWasAdded(Node* /* node */)
{
  nodePhysicalBoundsDidChange();
}

void GroupNode::doChildWasRemoved(Node* /* node */)
{
  nodePhysicalBoundsDidChange();
}

void GroupNode::doNodePhysicalBoundsDidChange()
{
  invalidateBounds();
}

void GroupNode::doChildPhysicalBoundsDidChange()
{
  invalidateBounds();
  nodePhysicalBoundsDidChange();
}

bool GroupNode::doSelectable() const
{
  return true;
}

void GroupNode::doPick(const EditorContext&, const vm::ray3& /* ray */, PickResult&)
{
  // For composite nodes (Groups, brush entities), pick rays don't hit the group
  // but instead just the primitives inside (brushes, point entities).
  // This avoids a potential performance trap where we'd have to exhaustively
  // test many objects if most of the map was inside groups, but it means
  // the pick results need to be postprocessed to account for groups (if desired).
  // See: https://github.com/TrenchBroom/TrenchBroom/issues/2742
}

void GroupNode::doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result)
{
  if (logicalBounds().contains(point))
  {
    result.push_back(this);
  }

  for (auto* child : Node::children())
  {
    child->findNodesContaining(point, result);
  }
}

void GroupNode::doAccept(NodeVisitor& visitor)
{
  visitor.visit(this);
}

void GroupNode::doAccept(ConstNodeVisitor& visitor) const
{
  visitor.visit(this);
}

Node* GroupNode::doGetContainer()
{
  return parent();
}

LayerNode* GroupNode::doGetContainingLayer()
{
  return findContainingLayer(this);
}

GroupNode* GroupNode::doGetContainingGroup()
{
  return findContainingGroup(this);
}

void GroupNode::invalidateBounds()
{
  m_boundsValid = false;
}

void GroupNode::validateBounds() const
{
  m_logicalBounds = computeLogicalBounds(children(), vm::bbox3{0.0});
  m_physicalBounds = computePhysicalBounds(children(), vm::bbox3{0.0});
  m_boundsValid = true;
}

void GroupNode::doAcceptTagVisitor(TagVisitor& visitor)
{
  visitor.visit(*this);
}

void GroupNode::doAcceptTagVisitor(ConstTagVisitor& visitor) const
{
  visitor.visit(*this);
}
} // namespace TrenchBroom::Model
