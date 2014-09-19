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
        m_descendantCount(0),
        m_childSelectionCount(0),
        m_descendantSelectionCount(0) {}
        
        Node::~Node() {
            VectorUtils::clearAndDelete(m_children);
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

        size_t Node::descendantCount() const {
            return m_descendantCount;
        }
        
        void Node::addChild(Node* child) {
            doAddChild(child);
            incDescendantCount(child->descendantCount() + 1);
        }
        
        void Node::removeChild(Node* child) {
            m_children.erase(doRemoveChild(child), m_children.end());
            decDescendantCount(child->descendantCount() + 1);
        }

        bool Node::canAddChild(Node* child) const {
            return doCanAddChild(child);
        }

        bool Node::canRemoveChild(Node* child) const {
            return doCanRemoveChild(child);
        }

        void Node::doAddChild(Node* child) {
            assert(child != NULL);
            assert(VectorUtils::contains(m_children, child));
            assert(child->parent() == NULL);
            assert(canAddChild(child));

            attachChild(child);
            m_children.push_back(child);
        }

        NodeList::iterator Node::doRemoveChild(Node* child) {
            assert(child != NULL);
            assert(child->parent() == this);
            assert(canRemoveChild(child));

            NodeList::iterator it = std::remove(m_children.begin(), m_children.end(), child);
            assert(it != m_children.end());
            detachChild(*it);
            return it;
        }

        void Node::attachChild(Node* child) {
            child->setParent(this);
        }
        
        void Node::detachChild(Node* child) {
            child->setParent(NULL);
        }

        void Node::incDescendantCount(const size_t count) {
            if (count == 0)
                return;
            m_descendantCount += count;
            if (parent() != NULL)
                parent()->incDescendantCount(count);
        }
        
        void Node::decDescendantCount(const size_t count) {
            if (count == 0)
                return;
            assert(m_descendantCount >= count);
            m_descendantCount -= count;
            if (parent() != NULL)
                parent()->decDescendantCount(count);
        }

        void Node::setParent(Node* parent) {
            assert(parent != NULL);
            assert(m_parent == NULL);
            assert(!descendantSelected());
            
            if (parent == m_parent)
                return;
            
            parentWillChange();
            ancestorWillChange();
            m_parent = parent;
            parentDidChange();
            ancestorDidChange();
        }
        
        void Node::parentWillChange() {
            parentWillChange();
        }
        
        void Node::parentDidChange() {
            parentDidChange();
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

        bool Node::childSelected() const {
            return m_childSelectionCount > 0;
        }
        
        size_t Node::childSelectionCount() const {
            return m_childSelectionCount;
        }
        
        bool Node::descendantSelected() const {
            return m_descendantSelectionCount > 0;
        }
        
        size_t Node::descendantSelectionCount() const {
            return m_descendantSelectionCount;
        }
        
        void Node::childWasSelected() {
            ++m_childSelectionCount;
            
            Node* node = this;
            do {
                node->descendantWasSelected();
                node = node->parent();
            } while (node != NULL);
        }
        
        void Node::childWasDeselected() {
            --m_childSelectionCount;
            
            Node* node = this;
            do {
                node->descendantWasDeselected();
                node = node->parent();
            } while (node != NULL);
        }
        
        void Node::descendantWasSelected() {
            ++m_descendantSelectionCount;
        }
        
        void Node::descendantWasDeselected() {
            --m_descendantSelectionCount;
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
