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

#ifndef TrenchBroom_BrushRenderer
#define TrenchBroom_BrushRenderer

#include "Color.h"
#include "Model/ModelTypes.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/FaceRenderer.h"
#include "Model/Brush.h"
#include "Renderer/AllocationTracker.h"

#include <tuple>
#include <map>
#include <unordered_map>

namespace TrenchBroom {
    namespace Model {
        class EditorContext;
    }
    
    namespace Renderer {
        class RenderBatch;
        class RenderContext;
        class Vbo;

        class BrushRenderer {
        public:
            class Filter {
            public:
                enum class RenderOpacity {
                    Opaque,
                    Transparent
                };

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

                using RenderSettings = std::tuple<RenderOpacity, FaceRenderPolicy, EdgeRenderPolicy>;

                Filter();
                Filter(const Filter& other);
                virtual ~Filter();
                
                Filter& operator=(const Filter& other);

                /**
                 * Classifies whether the brush will be rendered, and which faces/edges and the render opacity.
                 *
                 * If both FaceRenderPolicy::RenderNone and EdgeRenderPolicy::RenderNone are returned, the brush is
                 * skipped (not added to the vertex array or index arrays at all).
                 *
                 * Otherwise, markFaces() should call BrushFace::setMarked() on *all* faces, passing true or false
                 * as needed to select the faces to be rendered.
                 */
                virtual RenderSettings markFaces(const Model::Brush* brush) const = 0;

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
                virtual ~DefaultFilter();
            protected:
                DefaultFilter(const Model::EditorContext& context);
                DefaultFilter(const DefaultFilter& other);
                
                bool visible(const Model::Brush* brush) const;
                bool visible(const Model::BrushFace* face) const;
                bool visible(const Model::BrushEdge* edge) const;
                
                bool editable(const Model::Brush* brush) const;
                bool editable(const Model::BrushFace* face) const;
                
                bool selected(const Model::Brush* brush) const;
                bool selected(const Model::BrushFace* face) const;
                bool selected(const Model::BrushEdge* edge) const;
                bool hasSelectedFaces(const Model::Brush* brush) const;
            private:
                DefaultFilter& operator=(const DefaultFilter& other);
            };
            
            class NoFilter : public Filter {
            private:
                bool m_transparent;
            public:
                NoFilter(bool transparent);

                RenderSettings markFaces(const Model::Brush* brush) const override;
            private:
                NoFilter(const NoFilter& other);
                NoFilter& operator=(const NoFilter& other);
            };
        private:
            class FilterWrapper;
        private:
            Filter* m_filter;

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
            std::unordered_map<const Model::Brush*, BrushInfo> m_brushInfo;

            /**
             * If a brush is in the VBO, it's always valid.
             * If a brush is valid, it might not be in the VBO if it was hidden by the Filter.
             */
            std::set<const Model::Brush*> m_allBrushes;
            std::set<const Model::Brush*> m_invalidBrushes;

            BrushVertexArrayPtr m_vertexArray;
            BrushIndexArrayPtr m_edgeIndices;
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
            float m_transparencyAlpha;
            
            bool m_showHiddenBrushes;
        public:
            template <typename FilterT>
            explicit BrushRenderer(const FilterT& filter) :
            m_filter(new FilterT(filter)),
            m_showEdges(false),
            m_grayscale(false),
            m_tint(false),
            m_showOccludedEdges(false),
            m_transparencyAlpha(1.0f),
            m_showHiddenBrushes(false) {
                clear();
            }
            
            explicit BrushRenderer(bool transparent);
            
            ~BrushRenderer();

            /**
             * New brushes are invalidated, brushes already in the BrushRenderer are not invalidated.
             */
            void addBrushes(const Model::BrushList& brushes);
            /**
             * New brushes are invalidated, brushes already in the BrushRenderer are not invalidated.
             */
            void setBrushes(const Model::BrushList& brushes);
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
            void invalidateBrushes(const Model::BrushList& brushes);
            bool valid() const;

            void setFaceColor(const Color& faceColor);
            void setShowEdges(bool showEdges);
            void setEdgeColor(const Color& edgeColor);
            void setGrayscale(bool grayscale);
            void setTint(bool tint);
            void setTintColor(const Color& tintColor);
            void setShowOccludedEdges(bool showOccludedEdges);
            void setOccludedEdgeColor(const Color& occludedEdgeColor);
            void setTransparencyAlpha(float transparencyAlpha);
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
            void validateBrush(const Model::Brush* brush);
            void addBrush(const Model::Brush* brush);
            void removeBrush(const Model::Brush* brush);

            /**
             * If the given brush is not currently in the VBO, it's silently ignored.
             * Otherwise, it's removed from the VBO (having its indices zeroed out, causing it to no longer draw).
             * The brush's "valid" state is not touched inside here, but the m_brushInfo is updated.
             */
            void removeBrushFromVbo(const Model::Brush* brush);
        private:
            BrushRenderer(const BrushRenderer& other);
            BrushRenderer& operator=(const BrushRenderer& other);
        };
    }
}

#endif /* defined(TrenchBroom_BrushRenderer) */
