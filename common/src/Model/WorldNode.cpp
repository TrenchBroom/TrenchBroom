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

#include "WorldNode.h"

#include "AABBTree.h"
#include "Ensure.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/EntityNode.h"
#include "Model/EntityNodeIndex.h"
#include "Model/GroupNode.h"
#include "Model/IssueGenerator.h"
#include "Model/IssueGeneratorRegistry.h"
#include "Model/LayerNode.h"
#include "Model/TagVisitor.h"
#include "Model/UuidGenerator.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/bbox_io.h>

#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        WorldNode::WorldNode(Entity entity, const MapFormat mapFormat) :
        m_mapFormat(mapFormat),
        m_defaultLayer(nullptr),
        m_entityNodeIndex(std::make_unique<EntityNodeIndex>()),
        m_issueGeneratorRegistry(std::make_unique<IssueGeneratorRegistry>()),
        m_nodeTree(std::make_unique<NodeTree>()),
        m_updateNodeTree(true),
        m_uuidGenerator(std::make_unique<UuidGenerator>()) {
            entity.addOrUpdateProperty(PropertyKeys::Classname, PropertyValues::WorldspawnClassname);
            entity.setPointEntity(false);
            setEntity(std::move(entity));
            createDefaultLayer();
        }

        WorldNode::~WorldNode() = default;

        MapFormat WorldNode::mapFormat() const {
            return m_mapFormat;
        }

        LayerNode* WorldNode::defaultLayer() {
            ensure(m_defaultLayer != nullptr, "defaultLayer is null");
            return m_defaultLayer;
        }

        const LayerNode* WorldNode::defaultLayer() const {
            ensure(m_defaultLayer != nullptr, "defaultLayer is null");
            return m_defaultLayer;
        }

        std::vector<LayerNode*> WorldNode::allLayers() {
            std::vector<LayerNode*> layers;
            visitChildren(kdl::overload(
                [ ](WorldNode*) {},
                [&](LayerNode* layer) { layers.push_back(layer); },
                [ ](GroupNode*) {},
                [ ](EntityNode*) {},
                [ ](BrushNode*) {}
            ));
            return layers;
        }

        std::vector<const LayerNode*> WorldNode::allLayers() const {
            return kdl::vec_transform(const_cast<WorldNode*>(this)->allLayers(), [](const auto* l) { return l; });
        }

        std::vector<LayerNode*> WorldNode::customLayers() {
            std::vector<LayerNode*> layers;

            const std::vector<Node*>& children = Node::children();
            for (auto it = std::next(std::begin(children)), end = std::end(children); it != end; ++it) {
                (*it)->accept(kdl::overload(
                    [] (WorldNode*)       {},
                    [&](LayerNode* layer) { layers.push_back(layer); },
                    [] (GroupNode*)       {},
                    [] (EntityNode*)      {},
                    [] (BrushNode*)       {}
                ));
            }

            return layers;
        }

        std::vector<const LayerNode*> WorldNode::customLayers() const {
            return kdl::vec_transform(const_cast<WorldNode*>(this)->customLayers(), [](const auto* l) { return l; });
        }

        std::vector<LayerNode*> WorldNode::allLayersUserSorted() {
            std::vector<LayerNode*> result = allLayers();
            LayerNode::sortLayers(result);
            return result;
        }

        std::vector<const LayerNode*> WorldNode::allLayersUserSorted() const {
            return kdl::vec_transform(const_cast<WorldNode*>(this)->allLayersUserSorted(), [](const auto* l) { return l; });
        }

        std::vector<LayerNode*> WorldNode::customLayersUserSorted() {
            std::vector<LayerNode*> result = customLayers();
            LayerNode::sortLayers(result);
            return result;
        }

        std::vector<const LayerNode*> WorldNode::customLayersUserSorted() const {
            return kdl::vec_transform(const_cast<WorldNode*>(this)->customLayersUserSorted(), [](const auto* l) { return l; });
        }

        void WorldNode::createDefaultLayer() {
            m_defaultLayer = new LayerNode(Layer("Default Layer", true));
            addChild(m_defaultLayer);
            assert(m_defaultLayer->layer().sortIndex() == Layer::defaultLayerSortIndex());
        }

        const EntityNodeIndex& WorldNode::entityNodeIndex() const {
            return *m_entityNodeIndex;
        }

        const std::vector<IssueGenerator*>& WorldNode::registeredIssueGenerators() const {
            return m_issueGeneratorRegistry->registeredGenerators();
        }

        std::vector<IssueQuickFix*> WorldNode::quickFixes(const IssueType issueTypes) const {
            return m_issueGeneratorRegistry->quickFixes(issueTypes);
        }

        void WorldNode::registerIssueGenerator(IssueGenerator* issueGenerator) {
            m_issueGeneratorRegistry->registerGenerator(issueGenerator);
            invalidateAllIssues();
        }

        void WorldNode::unregisterAllIssueGenerators() {
            m_issueGeneratorRegistry->unregisterAllGenerators();
            invalidateAllIssues();
        }

        void WorldNode::disableNodeTreeUpdates() {
            m_updateNodeTree = false;
        }

        void WorldNode::enableNodeTreeUpdates() {
            m_updateNodeTree = true;
        }

        void WorldNode::rebuildNodeTree() {
            auto nodes = std::vector<Model::Node*>{};
            const auto addNode = [&](auto* node) {
                if (node->shouldAddToSpacialIndex()) {
                    nodes.push_back(node);
                }
            };

            accept(kdl::overload(
                [&](auto&& thisLambda, WorldNode* world)   { addNode(world); world->visitChildren(thisLambda); },
                [&](auto&& thisLambda, LayerNode* layer)   { addNode(layer); layer->visitChildren(thisLambda); },
                [&](auto&& thisLambda, GroupNode* group)   { addNode(group); group->visitChildren(thisLambda); },
                [&](auto&& thisLambda, EntityNode* entity) { addNode(entity); entity->visitChildren(thisLambda); },
                [&](BrushNode* brush)                      { addNode(brush); }
            ));

            m_nodeTree->clearAndBuild(nodes, [](const auto* node){ return node->physicalBounds(); });
        }

        void WorldNode::invalidateAllIssues() {
            accept([](auto&& thisLambda, Node* node) {
                node->invalidateIssues();
                node->visitChildren(thisLambda);
            });
        }

        const vm::bbox3& WorldNode::doGetLogicalBounds() const {
            // TODO: this should probably return the world bounds, as it does in Layer::doGetLogicalBounds
            static const vm::bbox3 bounds;
            return bounds;
        }

        const vm::bbox3& WorldNode::doGetPhysicalBounds() const {
            return logicalBounds();
        }

        Node* WorldNode::doClone(const vm::bbox3& /* worldBounds */) {
            WorldNode* worldNode = new WorldNode(entity(), mapFormat());
            cloneAttributes(worldNode);
            return worldNode;
        }

        Node* WorldNode::doCloneRecursively(const vm::bbox3& worldBounds) {
            const std::vector<Node*>& myChildren = children();
            assert(myChildren[0] == m_defaultLayer);

            WorldNode* worldNode = static_cast<WorldNode*>(clone(worldBounds));
            worldNode->defaultLayer()->addChildren(cloneRecursively(worldBounds, m_defaultLayer->children()));

            if (myChildren.size() > 1) {
                std::vector<Node*> childClones;
                childClones.reserve(myChildren.size() - 1);
                cloneRecursively(worldBounds, std::begin(myChildren) + 1, std::end(myChildren), std::back_inserter(childClones));
                worldNode->addChildren(childClones);
            }

            return worldNode;
        }

        bool WorldNode::doCanAddChild(const Node* child) const {
            return child->accept(kdl::overload(
                [](const WorldNode*)  { return false; },
                [](const LayerNode*)  { return true;  },
                [](const GroupNode*)  { return false; },
                [](const EntityNode*) { return false; },
                [](const BrushNode*)  { return false; }
            ));
        }

        bool WorldNode::doCanRemoveChild(const Node* child) const {
            return child->accept(kdl::overload(
                [] (const WorldNode*)        { return false; },
                [&](const LayerNode* layer)  { return (layer != defaultLayer()); },
                [] (const GroupNode*)        { return false; },
                [] (const EntityNode*)       { return false; },
                [] (const BrushNode*)        { return false; }
            ));
        }

        bool WorldNode::doRemoveIfEmpty() const {
            return false;
        }

        bool WorldNode::doShouldAddToSpacialIndex() const {
            return false;
        }

        void WorldNode::doDescendantWasAdded(Node* node, const size_t /* depth */) {
            // NOTE: `node` is just the root of a subtree that is being connected to this World.
            // In some cases, (e.g. if `node` is a Group), `node` will not be added to the spatial index, but some of its descendants may be.
            // We need to recursively search the `node` being connected and add it or any descendants that need to be added.
            if (m_updateNodeTree) {
                node->accept(kdl::overload(
                    [&](auto&& thisLambda, WorldNode* world)   { world->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, LayerNode* layer)   { layer->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, GroupNode* group)   { group->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, EntityNode* entity) { m_nodeTree->insert(entity->physicalBounds(), entity); entity->visitChildren(thisLambda); },
                    [&](BrushNode* brush)                      { m_nodeTree->insert(brush->physicalBounds(), brush); }
                ));
            }

            const auto updatePersistentId = [&](auto* persistentNode) {
                if (const auto persistentNodeId = persistentNode->persistentId()) {
                    ensure(*persistentNodeId < std::numeric_limits<IdType>::max(), "Persistent ID available");
                    m_nextPersistentId = std::max(m_nextPersistentId, *persistentNodeId + 1u);
                } else {
                    persistentNode->setPersistentId(m_nextPersistentId++);
                }
            };

            const auto updateSharedPersistentId = [&](GroupNode* groupNode) {
                if (!groupNode->sharedPersistentId()) {
                    groupNode->setSharedPersistentId(m_uuidGenerator->generateId());
                }
            };

            node->accept(kdl::overload(
                [&](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
                [&](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); if (layer != defaultLayer()) { updatePersistentId(layer); } },
                [&](auto&& thisLambda, GroupNode* group) { group->visitChildren(thisLambda); updatePersistentId(group); updateSharedPersistentId(group); },
                [&](EntityNode*)                         {},
                [&](BrushNode*)                          {}
                ));
        }

        void WorldNode::doDescendantWillBeRemoved(Node* node, const size_t /* depth */) {
            if (m_updateNodeTree) {
                const auto doRemove = [&](auto* nodeToRemove) {
                if (!m_nodeTree->remove(nodeToRemove)) {
                    auto str = std::stringstream();
                    str << "Node not found with bounds " << nodeToRemove->physicalBounds() << ": " << nodeToRemove;
                    throw NodeTreeException(str.str());
                }
                };

                node->accept(kdl::overload(
                    [&](auto&& thisLambda, WorldNode* world)   { world->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, LayerNode* layer)   { layer->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, GroupNode* group)   { group->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, EntityNode* entity) { doRemove(entity); entity->visitChildren(thisLambda); },
                    [&](BrushNode* brush)                      { doRemove(brush); }
                ));
            }
        }

        void WorldNode::doDescendantPhysicalBoundsDidChange(Node* node) {
            if (m_updateNodeTree) {
                node->accept(kdl::overload(
                    [] (WorldNode*) {},
                    [] (LayerNode*) {},
                    [] (GroupNode*) {},
                    [&](EntityNode* entity) { m_nodeTree->update(entity->physicalBounds(), entity); },
                    [&](BrushNode* brush)   { m_nodeTree->update(brush->physicalBounds(), brush); }
                ));
            }
        }

        bool WorldNode::doSelectable() const {
            return false;
        }

        void WorldNode::doPick(const vm::ray3& ray, PickResult& pickResult) {
            for (auto* node : m_nodeTree->findIntersectors(ray)) {
                node->pick(ray, pickResult);
            }
        }

        void WorldNode::doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) {
            for (auto* node : m_nodeTree->findContainers(point)) {
                node->findNodesContaining(point, result);
            }
        }

        void WorldNode::doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) {
            generator->generate(this, issues);
        }

        void WorldNode::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }

        void WorldNode::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        void WorldNode::doFindEntityNodesWithProperty(const std::string& name, const std::string& value, std::vector<Model::EntityNodeBase*>& result) const {
            result = kdl::vec_concat(std::move(result),
                m_entityNodeIndex->findEntityNodes(EntityNodeIndexQuery::exact(name), value));
        }

        void WorldNode::doFindEntityNodesWithNumberedProperty(const std::string& prefix, const std::string& value, std::vector<Model::EntityNodeBase*>& result) const {
            result = kdl::vec_concat(std::move(result),
                m_entityNodeIndex->findEntityNodes(EntityNodeIndexQuery::numbered(prefix), value));
        }

        void WorldNode::doAddToIndex(EntityNodeBase* node, const std::string& key, const std::string& value) {
            m_entityNodeIndex->addProperty(node, key, value);
        }

        void WorldNode::doRemoveFromIndex(EntityNodeBase* node, const std::string& key, const std::string& value) {
            m_entityNodeIndex->removeProperty(node, key, value);
        }

        void WorldNode::doPropertiesDidChange(const vm::bbox3& /* oldBounds */) {}

        vm::vec3 WorldNode::doGetLinkSourceAnchor() const {
            return vm::vec3::zero();
        }

        vm::vec3 WorldNode::doGetLinkTargetAnchor() const {
            return vm::vec3::zero();
        }

        void WorldNode::doAcceptTagVisitor(TagVisitor& visitor) {
            visitor.visit(*this);
        }

        void WorldNode::doAcceptTagVisitor(ConstTagVisitor& visitor) const {
            visitor.visit(*this);
        }
    }
}
