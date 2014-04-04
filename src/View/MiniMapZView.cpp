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

#include "MiniMapZView.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/MiniMapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/MapDocument.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        MiniMapZView::MiniMapZView(wxWindow* parent, GLContextHolder::Ptr sharedContext, View::MapDocumentWPtr document, Renderer::MiniMapRenderer& renderer, Renderer::Camera& camera) :
        MiniMapBaseView(parent, sharedContext, document, renderer, camera),
        m_camera(new Renderer::OrthographicCamera()) {
            const BBox3& worldBounds = lock(document)->worldBounds();
            m_camera->setNearPlane(0.0f);
            m_camera->setFarPlane(worldBounds.size().y());
            m_camera->setDirection(Vec3f::PosY, Vec3f::PosZ);
            m_camera->moveTo(Vec3f(0.0f, worldBounds.min.y(), 0.0f));
            m_camera->setZoom(Vec2f(0.15f, 0.15f));
        }

        MiniMapZView::~MiniMapZView() {
            delete m_camera;
            m_camera = NULL;
        }
        
        BBox1f MiniMapZView::zRange() const {
            const Vec2f& zoom = m_camera->zoom();
            const float h2 = static_cast<float>(m_camera->viewport().height) / zoom.y() / 2.0f;
            const float z = m_camera->position().z();
            return BBox1f(z - h2, z + h2);
        }

        void MiniMapZView::setXYRange(const BBox2f& xyRange) {
            m_xyRange = xyRange;
            const float x = m_xyRange.center().x();
            const float z = m_camera->position().z();
            const Vec3f& pos = m_camera->position();
            
            const float dx = x - pos.x();
            const float dz = z - pos.z();
            m_camera->moveBy(Vec3f(dx, 0.0f, dz));
            Refresh();
        }

        const Renderer::Camera& MiniMapZView::doGetViewCamera() const {
            return *m_camera;
        }
        
        void MiniMapZView::doComputeBounds(BBox3f& bounds) {
            const BBox3& worldBounds = document()->worldBounds();

            bounds.min[0] = m_xyRange.min.x();
            bounds.max[0] = m_xyRange.max.x();
            bounds.min[1] = m_xyRange.min.y();
            bounds.max[1] = m_xyRange.max.y();
            bounds.min[2] = static_cast<float>(worldBounds.min.z());
            bounds.max[2] = static_cast<float>(worldBounds.max.z());
        }

        void MiniMapZView::doUpdateViewport(int x, int y, int width, int height) {
            m_camera->setViewport(Renderer::Camera::Viewport(x, y, width, height));
        }
        
        void MiniMapZView::doPanView(const Vec3f& diff) {
            m_camera->moveBy(Vec3f(0.0f, 0.0f, diff.z()));
        }
        
        void MiniMapZView::doZoomView(const Vec3f& factors) {
            m_camera->zoom(factors);
        }

        void MiniMapZView::doShowDrag3DCameraCursor() {
            SetCursor(wxCursor(wxCURSOR_SIZENS));
        }

        void MiniMapZView::doDrag3DCamera(const Vec3f& delta, Renderer::Camera& camera) {
            camera.moveBy(Vec3f(0.0f, 0.0f, delta.z()));
        }

        float MiniMapZView::doPick3DCamera(const Ray3f& pickRay, const Renderer::Camera& camera) const {
            const float zoom = m_camera->zoom().x();
            const float w2 = 2.0f / zoom;
            const Vec3f& cameraPosition = camera.position();
            if (Math::between(pickRay.origin.z(), cameraPosition.z() - w2, cameraPosition.z() + w2))
                return 0.0f;
            return Math::nan<float>();
        }

        void MiniMapZView::doRender3DCamera(Renderer::RenderContext& renderContext, Renderer::Vbo& vbo, const Renderer::Camera& camera) {
            const BBox3& worldBounds = document()->worldBounds();
            const Vec3f& cameraPosition = camera.position();
            
            const Vec3f min(worldBounds.min.x(), 0.0f, cameraPosition.z());
            const Vec3f max(worldBounds.max.x(), 0.0f, cameraPosition.z());
            
            typedef Renderer::VertexSpecs::P3::Vertex Vertex;
            Vertex::List vertices(2);
            vertices[0] = Vertex(min);
            vertices[1] = Vertex(max);
            
            Renderer::VertexArray array = Renderer::VertexArray::ref(GL_LINES, vertices);
            
            Renderer::SetVboState setVboState(vbo);
            setVboState.mapped();
            array.prepare(vbo);
            setVboState.active();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
            shader.set("Color", prefs.get(Preferences::CameraFrustumColor));
            
            glLineWidth(2.0f);
            array.render();
            glLineWidth(1.0f);
        }
    }
}
