/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"
#include "Model/EditorContext.h"
#include "Model/NodeVisitor.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        BrushRenderer::Filter::~Filter() {}
        
        BrushRenderer::DefaultFilter::~DefaultFilter() {}
        BrushRenderer::DefaultFilter::DefaultFilter(const Model::EditorContext& context) : m_context(context) {}

        bool BrushRenderer::DefaultFilter::visible(const Model::Brush* brush) const { return m_context.visible(brush); }
        bool BrushRenderer::DefaultFilter::visible(const Model::BrushFace* face) const { return m_context.visible(face); }
        
        bool BrushRenderer::DefaultFilter::locked(const Model::Brush* brush) const { return m_context.locked(brush); }
        bool BrushRenderer::DefaultFilter::locked(const Model::BrushFace* face) const { return m_context.locked(face); }
        
        bool BrushRenderer::DefaultFilter::selected(const Model::Brush* brush) const { return brush->selected(); }
        bool BrushRenderer::DefaultFilter::selected(const Model::BrushFace* face) const { return face->selected(); }
        bool BrushRenderer::DefaultFilter::selected(const Model::BrushEdge* edge) const {
            const Model::BrushFace* left = edge->left->face;
            const Model::BrushFace* right = edge->right->face;
            const Model::Brush* brush = left->brush();
            return selected(brush) || selected(left) || selected(right);
        }
        bool BrushRenderer::DefaultFilter::hasSelectedFaces(const Model::Brush* brush) const { return brush->descendantSelected(); }

        bool BrushRenderer::NoFilter::operator()(const Model::Brush* brush) const { return true; }
        bool BrushRenderer::NoFilter::operator()(const Model::BrushFace* face) const { return true; }
        bool BrushRenderer::NoFilter::operator()(const Model::BrushEdge* edge) const { return true; }

        BrushRenderer::BrushRenderer() :
        m_filter(new NoFilter()),
        m_valid(false),
        m_grayscale(false),
        m_tint(false),
        m_showOccludedEdges(false),
        m_transparencyAlpha(1.0f),
        m_showHiddenBrushes(false) {}
        
        BrushRenderer::~BrushRenderer() {
            delete m_filter;
            m_filter = NULL;
        }

        void BrushRenderer::addBrush(Model::Brush* brush) {
            m_brushes.insert(brush);
            invalidate();
        }
        
        void BrushRenderer::removeBrush(Model::Brush* brush) {
            m_brushes.erase(brush);
            invalidate();
        }

        void BrushRenderer::updateBrush(Model::Brush* brush) {
            invalidate();
        }
        
        void BrushRenderer::invalidate() {
            m_valid = false;
        }
        
        void BrushRenderer::clear() {
            m_brushes.clear();
            invalidate();
        }

        void BrushRenderer::setFaceColor(const Color& faceColor) {
            m_faceColor = faceColor;
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
            if (showHiddenBrushes == m_showHiddenBrushes)
                return;
            m_showHiddenBrushes = showHiddenBrushes;
            invalidate();
        }

        void BrushRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_brushes.empty()) {
                if (!m_valid)
                    validate();
                if (renderContext.showFaces())
                    renderFaces(renderBatch);
                if (renderContext.showEdges())
                    renderEdges(renderBatch);
            }
        }
        

        void BrushRenderer::renderFaces(RenderBatch& renderBatch) {
            m_opaqueFaceRenderer.setGrayscale(m_grayscale);
            m_opaqueFaceRenderer.setTint(m_tint);
            m_opaqueFaceRenderer.setTintColor(m_tintColor);
            m_opaqueFaceRenderer.render(renderBatch);
            
            m_transparentFaceRenderer.setGrayscale(m_grayscale);
            m_transparentFaceRenderer.setTint(m_tint);
            m_transparentFaceRenderer.setTintColor(m_tintColor);
            m_transparentFaceRenderer.setAlpha(m_transparencyAlpha);
            m_transparentFaceRenderer.render(renderBatch);
        }
        
        void BrushRenderer::renderEdges(RenderBatch& renderBatch) {
            if (m_showOccludedEdges) {
                Renderer::RenderEdges* renderOccludedEdges = new Renderer::RenderEdges(Reference::ref(m_edgeRenderer));
                renderOccludedEdges->setRenderOccluded();
                renderOccludedEdges->setColor(m_occludedEdgeColor);
                renderBatch.addOneShot(renderOccludedEdges);
            }
            
            Renderer::RenderEdges* renderUnoccludedEdges = new Renderer::RenderEdges(Reference::ref(m_edgeRenderer));
            renderUnoccludedEdges->setColor(m_edgeColor);
            renderBatch.addOneShot(renderUnoccludedEdges);
        }

        class FilterWrapper : public BrushRenderer::Filter {
        private:
            const Filter& m_filter;
            bool m_showHiddenBrushes;
        public:
            FilterWrapper(const Filter& filter, const bool showHiddenBrushes) :
            m_filter(filter),
            m_showHiddenBrushes(showHiddenBrushes) {}
            
            bool operator()(const Model::Brush* brush) const    { return m_showHiddenBrushes || m_filter(brush); }
            bool operator()(const Model::BrushFace* face) const { return m_showHiddenBrushes || m_filter(face); }
            bool operator()(const Model::BrushEdge* edge) const { return m_showHiddenBrushes || m_filter(edge); }
        };

        class CountRenderDataSize : public Model::ConstNodeVisitor {
        private:
            const FilterWrapper& filter;
        public:
            Model::BrushFace::Mesh::MeshSize opaqueMeshSize;
            Model::BrushFace::Mesh::MeshSize transparentMeshSize;
            size_t edgeVertexCount;
        public:
            CountRenderDataSize(const FilterWrapper& i_filter) :
            filter(i_filter),
            edgeVertexCount(0) {}
        private:
            void doVisit(const Model::World* world) {}
            void doVisit(const Model::Layer* layer) {}
            void doVisit(const Model::Group* group) {}
            void doVisit(const Model::Entity* entity) {}
            void doVisit(const Model::Brush* brush) {
                countMesh(brush);
                countEdges(brush);
            }
            
            void countMesh(const Model::Brush* brush) {
                const Model::BrushFaceList& faces = brush->faces();
                Model::BrushFaceList::const_iterator it, end;
                for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                    const Model::BrushFace* face = *it;
                    if (filter(face)) {
                        if (brush->transparent())
                            transparentMeshSize.addSet(face->texture(), face->vertexCount());
                        else
                            opaqueMeshSize.addSet(face->texture(), face->vertexCount());
                    }
                }
            }
            
            void countEdges(const Model::Brush* brush) {
                const Model::BrushEdgeList edges = brush->edges();
                Model::BrushEdgeList::const_iterator it, end;
                for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                    const Model::BrushEdge* edge = *it;
                    if (filter(edge))
                        edgeVertexCount += 2;
                }
            }
        };
        
        class BuildRenderData : public Model::ConstNodeVisitor {
        private:
            const FilterWrapper& filter;
        public:
            Model::BrushFace::Mesh opaqueMesh;
            Model::BrushFace::Mesh transparentMesh;
            VertexSpecs::P3::Vertex::List edgeVertices;
        public:
            BuildRenderData(const FilterWrapper& i_filter,
                            const Model::BrushFace::Mesh::MeshSize& opaqueMeshSize,
                            const Model::BrushFace::Mesh::MeshSize& transparentMeshSize,
                            const size_t edgeVertexCount) :
            filter(i_filter),
            opaqueMesh(opaqueMeshSize),
            transparentMesh(transparentMeshSize) {
                edgeVertices.reserve(edgeVertexCount);
            }
        private:
            void doVisit(const Model::World* world) {}
            void doVisit(const Model::Layer* layer) {}
            void doVisit(const Model::Group* group) {}
            void doVisit(const Model::Entity* entity) {}
            void doVisit(const Model::Brush* brush) {
                buildMesh(brush);
                buildEdges(brush);
            }
            
            void buildMesh(const Model::Brush* brush) {
                const Model::BrushFaceList& faces = brush->faces();
                Model::BrushFaceList::const_iterator it, end;
                for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                    const Model::BrushFace* face = *it;
                    if (filter(face)) {
                        if (brush->transparent())
                            face->addToMesh(transparentMesh);
                        else
                            face->addToMesh(opaqueMesh);
                    }
                }
            }
            
            void buildEdges(const Model::Brush* brush) {
                const Model::BrushEdgeList edges = brush->edges();
                Model::BrushEdgeList::const_iterator it, end;
                for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                    const Model::BrushEdge* edge = *it;
                    if (filter(edge)) {
                        edgeVertices.push_back(VertexSpecs::P3::Vertex(edge->start->position));
                        edgeVertices.push_back(VertexSpecs::P3::Vertex(edge->end->position));
                    }
                }
            }
        };
        
        void BrushRenderer::validate() {
            const FilterWrapper wrapper(*m_filter, m_showHiddenBrushes);
            
            CountRenderDataSize counter(wrapper);
            Model::Node::accept(m_brushes.begin(), m_brushes.end(), counter);
            
            BuildRenderData builder(wrapper, counter.opaqueMeshSize, counter.transparentMeshSize, counter.edgeVertexCount);
            Model::Node::accept(m_brushes.begin(), m_brushes.end(), builder);
            
            m_opaqueFaceRenderer = FaceRenderer(builder.opaqueMesh, m_faceColor);
            m_transparentFaceRenderer = FaceRenderer(builder.transparentMesh, m_faceColor);
            m_edgeRenderer = EdgeRenderer(VertexArray::swap(GL_LINES, builder.edgeVertices));
            
            m_valid = true;
        }
    }
}
