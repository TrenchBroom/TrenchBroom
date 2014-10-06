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

#include "Renderer/MapRenderer.h"
#include "View/MapView3D.h"
#include "View/MapViewBar.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        SwitchableMapView::SwitchableMapView(wxWindow* parent, Logger* logger, MapDocumentWPtr document) :
        wxPanel(parent),
        m_logger(logger),
        m_document(document),
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
        }

        void SwitchableMapView::createGui() {
            m_mapRenderer = new Renderer::MapRenderer(m_document);
            m_mapViewBar = new MapViewBar(this, m_document);
            m_mapView = new MapView3D(this, m_logger, m_mapViewBar->toolBook(), m_document, *m_mapRenderer);
            
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
