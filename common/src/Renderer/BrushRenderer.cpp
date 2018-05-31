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

#include <algorithm>

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
        m_showEdges(false),
        m_grayscale(false),
        m_tint(false),
        m_showOccludedEdges(false),
        m_transparencyAlpha(1.0f),
        m_showHiddenBrushes(false) {
            clear();
        }
        
        BrushRenderer::~BrushRenderer() {
            delete m_filter;
            m_filter = nullptr;
        }

        void BrushRenderer::addBrushes(const Model::BrushList& brushes) {
            for (auto* brush : brushes) {
                addBrush(brush);
            }
        }

        void BrushRenderer::setBrushes(const Model::BrushList& brushes) {
            // start with adding nothing, and removing everything
            std::set<const Model::Brush*> toAdd;
            std::map<const Model::Brush*, bool> toRemove = m_brushValid;

            // update toAdd and toRemove using the input list
            for (const auto* brush : brushes) {
                if (auto it = toRemove.find(brush); it != toRemove.end()) {
                    toRemove.erase(it);
                } else {
                    toAdd.insert(brush);
                }
            }

            for (auto [brush, wasValid] : toRemove) {
                removeBrush(brush);
            }
            for (auto brush : toAdd) {
                addBrush(brush);
            }
        }

        void BrushRenderer::invalidate() {
            for (auto& [brush, valid] : m_brushValid) {
                if (valid) {
                    removeBrushFromVbo(brush);
                }
                valid = false;
            }
        }

        bool BrushRenderer::valid() const {
            // TODO: probably worth caching in a variable and updating when needed
            for (auto& [brush, valid] : m_brushValid) {
                if (!valid) {
                    return false;
                }
            }
            return true;
        }
        
        void BrushRenderer::clear() {
            m_brushValid.clear();

            m_vertexArray = std::make_shared<BrushVertexHolder>();
            m_edgeIndices = std::make_shared<BrushIndexHolder>();
            m_transparentFaces = std::make_shared<TextureToBrushIndicesMap>();
            m_opaqueFaces = std::make_shared<TextureToBrushIndicesMap>();

            m_opaqueFaceRenderer = FaceRenderer(m_vertexArray, m_opaqueFaces, m_faceColor);
            m_transparentFaceRenderer = FaceRenderer(m_vertexArray, m_transparentFaces, m_faceColor);
            m_edgeRenderer = IndexedEdgeRenderer(m_vertexArray, m_edgeIndices);
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
            if (!m_brushValid.empty()) {
                if (!valid())
                    validate();
                if (renderContext.showFaces())
                    renderOpaqueFaces(renderBatch);
                if (renderContext.showEdges() || m_showEdges)
                    renderEdges(renderBatch);
            }
        }
        
        void BrushRenderer::renderTransparent(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_brushValid.empty()) {
                if (!valid())
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
            assert(!valid());

            size_t validateCalls = 0;
            for (auto& [brush, valid] : m_brushValid) {
                if (!valid) {
                    validateCalls++;

                    validateBrush(brush);
                    valid = true;
                }
            }
            std::cout << "validate " << validateCalls << " brushes\n";

            assert(valid());

            m_opaqueFaceRenderer = FaceRenderer(m_vertexArray, m_opaqueFaces, m_faceColor);
            m_transparentFaceRenderer = FaceRenderer(m_vertexArray, m_transparentFaces, m_faceColor);
            m_edgeRenderer = IndexedEdgeRenderer(m_vertexArray, m_edgeIndices);
        }

        void BrushRenderer::validateBrush(const Model::Brush* brush) {
            assert(!m_brushValid.at(brush));
            assert(m_brushInfo.find(brush) == m_brushInfo.end());

            BrushInfo info;

            const FilterWrapper wrapper(*m_filter, m_showHiddenBrushes);

            // evaluate filter. only evaluate the filter once per brush.
            const auto settings = wrapper.markFaces(brush);
            brush->setRenderSettings(settings);

            const auto [renderType, facePolicy, edgePolicy] = brush->renderSettings();

            if (facePolicy == Filter::FaceRenderPolicy::RenderNone &&
                edgePolicy == Filter::EdgeRenderPolicy::RenderNone) {
                return;
            }

             // collect vertices
            {
                const size_t vertexCount = brush->cachedVertexCount();
                VertexListBuilder<Model::Brush::Vertex::Spec> builder(vertexCount);
                brush->getVertices(builder);

                assert(m_vertexArray != nullptr);
                const size_t vertOffset = m_vertexArray->insertVertices(builder.vertices());
                brush->setBrushVerticesStartIndex(vertOffset);

                info.vertexHolderKey = vertOffset;
            }

            // count indices
            TexturedIndexArrayMap::Size opaqueIndexSize;
            TexturedIndexArrayMap::Size transparentIndexSize;
            // TODO: These are only ever gl_lines, so get rid of the map.
            IndexArrayMap::Size edgeIndexSize;

            brush->countMarkedEdgeIndices(edgePolicy, edgeIndexSize);

            switch (renderType) {
                case Filter::RenderOpacity::Opaque:
                    brush->countMarkedFaceIndices(facePolicy, opaqueIndexSize);
                    break;
                case Filter::RenderOpacity::Transparent:
                    brush->countMarkedFaceIndices(facePolicy, transparentIndexSize);
                    break;
            }

            // collect indices

            TexturedIndexArrayBuilder opaqueFaceIndexBuilder(opaqueIndexSize);
            TexturedIndexArrayBuilder transparentFaceIndexBuilder(transparentIndexSize);
            IndexArrayMapBuilder edgeIndexBuilder(edgeIndexSize);

            brush->getMarkedEdgeIndices(edgePolicy, edgeIndexBuilder);

            switch (renderType) {
                case Filter::RenderOpacity::Opaque:
                    brush->getMarkedFaceIndices(facePolicy, opaqueFaceIndexBuilder);
                    break;
                case Filter::RenderOpacity::Transparent:
                    brush->getMarkedFaceIndices(facePolicy, transparentFaceIndexBuilder);
                    break;
            }

            // insert into Vbo's

            // the edges can only be lines.
            assert(edgeIndexBuilder.ranges().pointsRange().count == 0);
            assert(edgeIndexBuilder.ranges().trianglesRange().count == 0);
            assert(edgeIndexBuilder.ranges().linesRange().count == edgeIndexBuilder.indices().size());
            info.edgeIndicesKey = m_edgeIndices->insertElements(edgeIndexBuilder.indices());

            // the faces can only be tris
            for (const auto& [texture, range] : opaqueFaceIndexBuilder.ranges().ranges()) {
                // FIXME: skip copy
                std::vector<GLuint> textureTris;
                textureTris.resize(range.count);
                assert(range.count > 0);

                std::copy(opaqueFaceIndexBuilder.indices().cbegin() + range.offset,
                          opaqueFaceIndexBuilder.indices().cbegin() + range.offset + range.count,
                          textureTris.begin());

                assert(textureTris.size() == range.count);

                // FIXME: consolidate map lookups
                if ((*m_opaqueFaces)[texture] == nullptr) {
                    (*m_opaqueFaces)[texture] = std::make_shared<BrushIndexHolder>();
                }
                const size_t offset = (*m_opaqueFaces)[texture]->insertElements(textureTris);
                info.opaqueFaceIndicesKeys.push_back({texture, offset});
            }

            for (const auto& [texture, range] : transparentFaceIndexBuilder.ranges().ranges()) {
                // FIXME: skip copy
                std::vector<GLuint> textureTris;
                textureTris.resize(range.count);
                assert(range.count > 0);

                std::copy(transparentFaceIndexBuilder.indices().cbegin() + range.offset,
                          transparentFaceIndexBuilder.indices().cbegin() + range.offset + range.count,
                          textureTris.begin());

                assert(textureTris.size() == range.count);

                // FIXME: consolidate map lookups
                if ((*m_transparentFaces)[texture] == nullptr) {
                    (*m_transparentFaces)[texture] = std::make_shared<BrushIndexHolder>();
                }
                const size_t offset = (*m_transparentFaces)[texture]->insertElements(textureTris);
                info.transparentFaceIndicesKeys.push_back({texture, offset});
            }

            // FIXME: avoid copying
            m_brushInfo[brush] = info;
        }

        void BrushRenderer::addBrush(const Model::Brush* brush) {
            // i.e. insert the brush as "invalid" if it's not already present.
            // if it is present, its validity is unchanged.
            [[maybe_unused]] auto result = m_brushValid.insert(std::make_pair(brush, false));
        }

        void BrushRenderer::removeBrush(const Model::Brush* brush) {
            // update m_brushValid
            {
                auto it = m_brushValid.find(brush);
                assert(it != m_brushValid.end());

                const bool wasValid = it->second;
                m_brushValid.erase(it);

                if (!wasValid) {
                    // invalid brushes are not in the VBO, so we can return  now.
                    return;
                }
            }

            removeBrushFromVbo(brush);
        }

        void BrushRenderer::removeBrushFromVbo(const Model::Brush* brush) {
            auto it = m_brushInfo.find(brush);
            assert(it != m_brushInfo.end());

            const BrushInfo& info = it->second;

            // update Vbo's
            m_vertexArray->deleteVerticesWithKey(info.vertexHolderKey);
            m_edgeIndices->zeroElementsWithKey(info.edgeIndicesKey);

            for (const auto& [texture, opaqueKey] : info.opaqueFaceIndicesKeys) {
                std::shared_ptr<BrushIndexHolder> faceIndexHolder = m_opaqueFaces->at(texture);
                faceIndexHolder->zeroElementsWithKey(opaqueKey);
            }
            for (const auto& [texture, transparentKey] : info.transparentFaceIndicesKeys) {
                std::shared_ptr<BrushIndexHolder> faceIndexHolder = m_transparentFaces->at(texture);
                faceIndexHolder->zeroElementsWithKey(transparentKey);
            }

            m_brushInfo.erase(it);
        }
    }
}
