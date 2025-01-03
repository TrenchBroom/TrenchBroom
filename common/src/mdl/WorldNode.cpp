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

#include "WorldNode.h"

#include "Ensure.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityNodeIndex.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/TagVisitor.h"
#include "mdl/Validator.h"
#include "mdl/ValidatorRegistry.h"
#include "octree.h"

#include "kdl/k.h"
#include "kdl/overload.h"
#include "kdl/vector_utils.h"

#include "vm/bbox_io.h" // IWYU pragma: keep

#include <sstream>
#include <string>
#include <vector>

namespace tb::mdl
{

WorldNode::WorldNode(
  EntityPropertyConfig entityPropertyConfig, Entity entity, const MapFormat mapFormat)
  : m_entityPropertyConfig{std::move(entityPropertyConfig)}
  , m_mapFormat{mapFormat}
  , m_defaultLayer{nullptr}
  , m_entityNodeIndex{std::make_unique<EntityNodeIndex>()}
  , m_validatorRegistry{std::make_unique<ValidatorRegistry>()}
  , m_nodeTree{std::make_unique<NodeTree>(256.0)}
  , m_updateNodeTree{true}
{
  entity.addOrUpdateProperty(
    EntityPropertyKeys::Classname, EntityPropertyValues::WorldspawnClassname);
  entity.setPointEntity(false);
  setEntity(std::move(entity));
  createDefaultLayer();
}

WorldNode::WorldNode(
  EntityPropertyConfig entityPropertyConfig,
  std::initializer_list<EntityProperty> properties,
  const MapFormat mapFormat)
  : WorldNode{entityPropertyConfig, Entity{std::move(properties)}, mapFormat}
{
}

WorldNode::~WorldNode() = default;

EntityPropertyConfig& WorldNode::entityPropertyConfig()
{
  return m_entityPropertyConfig;
}

MapFormat WorldNode::mapFormat() const
{
  return m_mapFormat;
}

const WorldNode::NodeTree& WorldNode::nodeTree() const
{
  return *m_nodeTree;
}

LayerNode* WorldNode::defaultLayer()
{
  ensure(m_defaultLayer != nullptr, "defaultLayer is null");
  return m_defaultLayer;
}

const LayerNode* WorldNode::defaultLayer() const
{
  ensure(m_defaultLayer != nullptr, "defaultLayer is null");
  return m_defaultLayer;
}

std::vector<LayerNode*> WorldNode::allLayers()
{
  auto layers = std::vector<LayerNode*>{};
  visitChildren(kdl::overload(
    [](WorldNode*) {},
    [&](LayerNode* layer) { layers.push_back(layer); },
    [](GroupNode*) {},
    [](EntityNode*) {},
    [](BrushNode*) {},
    [](PatchNode*) {}));
  return layers;
}

std::vector<const LayerNode*> WorldNode::allLayers() const
{
  return kdl::vec_transform(
    const_cast<WorldNode*>(this)->allLayers(), [](const auto* l) { return l; });
}

std::vector<LayerNode*> WorldNode::customLayers()
{
  auto layers = std::vector<LayerNode*>{};

  const auto& children = Node::children();
  for (auto it = std::next(std::begin(children)); it != std::end(children); ++it)
  {
    (*it)->accept(kdl::overload(
      [](WorldNode*) {},
      [&](LayerNode* layer) { layers.push_back(layer); },
      [](GroupNode*) {},
      [](EntityNode*) {},
      [](BrushNode*) {},
      [](PatchNode*) {}));
  }

  return layers;
}

std::vector<const LayerNode*> WorldNode::customLayers() const
{
  return kdl::vec_transform(
    const_cast<WorldNode*>(this)->customLayers(), [](const auto* l) { return l; });
}

std::vector<LayerNode*> WorldNode::allLayersUserSorted()
{
  auto result = allLayers();
  LayerNode::sortLayers(result);
  return result;
}

std::vector<const LayerNode*> WorldNode::allLayersUserSorted() const
{
  return kdl::vec_transform(
    const_cast<WorldNode*>(this)->allLayersUserSorted(), [](const auto* l) { return l; });
}

std::vector<LayerNode*> WorldNode::customLayersUserSorted()
{
  auto result = customLayers();
  LayerNode::sortLayers(result);
  return result;
}

std::vector<const LayerNode*> WorldNode::customLayersUserSorted() const
{
  return kdl::vec_transform(
    const_cast<WorldNode*>(this)->customLayersUserSorted(),
    [](const auto* l) { return l; });
}

void WorldNode::createDefaultLayer()
{
  m_defaultLayer = new LayerNode{Layer{"Default Layer", K(defaultLayer)}};
  addChild(m_defaultLayer);
  assert(m_defaultLayer->layer().sortIndex() == Layer::defaultLayerSortIndex());
}

const EntityNodeIndex& WorldNode::entityNodeIndex() const
{
  return *m_entityNodeIndex;
}

std::vector<const Validator*> WorldNode::registeredValidators() const
{
  return m_validatorRegistry->registeredValidators();
}

std::vector<const IssueQuickFix*> WorldNode::quickFixes(const IssueType issueTypes) const
{
  return m_validatorRegistry->quickFixes(issueTypes);
}

void WorldNode::registerValidator(std::unique_ptr<Validator> validator)
{
  m_validatorRegistry->registerValidator(std::move(validator));
  invalidateAllIssues();
}

void WorldNode::unregisterAllValidators()
{
  m_validatorRegistry->unregisterAllValidators();
  invalidateAllIssues();
}

void WorldNode::disableNodeTreeUpdates()
{
  m_updateNodeTree = false;
}

void WorldNode::enableNodeTreeUpdates()
{
  m_updateNodeTree = true;
}

void WorldNode::rebuildNodeTree()
{
  auto nodes = std::vector<mdl::Node*>{};
  const auto addNode = [&](auto* node) {
    if (node->shouldAddToSpacialIndex())
    {
      nodes.push_back(node);
    }
  };

  accept(kdl::overload(
    [&](auto&& thisLambda, WorldNode* world) {
      addNode(world);
      world->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, LayerNode* layer) {
      addNode(layer);
      layer->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, GroupNode* group) {
      addNode(group);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, EntityNode* entity) {
      addNode(entity);
      entity->visitChildren(thisLambda);
    },
    [&](BrushNode* brush) { addNode(brush); },
    [&](PatchNode* patch) { addNode(patch); }));

  m_nodeTree->clear();
  for (auto* node : nodes)
  {
    m_nodeTree->insert(node->physicalBounds(), node);
  }
}

void WorldNode::invalidateAllIssues()
{
  accept([](auto&& thisLambda, Node* node) {
    node->invalidateIssues();
    node->visitChildren(thisLambda);
  });
}

const vm::bbox3d& WorldNode::doGetLogicalBounds() const
{
  // TODO: this should probably return the world bounds, as it does in
  // Layer::doGetLogicalBounds
  static const vm::bbox3d bounds;
  return bounds;
}

const vm::bbox3d& WorldNode::doGetPhysicalBounds() const
{
  return logicalBounds();
}

double WorldNode::doGetProjectedArea(const vm::axis::type) const
{
  return static_cast<double>(0);
}

Node* WorldNode::doClone(const vm::bbox3d& /* worldBounds */) const
{
  auto result =
    std::make_unique<WorldNode>(entityPropertyConfig(), entity(), mapFormat());
  cloneAttributes(*result);
  return result.release();
}

Node* WorldNode::doCloneRecursively(const vm::bbox3d& worldBounds) const
{
  const auto& myChildren = children();
  assert(myChildren[0] == m_defaultLayer);

  auto* worldNode = static_cast<WorldNode*>(clone(worldBounds));
  worldNode->defaultLayer()->addChildren(
    cloneRecursively(worldBounds, m_defaultLayer->children()));

  if (myChildren.size() > 1)
  {
    auto childClones = std::vector<Node*>{};
    childClones.reserve(myChildren.size() - 1);
    cloneRecursively(
      worldBounds,
      std::next(std::begin(myChildren)),
      std::end(myChildren),
      std::back_inserter(childClones));
    worldNode->addChildren(childClones);
  }

  return worldNode;
}

bool WorldNode::doCanAddChild(const Node* child) const
{
  return child->accept(kdl::overload(
    [](const WorldNode*) { return false; },
    [](const LayerNode*) { return true; },
    [](const GroupNode*) { return false; },
    [](const EntityNode*) { return false; },
    [](const BrushNode*) { return false; },
    [](const PatchNode*) { return false; }));
}

bool WorldNode::doCanRemoveChild(const Node* child) const
{
  return child->accept(kdl::overload(
    [](const WorldNode*) { return false; },
    [&](const LayerNode* layer) { return (layer != defaultLayer()); },
    [](const GroupNode*) { return false; },
    [](const EntityNode*) { return false; },
    [](const BrushNode*) { return false; },
    [](const PatchNode*) { return false; }));
}

bool WorldNode::doRemoveIfEmpty() const
{
  return false;
}

bool WorldNode::doShouldAddToSpacialIndex() const
{
  return false;
}

void WorldNode::doDescendantWasAdded(Node* node, const size_t /* depth */)
{
  // NOTE: `node` is just the root of a subtree that is being connected to this World.
  // In some cases, (e.g. if `node` is a Group), `node` will not be added to the spatial
  // index, but some of its descendants may be. We need to recursively search the `node`
  // being connected and add it or any descendants that need to be added.
  if (m_updateNodeTree)
  {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
      [&](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
      [&](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); },
      [&](auto&& thisLambda, EntityNode* entity) {
        m_nodeTree->insert(entity->physicalBounds(), entity);
        entity->visitChildren(thisLambda);
      },
      [&](BrushNode* brush) { m_nodeTree->insert(brush->physicalBounds(), brush); },
      [&](PatchNode* patch) { m_nodeTree->insert(patch->physicalBounds(), patch); }));
  }

  const auto updatePersistentId = [&](auto* persistentNode) {
    if (const auto persistentNodeId = persistentNode->persistentId())
    {
      ensure(
        *persistentNodeId < std::numeric_limits<IdType>::max(),
        "Persistent ID available");
      m_nextPersistentId = std::max(m_nextPersistentId, *persistentNodeId + 1u);
    }
    else
    {
      persistentNode->setPersistentId(m_nextPersistentId++);
    }
  };

  node->accept(kdl::overload(
    [&](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
    [&](auto&& thisLambda, LayerNode* layer) {
      layer->visitChildren(thisLambda);
      if (layer != defaultLayer())
      {
        updatePersistentId(layer);
      }
    },
    [&](auto&& thisLambda, GroupNode* group) {
      group->visitChildren(thisLambda);
      updatePersistentId(group);
    },
    [&](EntityNode*) {},
    [&](BrushNode*) {},
    [&](PatchNode*) {}));
}

