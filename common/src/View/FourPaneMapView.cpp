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

#include "FourPaneMapView.h"

#include "Renderer/Camera.h"
#include "View/CommandIds.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapView2D.h"
#include "View/MapView3D.h"
#include "View/SplitterWindow4.h"

#include <wx/persist.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        FourPaneMapView::FourPaneMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) :
        MultiMapView(parent),
        m_logger(logger),
        m_document(document),
        m_splitter(NULL),
        m_mapView3D(NULL),
        m_mapViewXY(NULL),
        m_mapViewXZ(NULL),
        m_mapViewYZ(NULL) {
            createGui(toolBox, mapRenderer, contextManager);
        }
        
        void FourPaneMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) {
            
            m_splitter = new SplitterWindow4(this);
            m_splitter->SetName("4PaneMapViewSplitter");
            
            m_mapView3D = new MapView3D(m_splitter, m_logger, m_document, toolBox, mapRenderer, contextManager);
            m_mapViewXY = new MapView2D(m_splitter, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XY);
            m_mapViewXZ = new MapView2D(m_splitter, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_XZ);
            m_mapViewYZ = new MapView2D(m_splitter, m_logger, m_document, toolBox, mapRenderer, contextManager, MapView2D::ViewPlane_YZ);
            
            m_mapView3D->linkCamera(m_linkHelper);
            m_mapViewXY->linkCamera(m_linkHelper);
            m_mapViewXZ->linkCamera(m_linkHelper);
            m_mapViewYZ->linkCamera(m_linkHelper);
            
            addMapView(m_mapView3D);
            addMapView(m_mapViewXY);
            addMapView(m_mapViewXZ);
            addMapView(m_mapViewYZ);
            
            m_splitter->split(m_mapView3D, m_mapViewXY, m_mapViewXZ, m_mapViewYZ);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_splitter, 1, wxEXPAND);
            
            SetSizer(sizer);

            wxPersistenceManager::Get().RegisterAndRestore(m_splitter);
        }

        void FourPaneMapView::doMaximizeView(MapView* view) {
            assert(view == m_mapView3D || view == m_mapViewXY || view == m_mapViewXZ || view == m_mapViewYZ);
            if (view == m_mapView3D)
                m_splitter->maximize(m_mapView3D);
            else if (view == m_mapViewXY)
                m_splitter->maximize(m_mapViewXY);
            else if (view == m_mapViewXZ)
                m_splitter->maximize(m_mapViewXZ);
            else if (view == m_mapViewYZ)
                m_splitter->maximize(m_mapViewYZ);
        }
        
        void FourPaneMapView::doRestoreViews() {
            m_splitter->restore();
        }
    }
}
