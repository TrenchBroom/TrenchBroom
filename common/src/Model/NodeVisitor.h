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
#include <optional>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class EntityNode;
        class GroupNode;
        class LayerNode;
        class Node;
        class WorldNode;

        class NodeVisitor {
        protected:
            NodeVisitor();
        public:
            virtual ~NodeVisitor();

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

        class ConstNodeVisitor {
        protected:
            ConstNodeVisitor();
        public:
            virtual ~ConstNodeVisitor();

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

        template <typename L, typename N, typename Enable = void>
        struct NodeLambdaInvokeResult {
            using type = std::invoke_result_t<L, N>;
        };

        template <typename L, typename N>
        struct NodeLambdaInvokeResult<L, N, typename std::enable_if_t<std::is_invocable_v<L, const L&, N>>> {
            using type = std::invoke_result_t<L, const L&, N>;
        };

        template <typename L, typename N>
        using NodeLambdaInvokeResult_t = typename NodeLambdaInvokeResult<L, N>::type;

        template <typename L>
        using NodeLambdaResultType = std::conditional_t<
            std::is_same_v<NodeLambdaInvokeResult_t<L, WorldNode*>, void>, void, NodeLambdaInvokeResult_t<L, WorldNode*>>;

        template <typename L>
        struct NodeLambdaHasResult : std::conditional_t<std::is_same_v<NodeLambdaResultType<L>, void>, std::false_type, std::true_type> {};

        template <typename L>
        inline constexpr bool NodeLambdaHasResult_v = NodeLambdaHasResult<L>::value;

        template <typename L>
        class NodeLambdaVisitorResult {
        public:
            using R = NodeLambdaResultType<L>;
        private:
            std::optional<R> m_result;
        public:
            R&& result() { return std::move(m_result).value(); }
        protected:
            void setResult(R&& result) {
                m_result = std::move(result);
            }
        };

        class NodeLambdaVisitorNoResult {
        public:
            void result() {}
        };

        template <typename L>
        class NodeLambdaVisitor : public NodeVisitor, public std::conditional_t<NodeLambdaHasResult_v<L>, NodeLambdaVisitorResult<L>, NodeLambdaVisitorNoResult> {
        private:
            const L& m_lambda;
        public:
            NodeLambdaVisitor(const L& lambda) : m_lambda(lambda) {}
        private:
            void doVisit(WorldNode* world) override   { _doVisit(world);  }
            void doVisit(LayerNode* layer) override   { _doVisit(layer);  }
            void doVisit(GroupNode* group) override   { _doVisit(group);  }
            void doVisit(EntityNode* entity) override { _doVisit(entity); }
            void doVisit(BrushNode* brush) override   { _doVisit(brush);  }

            template <typename N>
            void _doVisit(N* node) {
                if constexpr(std::is_invocable_v<L, const L&, N*>) {
                    if constexpr(NodeLambdaHasResult_v<L>) {
                        NodeLambdaVisitorResult<L>::setResult(m_lambda(m_lambda, node));
                    } else {
                        m_lambda(m_lambda, node);
                    }
                } else {
                    if constexpr(NodeLambdaHasResult_v<L>) {
                        NodeLambdaVisitorResult<L>::setResult(m_lambda(node));
                    } else {
                        m_lambda(node);
                    }
                }
            }
        };

        template <typename L>
        class ConstNodeLambdaVisitor : public ConstNodeVisitor, public std::conditional_t<NodeLambdaHasResult_v<L>, NodeLambdaVisitorResult<L>, NodeLambdaVisitorNoResult> {
        private:
            const L& m_lambda;
        public:
            ConstNodeLambdaVisitor(const L& lambda) : m_lambda(lambda) {}
        private:
            void doVisit(const WorldNode* world) override   { _doVisit(world);  }
            void doVisit(const LayerNode* layer) override   { _doVisit(layer);  }
            void doVisit(const GroupNode* group) override   { _doVisit(group);  }
            void doVisit(const EntityNode* entity) override { _doVisit(entity); }
            void doVisit(const BrushNode* brush) override   { _doVisit(brush);  }

            template <typename N>
            void _doVisit(const N* node) {
                if constexpr(std::is_invocable_v<L, const L&, const N*>) {
                    if constexpr(NodeLambdaHasResult_v<L>) {
                        NodeLambdaVisitorResult<L>::setResult(m_lambda(m_lambda, node));
                    } else {
                        m_lambda(m_lambda, node);
                    }
                } else {
                    if constexpr(NodeLambdaHasResult_v<L>) {
                        NodeLambdaVisitorResult<L>::setResult(m_lambda(node));
                    } else {
                        m_lambda(node);
                    }
                }
            }
        };
    }
}

#endif /* defined(TrenchBroom_NodeVisitor) */
