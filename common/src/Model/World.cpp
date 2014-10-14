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
#include "Model/Layer.h"
#include "Model/NodeVisitor.h"
#include "Model/Picker.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        World::World(MapFormat::Type mapFormat, const BrushContentTypeBuilder* brushContentTypeBuilder) :
        m_factory(mapFormat, brushContentTypeBuilder),
        m_defaultLayer(NULL),
        m_picker(BBox3(32768.0)) {
            createDefaultLayer();
        }

        Model::MapFormat::Type World::format() const {
            return m_factory.format();
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

        void World::createDefaultLayer() {
            m_defaultLayer = createLayer("Default Layer");
            addChild(m_defaultLayer);
        }

        class UpdateIssuesVisitor : public NodeVisitor {
        private:
            const IssueGenerator& m_generator;
        public:
            UpdateIssuesVisitor(const IssueGenerator& generator) :
            m_generator(generator) {}
        private:
            void doVisit(World* world)   {  world->updateIssues(m_generator); }
            void doVisit(Layer* layer)   {  layer->updateIssues(m_generator); }
            void doVisit(Group* group)   {  group->updateIssues(m_generator); }
            void doVisit(Entity* entity) { entity->updateIssues(m_generator); }
            void doVisit(Brush* brush)   {  brush->updateIssues(m_generator); }
        };
        
        void World::registerIssueGenerators(const IssueGeneratorList& generators) {
            IssueGeneratorList::const_iterator it, end;
            for (it = generators.begin(), end = generators.end(); it != end; ++it) {
                IssueGenerator* generator = *it;
                m_issueGeneratorRegistry.registerGenerator(generator);
            }
            updateAllIssues();
        }

        void World::unregisterAllIssueGenerators() {
            m_issueGeneratorRegistry.unregisterAllGenerators();
            updateAllIssues();
        }

        void World::updateAllIssues() {
            UpdateIssuesVisitor visitor(m_issueGeneratorRegistry);
            acceptAndRecurse(visitor);
        }

        Hits World::pick(const Ray3& ray) const {
            return m_picker.pick(ray);
        }

        Node* World::doClone(const BBox3& worldBounds) const {
            const NodeList& myChildren = children();
            assert(myChildren[0] == m_defaultLayer);

            World* world = m_factory.createWorld();
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

        class AddNodeToPicker : public NodeVisitor {
        private:
            Picker& m_picker;
        public:
            AddNodeToPicker(Picker& picker) :
            m_picker(picker) {}
        private:
            void doVisit(World* world)   { }
            void doVisit(Layer* layer)   { }
            void doVisit(Group* group)   { m_picker.addObject(group); }
            void doVisit(Entity* entity) { m_picker.addObject(entity); }
            void doVisit(Brush* brush)   { m_picker.addObject(brush); }
        };
        
        class RemoveNodeFromPicker : public NodeVisitor {
        private:
            Picker& m_picker;
        public:
            RemoveNodeFromPicker(Picker& picker) :
            m_picker(picker) {}
        private:
            void doVisit(World* world)   { }
            void doVisit(Layer* layer)   { }
            void doVisit(Group* group)   { m_picker.removeObject(group); }
            void doVisit(Entity* entity) { m_picker.removeObject(entity); }
            void doVisit(Brush* brush)   { m_picker.removeObject(brush); }
        };
        
        void World::doDescendantWasAdded(Node* node) {
            AddNodeToPicker visitor(m_picker);
            node->acceptAndRecurse(visitor);
        }
        
        void World::doDescendantWasRemoved(Node* node) {
            RemoveNodeFromPicker visitor(m_picker);
            node->acceptAndRecurse(visitor);
        }

        void World::doDescendantWillChange(Node* node) {
            RemoveNodeFromPicker visitor(m_picker);
            node->accept(visitor);
        }
        
        void World::doDescendantDidChange(Node* node) {
            AddNodeToPicker visitor(m_picker);
            node->accept(visitor);
        }

        bool World::doSelectable() const {
            return false;
        }

        void World::doUpdateIssues(Node* node) {
            node->updateIssues(m_issueGeneratorRegistry);
        }

        void World::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void World::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }
        
        void World::doFindAttributablesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableList& result) const {
            VectorUtils::append(result, m_attributableIndex.findAttributables(AttributableIndexQuery::exact(name),
                                                                              AttributableIndexQuery::exact(value)));
        }
        
        void World::doFindAttributablesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableList& result) const {
            VectorUtils::append(result, m_attributableIndex.findAttributables(AttributableIndexQuery::numbered(prefix),
                                                                              AttributableIndexQuery::exact(value)));
        }
        
        void World::doAddToIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            m_attributableIndex.addAttribute(attributable, name, value);
        }
        
        void World::doRemoveFromIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            m_attributableIndex.removeAttribute(attributable, name, value);
        }

        void World::doAttributesDidChange() {}

        bool isAttributeMutable(const AttributeName& name);
        bool isAttributeMutable(const AttributeName& name) {
            if (name == AttributeNames::Classname)
                return false;
            if (name == AttributeNames::Mods)
                return false;
            if (name == AttributeNames::EntityDefinitions)
                return false;
            if (name == AttributeNames::Wad)
                return false;
            if (name == AttributeNames::Wal)
                return false;
            return true;
        }
        
        bool World::doCanAddOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) const {
            return isAttributeMutable(name);
        }
        
        bool World::doCanRenameAttribute(const AttributeName& name, const AttributeName& newName) const {
            return isAttributeMutable(name) && isAttributeMutable(newName);
        }
        
        bool World::doCanRemoveAttribute(const AttributeName& name) const {
            return isAttributeMutable(name);
        }

        World* World::doCreateWorld() const {
            return m_factory.createWorld();
        }
        
        Layer* World::doCreateLayer(const String& name) const {
            return m_factory.createLayer(name);
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
        
        BrushFace* World::doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName) const {
            return m_factory.createFace(point1, point2, point3, textureName);
        }
        
        BrushFace* World::doCreateFace(const Vec3& point1, const Vec3& point2, const Vec3& point3, const String& textureName, const Vec3& texAxisX, const Vec3& texAxisY) const {
            return m_factory.createFace(point1, point2, point3, textureName, texAxisX, texAxisY);
        }
    }
}
