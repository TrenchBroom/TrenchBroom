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

#include "Model/Issue.h"

#include "Ensure.h"
#include "Model/BrushFace.h"
#include "Model/CollectSelectableNodesVisitor.h"
#include "Model/EditorContext.h"
#include "Model/Node.h"

#include <kdl/vector_utils.h>

#include <string>

namespace TrenchBroom {
    namespace Model {
        Issue::~Issue() = default;

        size_t Issue::seqId() const {
            return m_seqId;
        }

        size_t Issue::lineNumber() const {
            return doGetLineNumber();
        }

        std::string Issue::description() const {
            return doGetDescription();
        }

        IssueType Issue::type() const {
            return doGetType();
        }

        Node* Issue::node() const {
            return m_node;
        }

        class Issue::MatchSelectableIssueNodes {
        public:
            bool operator()(const Model::World*) const         { return false; }
            bool operator()(const Model::Layer*) const         { return false; }
            bool operator()(const Model::Group*) const         { return true; }
            bool operator()(const Model::Entity* entity) const { return !entity->hasChildren(); }
            bool operator()(const Model::BrushNode*) const         { return true; }
        };

        bool Issue::addSelectableNodes(const EditorContext& /* editorContext */, std::vector<Model::Node*>& nodes) const {
            if (m_node->parent() == nullptr) {
                return false;
            }

            using CollectSelectableIssueNodesVisitor = CollectMatchingNodesVisitor<MatchSelectableIssueNodes, StandardNodeCollectionStrategy, StopRecursionIfMatched>;

            CollectSelectableIssueNodesVisitor collect;
            m_node->acceptAndRecurse(collect);
            kdl::vec_append(nodes, collect.nodes());

            return true;
        }

        bool Issue::hidden() const {
            return m_node->issueHidden(type());
        }

        void Issue::setHidden(const bool hidden) {
            m_node->setIssueHidden(type(), hidden);
        }

        Issue::Issue(Node* node) :
        m_seqId(nextSeqId()),
        m_node(node) {
            ensure(m_node != nullptr, "node is null");
        }

        size_t Issue::nextSeqId() {
            static size_t seqId = 0;
            return seqId++;
        }

        IssueType Issue::freeType() {
            static IssueType type = 1;
            const IssueType result = type;
            type = (type << 1);
            return result;
        }

        size_t Issue::doGetLineNumber() const {
            return m_node->lineNumber();
        }

        BrushFaceIssue::BrushFaceIssue(BrushFace* face) :
        Issue(face->brush()),
        m_face(face) {}

        BrushFaceIssue::~BrushFaceIssue() = default;

        BrushFace* BrushFaceIssue::face() const {
            return m_face;
        }

        size_t BrushFaceIssue::doGetLineNumber() const {
            return m_face->lineNumber();
        }

        AttributeIssue::~AttributeIssue() = default;

        const std::string& AttributeIssue::attributeValue() const {
            const AttributableNode* attributableNode = static_cast<AttributableNode*>(node());
            return attributableNode->attribute(attributeName());
        }
    }
}
