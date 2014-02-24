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
        MiniMapZView::MiniMapZView(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources, Renderer::MiniMapRenderer& renderer) :
        MiniMapBaseView(parent, document, renderResources, renderer),
        m_camera(new Renderer::OrthographicCamera()) {
            m_camera->setNearPlane(-0xFFFF);
            m_camera->setFarPlane(0xFFFF);
            m_camera->setDirection(Vec3f::PosY, Vec3f::PosZ);
            m_camera->moveTo(Vec3f::Null);
            m_camera->setZoom(Vec2f(0.15f, 0.15f));
        }

        MiniMapZView::~MiniMapZView() {
            delete m_camera;
            m_camera = NULL;
        }
        
        void MiniMapZView::setXYPosition(const Vec2f& xyPosition) {
            const BBox3& worldBounds = document()->worldBounds();
            const Vec2f maxBounds(static_cast<float>(worldBounds.max.x() - worldBounds.min.x()),
                                  static_cast<float>(worldBounds.max.y() - worldBounds.min.y()));
            
            Vec3 cameraPosition = m_camera->position();
            cameraPosition[0] = Math::clamp(maxBounds.x()) * maxBounds.x() + worldBounds.min.x();
            cameraPosition[1] = Math::clamp(maxBounds.y()) * maxBounds.y() + worldBounds.min.y();
            m_camera->moveTo(cameraPosition);
            
            Refresh();
        }
        
        const Renderer::Camera& MiniMapZView::camera() const {
            return *m_camera;
        }
        
        void MiniMapZView::updateViewport(const Renderer::Camera::Viewport& viewport) {
            m_camera->setViewport(viewport);
        }
        
        void MiniMapZView::moveCamera(const Vec3f& diff) {
            m_camera->moveBy(Vec3f(0.0f, 0.0f, diff.z()));
        }
        
        void MiniMapZView::zoomCamera(const Vec3f& factors) {
            m_camera->zoom(factors);
        }
    }
}
