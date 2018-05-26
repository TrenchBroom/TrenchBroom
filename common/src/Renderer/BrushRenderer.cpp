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

#include "BrushRenderer.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/EditorContext.h"
#include "Model/NodeVisitor.h"
#include "Renderer/IndexArrayMapBuilder.h"
#include "Renderer/IndexArray.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/TexturedIndexArrayBuilder.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        // Filter

        BrushRenderer::Filter::Filter() {}
        
        BrushRenderer::Filter::Filter(const Filter& other) {}

        BrushRenderer::Filter::~Filter() {}
        
        BrushRenderer::Filter& BrushRenderer::Filter::operator=(const Filter& other) { return *this; }

        // DefaultFilter

        BrushRenderer::DefaultFilter::~DefaultFilter() {}
        BrushRenderer::DefaultFilter::DefaultFilter(const Model::EditorContext& context) : m_context(context) {}

        BrushRenderer::DefaultFilter::DefaultFilter(const DefaultFilter& other) :
        Filter(),
        m_context(other.m_context) {}

        bool BrushRenderer::DefaultFilter::visible(const Model::Brush* brush) const    {
            return m_context.visible(brush);
        }
        
        bool BrushRenderer::DefaultFilter::visible(const Model::BrushFace* face) const {
            return m_context.visible(face);
        }
        
        bool BrushRenderer::DefaultFilter::visible(const Model::BrushEdge* edge) const {
            return m_context.visible(edge->firstFace()->payload()) || m_context.visible(edge->secondFace()->payload());
        }
        
        bool BrushRenderer::DefaultFilter::editable(const Model::Brush* brush) const {
            return m_context.editable(brush);
        }
        
        bool BrushRenderer::DefaultFilter::editable(const Model::BrushFace* face) const {
            return m_context.editable(face);
        }
        
        bool BrushRenderer::DefaultFilter::selected(const Model::Brush* brush) const {
            return brush->selected() || brush->parentSelected();
        }
        
        bool BrushRenderer::DefaultFilter::selected(const Model::BrushFace* face) const {
            return face->selected();
        }
        
        bool BrushRenderer::DefaultFilter::selected(const Model::BrushEdge* edge) const {
            const Model::BrushFace* first = edge->firstFace()->payload();
            const Model::BrushFace* second = edge->secondFace()->payload();
            const Model::Brush* brush = first->brush();
            assert(second->brush() == brush);
            return selected(brush) || selected(first) || selected(second);
        }
        
        bool BrushRenderer::DefaultFilter::hasSelectedFaces(const Model::Brush* brush) const {
            return brush->descendantSelected();
        }

        // NoFilter

        BrushRenderer::NoFilter::NoFilter(const bool transparent) : m_transparent(transparent) {}

        BrushRenderer::Filter::RenderSettings BrushRenderer::NoFilter::markFaces(const Model::Brush* brush) const {
            for (Model::BrushFace* face : brush->faces()) {
                face->setMarked(true);
            }
            return std::make_tuple(m_transparent ? RenderOpacity::Transparent : RenderOpacity::Opaque,
                                   FaceRenderPolicy::RenderMarked,
                                   Model::Brush::EdgeRenderPolicy::RenderAll);
        }

        // BrushRenderer

        BrushRenderer::BrushRenderer(const bool transparent) :
        m_filter(new NoFilter(transparent)),
        m_valid(true),
        m_showEdges(false),
        m_grayscale(false),
        m_tint(false),
        m_showOccludedEdges(false),
        m_transparencyAlpha(1.0f),
        m_showHiddenBrushes(false) {}
        
        BrushRenderer::~BrushRenderer() {
            delete m_filter;
            m_filter = nullptr;
        }

        void BrushRenderer::addBrushes(const Model::BrushList& brushes) {
            VectorUtils::append(m_brushes, brushes);
            invalidate();
        }

        void BrushRenderer::setBrushes(const Model::BrushList& brushes) {
            m_brushes = brushes;
            invalidate();
        }

        void BrushRenderer::invalidate() {
            m_vertexArray = nullptr;
            m_valid = false;
        }
        
        void BrushRenderer::clear() {
            m_brushes.clear();
            m_transparentFaceRenderer = FaceRenderer();
            m_opaqueFaceRenderer = FaceRenderer();
            m_vertexArray = nullptr;
            m_valid = true;
        }

        void BrushRenderer::setFaceColor(const Color& faceColor) {
            m_faceColor = faceColor;
        }
        
        void BrushRenderer::setShowEdges(const bool showEdges) {
            m_showEdges = showEdges;
        }
        
        void BrushRenderer::setEdgeColor(const Color& edgeColor) {
            m_edgeColor = edgeColor;
        }
        
        void BrushRenderer::setGrayscale(const bool grayscale) {
            m_grayscale = grayscale;
        }
        
        void BrushRenderer::setTint(const bool tint) {
            m_tint = tint;
        }
        
        void BrushRenderer::setTintColor(const Color& tintColor) {
            m_tintColor = tintColor;
        }

        void BrushRenderer::setShowOccludedEdges(const bool showOccludedEdges) {
            m_showOccludedEdges = showOccludedEdges;
        }
        
        void BrushRenderer::setOccludedEdgeColor(const Color& occludedEdgeColor) {
            m_occludedEdgeColor = occludedEdgeColor;
        }
        
        void BrushRenderer::setTransparencyAlpha(const float transparencyAlpha) {
            m_transparencyAlpha = transparencyAlpha;
        }
        
        void BrushRenderer::setShowHiddenBrushes(const bool showHiddenBrushes) {
            if (showHiddenBrushes != m_showHiddenBrushes) {
                m_showHiddenBrushes = showHiddenBrushes;
                invalidate();
            }
        }

        void BrushRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            renderOpaque(renderContext, renderBatch);
            renderTransparent(renderContext, renderBatch);
        }
        
        void BrushRenderer::renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_brushes.empty()) {
                if (!m_valid)
                    validate();
                if (renderContext.showFaces())
                    renderOpaqueFaces(renderBatch);
                if (renderContext.showEdges() || m_showEdges)
                    renderEdges(renderBatch);
            }
        }
        
        void BrushRenderer::renderTransparent(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_brushes.empty()) {
                if (!m_valid)
                    validate();
                if (renderContext.showFaces())
                    renderTransparentFaces(renderBatch);
            }
        }

        void BrushRenderer::renderOpaqueFaces(RenderBatch& renderBatch) {
            m_opaqueFaceRenderer.setGrayscale(m_grayscale);
            m_opaqueFaceRenderer.setTint(m_tint);
            m_opaqueFaceRenderer.setTintColor(m_tintColor);
            m_opaqueFaceRenderer.render(renderBatch);
        }
        
        void BrushRenderer::renderTransparentFaces(RenderBatch& renderBatch) {
            m_transparentFaceRenderer.setGrayscale(m_grayscale);
            m_transparentFaceRenderer.setTint(m_tint);
            m_transparentFaceRenderer.setTintColor(m_tintColor);
            m_transparentFaceRenderer.setAlpha(m_transparencyAlpha);
            m_transparentFaceRenderer.render(renderBatch);
        }
        
        void BrushRenderer::renderEdges(RenderBatch& renderBatch) {
            if (m_showOccludedEdges)
                m_edgeRenderer.renderOnTop(renderBatch, m_occludedEdgeColor);
            m_edgeRenderer.render(renderBatch, m_edgeColor);
        }

        class BrushRenderer::FilterWrapper : public BrushRenderer::Filter {
        private:
            const Filter& m_filter;
            bool m_showHiddenBrushes;
            NoFilter m_noFilter;

            const Filter& resolve() const {
                if (m_showHiddenBrushes) {
                    return m_noFilter;
                } else {
                    return m_filter;
                }
            }
        public:
            FilterWrapper(const Filter& filter, const bool showHiddenBrushes) :
            m_filter(filter),
            m_showHiddenBrushes(showHiddenBrushes),
            m_noFilter(false) {}

            RenderSettings markFaces(const Model::Brush* brush) const override {
                return resolve().markFaces(brush);
            }
        };

        void BrushRenderer::validate() {
            assert(!m_valid);
            
            const FilterWrapper wrapper(*m_filter, m_showHiddenBrushes);

            // evaluate filter and count vertices
            // the goal is to only evaluate the filter once per brush.
            size_t vertexCount = 0;
            for (const auto *brush : m_brushes) {
                const auto settings = wrapper.markFaces(brush);
                brush->setRenderSettings(settings);

                const auto [renderType, facePolicy, edgePolicy] = settings;
                if (facePolicy != Filter::FaceRenderPolicy::RenderNone ||
                    edgePolicy != Filter::EdgeRenderPolicy::RenderNone) {
                    vertexCount += brush->cachedVertexCount();
                }
            }

            // collect vertices
            {
                VertexListBuilder<Model::Brush::Vertex::Spec> builder(vertexCount);
                for (const auto *brush : m_brushes) {
                    const auto [renderType, facePolicy, edgePolicy] = brush->renderSettings();

                    if (facePolicy != Filter::FaceRenderPolicy::RenderNone ||
                        edgePolicy != Filter::EdgeRenderPolicy::RenderNone) {
                        brush->getVertices(builder);
                    }
                }
                m_vertexArray = VertexHolder<Model::Brush::Vertex>::swap(builder.vertices());
            }

            // count indices
            TexturedIndexArrayMap::Size opaqueIndexSize;
            TexturedIndexArrayMap::Size transparentIndexSize;
            IndexArrayMap::Size edgeIndexSize;
            for (const auto *brush : m_brushes) {
                const auto [renderType, facePolicy, edgePolicy] = brush->renderSettings();

                brush->countMarkedEdgeIndices(edgePolicy, edgeIndexSize);

                switch (renderType) {
                    case Filter::RenderOpacity::Opaque:
                        brush->countMarkedFaceIndices(facePolicy, opaqueIndexSize);
                        break;
                    case Filter::RenderOpacity::Transparent:
                        brush->countMarkedFaceIndices(facePolicy, transparentIndexSize);
                        break;
                }
            }

            // collect indices
            TexturedIndexArrayBuilder opaqueFaceIndexBuilder(opaqueIndexSize);
            TexturedIndexArrayBuilder transparentFaceIndexBuilder(transparentIndexSize);
            IndexArrayMapBuilder edgeIndexBuilder(edgeIndexSize);
            for (const auto *brush : m_brushes) {
                const auto [renderType, facePolicy, edgePolicy] = brush->renderSettings();

                brush->getMarkedEdgeIndices(edgePolicy, edgeIndexBuilder);

                switch (renderType) {
                    case Filter::RenderOpacity::Opaque:
                        brush->getMarkedFaceIndices(facePolicy, opaqueFaceIndexBuilder);
                        break;
                    case Filter::RenderOpacity::Transparent:
                        brush->getMarkedFaceIndices(facePolicy, transparentFaceIndexBuilder);
                        break;
                }
            }

            // Move the vertices and indices into the VBO/index buffer wrapper objects.
            // (they're not actually copied to the GPU until later)
            IndexArrayPtr opaqueIndices = IndexHolder::swap(opaqueFaceIndexBuilder.indices());
            const TexturedIndexArrayMap& opaqueRanges = opaqueFaceIndexBuilder.ranges();

            IndexArrayPtr transparentIndices = IndexHolder::swap(transparentFaceIndexBuilder.indices());
            const TexturedIndexArrayMap& transparentRanges = transparentFaceIndexBuilder.ranges();

            m_opaqueFaceRenderer = FaceRenderer(m_vertexArray, opaqueIndices, opaqueRanges, m_faceColor);
            m_transparentFaceRenderer = FaceRenderer(m_vertexArray, transparentIndices, transparentRanges, m_faceColor);

            IndexArrayPtr edgeIndices = IndexHolder::swap(edgeIndexBuilder.indices());
            const IndexArrayMap& edgeRanges = edgeIndexBuilder.ranges();
            m_edgeRenderer = IndexedEdgeRenderer(m_vertexArray, edgeIndices, edgeRanges);

            m_valid = true;
        }
    }
}
