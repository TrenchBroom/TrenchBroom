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
#include "Model/Issue.h"
#include "Model/IssueGenerator.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Model {
        Node::Node() :
        m_parent(NULL),
        m_descendantCount(0),
        m_selected(false),
        m_descendantSelectionCount(0),
        m_familyIssueCount(0) {}
        
        Node::~Node() {
            clearChildren();
            clearIssues();
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

        size_t Node::descendantCount() const {
            return m_descendantCount;
        }
        
        size_t Node::familySize() const {
            return m_descendantCount + 1;
        }

        void Node::addChildren(const NodeList& children) {
            addChildren(children.begin(), children.end(), children.size());
        }

        void Node::addChild(Node* child) {
            doAddChild(child);
            incDescendantCount(child->descendantCount() + 1);
            incDescendantSelectionCount(child->descendantSelectionCount() + (child->selected() ? 1 : 0));
        }
        
        void Node::removeChild(Node* child) {
            m_children.erase(doRemoveChild(m_children.begin(), m_children.end(), child), m_children.end());
            decDescendantCount(child->descendantCount() + 1);
            decDescendantSelectionCount(child->descendantSelectionCount() + (child->selected() ? 1 : 0));
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
            descendantWasAdded(child);
            incFamilyIssueCount(child->familyIssueCount());
        }

        NodeList::iterator Node::doRemoveChild(NodeList::iterator begin, NodeList::iterator end, Node* child) {
            assert(child != NULL);
            assert(child->parent() == this);
            assert(canRemoveChild(child));

            NodeList::iterator it = std::remove(begin, end, child);
            assert(it != m_children.end());
            child->setParent(NULL);
            descendantWasRemoved(child);
            decFamilyIssueCount(child->familyIssueCount());
            return it;
        }
        
        void Node::clearChildren() {
            VectorUtils::clearAndDelete(m_children);
        }

        void Node::descendantWasAdded(Node* node) {
            doDescendantWasAdded(node);
            if (m_parent != NULL)
                m_parent->descendantWasAdded(node);
        }
        
        void Node::descendantWasRemoved(Node* node) {
            doDescendantWasRemoved(node);
            if (m_parent != NULL)
                m_parent->descendantWasRemoved(node);
        }

        void Node::incDescendantCount(const size_t delta) {
            if (delta == 0)
                return;
            m_descendantCount += delta;
            if (m_parent != NULL)
                m_parent->incDescendantCount(delta);
        }
        
        void Node::decDescendantCount(const size_t delta) {
            if (delta == 0)
                return;
            assert(m_descendantCount >= delta);
            m_descendantCount -= delta;
            if (m_parent != NULL)
                m_parent->decDescendantCount(delta);
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
        
        void Node::nodeWillChange() {
            if (m_parent != NULL)
                m_parent->childWillChange(this);
        }
        
        void Node::nodeDidChange() {
            if (m_parent != NULL)
                m_parent->childDidChange(this);
        }
        
        void Node::childWillChange(Node* node) {
            doChildWillChange(node);
            descendantWillChange(node);
        }
        
        void Node::childDidChange(Node* node) {
            doChildDidChange(node);
            descendantDidChange(node);
        }

        void Node::descendantWillChange(Node* node) {
            doDescendantWillChange(node);
            Node* parent = Node::parent();
            while (parent != NULL) {
                parent->descendantWillChange(node);
                parent = parent->parent();
            }
        }
        
        void Node::descendantDidChange(Node* node) {
            doDescendantDidChange(node);
            Node* parent = Node::parent();
            while (parent != NULL) {
                parent->descendantDidChange(node);
                parent = parent->parent();
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
            if (m_parent != NULL)
                m_parent->descendantWasSelected();
        }
        
        void Node::deselect() {
            if (!selectable())
                return;
            assert(m_selected);
            m_selected = false;
            if (m_parent != NULL)
                m_parent->descendantWasDeselected();
        }

        bool Node::descendantSelected() const {
            return m_descendantSelectionCount > 0;
        }
        
        size_t Node::descendantSelectionCount() const {
            return m_descendantSelectionCount;
        }
        
        void Node::descendantWasSelected() {
            incDescendantSelectionCount(1);
        }
        
        void Node::descendantWasDeselected() {
            decDescendantSelectionCount(1);
        }

        void Node::incDescendantSelectionCount(const size_t delta) {
            if (delta == 0)
                return;
            m_descendantSelectionCount += delta;
            if (m_parent != NULL)
                m_parent->incDescendantSelectionCount(delta);
        }
        
        void Node::decDescendantSelectionCount(const size_t delta) {
            if (delta == 0)
                return;
            assert(m_descendantSelectionCount >= delta);
            m_descendantSelectionCount -= delta;
            if (m_parent != NULL)
                m_parent->decDescendantSelectionCount(delta);
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
        
        size_t Node::familyIssueCount() const {
            return m_familyIssueCount;
        }

        const IssueList& Node::issues() const {
            return m_issues;
        }
        
        Issue* Node::findIssue(const size_t index) const {
            assert(index < m_familyIssueCount);
            
            if (index < m_issues.size())
                return m_issues[index];

            size_t childIndex = index - m_issues.size();
            NodeList::const_iterator it, end;
            for (it = m_children.begin(), end = m_children.end(); it != end; ++it) {
                Node* child = *it;
                const size_t childIssueCount = child->familyIssueCount();
                if (childIndex < childIssueCount)
                    return child->findIssue(childIndex);
                childIndex -= childIssueCount;
            }
            
            // should never happen!
            assert(false);
            return NULL;
        }

        void Node::updateIssues() {
            updateIssues(this);
        }

        void Node::updateIssues(Node* node) {
            doUpdateIssues(node);
        }

        void Node::updateIssues(const IssueGenerator& generator) {
            clearIssues();
            generateIssues(generator);
        }
        
        void Node::clearIssues() {
            decFamilyIssueCount(m_issues.size());
            VectorUtils::clearAndDelete(m_issues);
        }
        
        void Node::generateIssues(const IssueGenerator& generator) {
            const size_t oldIssueCount = m_issues.size();
            generator.generate(this, m_issues);
            const size_t newIssueCount = m_issues.size();
            if (newIssueCount > oldIssueCount)
                incFamilyIssueCount(newIssueCount - oldIssueCount);
            else if (newIssueCount < oldIssueCount)
                decFamilyIssueCount(oldIssueCount - newIssueCount);
        }

        void Node::incFamilyIssueCount(const size_t delta) {
            if (delta == 0)
                return;
            m_familyIssueCount += delta;
            if (m_parent != NULL)
                m_parent->incFamilyIssueCount(delta);
        }
        
        void Node::decFamilyIssueCount(const size_t delta) {
            if (delta == 0)
                return;
            assert(m_familyIssueCount >= delta);
            m_familyIssueCount -= delta;
            if (m_parent != NULL)
                m_parent->decFamilyIssueCount(delta);
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

        void Node::doDescendantWasAdded(Node* node) {}
        void Node::doDescendantWasRemoved(Node* node) {}

        void Node::doParentWillChange() {}
        void Node::doParentDidChange() {}
        void Node::doAncestorWillChange() {}
        void Node::doAncestorDidChange() {}

        void Node::doChildWillChange(Node* node) {}
        void Node::doChildDidChange(Node* node) {}
        void Node::doDescendantWillChange(Node* node) {}
        void Node::doDescendantDidChange(Node* node) {}

        void Node::doUpdateIssues(Node* node) {
            if (m_parent != NULL)
                m_parent->updateIssues(node);
        }

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
