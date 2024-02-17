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

#pragma once

#include "FloatType.h"
#include "Macros.h"
#include "Model/Group.h"
#include "Model/IdType.h"
#include "Model/Node.h"
#include "Model/Object.h"
#include "Result.h"

#include "vecmath/bbox.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace TrenchBroom::Model
{

/**
 * A group of nodes that can be edited as one.
 *
 * Group nodes can be linked together via a linked group ID. All groups sharing the same
 * linked group id form a link set. When a member of a link set is changed, all other
 * members of that link set are updated to reflect these changes via `updateLinkedGroups`.
 */
class GroupNode : public Node, public Object
{
private:
  enum class EditState
  {
    Open,
    Closed,
    DescendantOpen
  };

  Group m_group;
  EditState m_editState = EditState::Closed;
  mutable vm::bbox3 m_logicalBounds;
  mutable vm::bbox3 m_physicalBounds;
  mutable bool m_boundsValid = false;

  /**
   * The ID used to serialize group nodes (see MapReader and NodeSerializer). This is set
   * by MapReader when a layer is read, or by WorldNode when a group is added that doesn't
   * yet have a persistent ID.
   */
  std::optional<IdType> m_persistentId;

  bool m_hasPendingChanges = false;

public:
  explicit GroupNode(Group group);

  const Group& group() const;
  Group setGroup(Group group);

  bool opened() const;
  bool hasOpenedDescendant() const;
  bool closed() const;
  void open();
  void close();

  const std::optional<IdType>& persistentId() const;
  void setPersistentId(IdType persistentId);
  void resetPersistentId();

  bool hasPendingChanges() const;
  void setHasPendingChanges(bool hasPendingChanges);

private:
  void setEditState(EditState editState);
  void setAncestorEditState(EditState editState);

  void openAncestors();
  void closeAncestors();

private: // implement methods inherited from Node
  const std::string& doGetName() const override;
  const vm::bbox3& doGetLogicalBounds() const override;
  const vm::bbox3& doGetPhysicalBounds() const override;

  FloatType doGetProjectedArea(vm::axis::type axis) const override;

  Node* doClone(const vm::bbox3& worldBounds, SetLinkId setLinkIds) const override;

  bool doCanAddChild(const Node* child) const override;
  bool doCanRemoveChild(const Node* child) const override;
  bool doRemoveIfEmpty() const override;

  bool doShouldAddToSpacialIndex() const override;

  void doChildWasAdded(Node* node) override;
  void doChildWasRemoved(Node* node) override;

  void doNodePhysicalBoundsDidChange() override;
  void doChildPhysicalBoundsDidChange() override;

  bool doSelectable() const override;

  void doPick(
    const EditorContext& editorContext,
    const vm::ray3& ray,
    PickResult& pickResult) override;
  void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;

  void doAccept(NodeVisitor& visitor) override;
  void doAccept(ConstNodeVisitor& visitor) const override;

private: // implement methods inherited from Object
  Node* doGetContainer() override;
  LayerNode* doGetContainingLayer() override;
  GroupNode* doGetContainingGroup() override;

private:
  void invalidateBounds();
  void validateBounds() const;

private: // implement Taggable interface
  void doAcceptTagVisitor(TagVisitor& visitor) override;
  void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;

private:
  deleteCopyAndMove(GroupNode);
};

bool compareGroupNodesByLinkId(const GroupNode* lhs, const GroupNode* rhs);

} // namespace TrenchBroom::Model
