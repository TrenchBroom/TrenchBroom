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

#include "Object.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Layer.h"

#include <memory>

namespace TrenchBroom {
    namespace Model {
        ObjectVisitor::~ObjectVisitor() {}
        
        void ObjectVisitor::visit(Entity* entity) {
            doVisit(entity);
        }
        
        void ObjectVisitor::visit(Brush* brush) {
            doVisit(brush);
        }
        
        ObjectQuery::~ObjectQuery() {}
        
        void ObjectQuery::query(const Entity* entity) {
            return doQuery(entity);
        }
        
        void ObjectQuery::query(const Brush* brush) {
            return doQuery(brush);
        }

        Object::~Object() {}

        size_t Object::filePosition() const {
            return m_lineNumber;
        }

        void Object::setFilePosition(const size_t lineNumber, const size_t lineCount) {
            m_lineNumber = lineNumber;
            m_lineCount = lineCount;
        }

        bool Object::containsLine(const size_t line) const {
            return m_lineNumber <= line && line < m_lineNumber + m_lineCount;
        }

        bool Object::selected() const {
            return m_selected;
        }
        
        void Object::select() {
            assert(!m_selected);
            m_selected = true;
        }
        
        void Object::deselect() {
            assert(m_selected);
            m_selected = false;
        }

        bool Object::partiallySelected() const {
            return m_childSelectionCount > 0;
        }
        
        size_t Object::childSelectionCount() const {
            return m_childSelectionCount;
        }

        void Object::childSelectionChanged(const bool newValue) {
            if (newValue)
                incChildSelectionCount();
            else
                decChildSelectionCount();
        }

        IssueType Object::hiddenIssues() const {
            return m_hiddenIssues;
        }
        
        void Object::setHiddenIssues(IssueType hiddenIssues) {
            m_hiddenIssues = hiddenIssues;
        }
        
        bool Object::isIssueHidden(const Issue* issue) const {
            assert(issue != NULL);
            return issue->hasType(m_hiddenIssues);
        }
        
        void Object::setIssueHidden(const IssueType type, const bool hidden) {
            if (hidden)
                m_hiddenIssues |= type;
            else
                m_hiddenIssues &= ~type;
        }

        Layer* Object::layer() const {
            return m_layer;
        }
        
        class AddToLayer : public ObjectVisitor {
        private:
            Layer* m_layer;
        public:
            AddToLayer(Layer* layer) :
            m_layer(layer) {
                assert(m_layer != NULL);
            }
            
            void doVisit(Entity* entity) {
                m_layer->addEntity(entity);
            }
            
            void doVisit(Brush* brush) {
                m_layer->addBrush(brush);
            }
        };
        
        class RemoveFromLayer : public ObjectVisitor {
        private:
            Layer* m_layer;
        public:
            RemoveFromLayer(Layer* layer) :
            m_layer(layer) {
                assert(m_layer != NULL);
            }
            
            void doVisit(Entity* entity) {
                m_layer->removeEntity(entity);
            }
            
            void doVisit(Brush* brush) {
                m_layer->removeBrush(brush);
            }
        };

        void Object::setLayer(Layer* layer) {
            if (m_layer == layer)
                return;
            
            if (m_layer != NULL) {
                RemoveFromLayer remove(m_layer);
                accept(remove);
            }

            m_layer = layer;

            if (m_layer != NULL) {
                AddToLayer add(m_layer);
                accept(add);
            }
        }
        

        Object* Object::clone(const BBox3& worldBounds) const {
            return doClone(worldBounds);
        }

        void Object::transform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) {
            doTransform(transformation, lockTextures, worldBounds);
        }

        bool Object::contains(const Object& object) const {
            return doContains(object);
        }
        
        bool Object::intersects(const Object& object) const {
            return doIntersects(object);
        }
        
        void Object::accept(ObjectVisitor& visitor) {
            doAccept(visitor);
        }

        void Object::accept(ObjectQuery& query) const {
            return doAccept(query);
        }

        void Object::acceptRecursively(ObjectVisitor& visitor) {
            doAcceptRecursively(visitor);
        }

        void Object::acceptRecursively(ObjectQuery& visitor) const {
            doAcceptRecursively(visitor);
        }
        
        Object::Object() :
        m_lineNumber(0),
        m_lineCount(0),
        m_selected(false),
        m_childSelectionCount(0),
        m_hiddenIssues(0),
        m_layer(NULL) {}

        void Object::incChildSelectionCount() {
            ++m_childSelectionCount;
        }
        
        void Object::decChildSelectionCount() {
            --m_childSelectionCount;
        }
    }
}