void WorldNode::doDescendantWillBeRemoved(Node* node, const size_t /* depth */)
{
  if (m_updateNodeTree)
  {
    const auto doRemove = [&](auto* nodeToRemove) {
      if (!m_nodeTree->remove(nodeToRemove))
      {
        auto str = std::stringstream();
        str << "Node not found with bounds " << nodeToRemove->physicalBounds() << ": "
            << nodeToRemove;
        throw NodeTreeException{str.str()};
      }
    };

    node->accept(kdl::overload(
      [&](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
      [&](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
      [&](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); },
      [&](auto&& thisLambda, EntityNode* entity) {
        doRemove(entity);
        entity->visitChildren(thisLambda);
      },
      [&](BrushNode* brush) { doRemove(brush); },
      [&](PatchNode* patch) { doRemove(patch); }));
  }
}

void WorldNode::doDescendantPhysicalBoundsDidChange(Node* node)
{
  if (m_updateNodeTree)
  {
    node->accept(kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [](GroupNode*) {},
      [&](EntityNode* entity) { m_nodeTree->update(entity->physicalBounds(), entity); },
      [&](BrushNode* brush) { m_nodeTree->update(brush->physicalBounds(), brush); },
      [&](PatchNode* patch) { m_nodeTree->update(patch->physicalBounds(), patch); }));
  }
}

