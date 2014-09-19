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

#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/NodeVisitor.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        World::World() :
        m_defaultLayer(NULL) {
            createDefaultLayer();
        }

        Layer* World::createLayer(const String& name) const {
            return new Layer(name);
        }
        
        Group* World::createGroup(const String& name) const {
            return new Group(name);
        }
        
        Entity* World::createEntity() const {
            return new Entity();
        }
        
        Brush* World::createBrush(const BBox3& worldBounds, const BrushFaceList& faces) const {
            assert(false);
            return NULL;
        }

        void World::createDefaultLayer() {
            m_defaultLayer = createLayer("Default Layer");
            addChild(m_defaultLayer);
        }
        
        Layer* World::defaultLayer() const {
            assert(m_defaultLayer != NULL);
            return m_defaultLayer;
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
    }
}
