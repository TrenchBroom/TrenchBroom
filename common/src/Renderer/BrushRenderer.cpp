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
#include "Model/NodeVisitor.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/VertexSpec.h"
#include "View/EditorContext.h"

namespace TrenchBroom {
    namespace Renderer {
        BrushRenderer::Filter::~Filter() {}
        
        BrushRenderer::DefaultFilter::~DefaultFilter() {}
        BrushRenderer::DefaultFilter::DefaultFilter(const View::EditorContext& context) : m_context(context) {}

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

        BrushRenderer::~BrushRenderer() {
            delete m_filter;
            m_filter = NULL;
        }

        void BrushRenderer::addBrush(Model::Brush* brush) {
            m_brushes.push_back(brush);
            invalidate();
        }
        
        void BrushRenderer::removeBrush(Model::Brush* brush) {
            VectorUtils::erase(m_brushes, brush);
            invalidate();
        }

        void BrushRenderer::setBrushes(const Model::BrushList& brushes) {
            m_brushes = brushes;
            invalidate();
        }


        void BrushRenderer::invalidate() {
            m_valid = false;
        }
        
        void BrushRenderer::clear() {
            m_brushes.clear();
            invalidate();
        }

        void BrushRenderer::render(RenderContext& renderContext) {
            if (!m_valid)
                validate();
            
            if (renderContext.showFaces())
                renderFaces(renderContext);
            
            if (renderContext.showEdges())
                renderEdges(renderContext);
        }
        
        bool BrushRenderer::grayscale() const {
            return m_grayscale;
        }
        
        void BrushRenderer::setGrayscale(const bool grayscale) {
            m_grayscale = grayscale;
        }
        
        const Color& BrushRenderer::faceColor() const {
            return m_faceColor;
        }
        
        void BrushRenderer::setFaceColor(const Color& faceColor) {
            m_faceColor = faceColor;
        }
        
        const Color& BrushRenderer::edgeColor() const {
            return m_edgeColor;
        }
        
        void BrushRenderer::setEdgeColor(const Color& edgeColor) {
            m_edgeColor = edgeColor;
        }

        bool BrushRenderer::tintFaces() const {
            return m_tintFaces;
        }
        
        void BrushRenderer::setTintFaces(const bool tintFaces) {
            m_tintFaces = tintFaces;
        }
        
        const Color& BrushRenderer::tintColor() const {
            return m_tintColor;
        }

        void BrushRenderer::setTintColor(const Color& tintColor) {
            m_tintColor = tintColor;
        }

        bool BrushRenderer::renderOccludedEdges() const {
            return m_renderOccludedEdges;
        }
        
        void BrushRenderer::setRenderOccludedEdges(const bool renderOccludedEdges) {
            m_renderOccludedEdges = renderOccludedEdges;
        }
        
        const Color& BrushRenderer::occludedEdgeColor() const {
            return m_occludedEdgeColor;
        }
        
        void BrushRenderer::setOccludedEdgeColor(const Color& occludedEdgeColor) {
            m_occludedEdgeColor = occludedEdgeColor;
        }

        float BrushRenderer::transparencyAlpha() const {
            return m_transparencyAlpha;
        }
        
        bool BrushRenderer::showHiddenBrushes() const {
            return m_showHiddenBrushes;
        }
        
        void BrushRenderer::setShowHiddenBrushes(const bool showHiddenBrushes) {
            if (showHiddenBrushes == m_showHiddenBrushes)
                return;
            m_showHiddenBrushes = showHiddenBrushes;
            invalidate();
        }
        
        void BrushRenderer::setTransparencyAlpha(const float transparencyAlpha) {
            m_transparencyAlpha = transparencyAlpha;
        }

        void BrushRenderer::renderFaces(RenderContext& renderContext) {
            FaceRenderer::Config config;
            config.grayscale = grayscale();
            config.tinted = tintFaces();
            config.tintColor = tintColor();
            m_opaqueFaceRenderer.render(renderContext, config);
            
            config.alpha = transparencyAlpha();
            m_transparentFaceRenderer.render(renderContext, config);
        }
        
        void BrushRenderer::renderEdges(RenderContext& renderContext) {
            if (renderOccludedEdges()) {
                glDisable(GL_DEPTH_TEST);
                m_edgeRenderer.setUseColor(true);
                m_edgeRenderer.setColor(occludedEdgeColor());
                m_edgeRenderer.render(renderContext);
                glEnable(GL_DEPTH_TEST);
            }
            
            glSetEdgeOffset(0.02f);
            m_edgeRenderer.setUseColor(true);
            m_edgeRenderer.setColor(edgeColor());
            m_edgeRenderer.render(renderContext);
            glResetEdgeOffset();
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
            CountRenderDataSize(const FilterWrapper& i_filter) : filter(i_filter) {}
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
            
            m_opaqueFaceRenderer = FaceRenderer(builder.opaqueMesh, faceColor());
            m_transparentFaceRenderer = FaceRenderer(builder.transparentMesh, faceColor());
            m_edgeRenderer = EdgeRenderer(VertexArray::swap(GL_LINES, builder.edgeVertices));
            
            m_valid = true;
        }
    }
}
