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

#include "Model/AssortNodesVisitor.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/CollectNodesWithDescendantSelectionCountVisitor.h"
#include "Model/IssueGenerator.h"

namespace TrenchBroom {
    namespace Model {
        World::World(MapFormat::Type mapFormat, const BrushContentTypeBuilder* brushContentTypeBuilder, const BBox3& worldBounds) :
        m_factory(mapFormat, brushContentTypeBuilder),
        m_defaultLayer(nullptr) {
            addOrUpdateAttribute(AttributeNames::Classname, AttributeValues::WorldspawnClassname);
            createDefaultLayer(worldBounds);
        }

        Layer* World::defaultLayer() const {
            ensure(m_defaultLayer != nullptr, "defaultLayer is null");
            return m_defaultLayer;
        }

        LayerList World::allLayers() const {
            CollectLayersVisitor visitor;
            iterate(visitor);
            return visitor.layers();
        }
        
        LayerList World::customLayers() const {
            const NodeList& children = Node::children();
            CollectLayersVisitor visitor;
            accept(std::begin(children) + 1, std::end(children), visitor);
            return visitor.layers();
        }

        void World::createDefaultLayer(const BBox3& worldBounds) {
            m_defaultLayer = createLayer("Default Layer", worldBounds);
            addChild(m_defaultLayer);
        }
        
        const AttributableNodeIndex& World::attributableNodeIndex() const {
            return m_attributableIndex;
        }

        const IssueGeneratorList& World::registeredIssueGenerators() const {
            return m_issueGeneratorRegistry.registeredGenerators();
        }

        IssueQuickFixList World::quickFixes(const IssueType issueTypes) const {
            return m_issueGeneratorRegistry.quickFixes(issueTypes);
        }

        void World::registerIssueGenerator(IssueGenerator* issueGenerator) {
            m_issueGeneratorRegistry.registerGenerator(issueGenerator);
            invalidateAllIssues();
        }

        void World::unregisterAllIssueGenerators() {
            m_issueGeneratorRegistry.unregisterAllGenerators();
            invalidateAllIssues();
        }

        class World::InvalidateAllIssuesVisitor : public NodeVisitor {
        private:
            void doVisit(World* world) override   { invalidateIssues(world);  }
            void doVisit(Layer* layer) override   { invalidateIssues(layer);  }
            void doVisit(Group* group) override   { invalidateIssues(group);  }
            void doVisit(Entity* entity) override { invalidateIssues(entity); }
            void doVisit(Brush* brush) override   { invalidateIssues(brush);  }
            
            void invalidateIssues(Node* node) { node->invalidateIssues(); }
        };
        
        void World::invalidateAllIssues() {
            InvalidateAllIssuesVisitor visitor;
            acceptAndRecurse(visitor);
        }

        const BBox3& World::doGetBounds() const {
            // TODO: this should probably return the world bounds, as it does in Layer::doGetBounds
            static const BBox3 bounds;
            return bounds;
        }

        Node* World::doClone(const BBox3& worldBounds) const {
            World* world = m_factory.createWorld(worldBounds);
            cloneAttributes(world);
            return world;
        }

        Node* World::doCloneRecursively(const BBox3& worldBounds) const {
            const NodeList& myChildren = children();
            assert(myChildren[0] == m_defaultLayer);
            
            World* world = m_factory.createWorld(worldBounds);
            cloneAttributes(world);

            world->defaultLayer()->addChildren(cloneRecursively(worldBounds, m_defaultLayer->children()));
            
            if (myChildren.size() > 1) {
                NodeList childClones;
                childClones.reserve(myChildren.size() - 1);
                cloneRecursively(worldBounds, std::begin(myChildren) + 1, std::end(myChildren), std::back_inserter(childClones));
                world->addChildren(childClones);
            }
            
            return world;
        }

