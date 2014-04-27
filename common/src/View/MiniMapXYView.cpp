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

#include "MiniMapXYView.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/MiniMapRenderer.h"
#include "Renderer/RenderContext.h"
#include "View/MapDocument.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        MiniMapXYView::MiniMapXYView(wxWindow* parent, GLContextHolder::Ptr sharedContext, View::MapDocumentWPtr document, Renderer::MiniMapRenderer& renderer, Renderer::Camera& camera) :
        MiniMapBaseView(parent, sharedContext, document, renderer, camera),
        m_camera(new Renderer::OrthographicCamera()) {
            const BBox3& worldBounds = lock(document)->worldBounds();
            m_zRange = BBox1f(static_cast<float>(worldBounds.min.z()),
                              static_cast<float>(worldBounds.max.z()));
            
            m_camera->setNearPlane(0.0f);
            m_camera->setFarPlane(static_cast<float>(worldBounds.size().z()));
            m_camera->setDirection(Vec3f::NegZ, Vec3f::PosY);
            m_camera->moveTo(Vec3f(0.0f, 0.0f, worldBounds.max.z()));
            m_camera->setZoom(Vec2f(0.15f, 0.15f));
        }
        
        MiniMapXYView::~MiniMapXYView() {
            delete m_camera;
            m_camera = NULL;
        }
        
        BBox2f MiniMapXYView::xyRange() const {
            const Vec2f& zoom = m_camera->zoom();
            const Vec2f position(m_camera->position());
            const Vec2f halfSize(static_cast<float>(m_camera->viewport().width) / zoom.x() / 2.0f,
                                 static_cast<float>(m_camera->viewport().height) / zoom.y() / 2.0f);
            
            return BBox2f(position - halfSize, position + halfSize);
        }
        
        void MiniMapXYView::setZRange(const BBox1f& zRange) {
            m_zRange = zRange;
            Refresh();
        }
        
        const Renderer::Camera& MiniMapXYView::doGetViewCamera() const {
            return *m_camera;
        }
        
        void MiniMapXYView::doComputeBounds(BBox3f& bounds) {
            bounds = BBox3f(document()->worldBounds());
            bounds.min[2] = m_zRange.min[0];
            bounds.max[2] = m_zRange.max[0];
        }
        
        void MiniMapXYView::doUpdateViewport(int x, int y, int width, int height) {
            m_camera->setViewport(Renderer::Camera::Viewport(x, y, width, height));
        }
        
        void MiniMapXYView::doPanView(const Vec3f& diff) {
            m_camera->moveBy(Vec3f(diff.x(), diff.y(), 0.0f));
        }
        
        void MiniMapXYView::doZoomView(const Vec3f& factors) {
            m_camera->zoom(factors);
        }

        void MiniMapXYView::doShowDrag3DCameraCursor() {
            SetCursor(wxCursor(wxCURSOR_SIZING));
        }

        void MiniMapXYView::doDrag3DCamera(const Vec3f& delta, Renderer::Camera& camera) {
            camera.moveBy(Vec3f(delta.xy(), 0.0f));
        }

        float MiniMapXYView::doPick3DCamera(const Ray3f& pickRay, const Renderer::Camera& camera) const {
            const float zoom = m_camera->zoom().x();
            return camera.pickFrustum(16.0f / zoom, pickRay);
        }

        void MiniMapXYView::doRender3DCamera(Renderer::RenderContext& renderContext, Renderer::Vbo& vbo, const Renderer::Camera& camera) {
            const float zoom = m_camera->zoom().x();
            PreferenceManager& prefs = PreferenceManager::instance();
            camera.renderFrustum(renderContext, vbo, 16.0f / zoom, prefs.get(Preferences::CameraFrustumColor));
        }
    }
}
