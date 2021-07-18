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

#include "Color.h"
#include "Model/BrushGeometry.h"
#include "Renderer/AllocationTracker.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/FaceRenderer.h"

#include <memory>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class BrushFace;
        class EditorContext;
    }

    namespace Renderer {
        class BrushRenderer {
        public:
            class Filter {
            public:
                enum class FaceRenderPolicy {
                    RenderMarked,
                    RenderNone
                };

                enum class EdgeRenderPolicy {
                    RenderAll,
                    RenderIfEitherFaceMarked,
                    RenderIfBothFacesMarked,
                    RenderNone
                };

                using RenderSettings = std::tuple<FaceRenderPolicy, EdgeRenderPolicy>;

                Filter();
                Filter(const Filter& other);
                virtual ~Filter();

                Filter& operator=(const Filter& other);

                /**
                 * Classifies whether the brush will be rendered, and which faces/edges.
                 *
                 * If both FaceRenderPolicy::RenderNone and EdgeRenderPolicy::RenderNone are returned, the brush is
                 * skipped (not added to the vertex array or index arrays at all).
                 *
                 * Otherwise, markFaces() should call BrushFace::setMarked() on *all* faces, passing true or false
                 * as needed to select the faces to be rendered.
                 */
                virtual RenderSettings markFaces(const Model::BrushNode* brush) const = 0;

            protected:
                /**
                 * Return this from your markFaces() implementation to skip rendering of the brush.
                 */
                static RenderSettings renderNothing();
            };

            class DefaultFilter : public Filter {
            private:
                const Model::EditorContext& m_context;
            public:
                ~DefaultFilter() override;
            protected:
                explicit DefaultFilter(const Model::EditorContext& context);
                DefaultFilter(const DefaultFilter& other);

                bool visible(const Model::BrushNode* brush) const;
                bool visible(const Model::BrushNode* brush, const Model::BrushFace& face) const;
                bool visible(const Model::BrushNode* brush, const Model::BrushEdge* edge) const;

                bool editable(const Model::BrushNode* brush) const;
                bool editable(const Model::BrushNode* brush, const Model::BrushFace& face) const;

                bool selected(const Model::BrushNode* brush) const;
                bool selected(const Model::BrushNode* brush, const Model::BrushFace& face) const;
                bool selected(const Model::BrushNode* brush, const Model::BrushEdge* edge) const;
                bool hasSelectedFaces(const Model::BrushNode* brush) const;
            private:
                DefaultFilter& operator=(const DefaultFilter& other);
            };

            class NoFilter : public Filter {
            public:
                using Filter::Filter;
                RenderSettings markFaces(const Model::BrushNode* brushNode) const override;
            private:
                deleteCopyAndMove(NoFilter)
            };
        private:
            class FilterWrapper;
        private:
            std::unique_ptr<Filter> m_filter;

            struct BrushInfo {
                AllocationTracker::Block* vertexHolderKey;
                AllocationTracker::Block* edgeIndicesKey;
                std::vector<std::pair<const Assets::Texture*, AllocationTracker::Block*>> opaqueFaceIndicesKeys;
                std::vector<std::pair<const Assets::Texture*, AllocationTracker::Block*>> transparentFaceIndicesKeys;
            };
            /**
             * Tracks all brushes that are stored in the VBO, with the information necessary to remove them
             * from the VBO later.
             */
            std::unordered_map<const Model::BrushNode*, BrushInfo> m_brushInfo;

            /**
             * If a brush is in the VBO, it's always valid.
             * If a brush is valid, it might not be in the VBO if it was hidden by the Filter.
             *
             * Do not attempt to use vector_set here, it turns out to be slower.
             */
            std::unordered_set<const Model::BrushNode*> m_allBrushes;
            std::unordered_set<const Model::BrushNode*> m_invalidBrushes;

            std::shared_ptr<BrushVertexArray> m_vertexArray;
            std::shared_ptr<BrushIndexArray> m_edgeIndices;

            using TextureToBrushIndicesMap = std::unordered_map<const Assets::Texture*, std::shared_ptr<BrushIndexArray>>;
            std::shared_ptr<TextureToBrushIndicesMap> m_transparentFaces;
            std::shared_ptr<TextureToBrushIndicesMap> m_opaqueFaces;

            FaceRenderer m_opaqueFaceRenderer;
            FaceRenderer m_transparentFaceRenderer;
            IndexedEdgeRenderer m_edgeRenderer;

            Color m_faceColor;
            bool m_showEdges;
            Color m_edgeColor;
            bool m_grayscale;
            bool m_tint;
            Color m_tintColor;
            bool m_showOccludedEdges;
            Color m_occludedEdgeColor;
            bool m_forceTransparent;
            float m_transparencyAlpha;

            bool m_showHiddenBrushes;
        public:
            template <typename FilterT>
            explicit BrushRenderer(const FilterT& filter) :
            m_filter(std::make_unique<FilterT>(filter)),
            m_showEdges(false),
            m_grayscale(false),
            m_tint(false),
            m_showOccludedEdges(false),
            m_forceTransparent(false),
            m_transparencyAlpha(1.0f),
            m_showHiddenBrushes(false) {
                clear();
            }

            BrushRenderer();

            /**
             * New brushes are invalidated, brushes already in the BrushRenderer are not invalidated.
             */
            void addBrushes(const std::vector<Model::BrushNode*>& brushes);
            /**
             * New brushes are invalidated, brushes already in the BrushRenderer are not invalidated.
             */
            void setBrushes(const std::vector<Model::BrushNode*>& brushes);
            /**
             * Remove all brushes.
             */
            void clear();

            /**
             * Marks all of the brushes as invalid, meaning that next time one of the render() methods is called,
             * - the Filter will be re-evaluated for each brush, possibly changing whether the brush is included/excluded
             * - all brushes vertices will be re-fetched from the Brush object.
             *
             * Until a brush is invalidated, we don't re-evaluate the Filter, and don't check the Brush object for modification.
             *
             * Additionally, calling `invalidate()` guarantees the m_brushInfo, m_transparentFaces, and m_opaqueFaces
             * maps will be empty, so the BrushRenderer will not have any lingering Texture* pointers.
             */
            void invalidate();
            /**
             * Invalidates the given brushes, if they were previously added with addBrushes()/setBrushes().
             * Brushes not in the renderer are ignored.
             */
            void invalidateBrushes(const std::vector<Model::BrushNode*>& brushes);
            void invalidateBrush(const Model::BrushNode* brush);
            bool valid() const;

            /**
             * Sets the color to render untextured faces with.
             */
            void setFaceColor(const Color& faceColor);

            /**
             * Specifies whether or not brush edges should be rendered.
             */
            void setShowEdges(bool showEdges);

            /**
             * The color to render brush edges with.
             */
            void setEdgeColor(const Color& edgeColor);

            /**
             * Specifies whether or not to render faces in grayscale.
             */
            void setGrayscale(bool grayscale);

            /**
             * Specifies whether or not to render faces with a tint.
             *
             * @see setTintColor
             */
            void setTint(bool tint);

            /**
             * Sets the color to tint faces with.
             */
            void setTintColor(const Color& tintColor);

            /**
             * Specifies whether or not occluded edges should be visible.
             */
            void setShowOccludedEdges(bool showOccludedEdges);

            /**
             * The color to render occluded edges with.
             */
            void setOccludedEdgeColor(const Color& occludedEdgeColor);

            /**
             * Specifies whether or not faces should be rendered transparent. Overrides any transparency settings from
             * the face itself or its material.
             *
             * Note: setTransparencyAlpha must be set to something less than 1.0 for this to have any effect.
             *
             * @see setTransparencyAlpha
             */
            void setForceTransparent(bool transparent);

            /**
             * The alpha value to render transparent faces with.
             *
             * Note: this defaults to 1.0, which means requests for transparency from the brush, face,
             * or setForceTransparent() are ignored by default.
             */
            void setTransparencyAlpha(float transparencyAlpha);

            /**
             * Specifies whether or not brushes which are currently hidden should be rendered regardless.
             */
            void setShowHiddenBrushes(bool showHiddenBrushes);
        public: // rendering
            void render(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch);
            void renderTransparent(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            void renderOpaqueFaces(RenderBatch& renderBatch);
            void renderTransparentFaces(RenderBatch& renderBatch);
            void renderEdges(RenderBatch& renderBatch);

        public:
            /**
             * Only exposed for benchmarking.
             */
            void validate();
        private:
            bool shouldDrawFaceInTransparentPass(const Model::BrushNode* brush, const Model::BrushFace& face) const;
            void validateBrush(const Model::BrushNode* brush);
        public:
            void addBrush(const Model::BrushNode* brush);
            void removeBrush(const Model::BrushNode* brush);
        private:
            /**
             * If the given brush is not currently in the VBO, it's silently ignored.
             * Otherwise, it's removed from the VBO (having its indices zeroed out, causing it to no longer draw).
             * The brush's "valid" state is not touched inside here, but the m_brushInfo is updated.
             */
            void removeBrushFromVbo(const Model::BrushNode* brush);
        private:
            BrushRenderer(const BrushRenderer& other);
            BrushRenderer& operator=(const BrushRenderer& other);
        };
    }
}

