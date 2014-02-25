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
        MiniMapXYView::MiniMapXYView(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources, Renderer::MiniMapRenderer& renderer) :
        MiniMapBaseView(parent, document, renderResources, renderer),
        m_camera(new Renderer::OrthographicCamera()) {
            const BBox3& worldBounds = lock(document)->worldBounds();
            m_zRange = BBox1f(static_cast<float>(worldBounds.min.z()),
                                  static_cast<float>(worldBounds.max.z()));
            
            m_camera->setNearPlane(m_zRange.min.x());
            m_camera->setFarPlane(m_zRange.max.x());
            m_camera->setDirection(Vec3f::NegZ, Vec3f::PosY);
            m_camera->moveTo(Vec3f::Null);
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

        const Renderer::Camera& MiniMapXYView::camera() const {
            return *m_camera;
        }
        
        void MiniMapXYView::doComputeBounds(BBox3f& bounds) {
            bounds = BBox3f(document()->worldBounds());
            bounds.min[2] = m_zRange.min[0];
            bounds.max[2] = m_zRange.max[0];
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
