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
        ThreePaneMapView::ThreePaneMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) :
        MultiMapView(parent),
        m_logger(logger),
        m_document(document),
        m_hSplitter(NULL),
        m_vSplitter(NULL),
        m_mapView3D(NULL),
        m_mapViewXY(NULL),
        m_mapViewZZ(NULL) {
            createGui(toolBox, mapRenderer, contextManager);
        }
        
        void ThreePaneMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) {

            m_hSplitter = new SplitterWindow2(this);
            m_hSplitter->setSashGravity(0.5f);
            m_hSplitter->SetName("3PaneMapViewHSplitter");
            
            m_vSplitter = new SplitterWindow2(m_hSplitter);
            m_vSplitter->setSashGravity(0.5f);
            m_vSplitter->SetName("3PaneMapViewVSplitter");

            m_mapView3D = new MapView3D(m_hSplitter, m_logger, m_document, toolBox, mapRenderer, contextManager);
            m_mapViewXY = new MapView2D(m_vSplitter, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XY);
            m_mapViewZZ = new CyclingMapView(m_vSplitter, m_logger, m_document, toolBox, mapRenderer, contextManager, CyclingMapView::View_ZZ);
            
            m_mapView3D->linkCamera(m_linkHelper);
            m_mapViewXY->linkCamera(m_linkHelper);
            m_mapViewZZ->linkCamera(m_linkHelper);
            
            addMapView(m_mapView3D);
            addMapView(m_mapViewXY);
            addMapView(m_mapViewZZ);
            
            m_vSplitter->splitHorizontally(m_mapViewXY, m_mapViewZZ, wxSize(100, 100), wxSize(100, 100));
            m_hSplitter->splitVertically(m_mapView3D, m_vSplitter, wxSize(100, 100), wxSize(100, 100));
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_hSplitter, 1, wxEXPAND);
            
            SetSizer(sizer);

            wxPersistenceManager::Get().RegisterAndRestore(m_hSplitter);
            wxPersistenceManager::Get().RegisterAndRestore(m_vSplitter);
        }

        void ThreePaneMapView::doMaximizeView(MapView* view) {
            assert(view == m_mapView3D || view == m_mapViewXY || view == m_mapViewZZ);
            if (view == m_mapView3D) {
                m_hSplitter->maximize(m_mapView3D);
            } else if (view == m_mapViewXY) {
                m_vSplitter->maximize(m_mapViewXY);
                m_hSplitter->maximize(m_vSplitter);
            } else if (view == m_mapViewZZ) {
                m_vSplitter->maximize(m_mapViewZZ);
                m_hSplitter->maximize(m_vSplitter);
            }
        }
        
        void ThreePaneMapView::doRestoreViews() {
            m_hSplitter->restore();
            m_vSplitter->restore();
        }
    }
}
