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

#ifndef __TrenchBroom__Node__
#define __TrenchBroom__Node__

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class IssueGenerator;

        class Node {
        private:
            Node* m_parent;
            NodeList m_children;
            size_t m_descendantCount;
            bool m_selected;
            size_t m_descendantSelectionCount;

            size_t m_lineNumber;
            size_t m_lineCount;

            IssueList m_issues;
            size_t m_familyIssueCount;
            IssueType m_hiddenIssues;
        protected:
            Node();
        private:
            Node(const Node&);
            Node& operator=(const Node&);
        public:
            virtual ~Node();
        public: // cloning and snapshots
            Node* clone(const BBox3& worldBounds) const;
            NodeSnapshot* takeSnapshot();
        protected:
            static NodeList clone(const BBox3& worldBounds, const NodeList& nodes);
            
            template <typename I, typename O>
            static void clone(const BBox3& worldBounds, I cur, I end, O result) {
                while (cur != end) {
                    const Node* node = *cur;
                    result = node->clone(worldBounds);
                    ++cur;
                }
            }
        public: // tree management
            Node* parent() const;
            bool isAncestorOf(const Node* node) const;
            bool isDescendantOf(const Node* node) const;
            bool removeIfEmpty() const;
            
            bool hasChildren() const;
            size_t childCount() const;
            const NodeList& children() const;
            size_t descendantCount() const;
            size_t familySize() const;
            
            void addChildren(const NodeList& children);
            
            template <typename I>
            void addChildren(I cur, I end, size_t count = 0) {
                m_children.reserve(m_children.size() + count);
                size_t descendantCountDelta = 0;
                while (cur != end) {
                    Node* child = *cur;
                    doAddChild(child);
                    descendantCountDelta += child->descendantCount() + 1;
                    ++cur;
                }
                incDescendantCount(descendantCountDelta);
            }
            
            void addChild(Node* child);
            
            template <typename I>
            void removeChildren(I cur, I end) {
                size_t descendantCountDelta = 0;
                NodeList::iterator rem = m_children.end();
                while (cur != end) {
                    Node* child = *cur;
                    rem = doRemoveChild(m_children.begin(), rem, child);
                    descendantCountDelta += child->descendantCount() + 1;
                    ++cur;
                }
                m_children.erase(rem, m_children.end());
                decDescendantCount(descendantCountDelta);
            }
            
            void removeChild(Node* child);

            bool canAddChild(Node* child) const;
            bool canRemoveChild(Node* child) const;
        private:
            void doAddChild(Node* child);
            NodeList::iterator doRemoveChild(NodeList::iterator begin, NodeList::iterator end, Node* child);
            void clearChildren();
            
            void descendantWasAdded(Node* node);
            void descendantWasRemoved(Node* node);
            
            void incDescendantCount(size_t delta);
            void decDescendantCount(size_t delta);
            
            void setParent(Node* parent);
            void parentWillChange();
            void parentDidChange();
            void ancestorWillChange();
            void ancestorDidChange();
        protected: // notification for parents
            void nodeWillChange();
            void nodeDidChange();
        private:
            void childWillChange(Node* node);
            void childDidChange(Node* node);
            void descendantWillChange(Node* node);
            void descendantDidChange(Node* node);
        public: // selection
            bool selected() const;
            void select();
            void deselect();
            
            bool descendantSelected() const;
            size_t descendantSelectionCount() const;

            void descendantWasSelected();
            void descendantWasDeselected();
        protected:
            void incDescendantSelectionCount(size_t delta);
            void decDescendantSelectionCount(size_t delta);
        private:
            bool selectable() const;
        public: // file position
            size_t lineNumber() const;
            void setFilePosition(size_t lineNumber, size_t lineCount);
            bool containsLine(size_t lineNumber) const;
        public: // issue management
            size_t familyIssueCount() const;
            const IssueList& issues() const;
            Issue* findIssue(size_t index) const;

            bool issueHidden(IssueType type) const;
            void setIssueHidden(IssueType type, bool hidden);
            
            // should only be called by world
            void updateIssues(const IssueGenerator& generator);
        protected:
            void updateIssues();
        private:
            void updateIssues(Node* node);
            void clearIssues();
            void generateIssues(const IssueGenerator& generator);
            void incFamilyIssueCount(size_t delta);
            void decFamilyIssueCount(size_t delta);
        public: // visitors
            template <class V>
            void acceptAndRecurse(V& visitor) {
                accept(visitor);
                if (!visitor.recursionStopped())
                    recurse(visitor);
            }
            
            template <class V>
            void acceptAndRecurse(V& visitor) const {
                accept(visitor);
                if (!visitor.recursionStopped())
                    recurse(visitor);
            }
            
            template <typename I, typename V>
            static void acceptAndRecurse(I cur, I end, V& visitor) {
                while (cur != end) {
                    (*cur)->acceptAndRecurse(visitor);
                    ++cur;
                }
            }
            
            template <class V>
            void acceptAndEscalate(V& visitor) {
                accept(visitor);
                escalate(visitor);
            }
            
            template <class V>
            void acceptAndEscalate(V& visitor) const {
                accept(visitor);
                escalate(visitor);
            }
            
            template <typename I, typename V>
            static void acceptAndEscalate(I cur, I end, V& visitor) {
                while (cur != end) {
                    (*cur)->acceptAndEscalate(visitor);
                    ++cur;
                }
            }
            
            template <class V>
            void accept(V& visitor) {
                doAccept(visitor);
            }
            
            template <class V>
            void accept(V& visitor) const {
                doAccept(visitor);
            }
            
            template <typename I, typename V>
            static void accept(I cur, I end, V& visitor) {
                while (cur != end) {
                    (*cur)->accept(visitor);
                    ++cur;
                }
            }

            template <class V>
            void recurse(V& visitor) {
                NodeList::const_iterator it, end;
                for (it = m_children.begin(), end = m_children.end(); it != end && !visitor.cancelled(); ++it) {
                    Node* node = *it;
                    node->acceptAndRecurse(visitor);
                }
            }

            template <class V>
            void recurse(V& visitor) const {
                NodeList::const_iterator it, end;
                for (it = m_children.begin(), end = m_children.end(); it != end && !visitor.cancelled(); ++it) {
                    Node* node = *it;
                    node->acceptAndRecurse(visitor);
                }
            }

            template <typename I, typename V>
            static void recurse(I cur, I end, V& visitor) {
                while (cur != end) {
                    (*cur)->recurse(visitor);
                    ++cur;
                }
            }
            
            template <class V>
            void iterate(V& visitor) {
                NodeList::const_iterator it, end;
                for (it = m_children.begin(), end = m_children.end(); it != end && !visitor.cancelled(); ++it) {
                    Node* node = *it;
                    node->accept(visitor);
                }
            }
            
            template <class V>
            void iterate(V& visitor) const {
                NodeList::const_iterator it, end;
                for (it = m_children.begin(), end = m_children.end(); it != end && !visitor.cancelled(); ++it) {
                    Node* node = *it;
                    node->accept(visitor);
                }
            }
            
            template <typename I, typename V>
            static void iterate(I cur, I end, V& visitor) {
                while (cur != end) {
                    (*cur)->iterate(visitor);
                    ++cur;
                }
            }
            
            template <class V>
            void escalate(V& visitor) {
                if (parent() != NULL && !visitor.cancelled())
                    parent()->acceptAndEscalate(visitor);
            }
            
            template <class V>
            void escalate(V& visitor) const {
                if (parent() != NULL && !visitor.cancelled())
                    parent()->acceptAndEscalate(visitor);
            }

            template <typename I, typename V>
            static void escalate(I cur, I end, V& visitor) {
                while (cur != end) {
                    (*cur)->escalate(visitor);
                    ++cur;
                }
            }
        protected: // index management
            void findAttributablesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableList& result) const;
            void findAttributablesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableList& result) const;
            
            void addToIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
            void removeFromIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
        private: // subclassing interface
            virtual Node* doClone(const BBox3& worldBounds) const = 0;
            virtual NodeSnapshot* doTakeSnapshot();
            
            virtual bool doCanAddChild(const Node* child) const = 0;
            virtual bool doCanRemoveChild(const Node* child) const = 0;
            virtual bool doRemoveIfEmpty() const = 0;
            
            virtual void doDescendantWasAdded(Node* node);
            virtual void doDescendantWasRemoved(Node* node);

            virtual void doParentWillChange();
            virtual void doParentDidChange();
            virtual void doAncestorWillChange();
            virtual void doAncestorDidChange();
            
            virtual void doChildWillChange(Node* node);
            virtual void doChildDidChange(Node* node);
            virtual void doDescendantWillChange(Node* node);
            virtual void doDescendantDidChange(Node* node);
            
            virtual bool doSelectable() const = 0;
            
            virtual void doUpdateIssues(Node* node);
            
            virtual void doAccept(NodeVisitor& visitor) = 0;
            virtual void doAccept(ConstNodeVisitor& visitor) const = 0;
            
            virtual void doFindAttributablesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableList& result) const;
            virtual void doFindAttributablesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableList& result) const;
            
            virtual void doAddToIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
            virtual void doRemoveFromIndex(Attributable* attributable, const AttributeName& name, const AttributeValue& value);
        };
    }
}

#endif /* defined(__TrenchBroom__Node__) */
