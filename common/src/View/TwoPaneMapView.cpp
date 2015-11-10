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
        TwoPaneMapView::TwoPaneMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) :
        MultiMapView(parent),
        m_logger(logger),
        m_document(document),
        m_splitter(NULL),
        m_mapView3D(NULL),
        m_mapView2D(NULL) {
            createGui(toolBox, mapRenderer, contextManager);
        }
        
        void TwoPaneMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) {

            m_splitter = new SplitterWindow2(this);
            m_splitter->setSashGravity(0.5f);
            m_splitter->SetName("2PaneMapViewHSplitter");

            m_mapView3D = new MapView3D(m_splitter, m_logger, m_document, toolBox, mapRenderer, contextManager);
            m_mapView2D = new CyclingMapView(m_splitter, m_logger, m_document, toolBox, mapRenderer, contextManager, CyclingMapView::View_2D);
            
            m_mapView3D->linkCamera(m_linkHelper);
            m_mapView2D->linkCamera(m_linkHelper);
            
            addMapView(m_mapView3D);
            addMapView(m_mapView2D);
            
            m_splitter->splitVertically(m_mapView3D, m_mapView2D, wxSize(100, 100), wxSize(100, 100));
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_splitter, 1, wxEXPAND);
            
            SetSizer(sizer);

            wxPersistenceManager::Get().RegisterAndRestore(m_splitter);
        }

        void TwoPaneMapView::doMaximizeView(MapView* view) {
            assert(view == m_mapView2D || view == m_mapView3D);
            if (view == m_mapView2D)
                m_splitter->maximize(m_mapView2D);
            else
                m_splitter->maximize(m_mapView3D);
        }
        
        void TwoPaneMapView::doRestoreViews() {
            m_splitter->restore();
        }
    }
}
