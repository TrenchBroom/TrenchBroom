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
#include "Renderer/Camera.h"
#include "Renderer/MiniMapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderResources.h"
#include "View/MapDocument.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        MiniMapZView::MiniMapZView(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources, Renderer::MiniMapRenderer& renderer, Renderer::Camera& camera) :
        MiniMapBaseView(parent, document, renderResources, renderer, camera),
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
            const Vec2f xy = m_xyRange.center();
            const float z = m_camera->position().z();
            m_camera->moveTo(Vec3f(xy, z));
            Refresh();
        }

        const Renderer::Camera& MiniMapZView::camera() const {
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

        void MiniMapZView::doUpdateViewport(const Renderer::Camera::Viewport& viewport) {
            m_camera->setViewport(viewport);
        }
        
        void MiniMapZView::doMoveCamera(const Vec3f& diff) {
            m_camera->moveBy(Vec3f(0.0f, 0.0f, diff.z()));
        }
        
        void MiniMapZView::doZoomCamera(const Vec3f& factors) {
            m_camera->zoom(factors);
        }
    }
}
