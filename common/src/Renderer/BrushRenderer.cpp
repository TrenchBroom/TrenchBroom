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
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/TexturedIndexArrayBuilder.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        BrushRenderer::FaceAcceptor::~FaceAcceptor() {}
        BrushRenderer::EdgeAcceptor::~EdgeAcceptor() {}
        
        BrushRenderer::Filter::Filter() {}
        
        BrushRenderer::Filter::Filter(const Filter& other) {}

        BrushRenderer::Filter::~Filter() {}
        
        BrushRenderer::Filter& BrushRenderer::Filter::operator=(const Filter& other) { return *this; }

        void BrushRenderer::Filter:: provideFaces(const Model::Brush* brush, BrushRenderer::FaceAcceptor& collectFace) const {
            doProvideFaces(brush, collectFace);
        }
        
        void BrushRenderer::Filter:: provideEdges(const Model::Brush* brush, BrushRenderer::EdgeAcceptor& collectEdge) const {
            doProvideEdges(brush, collectEdge);
        }
        
        bool BrushRenderer::Filter::transparent(const Model::Brush* brush) const  {
            return doIsTransparent(brush);
        }

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

        BrushRenderer::NoFilter::NoFilter(const bool transparent) : m_transparent(transparent) {}

        void BrushRenderer::NoFilter::doProvideFaces(const Model::Brush* brush, BrushRenderer::FaceAcceptor& faceAcceptor) const {
            for (const Model::BrushFace* face : brush->faces())
                faceAcceptor.accept(face);
        }
        
        void BrushRenderer::NoFilter::doProvideEdges(const Model::Brush* brush, BrushRenderer::EdgeAcceptor& edgeAcceptor) const {
            for (const Model::BrushEdge* edge : brush->edges())
                edgeAcceptor.accept(edge);
        }
        
        
        bool BrushRenderer::NoFilter::doIsTransparent(const Model::Brush* brush) const {
            return m_transparent;
        }

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
            m_filter = NULL;
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
            m_vertexArray = VertexArray();
            m_valid = false;
        }
        
        void BrushRenderer::clear() {
            m_brushes.clear();
            m_transparentFaceRenderer = FaceRenderer();
            m_opaqueFaceRenderer = FaceRenderer();
            m_vertexArray = VertexArray();
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
        public:
            FilterWrapper(const Filter& filter, const bool showHiddenBrushes) :
            m_filter(filter),
            m_showHiddenBrushes(showHiddenBrushes),
            m_noFilter(false) {}
            
            void doProvideFaces(const Model::Brush* brush, FaceAcceptor& faceAcceptor) const {
                if (m_showHiddenBrushes)
                    m_noFilter.provideFaces(brush, faceAcceptor);
                else
                    m_filter.provideFaces(brush, faceAcceptor);
            }
            
            void doProvideEdges(const Model::Brush* brush, EdgeAcceptor& edgeAcceptor) const {
                if (m_showHiddenBrushes)
                    m_noFilter.provideEdges(brush, edgeAcceptor);
                else
                    m_filter.provideEdges(brush, edgeAcceptor);
            }

            bool doIsTransparent(const Model::Brush* brush) const { return m_filter.transparent(brush); }
        };
        
        class BrushRenderer::CountVertices : public Model::ConstNodeVisitor, public BrushRenderer::FaceAcceptor {
        private:
            const FilterWrapper& m_filter;
            size_t m_vertexCount;
        public:
            CountVertices(const FilterWrapper& filter) :
            m_filter(filter),
            m_vertexCount(0) {}
            
            size_t vertexCount() const {
                return m_vertexCount;
            }
        private:
            void doVisit(const Model::World* world) {}
            void doVisit(const Model::Layer* layer) {}
            void doVisit(const Model::Group* group) {}
            void doVisit(const Model::Entity* entity) {}
            void doVisit(const Model::Brush* brush) {
                countFaceVertices(brush);
            }
            
            void countFaceVertices(const Model::Brush* brush) {
                m_filter.provideFaces(brush, *this);
            }
            
            void accept(const Model::BrushFace* face) {
                m_vertexCount += face->vertexCount();
            }
        };

        class BrushRenderer::CollectVertices : public Model::ConstNodeVisitor, public BrushRenderer::FaceAcceptor {
        private:
            const FilterWrapper& m_filter;
            VertexListBuilder<Model::BrushFace::Vertex::Spec> m_builder;
        public:
            CollectVertices(const FilterWrapper& filter, const size_t faceVertexCount) :
            m_filter(filter),
            m_builder(faceVertexCount) {}
            
            VertexArray vertexArray() {
                return VertexArray::swap(m_builder.vertices());
            }
        private:
            void doVisit(const Model::World* world) {}
            void doVisit(const Model::Layer* layer) {}
            void doVisit(const Model::Group* group) {}
            void doVisit(const Model::Entity* entity) {}
            void doVisit(const Model::Brush* brush) {
                collectFaceVertices(brush);
            }
            
            void collectFaceVertices(const Model::Brush* brush) {
                m_filter.provideFaces(brush, *this);
            }
            
            void accept(const Model::BrushFace* face) {
                face->getVertices(m_builder);
            }
        };
        
        class BrushRenderer::CountIndices : public Model::ConstNodeVisitor, public BrushRenderer::FaceAcceptor, public BrushRenderer::EdgeAcceptor {
        private:
            bool m_brushTransparent;
            const FilterWrapper& m_filter;
            TexturedIndexArrayMap::Size m_opaqueIndexSize;
            TexturedIndexArrayMap::Size m_transparentIndexSize;
            IndexArrayMap::Size m_edgeIndexSize;
        public:
            CountIndices(const FilterWrapper& filter) :
            m_filter(filter) {}
            
            const TexturedIndexArrayMap::Size& opaqueIndexSize() const {
                return m_opaqueIndexSize;
            }
            
            const TexturedIndexArrayMap::Size& transparentIndexSize() const {
                return m_transparentIndexSize;
            }
            
            const IndexArrayMap::Size& edgeIndexSize() const {
                return m_edgeIndexSize;
            }
        private:
            void doVisit(const Model::World* world) {}
            void doVisit(const Model::Layer* layer) {}
            void doVisit(const Model::Group* group) {}
            void doVisit(const Model::Entity* entity) {}
            void doVisit(const Model::Brush* brush) {
                countFaceIndices(brush);
                countEdgeIndices(brush);
            }
            
            void countFaceIndices(const Model::Brush* brush) {
                // HACK:
                m_brushTransparent = m_filter.transparent(brush);
                m_filter.provideFaces(brush, *this);
            }
            
            void accept(const Model::BrushFace* face) {
                if (m_brushTransparent)
                    face->countIndices(m_transparentIndexSize);
                else
                    face->countIndices(m_opaqueIndexSize);
            }
            
            void countEdgeIndices(const Model::Brush* brush) {
                m_filter.provideEdges(brush, *this);
            }
            
            void accept(const Model::BrushEdge* edge) {
                m_edgeIndexSize.inc(GL_LINES, 2);
            }
        };
        
        class BrushRenderer::CollectIndices : public Model::ConstNodeVisitor, public BrushRenderer::FaceAcceptor, public BrushRenderer::EdgeAcceptor {
        private:
            bool m_brushTransparent;
            const FilterWrapper& m_filter;
            TexturedIndexArrayBuilder m_opaqueFaceIndexBuilder;
            TexturedIndexArrayBuilder m_transparentFaceIndexBuilder;
            IndexArrayMapBuilder m_edgeIndexBuilder;
        public:
            CollectIndices(const FilterWrapper& filter, const CountIndices& indexSize) :
            m_filter(filter),
            m_opaqueFaceIndexBuilder(indexSize.opaqueIndexSize()),
            m_transparentFaceIndexBuilder(indexSize.transparentIndexSize()),
            m_edgeIndexBuilder(indexSize.edgeIndexSize()) {}
            
            TexturedIndexArrayBuilder& opaqueFaceIndices() {
                return m_opaqueFaceIndexBuilder;
            }
            
            TexturedIndexArrayBuilder& transparentFaceIndices() {
                return m_transparentFaceIndexBuilder;
            }
            
            IndexArrayMapBuilder& edgeIndices() {
                return m_edgeIndexBuilder;
            }
        private:
            void doVisit(const Model::World* world) {}
            void doVisit(const Model::Layer* layer) {}
            void doVisit(const Model::Group* group) {}
            void doVisit(const Model::Entity* entity) {}
            void doVisit(const Model::Brush* brush) {
                collectFaceIndices(brush);
                collectEdgeIndices(brush);
            }
            
            void collectFaceIndices(const Model::Brush* brush) {
                // HACK: cache this now
                m_brushTransparent = m_filter.transparent(brush);
                
                m_filter.provideFaces(brush, *this);
            }
            
            void accept(const Model::BrushFace* face) {
                if (m_brushTransparent)
                    face->getFaceIndices(m_transparentFaceIndexBuilder);
                else
                    face->getFaceIndices(m_opaqueFaceIndexBuilder);
            }
            
            void collectEdgeIndices(const Model::Brush* brush) {
                m_filter.provideEdges(brush, *this);
            }
            
            void accept(const Model::BrushEdge* edge) {
                const Model::BrushVertex* v1 = edge->firstVertex();
                const Model::BrushVertex* v2 = edge->secondVertex();
                m_edgeIndexBuilder.addLine(v1->payload(), v2->payload());
            }
        };
        
        void BrushRenderer::validate() {
            assert(!m_valid);
            validateVertices();
            validateIndices();
            m_valid = true;
        }
        
        void BrushRenderer::validateVertices() {
            const FilterWrapper wrapper(*m_filter, m_showHiddenBrushes);
            CountVertices countVertices(wrapper);
            Model::Node::accept(std::begin(m_brushes), std::end(m_brushes), countVertices);
            
            CollectVertices collectVertices(wrapper, countVertices.vertexCount());
            Model::Node::accept(std::begin(m_brushes), std::end(m_brushes), collectVertices);
            
            m_vertexArray = collectVertices.vertexArray();
        }
        
        void BrushRenderer::validateIndices() {
            const FilterWrapper wrapper(*m_filter, m_showHiddenBrushes);
            CountIndices countIndices(wrapper);
            Model::Node::accept(std::begin(m_brushes), std::end(m_brushes), countIndices);
            
            CollectIndices collectIndices(wrapper, countIndices);
            Model::Node::accept(std::begin(m_brushes), std::end(m_brushes), collectIndices);
            
            const IndexArray opaqueIndices = IndexArray::swap(collectIndices.opaqueFaceIndices().indices());
            const TexturedIndexArrayMap& opaqueRanges = collectIndices.opaqueFaceIndices().ranges();
            
            const IndexArray transparentIndices = IndexArray::swap(collectIndices.transparentFaceIndices().indices());
            const TexturedIndexArrayMap& transparentRanges = collectIndices.transparentFaceIndices().ranges();
            
            m_opaqueFaceRenderer = FaceRenderer(m_vertexArray, opaqueIndices, opaqueRanges, m_faceColor);
            m_transparentFaceRenderer = FaceRenderer(m_vertexArray, transparentIndices, transparentRanges, m_faceColor);
            
            const IndexArray edgeIndices = IndexArray::swap(collectIndices.edgeIndices().indices());
            const IndexArrayMap& edgeRanges = collectIndices.edgeIndices().ranges();
            m_edgeRenderer = IndexedEdgeRenderer(m_vertexArray, edgeIndices, edgeRanges);
        }
    }
}
