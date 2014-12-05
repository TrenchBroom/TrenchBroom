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

#include "ThreePaneMapView.h"

#include "Hit.h"
#include "Model/Brush.h"
#include "Model/HitAdapter.h"
#include "Model/ModelHitFilters.h"
#include "Model/PointFile.h"
#include "Renderer/Camera.h"
#include "View/CommandIds.h"
#include "View/CyclingMapView.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView2D.h"
#include "View/MapView3D.h"
#include "View/SplitterWindow2.h"

#include <wx/persist.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        ThreePaneMapView::ThreePaneMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, Renderer::Vbo& vbo, GLContextManager& contextManager) :
        MapViewContainer(parent),
        m_logger(logger),
        m_document(document),
        m_mapView3D(NULL),
        m_mapViewXY(NULL),
        m_mapViewZZ(NULL) {
            createGui(toolBox, mapRenderer, vbo, contextManager);
            bindEvents();
        }
        
        void ThreePaneMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, Renderer::Vbo& vbo, GLContextManager& contextManager) {

            SplitterWindow2* hSplitter = new SplitterWindow2(this);
            hSplitter->setSashGravity(0.5f);
            hSplitter->SetName("3PaneMapViewHSplitter");
            
            SplitterWindow2* vSplitter = new SplitterWindow2(hSplitter);
            vSplitter->setSashGravity(0.5f);
            vSplitter->SetName("3PaneMapViewVSplitter");

            m_mapView3D = new MapView3D(hSplitter, m_logger, m_document, toolBox, mapRenderer, vbo, contextManager);
            m_mapViewXY = new MapView2D(vSplitter, m_logger, m_document, toolBox, mapRenderer, vbo, contextManager, MapView2D::ViewPlane_XY);
            m_mapViewZZ = new CyclingMapView(vSplitter, m_logger, m_document, toolBox, mapRenderer, vbo, contextManager, CyclingMapView::View_ZZ);
            
            vSplitter->splitHorizontally(m_mapViewXY, m_mapViewZZ, wxSize(100, 100), wxSize(100, 100));
            hSplitter->splitVertically(m_mapView3D, vSplitter, wxSize(100, 100), wxSize(100, 100));
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(hSplitter, 1, wxEXPAND);
            
            SetSizer(sizer);

            wxPersistenceManager::Get().RegisterAndRestore(hSplitter);
            wxPersistenceManager::Get().RegisterAndRestore(vSplitter);
        }
        
        void ThreePaneMapView::bindEvents() {
            Bind(wxEVT_IDLE, &ThreePaneMapView::OnIdleSetFocus, this);
        }
        
        void ThreePaneMapView::OnIdleSetFocus(wxIdleEvent& event) {
            // we use this method to ensure that the 3D view gets the focus after startup has settled down
            if (!m_mapView3D->HasFocus()) {
                m_mapView3D->SetFocus();
            } else {
                Unbind(wxEVT_IDLE, &ThreePaneMapView::OnIdleSetFocus, this);
                m_mapView3D->Refresh();
            }
        }
        
        MapView* ThreePaneMapView::currentMapView() const {
            if (m_mapViewXY->HasFocus())
                return m_mapViewXY;
            if (m_mapViewZZ->HasFocus())
                return m_mapViewZZ;
            return m_mapView3D;
        }

        Vec3 ThreePaneMapView::doGetPasteObjectsDelta(const BBox3& bounds) const {
            return currentMapView()->pasteObjectsDelta(bounds);
        }
        
        void ThreePaneMapView::doCenterCameraOnSelection() {
            currentMapView()->centerCameraOnSelection();
        }
        
        void ThreePaneMapView::doMoveCameraToPosition(const Vec3& position) {
            currentMapView()->moveCameraToPosition(position);
        }
        
        void ThreePaneMapView::doMoveCameraToCurrentTracePoint() {
            m_mapView3D->moveCameraToCurrentTracePoint();
            m_mapViewXY->moveCameraToCurrentTracePoint();
            m_mapViewZZ->moveCameraToCurrentTracePoint();
        }
    }
}
