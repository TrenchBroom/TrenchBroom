/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BrushRenderer.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushFacesIterator.h"
#include "Model/BrushEdge.h"
#include "Model/BrushVertex.h"
#include "Model/Filter.h"
#include "Model/ModelUtils.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        struct BuildBrushEdges {
            VertexSpecs::P3::Vertex::List vertices;
            BrushRenderer::Filter& filter;
            
            BuildBrushEdges(BrushRenderer::Filter& i_filter) :
            filter(i_filter) {}
            
            inline void operator()(Model::Brush* brush) {
                const Model::BrushEdge::List edges = brush->edges();
                Model::BrushEdge::List::const_iterator it, end;
                for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                    const Model::BrushEdge* edge = *it;
                    if (filter(edge)) {
                        vertices.push_back(VertexSpecs::P3::Vertex(edge->start()->position()));
                        vertices.push_back(VertexSpecs::P3::Vertex(edge->end()->position()));
                    }
                }
            }
        };
        
        struct BuildBrushFaceMesh {
            Model::BrushFace::Mesh mesh;
            
            inline bool operator()(Model::BrushFace* face) {
                face->addToMesh(mesh);
                return true;
            }
        };
        
        BrushRenderer::Filter::~Filter() {}
        
        BrushRenderer::~BrushRenderer() {
            delete m_filter;
            m_filter = NULL;
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

        void BrushRenderer::render(RenderContext& context) {
            if (!m_valid)
                validate();
            
            if (tintFaces())
                m_faceRenderer.render(context, grayscale(), tintColor());
            else
                m_faceRenderer.render(context, grayscale());
            
            if (renderOccludedEdges()) {
                glDisable(GL_DEPTH_TEST);
                m_edgeRenderer.setColor(occludedEdgeColor());
                m_edgeRenderer.render(context);
                glEnable(GL_DEPTH_TEST);
            }
            
            glSetEdgeOffset(0.02f);
            m_edgeRenderer.setColor(edgeColor());
            m_edgeRenderer.render(context);
            glResetEdgeOffset();
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

        void BrushRenderer::validate() {
            BuildBrushFaceMesh buildFaces;
            each(Model::BrushFacesIterator::begin(m_brushes),
                 Model::BrushFacesIterator::end(m_brushes),
                 buildFaces,
                 *m_filter);
            
            BuildBrushEdges buildEdges(*m_filter);
            each(m_brushes.begin(),
                 m_brushes.end(),
                 buildEdges,
                 *m_filter);
            
            m_faceRenderer = FaceRenderer(buildFaces.mesh, faceColor());
            m_edgeRenderer = EdgeRenderer(buildEdges.vertices);
            
            m_valid = true;
        }
    }
}
