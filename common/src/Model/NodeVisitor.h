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

#ifndef TrenchBroom_NodeVisitor
#define TrenchBroom_NodeVisitor

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class EntityNode;
        class GroupNode;
        class LayerNode;
        class Node;
        class WorldNode;

        class BaseNodeVisitor {
        private:
            bool m_cancelled;
            bool m_recursionStopped;
        protected:
            BaseNodeVisitor();
        public:
            virtual ~BaseNodeVisitor();
            bool cancelled() const;
            bool recursionStopped();
        protected:
            void cancel();
            void stopRecursion();
        };

        class NodeVisitor : public BaseNodeVisitor {
        protected:
            NodeVisitor();
        public:
            ~NodeVisitor() override;

            virtual void visit(WorldNode* world);
            virtual void visit(LayerNode* layer);
            virtual void visit(GroupNode* group);
            virtual void visit(EntityNode* entity);
            virtual void visit(BrushNode* brush);
        private:
            virtual void doVisit(WorldNode* world)   = 0;
            virtual void doVisit(LayerNode* layer)   = 0;
            virtual void doVisit(GroupNode* group)   = 0;
            virtual void doVisit(EntityNode* entity) = 0;
            virtual void doVisit(BrushNode* brush)   = 0;
        };

        class ConstNodeVisitor : public BaseNodeVisitor {
        protected:
            ConstNodeVisitor();
        public:
            ~ConstNodeVisitor() override;

            virtual void visit(const WorldNode* world);
            virtual void visit(const LayerNode* layer);
            virtual void visit(const GroupNode* group);
            virtual void visit(const EntityNode* entity);
            virtual void visit(const BrushNode* brush);
        private:
            virtual void doVisit(const WorldNode* world)   = 0;
            virtual void doVisit(const LayerNode* layer)   = 0;
            virtual void doVisit(const GroupNode* group)   = 0;
            virtual void doVisit(const EntityNode* entity) = 0;
            virtual void doVisit(const BrushNode* brush)   = 0;
        };


        struct NeverStopRecursion {
            bool operator()(const Node* node, bool matched) const;
        };

        struct StopRecursionIfMatched {
            bool operator()(const Node* node, bool matched) const;
        };

        template <class P, class S = NeverStopRecursion>
        class MatchingNodeVisitor : public NodeVisitor {
        private:
            P m_p;
            S m_s;
        protected:
            explicit MatchingNodeVisitor(const P& p = P(), const S& s = S()) : m_p(p), m_s(s) {}
        public:
            ~MatchingNodeVisitor() override = default;

            void visit(WorldNode* world) override {
                const bool match = m_p(world);
                if (match) NodeVisitor::visit(world);
                if (m_s(world, match)) stopRecursion();
            }

            void visit(LayerNode* layer) override {
                const bool match = m_p(layer);
                if (match) NodeVisitor::visit(layer);
                if (m_s(layer, match)) stopRecursion();
            }

            void visit(GroupNode* group) override {
                const bool match = m_p(group);
                if (match) NodeVisitor::visit(group);
                if (m_s(group, match)) stopRecursion();
            }

            void visit(EntityNode* entity) override {
                const bool match = m_p(entity);
                if (match) NodeVisitor::visit(entity);
                if (m_s(entity, match)) stopRecursion();
            }

            void visit(BrushNode* brush) override {
                const bool match = m_p(brush);
                if (match) NodeVisitor::visit(brush);
                if (m_s(brush, match)) stopRecursion();
            }
        };

        template <class P, class S = NeverStopRecursion>
        class ConstMatchingNodeVisitor : public ConstNodeVisitor {
        private:
            P m_p;
            S m_s;
        protected:
            explicit ConstMatchingNodeVisitor(const P& p = P(), const S& s = S()) : m_p(p), m_s(s) {}
        public:
            ~ConstMatchingNodeVisitor() override = default;

            void visit(const WorldNode* world) override {
                const bool match = m_p(world);
                if (match) ConstNodeVisitor::visit(world);
                if (m_s(world, match)) stopRecursion();
            }

            void visit(const LayerNode* layer) override {
                const bool match = m_p(layer);
                if (match) ConstNodeVisitor::visit(layer);
                if (m_s(layer, match)) stopRecursion();
            }

            void visit(const GroupNode* group) override {
                const bool match = m_p(group);
                if (match) ConstNodeVisitor::visit(group);
                if (m_s(group, match)) stopRecursion();
            }

            void visit(const EntityNode* entity) override {
                const bool match = m_p(entity);
                if (match) ConstNodeVisitor::visit(entity);
                if (m_s(entity, match)) stopRecursion();
            }

            void visit(const BrushNode* brush) override {
                const bool match = m_p(brush);
                if (match) ConstNodeVisitor::visit(brush);
                if (m_s(brush, match)) stopRecursion();
            }
        };

        template <typename T>
        class NodeQuery {
        private:
            bool m_hasResult;
            T m_result;
        public:
            explicit NodeQuery(const T& defaultResult = T()) :
            m_hasResult(false),
            m_result(defaultResult) {}

            virtual ~NodeQuery() = default;

            bool hasResult() const {
                return m_hasResult;
            }

            T result() const {
                assert(hasResult());
                return m_result;
            }
        protected:
            void setResult(T result) {
                if (!m_hasResult) {
                    m_result = result;
                    m_hasResult = true;
                } else {
                    m_result = doCombineResults(m_result, result);
                }
            }
        private:
            virtual T doCombineResults(T /* oldResult */, T newResult) const { return newResult; }
        };
    }
}

#endif /* defined(TrenchBroom_NodeVisitor) */
