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

#include "World.h"

#include "AABBTree.h"
#include "Ensure.h"
#include "Model/AssortNodesVisitor.h"
#include "Model/AttributableNodeIndex.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/CollectNodesWithDescendantSelectionCountVisitor.h"
#include "Model/IssueGenerator.h"
#include "Model/IssueGeneratorRegistry.h"
#include "Model/ModelFactoryImpl.h"
#include "Model/TagVisitor.h"

#include <kdl/vector_utils.h>

#include <vecmath/bbox_io.h>

#include <sstream>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        World::World(MapFormat mapFormat) :
        m_factory(std::make_unique<ModelFactoryImpl>(mapFormat)),
        m_defaultLayer(nullptr),
        m_attributableIndex(std::make_unique<AttributableNodeIndex>()),
        m_issueGeneratorRegistry(std::make_unique<IssueGeneratorRegistry>()),
        m_nodeTree(std::make_unique<NodeTree>()),
        m_updateNodeTree(true) {
            addOrUpdateAttribute(AttributeNames::Classname, AttributeValues::WorldspawnClassname);
            createDefaultLayer();
        }

        World::~World() = default;

        LayerNode* World::defaultLayer() const {
            ensure(m_defaultLayer != nullptr, "defaultLayer is null");
            return m_defaultLayer;
        }

        std::vector<LayerNode*> World::allLayers() const {
            CollectLayersVisitor visitor;
            iterate(visitor);
            return visitor.layers();
        }

        std::vector<LayerNode*> World::customLayers() const {
            const std::vector<Node*>& children = Node::children();
            CollectLayersVisitor visitor;
            accept(std::next(std::begin(children)), std::end(children), visitor);
            return visitor.layers();
        }

        void World::createDefaultLayer() {
            m_defaultLayer = createLayer("Default Layer");
            addChild(m_defaultLayer);
        }

        const AttributableNodeIndex& World::attributableNodeIndex() const {
            return *m_attributableIndex;
        }

        const std::vector<IssueGenerator*>& World::registeredIssueGenerators() const {
            return m_issueGeneratorRegistry->registeredGenerators();
        }

        std::vector<IssueQuickFix*> World::quickFixes(const IssueType issueTypes) const {
            return m_issueGeneratorRegistry->quickFixes(issueTypes);
        }

        void World::registerIssueGenerator(IssueGenerator* issueGenerator) {
            m_issueGeneratorRegistry->registerGenerator(issueGenerator);
            invalidateAllIssues();
        }

        void World::unregisterAllIssueGenerators() {
            m_issueGeneratorRegistry->unregisterAllGenerators();
            invalidateAllIssues();
        }

        class World::AddNodeToNodeTree : public NodeVisitor {
        private:
            NodeTree& m_nodeTree;
        public:
            explicit AddNodeToNodeTree(NodeTree& nodeTree) :
            m_nodeTree(nodeTree) {}
        private:
            void doVisit(World*) override         {}
            void doVisit(LayerNode*) override         {}
            void doVisit(GroupNode*) override         {}
            void doVisit(Entity* entity) override { m_nodeTree.insert(entity->physicalBounds(), entity); }
            void doVisit(BrushNode* brush) override   { m_nodeTree.insert(brush->physicalBounds(), brush); }
        };

        class World::RemoveNodeFromNodeTree : public NodeVisitor {
        private:
            NodeTree& m_nodeTree;
        public:
            explicit RemoveNodeFromNodeTree(NodeTree& nodeTree) :
            m_nodeTree(nodeTree) {}
        private:
            void doVisit(World*) override         {}
            void doVisit(LayerNode*) override         {}
            void doVisit(GroupNode*) override         {}
            void doVisit(Entity* entity) override { doRemove(entity, entity->physicalBounds()); }
            void doVisit(BrushNode* brush) override   { doRemove(brush, brush->physicalBounds()); }

            void doRemove(Node* node, const vm::bbox3& bounds) {
                if (!m_nodeTree.remove(node)) {
                    auto str = std::stringstream();
                    str << "Node not found with bounds " << bounds << ": " << node;
                    throw NodeTreeException(str.str());
                }
            }
        };

        class World::UpdateNodeInNodeTree : public NodeVisitor {
        private:
            NodeTree& m_nodeTree;
        public:
            explicit UpdateNodeInNodeTree(NodeTree& nodeTree) :
            m_nodeTree(nodeTree) {}
        private:
            void doVisit(World*) override         {}
            void doVisit(LayerNode*) override         {}
            void doVisit(GroupNode*) override         {}
            void doVisit(Entity* entity) override { m_nodeTree.update(entity->physicalBounds(), entity); }
            void doVisit(BrushNode* brush) override   { m_nodeTree.update(brush->physicalBounds(), brush); }
        };

        class World::MatchTreeNodes {
        public:
            bool operator()(const Model::Node* node) const   { return node->shouldAddToSpacialIndex(); }
        };

        void World::disableNodeTreeUpdates() {
            m_updateNodeTree = false;
        }

        void World::enableNodeTreeUpdates() {
            m_updateNodeTree = true;
        }

        void World::rebuildNodeTree() {
            using CollectTreeNodes = CollectMatchingNodesVisitor<MatchTreeNodes>;

            CollectTreeNodes collect;
            acceptAndRecurse(collect);

            m_nodeTree->clearAndBuild(collect.nodes(), [](const auto* node){ return node->physicalBounds(); });
        }

        class World::InvalidateAllIssuesVisitor : public NodeVisitor {
        private:
            void doVisit(World* world) override   { invalidateIssues(world);  }
            void doVisit(LayerNode* layer) override   { invalidateIssues(layer);  }
            void doVisit(GroupNode* group) override   { invalidateIssues(group);  }
            void doVisit(Entity* entity) override { invalidateIssues(entity); }
            void doVisit(BrushNode* brush) override   { invalidateIssues(brush);  }

            void invalidateIssues(Node* node) { node->invalidateIssues(); }
        };

        void World::invalidateAllIssues() {
            InvalidateAllIssuesVisitor visitor;
            acceptAndRecurse(visitor);
        }

        const vm::bbox3& World::doGetLogicalBounds() const {
            // TODO: this should probably return the world bounds, as it does in Layer::doGetLogicalBounds
            static const vm::bbox3 bounds;
            return bounds;
        }

        const vm::bbox3& World::doGetPhysicalBounds() const {
            return logicalBounds();
        }

        Node* World::doClone(const vm::bbox3& /* worldBounds */) const {
            World* world = m_factory->createWorld();
            cloneAttributes(world);
            return world;
        }

        Node* World::doCloneRecursively(const vm::bbox3& worldBounds) const {
            const std::vector<Node*>& myChildren = children();
            assert(myChildren[0] == m_defaultLayer);

            World* world = m_factory->createWorld();
            cloneAttributes(world);

            world->defaultLayer()->addChildren(cloneRecursively(worldBounds, m_defaultLayer->children()));

            if (myChildren.size() > 1) {
                std::vector<Node*> childClones;
                childClones.reserve(myChildren.size() - 1);
                cloneRecursively(worldBounds, std::begin(myChildren) + 1, std::end(myChildren), std::back_inserter(childClones));
                world->addChildren(childClones);
            }

            return world;
        }

        class CanAddChildToWorld : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const World*) override  { setResult(false); }
            void doVisit(const LayerNode*) override  { setResult(true); }
            void doVisit(const GroupNode*) override  { setResult(false); }
            void doVisit(const Entity*) override { setResult(false); }
            void doVisit(const BrushNode*) override  { setResult(false); }
        };

        bool World::doCanAddChild(const Node* child) const {
            CanAddChildToWorld visitor;
            child->accept(visitor);
            return visitor.result();
        }

        class CanRemoveChildFromWorld : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            const World* m_this;
        public:
            explicit CanRemoveChildFromWorld(const World* i_this) :
            m_this(i_this) {}
        private:
            void doVisit(const World*) override        { setResult(false); }
            void doVisit(const LayerNode* layer) override  { setResult(layer != m_this->defaultLayer()); }
            void doVisit(const GroupNode*) override        { setResult(false); }
            void doVisit(const Entity*) override       { setResult(false); }
            void doVisit(const BrushNode*) override        { setResult(false); }
        };

        bool World::doCanRemoveChild(const Node* child) const {
            CanRemoveChildFromWorld visitor(this);
            child->accept(visitor);
            return visitor.result();
        }

        bool World::doRemoveIfEmpty() const {
            return false;
        }

        bool World::doShouldAddToSpacialIndex() const {
            return false;
        }

        void World::doDescendantWasAdded(Node* node, const size_t /* depth */) {
            // NOTE: `node` is just the root of a subtree that is being connected to this World.
            // In some cases, (e.g. if `node` is a Group), `node` will not be added to the spatial index, but some of its descendants may be.
            // We need to recursively search the `node` being connected and add it or any descendants that need to be added.
            if (m_updateNodeTree) {
                AddNodeToNodeTree visitor(*m_nodeTree);
                node->acceptAndRecurse(visitor);
            }
        }

        void World::doDescendantWillBeRemoved(Node* node, const size_t /* depth */) {
            if (m_updateNodeTree) {
                RemoveNodeFromNodeTree visitor(*m_nodeTree);
                node->acceptAndRecurse(visitor);
            }
        }

        void World::doDescendantPhysicalBoundsDidChange(Node* node) {
            if (m_updateNodeTree) {
                UpdateNodeInNodeTree visitor(*m_nodeTree);
                node->accept(visitor);
            }
        }

        bool World::doSelectable() const {
            return false;
        }

        void World::doPick(const vm::ray3& ray, PickResult& pickResult) {
            for (auto* node : m_nodeTree->findIntersectors(ray)) {
                node->pick(ray, pickResult);
            }
        }

        void World::doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) {
            for (auto* node : m_nodeTree->findContainers(point)) {
                node->findNodesContaining(point, result);
            }
        }

        void World::doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) {
            generator->generate(this, issues);
        }

        void World::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }

        void World::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        void World::doFindAttributableNodesWithAttribute(const std::string& name, const std::string& value, std::vector<Model::AttributableNode*>& result) const {
            kdl::vec_append(result,
                m_attributableIndex->findAttributableNodes(AttributableNodeIndexQuery::exact(name), value));
        }

        void World::doFindAttributableNodesWithNumberedAttribute(const std::string& prefix, const std::string& value, std::vector<Model::AttributableNode*>& result) const {
            kdl::vec_append(result,
                m_attributableIndex->findAttributableNodes(AttributableNodeIndexQuery::numbered(prefix), value));
        }

        void World::doAddToIndex(AttributableNode* attributable, const std::string& name, const std::string& value) {
            m_attributableIndex->addAttribute(attributable, name, value);
        }

        void World::doRemoveFromIndex(AttributableNode* attributable, const std::string& name, const std::string& value) {
            m_attributableIndex->removeAttribute(attributable, name, value);
        }

        void World::doAttributesDidChange(const vm::bbox3& /* oldBounds */) {}

        bool World::doIsAttributeNameMutable(const std::string& name) const {
            if (name == AttributeNames::Classname)
                return false;
            if (name == AttributeNames::Mods)
                return false;
            if (name == AttributeNames::EntityDefinitions)
                return false;
            if (name == AttributeNames::Wad)
                return false;
            if (name == AttributeNames::Textures)
                return false;
            return true;
        }

        bool World::doIsAttributeValueMutable(const std::string& name) const {
            if (name == AttributeNames::Mods)
                return false;
            if (name == AttributeNames::EntityDefinitions)
                return false;
            if (name == AttributeNames::Wad)
                return false;
            if (name == AttributeNames::Textures)
                return false;
            return true;
        }

        vm::vec3 World::doGetLinkSourceAnchor() const {
            return vm::vec3::zero();
        }

        vm::vec3 World::doGetLinkTargetAnchor() const {
            return vm::vec3::zero();
        }

        MapFormat World::doGetFormat() const {
            return m_factory->format();
        }

        World* World::doCreateWorld() const {
            return m_factory->createWorld();
        }

        LayerNode* World::doCreateLayer(const std::string& name) const {
            return m_factory->createLayer(name);
        }

        GroupNode* World::doCreateGroup(const std::string& name) const {
            return m_factory->createGroup(name);
        }

        Entity* World::doCreateEntity() const {
            return m_factory->createEntity();
        }

        BrushNode* World::doCreateBrush(const vm::bbox3& worldBounds, const std::vector<BrushFace*>& faces) const {
            return m_factory->createBrush(worldBounds, faces);
        }

        BrushFace* World::doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs) const {
            return m_factory->createFace(point1, point2, point3, attribs);
        }

        BrushFace* World::doCreateFace(const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY) const {
            return m_factory->createFace(point1, point2, point3, attribs, texAxisX, texAxisY);
        }

        void World::doAcceptTagVisitor(TagVisitor& visitor) {
            visitor.visit(*this);
        }

        void World::doAcceptTagVisitor(ConstTagVisitor& visitor) const {
            visitor.visit(*this);
        }
    }
}
