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
#include "Renderer/Camera.h"
#include "Renderer/MiniMapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderResources.h"
#include "View/MapDocument.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        MiniMapXYView::MiniMapXYView(wxWindow* parent, View::MapDocumentWPtr document, BBox3f& bounds, Renderer::RenderResources& renderResources, Renderer::MiniMapRenderer& renderer) :
        MiniMapBaseView(parent, document, bounds, renderResources, renderer),
        m_camera(new Renderer::OrthographicCamera()) {
            m_camera->setNearPlane(-0xFFFF);
            m_camera->setFarPlane(0xFFFF);
            m_camera->setDirection(Vec3f::NegZ, Vec3f::PosY);
            m_camera->moveTo(Vec3f::Null);
            m_camera->setZoom(Vec2f(0.15f, 0.15f));
            updateBounds();
        }
        
        MiniMapXYView::~MiniMapXYView() {
            delete m_camera;
            m_camera = NULL;
        }

        void MiniMapXYView::setZPosition(const float zPosition) {
            const BBox3& worldBounds = document()->worldBounds();
            const float maxBounds = static_cast<float>(worldBounds.max.z() - worldBounds.min.z());

            const float diff = Math::clamp(zPosition) * maxBounds + worldBounds.min.z();
            moveCamera(Vec3f(0.0f, 0.0f, diff));
        }

        const Renderer::Camera& MiniMapXYView::camera() const {
            return *m_camera;
        }
        
        void MiniMapXYView::doUpdateBounds(BBox3f& bounds) {
            const Vec2f& zoom = m_camera->zoom();
            const float w = static_cast<float>(m_camera->viewport().width) / zoom.x();
            const float h = static_cast<float>(m_camera->viewport().height) / zoom.y();
            
            const Vec3f& center = m_camera->position();
            bounds.min[0] = center[0];
            bounds.min[1] = center[1];
            bounds.max[0] = center[0] + w;
            bounds.max[1] = center[1] + h;
        }

        void MiniMapXYView::doUpdateViewport(const Renderer::Camera::Viewport& viewport) {
            m_camera->setViewport(viewport);
        }
        
        void MiniMapXYView::doMoveCamera(const Vec3f& diff) {
            m_camera->moveBy(Vec3f(diff.x(), diff.y(), 0.0f));
        }
        
        void MiniMapXYView::doZoomCamera(const Vec3f& factors) {
            m_camera->zoom(factors);
        }
    }
}
