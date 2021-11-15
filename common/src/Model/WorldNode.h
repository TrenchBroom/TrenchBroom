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
#include "Model/EntityNodeBase.h"
#include "Model/EntityProperties.h"
#include "Model/IdType.h"
#include "Model/MapFormat.h"
#include "Model/Node.h"

#include <kdl/result_forward.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
template <typename T, size_t S, typename U> class AABBTree;

namespace Model {
class EntityNodeIndex;
enum class BrushError;
class BrushFace;
class IssueGeneratorRegistry;
class IssueQuickFix;
enum class MapFormat;
class PickResult;

class WorldNode : public EntityNodeBase {
private:
  EntityPropertyConfig m_entityPropertyConfig;
  MapFormat m_mapFormat;
  LayerNode* m_defaultLayer;
  std::unique_ptr<EntityNodeIndex> m_entityNodeIndex;
  std::unique_ptr<IssueGeneratorRegistry> m_issueGeneratorRegistry;

  using NodeTree = AABBTree<FloatType, 3, Node*>;
  std::unique_ptr<NodeTree> m_nodeTree;
  bool m_updateNodeTree;

  IdType m_nextPersistentId = 1;

public:
  WorldNode(EntityPropertyConfig entityPropertyConfig, Entity entity, MapFormat mapFormat);
  WorldNode(
    EntityPropertyConfig entityPropertyConfig, std::initializer_list<EntityProperty> properties,
    MapFormat mapFormat);
  ~WorldNode() override;

  MapFormat mapFormat() const;

  const NodeTree& nodeTree() const;

public: // layer management
  LayerNode* defaultLayer();

  const LayerNode* defaultLayer() const;

  /**
   * Returns defaultLayer() plus customLayers()
   */
  std::vector<LayerNode*> allLayers();

  /**
   * Returns defaultLayer() plus customLayers()
   */
  std::vector<const LayerNode*> allLayers() const;

  /**
   * Returns the custom layers in file order
   */
  std::vector<LayerNode*> customLayers();

  /**
   * Returns the custom layers in file order
   */
  std::vector<const LayerNode*> customLayers() const;

  /**
   * Returns defaultLayer() plus customLayers() ordered by LayerNode::sortIndex(). The default layer
   * is always first.
   */
  std::vector<LayerNode*> allLayersUserSorted();

  /**
   * Returns defaultLayer() plus customLayers() ordered by LayerNode::sortIndex(). The default layer
   * is always first.
   */
  std::vector<const LayerNode*> allLayersUserSorted() const;

  /**
   * Returns customLayers() ordered by LayerNode::sortIndex()
   */
  std::vector<LayerNode*> customLayersUserSorted();

  /**
   * Returns customLayers() ordered by LayerNode::sortIndex()
   */
  std::vector<const LayerNode*> customLayersUserSorted() const;

private:
  void createDefaultLayer();

public: // index
  const EntityNodeIndex& entityNodeIndex() const;

public: // selection
  // issue generator registration
  const std::vector<IssueGenerator*>& registeredIssueGenerators() const;
  std::vector<IssueQuickFix*> quickFixes(IssueType issueTypes) const;
  void registerIssueGenerator(IssueGenerator* issueGenerator);
  void unregisterAllIssueGenerators();

public: // node tree bulk updating
  void disableNodeTreeUpdates();
  void enableNodeTreeUpdates();
  void rebuildNodeTree();

private:
  void invalidateAllIssues();

private: // implement Node interface
  const vm::bbox3& doGetLogicalBounds() const override;
  const vm::bbox3& doGetPhysicalBounds() const override;
  FloatType doGetProjectedArea(vm::axis::type axis) const override;
  Node* doClone(const vm::bbox3& worldBounds) const override;
  Node* doCloneRecursively(const vm::bbox3& worldBounds) const override;
  bool doCanAddChild(const Node* child) const override;
  bool doCanRemoveChild(const Node* child) const override;
  bool doRemoveIfEmpty() const override;
  bool doShouldAddToSpacialIndex() const override;

  void doDescendantWasAdded(Node* node, size_t depth) override;
  void doDescendantWillBeRemoved(Node* node, size_t depth) override;
  void doDescendantPhysicalBoundsDidChange(Node* node) override;

  bool doSelectable() const override;
  void doPick(
    const EditorContext& editorContext, const vm::ray3& ray, PickResult& pickResult) override;
  void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;
  void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) override;
  void doAccept(NodeVisitor& visitor) override;
  void doAccept(ConstNodeVisitor& visitor) const override;
  const EntityPropertyConfig& doGetEntityPropertyConfig() const override;
  void doFindEntityNodesWithProperty(
    const std::string& name, const std::string& value,
    std::vector<EntityNodeBase*>& result) const override;
  void doFindEntityNodesWithNumberedProperty(
    const std::string& prefix, const std::string& value,
    std::vector<EntityNodeBase*>& result) const override;
  void doAddToIndex(
    EntityNodeBase* node, const std::string& key, const std::string& value) override;
  void doRemoveFromIndex(
    EntityNodeBase* node, const std::string& key, const std::string& value) override;

private: // implement EntityNodeBase interface
  void doPropertiesDidChange(const vm::bbox3& oldBounds) override;
  vm::vec3 doGetLinkSourceAnchor() const override;
  vm::vec3 doGetLinkTargetAnchor() const override;

private: // implement Taggable interface
  void doAcceptTagVisitor(TagVisitor& visitor) override;
  void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;

private:
  deleteCopyAndMove(WorldNode);
};
} // namespace Model
} // namespace TrenchBroom
