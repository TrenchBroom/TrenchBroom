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
        TexturingView::TexturingView(wxWindow* parent, MapDocumentWPtr document, Renderer::RenderResources& renderResources) :
        RenderView(parent, renderResources.glAttribs(), renderResources.sharedContext()),
        m_document(document),
        m_renderResources(renderResources),
        m_face(NULL) {}

        void TexturingView::doUpdateViewport(int x, int y, int width, int height) {
            m_camera.setViewport(Renderer::Camera::Viewport(-width/2, -height/2, width, height));
        }

        void TexturingView::doRender() {
            if (m_face != NULL) {
                MapDocumentSPtr document = lock(m_document);
                document->commitPendingRenderStateChanges();
                
                const View::Grid& grid = document->grid();
                Renderer::RenderContext renderContext(m_camera, m_renderResources.shaderManager(), grid.visible(), grid.actualSize());
                
                const Mat4x4 transform = faceCoordinateSystem();
                
                setupGL(renderContext);
                setupCamera(renderContext, transform);
                
                Renderer::SetVboState activateVbo(m_vbo);
                activateVbo.active();
                
                renderTexture(renderContext, transform);
                renderFace(renderContext, transform);
            }
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

        void TexturingView::setupCamera(Renderer::RenderContext& renderContext, const Mat4x4& transform) {
            assert(m_face != NULL);

            m_camera.setNearPlane(-1.0);
            m_camera.setFarPlane(1.0);
            m_camera.setDirection(Vec3f::NegZ, Vec3f::PosY);
            
            const Vec3 position = transform * Vec3::Null;
            m_camera.moveTo(Vec3f(position));
        }

        void TexturingView::renderTexture(Renderer::RenderContext& renderContext, const Mat4x4& transform) {
            bool invertible = false;
            const Mat4x4 inverted = invertedMatrix(transform, invertible);
            assert(invertible);
            
            const Renderer::Camera::Viewport& viewport = renderContext.camera().viewport();
            const Vec3 topLeft(viewport.x, viewport.y);
            const Vec3 topRight(viewport.x + viewport.width, viewport.y);
            const Vec3 bottomRight(viewport.x + viewport.width, viewport.y + viewport.height);
            const Vec3 bottomLeft(viewport.x, viewport.y + viewport.height);
            
            typedef Renderer::VertexSpecs::P3T2::Vertex Vertex;
            Vertex::List vertices(4);
            
            vertices[0] = Vertex(Vec3f(topLeft),     m_face->textureCoords(inverted * topLeft));
            vertices[1] = Vertex(Vec3f(topRight),    m_face->textureCoords(inverted * topRight));
            vertices[2] = Vertex(Vec3f(bottomRight), m_face->textureCoords(inverted * bottomRight));
            vertices[3] = Vertex(Vec3f(bottomLeft),  m_face->textureCoords(inverted * bottomLeft));
            
            Renderer::VertexArray vertexArray = Renderer::VertexArray::ref(GL_QUADS, vertices);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            vertexArray.prepare(m_vbo);
            setVboState.active();
            vertexArray.render();
        }

        void TexturingView::renderFace(Renderer::RenderContext& renderContext, const Mat4x4& transform) {
        }

        Mat4x4 TexturingView::faceCoordinateSystem() const {
            assert(m_face != NULL);
            
            const Vec3& origin = m_face->center();
            const Vec3& normal = m_face->boundary().normal;
            if (Math::lt(Math::abs(Vec3::PosZ.dot(normal)), 1.0)) {
                const Vec3 x = crossed(Vec3::PosZ, normal).normalized();
                const Vec3 y = crossed(normal, x).normalized();
                return coordinateSystemMatrix(x, y, normal, origin);
            } else if (normal.z() > 0.0) {
                return coordinateSystemMatrix(Vec3::PosX, Vec3::PosY, Vec3::PosZ, origin);
            } else {
                return coordinateSystemMatrix(Vec3::PosX, Vec3::NegY, Vec3::NegZ, origin);
            }
        }

        Vec3::List TexturingView::transformVertices(const Mat4x4& transform) const {
            assert(m_face != NULL);
            
            const Model::BrushVertexList& vertices = m_face->vertices();
            const size_t vertexCount = vertices.size();
            Vec3::List result(vertexCount);
            
            for (size_t i = 0; i < vertexCount; ++i) {
                const Model::BrushVertex* vertex = vertices[i];
                result[i] = transform * vertex->position;
            }
            
            return result;
        }
    }
}