        class CanAddChildToWorld : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const World* world) override   { setResult(false); }
            void doVisit(const Layer* layer) override   { setResult(true); }
            void doVisit(const Group* group) override   { setResult(false); }
            void doVisit(const Entity* entity) override { setResult(false); }
            void doVisit(const Brush* brush) override   { setResult(false); }
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
            void doVisit(const World* world) override   { setResult(false); }
            void doVisit(const Layer* layer) override   { setResult(layer != m_this->defaultLayer()); }
            void doVisit(const Group* group) override   { setResult(false); }
            void doVisit(const Entity* entity) override { setResult(false); }
            void doVisit(const Brush* brush) override   { setResult(false); }
        };
        
        bool World::doCanRemoveChild(const Node* child) const {
            CanRemoveChildFromWorld visitor(this);
            child->accept(visitor);
            return visitor.result();
        }

        bool World::doRemoveIfEmpty() const {
            return false;
        }

        class World::AddNodeToNodeTree : public NodeVisitor {
        private:
            NodeTree& m_nodeTree;
        public:
            AddNodeToNodeTree(NodeTree& nodeTree) :
                    m_nodeTree(nodeTree) {}
        private:
            void doVisit(World* world) override   {}
            void doVisit(Layer* layer) override   {}
            void doVisit(Group* group) override   { m_nodeTree.insert(group->bounds(), group); }
            void doVisit(Entity* entity) override { m_nodeTree.insert(entity->bounds(), entity); }
            void doVisit(Brush* brush) override   { m_nodeTree.insert(brush->bounds(), brush); }
        };

        class World::RemoveNodeFromNodeTree : public NodeVisitor {
        private:
            NodeTree& m_nodeTree;
            const BBox3 m_oldBounds;
        public:
            RemoveNodeFromNodeTree(NodeTree& nodeTree, const BBox3& oldBounds) :
                    m_nodeTree(nodeTree),
                    m_oldBounds(oldBounds) {}
        private:
            void doVisit(World* world) override   {}
            void doVisit(Layer* layer) override   {}
            void doVisit(Group* group) override   { m_nodeTree.remove(m_oldBounds, group); }
            void doVisit(Entity* entity) override { m_nodeTree.remove(m_oldBounds, entity); }
            void doVisit(Brush* brush) override   { m_nodeTree.remove(m_oldBounds, brush); }
        };

        class World::UpdateNodeInNodeTree : public NodeVisitor {
        private:
            NodeTree& m_nodeTree;
            const BBox3 m_oldBounds;
        public:
            UpdateNodeInNodeTree(NodeTree& nodeTree, const BBox3& oldBounds) :
                    m_nodeTree(nodeTree),
                    m_oldBounds(oldBounds) {}
        private:
            void doVisit(World* world) override   {}
            void doVisit(Layer* layer) override   {}
            void doVisit(Group* group) override   { m_nodeTree.update(m_oldBounds, group->bounds(), group); }
            void doVisit(Entity* entity) override { m_nodeTree.update(m_oldBounds, entity->bounds(), entity); }
            void doVisit(Brush* brush) override   { m_nodeTree.update(m_oldBounds, brush->bounds(), brush); }
        };

        void World::doDescendantWasAdded(Node* node, const size_t depth) {
            if (depth == 2) {
                AddNodeToNodeTree visitor(m_nodeTree);
                node->accept(visitor);
            }
        }

        void World::doDescendantWillBeRemoved(Node* node, const size_t depth) {
            if (depth == 2) {
                RemoveNodeFromNodeTree visitor(m_nodeTree, node->bounds());
                node->accept(visitor);
            }
        }

        void World::doDescendantBoundsDidChange(Node* node, const BBox3& oldBounds, const size_t depth) {
            if (depth == 2) {
                UpdateNodeInNodeTree visitor(m_nodeTree, oldBounds);
                node->accept(visitor);
            }
        }

        bool World::doSelectable() const {
            return false;
        }

        void World::doPick(const Ray3& ray, PickResult& pickResult) const {
            for (const auto* node : m_nodeTree.findIntersectors(ray)) {
                node->pick(ray, pickResult);
            }
        }
        
        void World::doFindNodesContaining(const Vec3& point, NodeList& result) {
            for (auto* node : m_nodeTree.findContainers(point)) {
                node->findNodesContaining(point, result);
            }
        }

        FloatType World::doIntersectWithRay(const Ray3& ray) const {
            return Math::nan<FloatType>();
        }

        void World::doGenerateIssues(const IssueGenerator* generator, IssueList& issues) {
            generator->generate(this, issues);
        }

        void World::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void World::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }
        
        void World::doFindAttributableNodesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableNodeList& result) const {
            VectorUtils::append(result, m_attributableIndex.findAttributableNodes(AttributableNodeIndexQuery::exact(name), value));
        }
        
        void World::doFindAttributableNodesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableNodeList& result) const {
            VectorUtils::append(result, m_attributableIndex.findAttributableNodes(AttributableNodeIndexQuery::numbered(prefix), value));
        }
        
        void World::doAddToIndex(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value) {
            m_attributableIndex.addAttribute(attributable, name, value);
        }
        
        void World::doRemoveFromIndex(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value) {
            m_attributableIndex.removeAttribute(attributable, name, value);
        }

        void World::doAttributesDidChange(const BBox3& oldBounds) {}

        bool World::doIsAttributeNameMutable(const AttributeName& name) const {
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
        
        bool World::doIsAttributeValueMutable(const AttributeName& name) const {
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

        Vec3 World::doGetLinkSourceAnchor() const {
            return Vec3::Null;
        }
        
        Vec3 World::doGetLinkTargetAnchor() const {
            return Vec3::Null;
        }

        MapFormat::Type World::doGetFormat() const {
            return m_factory.format();
        }

        World* World::doCreateWorld(const BBox3& worldBounds) const {
            return m_factory.createWorld(worldBounds);
        }
        
        Layer* World::doCreateLayer(const String& name, const BBox3& worldBounds) const {
            return m_factory.createLayer(name, worldBounds);
        }
        
        Group* World::doCreateGroup(const String& name) const {
            return m_factory.createGroup(name);
        }
        
        Entity* World::doCreateEntity() const {
            return m_factory.createEntity();
        }
        
        Brush* World::doCreateBrush(const BBox3& worldBounds, const BrushFaceList& faces) const {
            return m_factory.createBrush(worldBounds, faces);
        }
        
        BrushFace* World::doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const BrushFaceAttributes& attribs) const {
            return m_factory.createFace(point1, point2, point3, attribs);
        }
        
        BrushFace* World::doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const BrushFaceAttributes& attribs, const Vec3& texAxisX, const Vec3& texAxisY) const {
            return m_factory.createFace(point1, point2, point3, attribs, texAxisX, texAxisY);
        }
    }
}
