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

#include "SwitchableMapView.h"

#include "Hit.h"
#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "Model/ModelHitFilters.h"
#include "Renderer/Camera.h"
#include "Renderer/MapRenderer.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView3D.h"
#include "View/MapViewBar.h"
#include "View/MapViewToolBox.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        SwitchableMapView::SwitchableMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document) :
        wxPanel(parent),
        m_logger(logger),
        m_document(document),
        m_toolBox(NULL),
        m_mapRenderer(NULL),
        m_mapViewBar(NULL),
        m_mapView(NULL) {
            createGui();
            bindEvents();
        }
        
        SwitchableMapView::~SwitchableMapView() {
            // this might lead to crashes because my children, including the map views, will be
            // deleted after the map renderer is deleted
            // possible solution: force deletion of all children here?
            delete m_mapRenderer;
            m_mapRenderer = NULL;
            
            delete m_toolBox;
            m_toolBox = NULL;
        }

        Vec3 SwitchableMapView::pasteObjectsDelta(const BBox3& bounds) const {
            MapDocumentSPtr document = lock(m_document);
            const Renderer::Camera* camera = m_mapView->camera();
            const Grid& grid = document->grid();

            const wxMouseState mouseState = wxGetMouseState();
            const wxPoint clientCoords = ScreenToClient(mouseState.GetPosition());
            
            if (HitTest(clientCoords) == wxHT_WINDOW_INSIDE) {
                const Ray3f pickRay = camera->pickRay(clientCoords.x, clientCoords.y);
                const Hits& hits = document->pick(Ray3(pickRay));
                const Hit& hit = Model::firstHit(hits, Model::Brush::BrushHit, document->editorContext(), true);
                if (hit.isMatch()) {
                    const Model::BrushFace* face = Model::hitToFace(hit);
                    const Vec3 snappedHitPoint = grid.snap(hit.hitPoint());
                    return grid.moveDeltaForBounds(face, bounds, document->worldBounds(), pickRay, snappedHitPoint);
                } else {
                    const Vec3 snappedCenter = grid.snap(bounds.center());
                    const Vec3 snappedDefaultPoint = grid.snap(camera->defaultPoint(pickRay));
                    return snappedDefaultPoint - snappedCenter;
                }
            } else {
                const Vec3 snappedCenter = grid.snap(bounds.center());
                const Vec3 snappedDefaultPoint = grid.snap(camera->defaultPoint());
                return snappedDefaultPoint - snappedCenter;
            }
        }
        
        void SwitchableMapView::centerCameraOnSelection() {
            m_mapView->centerCameraOnSelection();
        }
        
        void SwitchableMapView::moveCameraToPosition(const Vec3& position) {
            m_mapView->moveCameraToPosition(position);
        }
        
        void SwitchableMapView::animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration) {
            m_mapView->animateCamera(position, direction, up, duration);
        }

        void SwitchableMapView::createGui() {
            m_mapRenderer = new Renderer::MapRenderer(m_document);
            m_mapViewBar = new MapViewBar(this, m_document);
            
            m_toolBox = new MapViewToolBox(m_document, m_mapViewBar->toolBook());
            m_mapView = new MapView3D(this, m_logger, m_document, *m_toolBox, *m_mapRenderer);
            
            // this must be updated appropriately when the map view is switched
            m_toolBox->setCamera(m_mapView->camera());
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_mapViewBar, 0, wxEXPAND);
            sizer->Add(m_mapView, 1, wxEXPAND);
            SetSizer(sizer);
        }

        void SwitchableMapView::bindEvents() {
            Bind(wxEVT_IDLE, &SwitchableMapView::OnIdleSetFocus, this);
        }
        
        void SwitchableMapView::OnIdleSetFocus(wxIdleEvent& event) {
            // we use this method to ensure that the 3D view gets the focus after startup has settled down
            if (m_mapView != NULL) {
                if (!m_mapView->HasFocus()) {
                    m_mapView->SetFocus();
                } else {
                    Unbind(wxEVT_IDLE, &SwitchableMapView::OnIdleSetFocus, this);
                    m_mapView->Refresh();
                }
            }
        }
    }
}
