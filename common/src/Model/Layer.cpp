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

#include "Layer.h"

#include "Model/Brush.h"
#include "Model/Group.h"
#include "Model/Entity.h"
#include "Model/IssueGenerator.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        Layer::Layer(const String& name, const BBox3& worldBounds) :
        m_name(name),
        m_octree(worldBounds, static_cast<FloatType>(64.0f)) {}
        
        void Layer::setName(const String& name) {
            m_name = name;
        }

        const String& Layer::doGetName() const {
            return m_name;
        }

        const BBox3& Layer::doGetBounds() const {
            return m_octree.bounds();
        }

        Node* Layer::doClone(const BBox3& worldBounds) const {
            Layer* layer = new Layer(m_name, worldBounds);
            cloneAttributes(layer);
            layer->addChildren(clone(worldBounds, children()));
            return layer;
        }

        class CanAddChildToLayer : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const World* world)   { setResult(false); }
            void doVisit(const Layer* layer)   { setResult(false); }
            void doVisit(const Group* group)   { setResult(true); }
            void doVisit(const Entity* entity) { setResult(true); }
            void doVisit(const Brush* brush)   { setResult(true); }
        };

        bool Layer::doCanAddChild(const Node* child) const {
            CanAddChildToLayer visitor;
            child->accept(visitor);
            return visitor.result();
        }
        
        bool Layer::doCanRemoveChild(const Node* child) const {
            return true;
        }
        
        bool Layer::doRemoveIfEmpty() const {
            return false;
        }

        class Layer::AddNodeToOctree : public NodeVisitor {
        private:
            NodeTree& m_octree;
        public:
            AddNodeToOctree(NodeTree& octree) :
            m_octree(octree) {}
        private:
            void doVisit(World* world)   {}
            void doVisit(Layer* layer)   {}
            void doVisit(Group* group)   { m_octree.addObject(group->bounds(), group); }
            void doVisit(Entity* entity) { m_octree.addObject(entity->bounds(), entity); }
            void doVisit(Brush* brush)   { m_octree.addObject(brush->bounds(), brush); }
        };
        
        class Layer::RemoveNodeFromOctree : public NodeVisitor {
        private:
            NodeTree& m_octree;
        public:
            RemoveNodeFromOctree(NodeTree& octree) :
            m_octree(octree) {}
        private:
            void doVisit(World* world)   {}
            void doVisit(Layer* layer)   {}
            void doVisit(Group* group)   { m_octree.removeObject(group); }
            void doVisit(Entity* entity) { m_octree.removeObject(entity); }
            void doVisit(Brush* brush)   { m_octree.removeObject(brush); }
        };
        
        class Layer::UpdateNodeInOctree : public NodeVisitor {
        private:
            NodeTree& m_octree;
        public:
            UpdateNodeInOctree(NodeTree& octree) :
            m_octree(octree) {}
        private:
            void doVisit(World* world)   {}
            void doVisit(Layer* layer)   {}
            void doVisit(Group* group)   { m_octree.updateObject(group->bounds(), group); }
            void doVisit(Entity* entity) { m_octree.updateObject(entity->bounds(), entity); }
            void doVisit(Brush* brush)   { m_octree.updateObject(brush->bounds(), brush); }
        };

        void Layer::doChildWasAdded(Node* node) {
            AddNodeToOctree visitor(m_octree);
            node->accept(visitor);
        }
        
        void Layer::doChildWillBeRemoved(Node* node) {
            RemoveNodeFromOctree visitor(m_octree);
            node->accept(visitor);
        }
        
        void Layer::doChildBoundsDidChange(Node* node) {
            UpdateNodeInOctree visitor(m_octree);
            node->accept(visitor);
        }

        bool Layer::doSelectable() const {
            return false;
        }

        void Layer::doGenerateIssues(const IssueGenerator* generator, IssueList& issues) {
            generator->generate(this, issues);
        }
        
        void Layer::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void Layer::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        void Layer::doPick(const Ray3& ray, PickResult& pickResult) const {
            const Model::NodeList candidates = m_octree.findObjects(ray);
            NodeList::const_iterator it, end;
            for (it = candidates.begin(), end = candidates.end(); it != end; ++it) {
                const Node* node = *it;
                node->pick(ray, pickResult);
            }
        }
        
        FloatType Layer::doIntersectWithRay(const Ray3& ray) const {
            return Math::nan<FloatType>();
        }
    }
}
