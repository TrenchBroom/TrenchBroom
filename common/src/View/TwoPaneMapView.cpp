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

#include "TwoPaneMapView.h"

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
#include "View/MapView3D.h"
#include "View/SplitterWindow2.h"

#include <wx/persist.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        TwoPaneMapView::TwoPaneMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, Renderer::Vbo& vbo, GLContextManager& contextManager) :
        MapViewContainer(parent),
        m_logger(logger),
        m_document(document),
        m_mapView3D(NULL),
        m_mapView2D(NULL) {
            createGui(toolBox, mapRenderer, vbo, contextManager);
        }
        
        void TwoPaneMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, Renderer::Vbo& vbo, GLContextManager& contextManager) {

            SplitterWindow2* splitter = new SplitterWindow2(this);
            splitter->setSashGravity(0.5f);
            splitter->SetName("2PaneMapViewHSplitter");

            m_mapView3D = new MapView3D(splitter, m_logger, m_document, toolBox, mapRenderer, vbo, contextManager);
            m_mapView2D = new CyclingMapView(splitter, m_logger, m_document, toolBox, mapRenderer, vbo, contextManager, CyclingMapView::View_2D);
            
            splitter->splitVertically(m_mapView3D, m_mapView2D, wxSize(100, 100), wxSize(100, 100));
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(splitter, 1, wxEXPAND);
            
            SetSizer(sizer);

            wxPersistenceManager::Get().RegisterAndRestore(splitter);
        }
        
        MapView* TwoPaneMapView::currentMapView() const {
            if (m_mapView2D->HasFocus())
                return m_mapView2D;
            return m_mapView3D;
        }

        Vec3 TwoPaneMapView::doGetPasteObjectsDelta(const BBox3& bounds) const {
            return currentMapView()->pasteObjectsDelta(bounds);
        }
        
        void TwoPaneMapView::doCenterCameraOnSelection() {
            m_mapView3D->centerCameraOnSelection();
            m_mapView2D->centerCameraOnSelection();
        }
        
        void TwoPaneMapView::doMoveCameraToPosition(const Vec3& position) {
            currentMapView()->moveCameraToPosition(position);
        }
        
        void TwoPaneMapView::doMoveCameraToCurrentTracePoint() {
            m_mapView3D->moveCameraToCurrentTracePoint();
            m_mapView2D->moveCameraToCurrentTracePoint();
        }
    }
}
