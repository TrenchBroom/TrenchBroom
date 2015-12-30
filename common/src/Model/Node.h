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

#ifndef TrenchBroom_Node
#define TrenchBroom_Node

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class IssueGeneratorRegistry;
        class PickResult;

        class Node {
        private:
            Node* m_parent;
            NodeList m_children;
            size_t m_descendantCount;
            bool m_selected;
            
            size_t m_childSelectionCount;
            size_t m_descendantSelectionCount;

            VisibilityState m_visibilityState;
            LockState m_lockState;
            
            size_t m_lineNumber;
            size_t m_lineCount;

            mutable IssueList m_issues;
            mutable bool m_issuesValid;
            IssueType m_hiddenIssues;
        protected:
            Node();
        private:
            Node(const Node&);
            Node& operator=(const Node&);
        public:
            virtual ~Node();
        public: // getters
            const String& name() const;
            const BBox3& bounds() const;
        public: // cloning and snapshots
            Node* clone(const BBox3& worldBounds) const;
            Node* cloneRecursively(const BBox3& worldBounds) const;
            NodeSnapshot* takeSnapshot();
        protected:
            void cloneAttributes(Node* node) const;
            
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
            size_t depth() const;
            Node* parent() const;
            bool isAncestorOf(const Node* node) const;
            bool isDescendantOf(const Node* node) const;
            bool removeIfEmpty() const;
            
            bool hasChildren() const;
            size_t childCount() const;
            const NodeList& children() const;
            size_t descendantCount() const;
            size_t familySize() const;
        public:
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
                while (cur != end) {
                    Node* child = *cur;
                    doRemoveChild(child);
                    descendantCountDelta += child->descendantCount() + 1;
                    ++cur;
                }
                decDescendantCount(descendantCountDelta);
            }
            
            void removeChild(Node* child);

            bool canAddChild(Node* child) const;
            bool canRemoveChild(Node* child) const;
        private:
            void doAddChild(Node* child);
            void doRemoveChild(Node* child);
            void clearChildren();
            
            void childWillBeAdded(Node* node);
            void childWasAdded(Node* node);
            void childWillBeRemoved(Node* node);
            void childWasRemoved(Node* node);
            
            void descendantWillBeAdded(Node* newParent, Node* node);
            void descendantWasAdded(Node* node);
            void descendantWillBeRemoved(Node* node);
            void descendantWasRemoved(Node* oldParent, Node* node);
            bool shouldPropagateDescendantEvents() const;
            
            void incDescendantCount(size_t delta);
            void decDescendantCount(size_t delta);
            
            void setParent(Node* parent);
            void parentWillChange();
            void parentDidChange();
            void ancestorWillChange();
            void ancestorDidChange();
        protected: // notification for parents
            class NotifyNodeChange {
            private:
                Node* m_node;
            public:
                NotifyNodeChange(Node* node);
                ~NotifyNodeChange();
            };
            
            // call these methods via the NotifyNodeChange class, it's much safer
            void nodeWillChange();
            void nodeDidChange();
            
            void nodeBoundsDidChange();
        private:
            void childWillChange(Node* node);
            void childDidChange(Node* node);
            void descendantWillChange(Node* node);
            void descendantDidChange(Node* node);
            
            void childBoundsDidChange(Node* node);
        public: // selection
            bool selected() const;
            void select();
            void deselect();

            bool parentSelected() const;
            
            bool childSelected() const;
            size_t childSelectionCount() const;
            
            bool descendantSelected() const;
            size_t descendantSelectionCount() const;

            void childWasSelected();
            void childWasDeselected();
        protected:
            void incChildSelectionCount(size_t delta);
            void decChildSelectionCount(size_t delta);
        private:
            void incDescendantSelectionCount(size_t delta);
            void decDescendantSelectionCount(size_t delta);
        private:
            bool selectable() const;
        public: // visibility, locking
            bool visible() const;
            bool shown() const;
            bool hidden() const;
            VisibilityState visibilityState() const;
            bool setVisiblityState(VisibilityState visibility);
            bool ensureVisible();
            
            bool editable() const;
            bool locked() const;
            LockState lockState() const;
            bool setLockState(LockState lockState);
        public: // picking
            void pick(const Ray3& ray, PickResult& result) const;
            FloatType intersectWithRay(const Ray3& ray) const;
        public: // file position
            size_t lineNumber() const;
            void setFilePosition(size_t lineNumber, size_t lineCount);
            bool containsLine(size_t lineNumber) const;
        public: // issue management
            const IssueList& issues(const IssueGeneratorList& issueGenerators);
            
            bool issueHidden(IssueType type) const;
            void setIssueHidden(IssueType type, bool hidden);
        public: // should only be called from this and from the world
            void invalidateIssues() const;
        private:
            void validateIssues(const IssueGeneratorList& issueGenerators);
            void clearIssues() const;
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
            void findAttributableNodesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableNodeList& result) const;
            void findAttributableNodesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableNodeList& result) const;
            
            void addToIndex(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value);
            void removeFromIndex(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value);
        private: // subclassing interface
            virtual const String& doGetName() const = 0;
            virtual const BBox3& doGetBounds() const = 0;
            
            virtual Node* doClone(const BBox3& worldBounds) const = 0;
            virtual Node* doCloneRecursively(const BBox3& worldBounds) const;
            virtual NodeSnapshot* doTakeSnapshot();
            
            virtual bool doCanAddChild(const Node* child) const = 0;
            virtual bool doCanRemoveChild(const Node* child) const = 0;
            virtual bool doRemoveIfEmpty() const = 0;
            
            virtual void doChildWillBeAdded(Node* node);
            virtual void doChildWasAdded(Node* node);
            virtual void doChildWillBeRemoved(Node* node);
            virtual void doChildWasRemoved(Node* node);
            
            virtual void doDescendantWillBeAdded(Node* newParent, Node* node);
            virtual void doDescendantWasAdded(Node* node);
            virtual void doDescendantWillBeRemoved(Node* node);
            virtual void doDescendantWasRemoved(Node* oldParent, Node* node);
            virtual bool doShouldPropagateDescendantEvents() const;

            virtual void doParentWillChange();
            virtual void doParentDidChange();
            virtual void doAncestorWillChange();
            virtual void doAncestorDidChange();
            
            virtual void doNodeBoundsDidChange();
            virtual void doChildBoundsDidChange(Node* node);
            
            virtual void doChildWillChange(Node* node);
            virtual void doChildDidChange(Node* node);
            virtual void doDescendantWillChange(Node* node);
            virtual void doDescendantDidChange(Node* node);
            
            virtual bool doSelectable() const = 0;
            
            virtual void doPick(const Ray3& ray, PickResult& pickResult) const = 0;
            virtual FloatType doIntersectWithRay(const Ray3& ray) const = 0;
            
            virtual void doGenerateIssues(const IssueGenerator* generator, IssueList& issues) = 0;
            
            virtual void doAccept(NodeVisitor& visitor) = 0;
            virtual void doAccept(ConstNodeVisitor& visitor) const = 0;
            
            virtual void doFindAttributableNodesWithAttribute(const AttributeName& name, const AttributeValue& value, AttributableNodeList& result) const;
            virtual void doFindAttributableNodesWithNumberedAttribute(const AttributeName& prefix, const AttributeValue& value, AttributableNodeList& result) const;
            
            virtual void doAddToIndex(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value);
            virtual void doRemoveFromIndex(AttributableNode* attributable, const AttributeName& name, const AttributeValue& value);
        };
    }
}

#endif /* defined(TrenchBroom_Node) */
