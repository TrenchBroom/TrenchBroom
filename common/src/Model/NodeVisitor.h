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

            virtual void visit(WorldNode* world)   = 0;
            virtual void visit(LayerNode* layer)   = 0;
            virtual void visit(GroupNode* group)   = 0;
            virtual void visit(EntityNode* entity) = 0;
            virtual void visit(BrushNode* brush)   = 0;
        };

        class ConstNodeVisitor {
        protected:
            ConstNodeVisitor();
        public:
            virtual ~ConstNodeVisitor();

            virtual void visit(const WorldNode* world)   = 0;
            virtual void visit(const LayerNode* layer)   = 0;
            virtual void visit(const GroupNode* group)   = 0;
            virtual void visit(const EntityNode* entity) = 0;
            virtual void visit(const BrushNode* brush)   = 0;
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
            void visit(WorldNode* world) override   { doVisit(world);  }
            void visit(LayerNode* layer) override   { doVisit(layer);  }
            void visit(GroupNode* group) override   { doVisit(group);  }
            void visit(EntityNode* entity) override { doVisit(entity); }
            void visit(BrushNode* brush) override   { doVisit(brush);  }

            template <typename N>
            void doVisit(N* node) {
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
            void visit(const WorldNode* world) override   { doVisit(world);  }
            void visit(const LayerNode* layer) override   { doVisit(layer);  }
            void visit(const GroupNode* group) override   { doVisit(group);  }
            void visit(const EntityNode* entity) override { doVisit(entity); }
            void visit(const BrushNode* brush) override   { doVisit(brush);  }

            template <typename N>
            void doVisit(const N* node) {
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