bool WorldNode::doSelectable() const
{
  return false;
}

void WorldNode::doPick(
  const EditorContext& editorContext, const vm::ray3d& ray, PickResult& pickResult)
{
  for (auto* node : m_nodeTree->find_intersectors(ray))
  {
    node->pick(editorContext, ray, pickResult);
  }
}

void WorldNode::doFindNodesContaining(const vm::vec3d& point, std::vector<Node*>& result)
{
  for (auto* node : m_nodeTree->find_containers(point))
  {
    node->findNodesContaining(point, result);
  }
}

void WorldNode::doAccept(NodeVisitor& visitor)
{
  visitor.visit(this);
}

void WorldNode::doAccept(ConstNodeVisitor& visitor) const
{
  visitor.visit(this);
}

const EntityPropertyConfig& WorldNode::doGetEntityPropertyConfig() const
{
  return m_entityPropertyConfig;
}

void WorldNode::doFindEntityNodesWithProperty(
  const std::string& name,
  const std::string& value,
  std::vector<mdl::EntityNodeBase*>& result) const
{
  result = kdl::vec_concat(
    std::move(result),
    m_entityNodeIndex->findEntityNodes(EntityNodeIndexQuery::exact(name), value));
}

void WorldNode::doFindEntityNodesWithNumberedProperty(
  const std::string& prefix,
  const std::string& value,
  std::vector<mdl::EntityNodeBase*>& result) const
{
  result = kdl::vec_concat(
    std::move(result),
    m_entityNodeIndex->findEntityNodes(EntityNodeIndexQuery::numbered(prefix), value));
}

void WorldNode::doAddToIndex(
  EntityNodeBase* node, const std::string& key, const std::string& value)
{
  m_entityNodeIndex->addProperty(node, key, value);
}

void WorldNode::doRemoveFromIndex(
  EntityNodeBase* node, const std::string& key, const std::string& value)
{
  m_entityNodeIndex->removeProperty(node, key, value);
}

void WorldNode::doPropertiesDidChange(const vm::bbox3d& /* oldBounds */) {}

vm::vec3d WorldNode::doGetLinkSourceAnchor() const
{
  return vm::vec3d{0, 0, 0};
}

vm::vec3d WorldNode::doGetLinkTargetAnchor() const
{
  return vm::vec3d{0, 0, 0};
}

void WorldNode::doAcceptTagVisitor(TagVisitor& visitor)
{
  visitor.visit(*this);
}

void WorldNode::doAcceptTagVisitor(ConstTagVisitor& visitor) const
{
  visitor.visit(*this);
}

} // namespace tb::mdl
