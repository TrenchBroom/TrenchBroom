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
#include "Macros.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/HitType.h"
#include "Model/Node.h"
#include "Model/Object.h"
#include "Model/TagType.h"

#include <kdl/result_forward.h>

#include <vecmath/forward.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Renderer {
        class BrushRendererBrushCache;
    }

    namespace Model {
        class BrushFace;
        class GroupNode;
        class LayerNode;

        class ModelFactory;

        class BrushNode : public Node, public Object {
        public:
            static const HitType::Type BrushHitType;
        public:
            using VertexList = BrushVertexList;
            using EdgeList = BrushEdgeList;
        private:
            mutable std::unique_ptr<Renderer::BrushRendererBrushCache> m_brushRendererBrushCache; // unique_ptr for breaking header dependencies
            Brush m_brush; // must be destroyed before the brush renderer cache
            size_t m_selectedFaceCount = 0u;
        public:
            explicit BrushNode(Brush brush);
            ~BrushNode() override;
        public:
            BrushNode* clone(const vm::bbox3& worldBounds) const;

            AttributableNode* entity();
            const AttributableNode* entity() const;
            
            const Brush& brush() const;
            Brush setBrush(Brush brush);

            bool hasSelectedFaces() const;
            void selectFace(size_t faceIndex);
            void deselectFace(size_t faceIndex);
            
            void updateFaceTags(size_t faceIndex, TagManager& tagManager);
            
            void setFaceTexture(size_t faceIndex, Assets::Texture* texture);
        private:
            void updateSelectedFaceCount();
        private: // implement Node interface
            const std::string& doGetName() const override;
            const vm::bbox3& doGetLogicalBounds() const override;
            const vm::bbox3& doGetPhysicalBounds() const override;

            Node* doClone(const vm::bbox3& worldBounds) const override;

            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;

            bool doShouldAddToSpacialIndex() const override;

            bool doSelectable() const override;

            void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;
        private: // implement Object interface
            void doPick(const vm::ray3& ray, PickResult& pickResult) override;
            void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;

            std::optional<std::tuple<FloatType, size_t>> findFaceHit(const vm::ray3& ray) const;

            Node* doGetContainer() override;
            LayerNode* doGetLayer() override;
            GroupNode* doGetGroup() override;

            kdl::result<void, TransformError> doTransform(const vm::bbox3& worldBounds, const vm::mat4x4& transformation, bool lockTextures) override;

            bool doContains(const Node* node) const override;
            bool doIntersects(const Node* node) const override;
        public: // renderer cache
            /**
             * Only exposed to be called by BrushFace
             */
            void invalidateVertexCache();
            Renderer::BrushRendererBrushCache& brushRendererBrushCache() const;
        private: // implement Taggable interface
        public:
            void initializeTags(TagManager& tagManager) override;
            void clearTags() override;
            void updateTags(TagManager& tagManager) override;

            /**
             * Indicates whether all of the faces of this brush have any of the given tags.
             *
             * @param tagMask the tags to check
             * @return true whether all faces of this brush have any of the given tags
             */
            bool allFacesHaveAnyTagInMask(TagType::Type tagMask) const;

            /**
             * Indicates whether any of the faces of this brush have any tags.
             *
             * @return true whether any faces of this brush have any tags
             */
            bool anyFaceHasAnyTag() const;

            /**
             * Indicates whether any of the faces of this brush have any of the given tags.
             *
             * @param tagMask the tags to check
             * @return true whether any faces of this brush have any of the given tags
             */
            bool anyFacesHaveAnyTagInMask(TagType::Type tagMask) const;
        private:
            void doAcceptTagVisitor(TagVisitor& visitor) override;
            void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
        private:
            deleteCopyAndMove(BrushNode)
        };

        bool operator==(const BrushNode& lhs, const BrushNode& rhs);
        bool operator!=(const BrushNode& lhs, const BrushNode& rhs);
    }
}

