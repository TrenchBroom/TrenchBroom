/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "OnePaneMapView.h"

#include "Model/PointFile.h"
#include "Renderer/Camera.h"
#include "View/CommandIds.h"
#include "View/CyclingMapView.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        OnePaneMapView::OnePaneMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document, MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) :
        MultiMapView(parent),
        m_logger(logger),
        m_document(document),
        m_mapView(NULL) {
            createGui(toolBox, mapRenderer, contextManager);
        }
        
        void OnePaneMapView::createGui(MapViewToolBox& toolBox, Renderer::MapRenderer& mapRenderer, GLContextManager& contextManager) {
            
            m_mapView = new CyclingMapView(this, m_logger, m_document, toolBox, mapRenderer, contextManager, CyclingMapView::View_ALL);
            m_mapView->linkCamera(m_linkHelper);
            addMapView(m_mapView);
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_mapView, 1, wxEXPAND);
            
            SetSizer(sizer);
        }
    }
}
