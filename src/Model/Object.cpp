/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
#include "Model/Entity.h"
#include "Model/Brush.h"

namespace TrenchBroom {
    namespace Model {
        ObjectVisitor::~ObjectVisitor() {}
        
        void ObjectVisitor::visit(Entity* entity) {
            doVisit(entity);
        }
        
        void ObjectVisitor::visit(Brush* brush) {
            doVisit(brush);
        }
        
        void ObjectVisitor::doVisit(Entity* entity) {}
        void ObjectVisitor::doVisit(Brush* brush) {}

        Object::~Object() {}

        BBox3 Object::bounds(const ObjectList& objects) {
            if (objects.empty())
                return BBox3();
            
            ObjectList::const_iterator it = objects.begin();
            const ObjectList::const_iterator end = objects.end();

            BBox3 bounds = static_cast<const Pickable*>(*it)->bounds();
            while (++it != end)
                bounds.mergeWith(static_cast<const Pickable*>(*it)->bounds());
            return bounds;
        }

        Object::Type Object::type() const {
            return m_type;
        }

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

        void Object::incChildSelectionCount() {
            ++m_childSelectionCount;
        }
        
        void Object::decChildSelectionCount() {
            --m_childSelectionCount;
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

        Object* Object::clone(const BBox3& worldBounds) const {
            return doClone(worldBounds);
        }

        void Object::transform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) {
            doTransform(transformation, lockTextures, worldBounds);
        }

        bool Object::contains(const Object& object) const {
            return doContains(object);
        }
        
        bool Object::contains(const Entity& entity) const {
            return doContains(entity);
        }
        
        bool Object::contains(const Brush& brush) const {
            return doContains(brush);
        }

        bool Object::containedBy(const Object& object) const {
            return doContainedBy(object);
        }
        
        bool Object::containedBy(const Entity& entity) const {
            return doContainedBy(entity);
        }
        
        bool Object::containedBy(const Brush& brush) const {
            return doContainedBy(brush);
        }

        bool Object::intersects(const Object& object) const {
            return doIntersects(object);
        }
        
        bool Object::intersects(const Entity& entity) const {
            return doIntersects(entity);
        }
        
        bool Object::intersects(const Brush& brush) const {
            return doIntersects(brush);
        }
        
        void Object::visit(ObjectVisitor& visitor) {
            doVisit(visitor);
        }

        Object::Object(const Type type) :
        m_type(type),
        m_lineNumber(0),
        m_lineCount(0),
        m_selected(false),
        m_childSelectionCount(0),
        m_hiddenIssues(0) {}
    }
}
