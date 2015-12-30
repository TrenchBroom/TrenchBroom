/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/IssueGenerator.h"
#include "Model/Layer.h"
#include "Model/NodeVisitor.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        World::World(MapFormat::Type mapFormat, const BrushContentTypeBuilder* brushContentTypeBuilder, const BBox3& worldBounds) :
        m_factory(mapFormat, brushContentTypeBuilder),
        m_defaultLayer(NULL) {
            addOrUpdateAttribute(AttributeNames::Classname, AttributeValues::WorldspawnClassname);
            createDefaultLayer(worldBounds);
        }

        Layer* World::defaultLayer() const {
            assert(m_defaultLayer != NULL);
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
            accept(children.begin() + 1, children.end(), visitor);
            return visitor.layers();
        }

        void World::createDefaultLayer(const BBox3& worldBounds) {
            m_defaultLayer = createLayer("Default Layer", worldBounds);
            addChild(m_defaultLayer);
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
            void doVisit(World* world)   { invalidateIssues(world);  }
            void doVisit(Layer* layer)   { invalidateIssues(layer);  }
            void doVisit(Group* group)   { invalidateIssues(group);  }
            void doVisit(Entity* entity) { invalidateIssues(entity); }
            void doVisit(Brush* brush)   { invalidateIssues(brush);  }
            
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

            world->defaultLayer()->addChildren(clone(worldBounds, m_defaultLayer->children()));
            
            if (myChildren.size() > 1) {
                NodeList childClones;
                childClones.reserve(myChildren.size() - 1);
                clone(worldBounds, myChildren.begin() + 1, myChildren.end(), std::back_inserter(childClones));
                world->addChildren(childClones);
            }
            
            return world;
        }

        class CanAddChildToWorld : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const World* world)   { setResult(false); }
            void doVisit(const Layer* layer)   { setResult(true); }
            void doVisit(const Group* group)   { setResult(false); }
            void doVisit(const Entity* entity) { setResult(false); }
            void doVisit(const Brush* brush)   { setResult(false); }
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
            CanRemoveChildFromWorld(const World* i_this) :
            m_this(i_this) {}
        private:
            void doVisit(const World* world)   { setResult(false); }
            void doVisit(const Layer* layer)   { setResult(layer != m_this->defaultLayer()); }
            void doVisit(const Group* group)   { setResult(false); }
            void doVisit(const Entity* entity) { setResult(false); }
            void doVisit(const Brush* brush)   { setResult(false); }
        };
        
        bool World::doCanRemoveChild(const Node* child) const {
            CanRemoveChildFromWorld visitor(this);
            child->accept(visitor);
            return visitor.result();
        }

        bool World::doRemoveIfEmpty() const {
            return false;
        }

        bool World::doSelectable() const {
            return false;
        }

        void World::doPick(const Ray3& ray, PickResult& pickResult) const {
            const NodeList& children = Node::children();
            NodeList::const_iterator it, end;
            for (it = children.begin(), end = children.end(); it != end; ++it) {
                const Node* child = *it;
                child->pick(ray, pickResult);
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

        void World::doAttributesDidChange() {}

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
