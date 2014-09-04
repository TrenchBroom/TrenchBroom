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
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushFacesIterator.h"
#include "Model/BrushEdge.h"
#include "Model/BrushVertex.h"
#include "Model/ModelFilter.h"
#include "Model/ModelUtils.h"
#include "Renderer/RenderConfig.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        BrushRenderer::FilterWrapper::FilterWrapper(const Filter& filter, const bool showHiddenBrushes) :
        m_filter(filter),
        m_showHiddenBrushes(showHiddenBrushes) {}
        
        bool BrushRenderer::FilterWrapper::operator()(const Model::Brush* brush) const {
            return m_showHiddenBrushes || m_filter(brush);
        }
        
        bool BrushRenderer::FilterWrapper::operator()(const Model::BrushFace* face) const {
            return m_showHiddenBrushes || m_filter(face);
        }
        
        bool BrushRenderer::FilterWrapper::operator()(const Model::BrushEdge* edge) const {
            return m_showHiddenBrushes || m_filter(edge);
        }

        BrushRenderer::BuildBrushEdges::BuildBrushEdges(BrushRenderer::Filter& i_filter) :
        filter(i_filter) {}
        
        void BrushRenderer::BuildBrushEdges::operator()(Model::Brush* brush) {
            const Model::BrushEdgeList edges = brush->edges();
            Model::BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const Model::BrushEdge* edge = *it;
                if (filter(edge)) {
                    vertices.push_back(VertexSpecs::P3::Vertex(edge->start->position));
                    vertices.push_back(VertexSpecs::P3::Vertex(edge->end->position));
                }
            }
        }
        
        bool BrushRenderer::BuildBrushFaceMesh::operator()(Model::BrushFace* face) {
            const Model::Brush* brush = face->parent();
            if (brush->transparent())
                face->addToMesh(transparentMesh);
            else
                face->addToMesh(opaqueMesh);
            return true;
        }
        
        BrushRenderer::Filter::~Filter() {}
        
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
            
            if (renderContext.renderConfig().showFaces())
                renderFaces(renderContext);
            
            if (renderContext.renderConfig().showEdges())
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

        void BrushRenderer::validate() {
            FilterWrapper wrapper(*m_filter, m_showHiddenBrushes);
            
            BuildBrushFaceMesh buildFaces;
            each(Model::BrushFacesIterator::begin(m_brushes),
                 Model::BrushFacesIterator::end(m_brushes),
                 buildFaces,
                 wrapper);
            
            BuildBrushEdges buildEdges(*m_filter);
            each(m_brushes.begin(),
                 m_brushes.end(),
                 buildEdges,
                 wrapper);
            
            m_opaqueFaceRenderer = FaceRenderer(buildFaces.opaqueMesh, faceColor());
            m_transparentFaceRenderer = FaceRenderer(buildFaces.transparentMesh, faceColor());
            m_edgeRenderer = EdgeRenderer(VertexArray::swap(GL_LINES, buildEdges.vertices));
            
            m_valid = true;
        }
    }
}
