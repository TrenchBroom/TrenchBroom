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

#include "MiniMap.h"

#include "View/ViewConstants.h"
#include "View/MiniMapXYView.h"
#include "View/MiniMapZView.h"

#include <wx/sizer.h>
#include <wx/slider.h>

namespace TrenchBroom {
    namespace View {
        MiniMap::MiniMap(wxWindow* parent, GLContextHolder::Ptr sharedContext, View::MapDocumentWPtr document, Renderer::Camera& camera) :
        wxPanel(parent),
        m_renderer(document),
        m_miniMapZView(NULL),
        m_miniMapXYView(NULL) {
            createGui(sharedContext, document, camera);
            bindEvents();
        }

        void MiniMap::OnXYMiniMapChanged(wxCommandEvent& event) {
            m_miniMapZView->setXYRange(m_miniMapXYView->xyRange());
        }
        
        void MiniMap::OnZMiniMapChanged(wxCommandEvent& event) {
            m_miniMapXYView->setZRange(m_miniMapZView->zRange());
        }

        void MiniMap::createGui(GLContextHolder::Ptr sharedContext, View::MapDocumentWPtr document, Renderer::Camera& camera) {
            m_miniMapXYView = new MiniMapXYView(this, sharedContext, document, m_renderer, camera);
            m_miniMapZView = new MiniMapZView(this, sharedContext, document, m_renderer, camera);
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_miniMapZView, 0, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::ControlMargin);
            sizer->Add(m_miniMapXYView, 1, wxEXPAND);
            sizer->SetItemMinSize(m_miniMapZView, 32, wxDefaultSize.y);
            
            SetSizer(sizer);
        }
        
        void MiniMap::bindEvents() {
            m_miniMapXYView->Bind(EVT_MINIMAP_VIEW_CHANGED_EVENT, EVT_MINIMAP_VIEW_CHANGED_HANDLER(MiniMap::OnXYMiniMapChanged), this);
            m_miniMapZView->Bind(EVT_MINIMAP_VIEW_CHANGED_EVENT, EVT_MINIMAP_VIEW_CHANGED_HANDLER(MiniMap::OnZMiniMapChanged), this);
        }
    }
}
