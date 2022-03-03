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

#include <kdl/result_forward.h>

#include <vecmath/bbox.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace TrenchBroom {
namespace Model {
enum class UpdateLinkedGroupsError;
using UpdateLinkedGroupsResult = std::vector<std::pair<Node*, std::vector<std::unique_ptr<Node>>>>;

/**
 * Updates the given target group nodes from the given source group node.
 *
 * The children of the source node are cloned (recursively) and transformed into the target nodes by
 * means of the recorded transformations of the source group and the corresponding target groups.
 *
 * Depending on the protected property keys of the cloned entities and their corresponding entities
 * in the target groups, some entity property changes may not be propagated from the source group to
 * the target groups. Specifically, if an entity property is protected in either the cloned entity
 * or its corresponding entity in a target group, then changes to that entity property incl. removal
 * are not propagated. This also applies to numbered properties, i.e. properties whose names end in
 * a number. So if the entity property "target" is protected, then changes to the property "target2"
 * are not propagated or overwritten during propagation.
 *
 * If this operation fails for any child and target group, then an error is returned. The operation
 * can fail if any of the following conditions arises:
 *
 * - the transformation of the source group node is not invertible
 * - transforming any of the source node's children fails
 * - any of the transformed children is no longer within the world bounds
 *
 * If this operation succeeds, a vector of pairs is returned where each pair consists of the target
 * node that should be updated, and the new children that should replace the target node's children.
 */
kdl::result<UpdateLinkedGroupsResult, UpdateLinkedGroupsError> updateLinkedGroups(
  const GroupNode& sourceGroupNode, const std::vector<Model::GroupNode*>& targetGroupNodes,
  const vm::bbox3& worldBounds);

/**
 * A group of nodes that can be edited as one.
 *
 * Group nodes can be linked together via a linked group ID. All groups sharing the same linked
 * group id form a link set. When a member of a link set is changed, all other members of that link
 * set are updated to reflect these changes via `updateLinkedGroups`.
 */
class GroupNode : public Node, public Object {
private:
  enum class EditState {
    Open,
    Closed,
    DescendantOpen
  };

  Group m_group;
  EditState m_editState;
  mutable vm::bbox3 m_logicalBounds;
  mutable vm::bbox3 m_physicalBounds;
  mutable bool m_boundsValid;

  /**
   * The ID used to serialize group nodes (see MapReader and NodeSerializer). This is set by
   * MapReader when a layer is read, or by WorldNode when a group is added that doesn't yet have a
   * persistent ID.
   */
  std::optional<IdType> m_persistentId;

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

  Node* doClone(const vm::bbox3& worldBounds) const override;

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
    const EditorContext& editorContext, const vm::ray3& ray, PickResult& pickResult) override;
  void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;

  void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) override;
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
} // namespace Model
} // namespace TrenchBroom
