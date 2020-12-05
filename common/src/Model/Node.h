/*
 Copyright (C) 2010-2017 Kristian Duske

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

#pragma once

#include "FloatType.h"
#include "Model/IssueType.h"
#include "Model/NodeVisitor.h"
#include "Model/Tag.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class AttributableNode;
        class ConstNodeVisitor;
        class Issue;
        class IssueGenerator;
        enum class LockState;
        class NodeVisitor;
        class PickResult;
        enum class VisibilityState;

        class Node : public Taggable {
        private:
            Node* m_parent;
            std::vector<Node*> m_children;
            size_t m_descendantCount;
            bool m_selected;

            size_t m_childSelectionCount;
            size_t m_descendantSelectionCount;

            VisibilityState m_visibilityState;
            LockState m_lockState;

            mutable size_t m_lineNumber;
            mutable size_t m_lineCount;

            mutable std::vector<Issue*> m_issues;
            mutable bool m_issuesValid;
            IssueType m_hiddenIssues;
        protected:
            Node();
        private:
            Node(const Node&);
            Node& operator=(const Node&);
        public:
            ~Node() override;
        public: // getters
            const std::string& name() const;
            /**
             * Returns a box that encloses the "logical" part of this node and its children; these are the bounds that are
             * used in game (for entities, the bounds specified in the entity definition file), and used
             * for grid snapping, for example.
             * Nodes can render or hit test outside of these logicalBounds if necessary (see `physicalBounds()`).
             */
            const vm::bbox3& logicalBounds() const;
            /**
             * Returns a box that encloses all rendering and hit testing for this node and its children.
             * Equal to or larger than `logicalBounds()`.
             * Currently, the only case where this differs from `logicalBounds()` is with entity models that extend
             * beyond the bounds specified in the .fgd.
             */
            const vm::bbox3& physicalBounds() const;
        public: // cloning and snapshots
            Node* clone(const vm::bbox3& worldBounds) const;
            Node* cloneRecursively(const vm::bbox3& worldBounds) const;
        protected:
            void cloneAttributes(Node* node) const;

            static std::vector<Node*> clone(const vm::bbox3& worldBounds, const std::vector<Node*>& nodes);
            static std::vector<Node*> cloneRecursively(const vm::bbox3& worldBounds, const std::vector<Node*>& nodes);

            template <typename I, typename O>
            static void clone(const vm::bbox3& worldBounds, I cur, I end, O result) {
                while (cur != end) {
                    const Node* node = *cur;
                    result++ = node->clone(worldBounds);
                    ++cur;
                }
            }

            template <typename I, typename O>
            static void cloneRecursively(const vm::bbox3& worldBounds, I cur, I end, O result) {
                while (cur != end) {
                    const Node* node = *cur;
                    result++ = node->cloneRecursively(worldBounds);
                    ++cur;
                }
            }
        public: // tree management
            size_t depth() const;
            Node* parent() const;
            bool isAncestorOf(const Node* node) const;
            bool isAncestorOf(const std::vector<Node*>& nodes) const;
            bool isDescendantOf(const Node* node) const;
            bool isDescendantOf(const std::vector<Node*>& nodes) const;
            std::vector<Node*> findDescendants(const std::vector<Node*>& nodes) const;

            bool removeIfEmpty() const;

            bool hasChildren() const;
            size_t childCount() const;
            const std::vector<Node*>& children() const;
            size_t descendantCount() const;
            size_t familySize() const;

            bool shouldAddToSpacialIndex() const;
        public:
            void addChildren(const std::vector<Node*>& children);

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

            bool canAddChild(const Node* child) const;
            bool canRemoveChild(const Node* child) const;

            template <typename I>
            bool canAddChildren(I cur, I end) const {
                while (cur != end) {
                    if (!canAddChild(*cur++))
                        return false;
                }
                return true;
            }

            template <typename I>
            bool canRemoveChildren(I cur, I end) const {
                while (cur != end) {
                    if (!canRemoveChild(*cur++))
                        return false;
                }
                return true;
            }
        private:
            void doAddChild(Node* child);
            void doRemoveChild(Node* child);
            void clearChildren();

            void childWillBeAdded(Node* node);
            void childWasAdded(Node* node);
            void childWillBeRemoved(Node* node);
            void childWasRemoved(Node* node);

            void descendantWillBeAdded(Node* newParent, Node* node, size_t depth);
            void descendantWasAdded(Node* node, size_t depth);
            void descendantWillBeRemoved(Node* node, size_t depth);
            void descendantWasRemoved(Node* oldParent, Node* node, size_t depth);
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
                explicit NotifyNodeChange(Node* node);
                ~NotifyNodeChange();
            };

            // call these methods via the NotifyNodeChange class, it's much safer
            void nodeWillChange();
            void nodeDidChange();

            friend class NotifyPhysicalBoundsChange;
            class NotifyPhysicalBoundsChange {
            private:
                Node* m_node;
                vm::bbox3 m_oldBounds;
            public:
                explicit NotifyPhysicalBoundsChange(Node* node);
                ~NotifyPhysicalBoundsChange();
            };
            void nodePhysicalBoundsDidChange(vm::bbox3 oldBounds);
        private:
            void childWillChange(Node* node);
            void childDidChange(Node* node);
            void descendantWillChange(Node* node);
            void descendantDidChange(Node* node);

            void childPhysicalBoundsDidChange(Node* node, const vm::bbox3& oldBounds);
            void descendantPhysicalBoundsDidChange(Node* node, const vm::bbox3& oldBounds, size_t depth);
        public: // selection
            bool selected() const;
            void select();
            void deselect();

            /**
             * Returns true if this node or our parent or grandparent, etc., is selected
             */
            bool transitivelySelected() const;
            /**
             * Returns true if our parent or grandparent, etc., is selected
             */
            bool parentSelected() const;

            bool childSelected() const;
            size_t childSelectionCount() const;

            bool descendantSelected() const;
            size_t descendantSelectionCount() const;

            void childWasSelected();
            void childWasDeselected();

            /**
             * Returns the nodes that must be selected for this node
             * to be selected in the UI.
             *
             * Normally just a list of `this`, but for brush entities,
             * it's a list of the contained brushes (excluding the Entity itself).
             */
            virtual std::vector<Node*> nodesRequiredForViewSelection();
        private:
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
            bool setVisibilityState(VisibilityState visibility);
            bool ensureVisible();

            bool editable() const;
            bool locked() const;
            LockState lockState() const;
            bool setLockState(LockState lockState);
        public: // picking
            void pick(const vm::ray3& ray, PickResult& result);
            void findNodesContaining(const vm::vec3& point, std::vector<Node*>& result);
        public: // file position
            size_t lineNumber() const;
            void setFilePosition(size_t lineNumber, size_t lineCount) const;
            bool containsLine(size_t lineNumber) const;
        public: // issue management
            const std::vector<Issue*>& issues(const std::vector<IssueGenerator*>& issueGenerators);

            bool issueHidden(IssueType type) const;
            void setIssueHidden(IssueType type, bool hidden);
        public: // should only be called from this and from the world
            void invalidateIssues() const;
        private:
            void validateIssues(const std::vector<IssueGenerator*>& issueGenerators);
            void clearIssues() const;
        public: // visitors
            /**
             * Visit this node with the given lambda and return the lambda's return value or nothing
             * if the lambda doesn't return anything.
             *
             * Passes a non-const pointer to this node to the lambda.
             */
            template <typename L>
            auto accept(const L& lambda) {
                NodeLambdaVisitor<L> visitor(lambda);
                doAccept(visitor);
                return visitor.result();
            }

            /**
             * Visit this node with the given lambda and return the lambda's return value or nothing
             * if the lambda doesn't return anything.
             *
             * Passes a const pointer to this node to the lambda.
             */
            template <typename L>
            auto accept(const L& lambda) const {
                ConstNodeLambdaVisitor<L> visitor(lambda);
                doAccept(visitor);
                return visitor.result();
            }

            /**
             * Visit this node's parent with the given lambda and return the lambda's return value or
             * nothing if the lambda doesn't return anything.
             *
             * If the lambda returns a value and not void, the value is wrapped in std::optional. If
             * this node does not have a parent, then an empty optional is returned.
             *
             * Passes a non-const pointer to this node's parent to the lambda.
             */
            template <typename L>
            auto visitParent(const L& lambda) {
                if constexpr(NodeLambdaHasResult_v<L>) {
                    using R = typename NodeLambdaVisitorResult<L>::R;
                    if (auto* parent = this->parent()) {
                        return std::optional<R>{parent->accept(lambda)};
                    } else {
                        return std::optional<R>{};
                    }
                } else {
                    if (auto* parent = this->parent()) {
                        parent->accept(lambda);
                    }
                }
            }


            /**
             * Visit this node's parent with the given lambda and return the lambda's return value or
             * nothing if the lambda doesn't return anything.
             *
             * If the lambda returns a value and not void, the value is wrapped in std::optional. If
             * this node does not have a parent, then an empty optional is returned.
             *
             * Passes a const pointer to this node's parent to the lambda.
             */
            template <typename L>
            auto visitParent(const L& lambda) const {
                if constexpr(NodeLambdaHasResult_v<L>) {
                    using R = typename NodeLambdaVisitorResult<L>::R;
                    if (const auto* parent = this->parent()) {
                        return std::optional<R>{parent->accept(lambda)};
                    } else {
                        return std::optional<R>{};
                    }
                } else {
                    if (const auto* parent = this->parent()) {
                        parent->accept(lambda);
                    }
                }
            }

            /**
             * Visit every node in the given vector with the given lambda.
             */
            template <typename N, typename L>
            static void visitAll(const std::vector<N*>& nodes, const L& lambda) {
                for (auto* node : nodes) {
                    node->accept(lambda);
                }
            }

            /**
             * Visit all children of this node with the given lambda.
             */
            template <typename L>
            void visitChildren(const L& lambda) {
                visitAll(m_children, lambda);
            }

            /**
             * Visit all children of this node with the given lambda.
             */
            template <typename L>
            void visitChildren(const L& lambda) const {
                visitAll(m_children, lambda);
            }
        protected: // index management
            void findAttributableNodesWithAttribute(const std::string& name, const std::string& value, std::vector<AttributableNode*>& result) const;
            void findAttributableNodesWithNumberedAttribute(const std::string& prefix, const std::string& value, std::vector<AttributableNode*>& result) const;

            void addToIndex(AttributableNode* attributable, const std::string& name, const std::string& value);
            void removeFromIndex(AttributableNode* attributable, const std::string& name, const std::string& value);
        private: // subclassing interface
            virtual const std::string& doGetName() const = 0;
            virtual const vm::bbox3& doGetLogicalBounds() const = 0;
            virtual const vm::bbox3& doGetPhysicalBounds() const = 0;

            virtual Node* doClone(const vm::bbox3& worldBounds) const = 0;
            virtual Node* doCloneRecursively(const vm::bbox3& worldBounds) const;

            virtual bool doCanAddChild(const Node* child) const = 0;
            virtual bool doCanRemoveChild(const Node* child) const = 0;
            virtual bool doRemoveIfEmpty() const = 0;

            virtual bool doShouldAddToSpacialIndex() const = 0;

            virtual void doChildWillBeAdded(Node* node);
            virtual void doChildWasAdded(Node* node);
            virtual void doChildWillBeRemoved(Node* node);
            virtual void doChildWasRemoved(Node* node);

            virtual void doDescendantWillBeAdded(Node* newParent, Node* node, size_t depth);
            virtual void doDescendantWasAdded(Node* node, size_t depth);
            virtual void doDescendantWillBeRemoved(Node* node, size_t depth);
            virtual void doDescendantWasRemoved(Node* oldParent, Node* node, size_t depth);
            virtual bool doShouldPropagateDescendantEvents() const;

            virtual void doParentWillChange();
            virtual void doParentDidChange();
            virtual void doAncestorWillChange();
            virtual void doAncestorDidChange();

            virtual void doNodePhysicalBoundsDidChange();
            virtual void doChildPhysicalBoundsDidChange();
            virtual void doDescendantPhysicalBoundsDidChange(Node* node);

            virtual void doChildWillChange(Node* node);
            virtual void doChildDidChange(Node* node);
            virtual void doDescendantWillChange(Node* node);
            virtual void doDescendantDidChange(Node* node);

            virtual bool doSelectable() const = 0;

            virtual void doPick(const vm::ray3& ray, PickResult& pickResult) = 0;
            virtual void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) = 0;

            virtual void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) = 0;

            virtual void doAccept(NodeVisitor& visitor) = 0;
            virtual void doAccept(ConstNodeVisitor& visitor) const = 0;

            virtual void doFindAttributableNodesWithAttribute(const std::string& name, const std::string& value, std::vector<AttributableNode*>& result) const;
            virtual void doFindAttributableNodesWithNumberedAttribute(const std::string& prefix, const std::string& value, std::vector<AttributableNode*>& result) const;

            virtual void doAddToIndex(AttributableNode* attributable, const std::string& name, const std::string& value);
            virtual void doRemoveFromIndex(AttributableNode* attributable, const std::string& name, const std::string& value);
        };
    }
}

