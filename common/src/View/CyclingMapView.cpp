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

#include "CyclingMapView.h"

#include "Hit.h"
#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "Model/ModelHitFilters.h"
#include "Model/PointFile.h"
#include "Renderer/Camera.h"
#include "View/CommandIds.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView2D.h"
#include "View/MapView3D.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        CyclingMapView::CyclingMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, Renderer::Vbo& vbo, GLContextManager& contextManager) :
        MapViewContainer(parent),
        m_logger(logger),
        m_document(document),
        m_currentMapView(NULL) {
            for (size_t i = 0; i < 4; ++i)
                m_mapViews[i] = NULL;
            createGui(toolBox, mapRenderer, vbo, contextManager);
            bindEvents();
        }

        void CyclingMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, Renderer::Vbo& vbo, GLContextManager& contextManager) {
            m_mapViews[0] = new MapView3D(this, m_logger, m_document, toolBox, mapRenderer, vbo, contextManager);
            m_mapViews[1] = new MapView2D(this, m_logger, m_document, toolBox, mapRenderer, vbo, contextManager, MapView2D::ViewPlane_XY);
            m_mapViews[2] = new MapView2D(this, m_logger, m_document, toolBox, mapRenderer, vbo, contextManager, MapView2D::ViewPlane_XZ);
            m_mapViews[3] = new MapView2D(this, m_logger, m_document, toolBox, mapRenderer, vbo, contextManager, MapView2D::ViewPlane_YZ);
            
            for (size_t i = 0; i < 4; ++i)
                m_mapViews[i]->Hide();
            
            switchToMapView(m_mapViews[0]);
        }

        void CyclingMapView::bindEvents() {
            Bind(wxEVT_IDLE, &CyclingMapView::OnIdleSetFocus, this);
            Bind(wxEVT_MENU, &CyclingMapView::OnCycleMapView, this, CommandIds::Actions::CycleMapViews);
        }
        
        void CyclingMapView::OnIdleSetFocus(wxIdleEvent& event) {
            // we use this method to ensure that the 3D view gets the focus after startup has settled down
            if (m_currentMapView != NULL) {
                if (!m_currentMapView->HasFocus()) {
                    m_currentMapView->SetFocus();
                } else {
                    Unbind(wxEVT_IDLE, &CyclingMapView::OnIdleSetFocus, this);
                    m_currentMapView->Refresh();
                }
            }
        }

        void CyclingMapView::OnCycleMapView(wxCommandEvent& event) {
            for (size_t i = 0; i < 4; ++i) {
                if (m_currentMapView == m_mapViews[i]) {
                    switchToMapView(m_mapViews[Math::succ(i, 4)]);
                    break;
                }
            }
        }

        void CyclingMapView::switchToMapView(MapViewBase* mapView) {
            if (m_currentMapView != NULL)
                m_currentMapView->Hide();
            m_currentMapView = mapView;
            m_currentMapView->Show();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_currentMapView, 1, wxEXPAND);
            SetSizer(sizer);
            Layout();
            m_currentMapView->SetFocus();
        }

        Vec3 CyclingMapView::doGetPasteObjectsDelta(const BBox3& bounds) const {
            MapDocumentSPtr document = lock(m_document);
            const Renderer::Camera* camera = m_currentMapView->camera();
            const Grid& grid = document->grid();
            
            const wxMouseState mouseState = wxGetMouseState();
            const wxPoint clientCoords = ScreenToClient(mouseState.GetPosition());
            
            if (HitTest(clientCoords) == wxHT_WINDOW_INSIDE) {
                const Ray3f pickRay = camera->pickRay(clientCoords.x, clientCoords.y);
                Hits hits = hitsByDistance();
                document->pick(Ray3(pickRay), hits);
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
        
        void CyclingMapView::doCenterCameraOnSelection() {
            m_currentMapView->centerCameraOnSelection();
        }
        
        void CyclingMapView::doMoveCameraToPosition(const Vec3& position) {
            m_currentMapView->moveCameraToPosition(position);
        }
        
        bool CyclingMapView::doCanMoveCameraToNextTracePoint() const {
            if (m_currentMapView != m_mapViews[0])
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->isPointFileLoaded())
                return false;
            
            Model::PointFile* pointFile = document->pointFile();
            return pointFile->hasNextPoint();
        }
        
        bool CyclingMapView::doCanMoveCameraToPreviousTracePoint() const {
            if (m_currentMapView != m_mapViews[0])
                return false;
            
            MapDocumentSPtr document = lock(m_document);
            if (!document->isPointFileLoaded())
                return false;
            
            Model::PointFile* pointFile = document->pointFile();
            return pointFile->hasPreviousPoint();
        }
        
        void CyclingMapView::doMoveCameraToNextTracePoint() {
            MapView3D* mapView3D = static_cast<MapView3D*>(m_currentMapView);
            mapView3D->moveCameraToNextTracePoint();
        }
        
        void CyclingMapView::doMoveCameraToPreviousTracePoint() {
            MapView3D* mapView3D = static_cast<MapView3D*>(m_currentMapView);
            mapView3D->moveCameraToPreviousTracePoint();
        }
    }
}
