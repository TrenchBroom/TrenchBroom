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

#include "TexturingView.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Renderer/Camera.h"
#include "Renderer/EdgeRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderResources.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        TexturingViewState::TexturingViewState() :
        m_face(NULL) {}
        
        bool TexturingViewState::valid() const {
            return m_face != NULL;
        }
        
        Model::BrushFace* TexturingViewState::face() const {
            return m_face;
        }
        
        const Vec3& TexturingViewState::origin() const {
            assert(valid());
            return m_origin;
        }
        
        const Vec3& TexturingViewState::xAxis() const {
            assert(valid());
            return m_xAxis;
        }
        
        const Vec3& TexturingViewState::yAxis() const {
            assert(valid());
            return m_yAxis;
        }
        
        const Vec3& TexturingViewState::zAxis() const {
            assert(valid());
            return m_zAxis;
        }
        
        BBox3 TexturingViewState::computeBounds() const {
            assert(valid());
            
            BBox3 result;
            const Model::BrushVertexList& vertices = m_face->vertices();
            result.min = result.max = transformToFace(vertices[0]->position);
            for (size_t i = 1; i < vertices.size(); ++i)
                result.mergeWith(transformToFace(vertices[i]->position));
            return result;
        }
        
        Vec3 TexturingViewState::transformToFace(const Vec3& point) const {
            assert(valid());
            return m_toFaceTransform * point;
        }
        
        Vec3 TexturingViewState::transformToWorld(const Vec3& point) const {
            assert(valid());
            return m_toWorldTransform * point;
        }

        void TexturingViewState::setFace(Model::BrushFace* face) {
            m_face = face;
            validate();
        }
        
        void TexturingViewState::validate() {
            if (m_face != NULL) {
                m_origin = m_face->center();
                m_zAxis = m_face->boundary().normal;
                
                if (Math::lt(Math::abs(Vec3::PosZ.dot(m_zAxis)), 1.0))
                    m_xAxis = crossed(Vec3::PosZ, m_zAxis).normalized();
                else
                    m_xAxis = Vec3::PosX;
                m_yAxis = crossed(m_zAxis, m_xAxis).normalized();
                
                m_toWorldTransform = coordinateSystemMatrix(m_xAxis, m_yAxis, m_zAxis, m_origin);
                bool invertible = true;
                m_toFaceTransform = invertedMatrix(m_toWorldTransform, invertible);
                assert(invertible);
            }
        }
        
        TexturingView::TexturingView(wxWindow* parent, MapDocumentWPtr document, Renderer::RenderResources& renderResources) :
        RenderView(parent, renderResources.glAttribs(), renderResources.sharedContext()),
        m_document(document),
        m_renderResources(renderResources) {
            bindObservers();
        }
        
        TexturingView::~TexturingView() {
            unbindObservers();
        }

        void TexturingView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->faceDidChangeNotifier.addObserver(this, &TexturingView::faceDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &TexturingView::selectionDidChange);
        }
        
        void TexturingView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->faceDidChangeNotifier.removeObserver(this, &TexturingView::faceDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &TexturingView::selectionDidChange);
            }
        }
        
        void TexturingView::faceDidChange(Model::BrushFace* face) {
            Refresh();
        }
        
        void TexturingView::selectionDidChange(const Model::SelectionResult& result) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushFaceList& faces = document->selectedFaces();
            if (faces.empty())
                m_state.setFace(NULL);
            else
                m_state.setFace(faces.back());
            Refresh();
        }

        void TexturingView::doUpdateViewport(int x, int y, int width, int height) {
            m_camera.setViewport(Renderer::Camera::Viewport(x, y, width, height));
        }
        
        void TexturingView::doRender() {
            if (m_state.valid()) {
                setupCamera();

                MapDocumentSPtr document = lock(m_document);
                document->commitPendingRenderStateChanges();

                const View::Grid& grid = document->grid();
                Renderer::RenderContext renderContext(m_camera, m_renderResources.shaderManager(), grid.visible(), grid.actualSize());
                
                setupGL(renderContext);
                renderTexture(renderContext);
                renderFace(renderContext);
            }
        }
        
        void TexturingView::setupCamera() {
            assert(m_state.valid());
            
            m_camera.setZoom(computeZoomFactor());
            m_camera.setNearPlane(-1.0);
            m_camera.setFarPlane(1.0);
            m_camera.setDirection(-m_state.zAxis(), m_state.yAxis());
            m_camera.moveTo(Vec3f(m_state.origin()));
        }
        
        void TexturingView::setupGL(Renderer::RenderContext& renderContext) {
            const Renderer::Camera::Viewport& viewport = renderContext.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
            glDisable(GL_DEPTH_TEST);
        }
        
        void TexturingView::renderTexture(Renderer::RenderContext& renderContext) {
        }
        
        void TexturingView::renderFace(Renderer::RenderContext& renderContext) {
            assert(m_state.valid());
            
            const Model::BrushFace* face = m_state.face();
            const Model::BrushVertexList& faceVertices = face->vertices();
            const size_t count = faceVertices.size();
            
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List edgeVertices(count);
            
            for (size_t i = 0; i < count; ++i)
                edgeVertices[i] = Vertex(faceVertices[i]->position);
            
            Renderer::EdgeRenderer edgeRenderer(Renderer::VertexArray::swap(GL_LINE_LOOP, edgeVertices));
            edgeRenderer.setUseColor(true);
            edgeRenderer.setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            edgeRenderer.render(renderContext);
        }

        float TexturingView::computeZoomFactor() const {
            m_state.transformToFace(Vec3::Null);
            const BBox3 bounds = m_state.computeBounds();
            const Vec3f size(bounds.size());
            const float w = static_cast<float>(m_camera.viewport().width - 20);
            const float h = static_cast<float>(m_camera.viewport().height - 20);
            
            float zoom = 1.0f;
            if (size.x() > w)
                zoom = Math::min(zoom, w / size.x());
            if (size.y() > h)
                zoom = Math::min(zoom, h / size.y());
            return zoom;
        }
    }
}
