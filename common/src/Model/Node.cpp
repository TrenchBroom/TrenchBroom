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

#include "Node.h"

#include "CollectionUtils.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Model {
        Node::Node() :
        m_parent(NULL),
        m_familySize(1),
        m_selected(false),
        m_familyMemberSelectionCount(0) {}
        
        Node::~Node() {
            VectorUtils::clearAndDelete(m_children);
        }
        
        Node* Node::clone(const BBox3& worldBounds) const {
            return doClone(worldBounds);
        }

        NodeList Node::clone(const BBox3& worldBounds, const NodeList& nodes) {
            NodeList clones;
            clones.reserve(nodes.size());
            clone(worldBounds, nodes.begin(), nodes.end(), std::back_inserter(clones));
            return clones;
        }

        Node* Node::parent() const {
            return m_parent;
        }
        
        bool Node::hasChildren() const {
            return !m_children.empty();
        }

        size_t Node::childCount() const {
            return m_children.size();
        }

        const NodeList& Node::children() const {
            return m_children;
        }

        size_t Node::familySize() const {
            return m_familySize;
        }
        
        void Node::addChildren(const NodeList& children) {
            addChildren(children.begin(), children.end(), children.size());
        }

        void Node::addChild(Node* child) {
            doAddChild(child);
            incFamilySize(child->familySize());
            incFamilyMemberSelectionCount(child->familyMemberSelectionCount());
        }
        
        void Node::removeChild(Node* child) {
            m_children.erase(doRemoveChild(m_children.begin(), m_children.end(), child), m_children.end());
            decFamilySize(child->familySize());
            decFamilyMemberSelectionCount(child->familyMemberSelectionCount());
        }

        bool Node::canAddChild(Node* child) const {
            return doCanAddChild(child);
        }

        bool Node::canRemoveChild(Node* child) const {
            return doCanRemoveChild(child);
        }

        void Node::doAddChild(Node* child) {
            assert(child != NULL);
            assert(!VectorUtils::contains(m_children, child));
            assert(child->parent() == NULL);
            assert(canAddChild(child));

            m_children.push_back(child);
            child->setParent(this);
        }

        NodeList::iterator Node::doRemoveChild(NodeList::iterator begin, NodeList::iterator end, Node* child) {
            assert(child != NULL);
            assert(child->parent() == this);
            assert(canRemoveChild(child));

            NodeList::iterator it = std::remove(begin, end, child);
            assert(it != m_children.end());
            child->setParent(NULL);
            return it;
        }

        void Node::incFamilySize(const size_t delta) {
            if (delta == 0)
                return;
            m_familySize += delta;
            if (parent() != NULL)
                parent()->incFamilySize(delta);
        }
        
        void Node::decFamilySize(const size_t delta) {
            if (delta == 0)
                return;
            assert(m_familySize > delta);
            m_familySize -= delta;
            if (parent() != NULL)
                parent()->decFamilySize(delta);
        }

        void Node::setParent(Node* parent) {
            assert((m_parent == NULL) ^ (parent == NULL));
            if (parent == m_parent)
                return;
            
            parentWillChange();
            m_parent = parent;
            parentDidChange();
        }
        
        void Node::parentWillChange() {
            doParentWillChange();
            ancestorWillChange();
        }
        
        void Node::parentDidChange() {
            doParentDidChange();
            ancestorDidChange();
        }

        void Node::ancestorWillChange() {
            doAncestorWillChange();
            NodeList::const_iterator it, end;
            for (it = m_children.begin(), end = m_children.end(); it != end; ++it) {
                Node* child = *it;
                child->ancestorWillChange();
            }
        }

        void Node::ancestorDidChange() {
            doAncestorDidChange();
            NodeList::const_iterator it, end;
            for (it = m_children.begin(), end = m_children.end(); it != end; ++it) {
                Node* child = *it;
                child->ancestorDidChange();
            }
        }
        
        void Node::nodeDidChange() {
            if (m_parent != NULL)
                parent()->childDidChange();
        }
        
        void Node::childDidChange() {
            doChildDidChange();
            descendantDidChange();
        }

        void Node::descendantDidChange() {
            doDescendantDidChange();
            Node* node = parent();
            while (node != NULL) {
                node->descendantDidChange();
                node = node->parent();
            }
        }

        bool Node::selected() const {
            return m_selected;
        }
        
        void Node::select() {
            if (!selectable())
                return;
            assert(!m_selected);
            m_selected = true;
            incFamilyMemberSelectionCount(1);
        }
        
        void Node::deselect() {
            if (!selectable())
                return;
            assert(m_selected);
            m_selected = false;
            decFamilyMemberSelectionCount(1);
        }

        bool Node::familyMemberSelected() const {
            return m_familyMemberSelectionCount > 0;
        }
        
        size_t Node::familyMemberSelectionCount() const {
            return m_familyMemberSelectionCount;
        }
        
        void Node::familyMemberWasSelected() {
            incFamilyMemberSelectionCount(1);
        }
        
        void Node::familyMemberWasDeselected() {
            decFamilyMemberSelectionCount(1);
        }

        void Node::incFamilyMemberSelectionCount(const size_t delta) {
            if (delta == 0)
                return;
            m_familyMemberSelectionCount += delta;
            if (parent() != NULL)
                parent()->incFamilyMemberSelectionCount(delta);
        }
        
        void Node::decFamilyMemberSelectionCount(const size_t delta) {
            if (delta == 0)
                return;
            assert(m_familyMemberSelectionCount >= delta);
            m_familyMemberSelectionCount -= delta;
            if (parent() != NULL)
                parent()->decFamilyMemberSelectionCount(delta);
        }
        
        bool Node::selectable() const {
            return doSelectable();
        }

        void Node::setFilePosition(const size_t lineNumber, const size_t lineCount) {
            m_lineNumber = lineNumber;
            m_lineCount = lineCount;
        }
        
        bool Node::containsLine(const size_t lineNumber) const {
            return lineNumber >= m_lineNumber && lineNumber < m_lineNumber + m_lineCount;
        }
        
        void Node::findAttributablesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableList& result) const {
            return doFindAttributablesWithAttribute(name, value, result);
        }
        
        void Node::findAttributablesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableList& result) const {
            return doFindAttributablesWithNumberedAttribute(prefix, value, result);
        }

        void Node::addToIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            doAddToIndex(attributable, name, value);
        }
        
        void Node::removeFromIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            doRemoveFromIndex(attributable, name, value);
        }

        void Node::doAddToIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            if (m_parent != NULL)
                m_parent->addToIndex(attributable, name, value);
        }
        
        void Node::doRemoveFromIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value) {
            if (m_parent != NULL)
                m_parent->removeFromIndex(attributable, name, value);
        }

        void Node::doParentWillChange() {}
        void Node::doParentDidChange() {}
        void Node::doAncestorWillChange() {}
        void Node::doAncestorDidChange() {}

        void Node::doChildDidChange() {}
        void Node::doDescendantDidChange() {}

        void Node::doFindAttributablesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableList& result) const {
            if (m_parent != NULL)
                m_parent->findAttributablesWithAttribute(name, value, result);
        }
        
        void Node::doFindAttributablesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableList& result) const {
            if (m_parent != NULL)
                m_parent->findAttributablesWithNumberedAttribute(prefix, value, result);
        }
 }
}
