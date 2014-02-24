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
        MiniMap::MiniMap(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources) :
        wxPanel(parent),
        m_renderer(document),
        m_miniMapZView(NULL),
        m_miniMapXYView(NULL) {
            createGui(document, renderResources);
            bindEvents();
        }

        void MiniMap::OnXYMiniMapChanged(wxCommandEvent& event) {
            m_miniMapZView->Refresh();
        }
        
        void MiniMap::OnZMiniMapChanged(wxCommandEvent& event) {
            m_miniMapXYView->Refresh();
        }

        void MiniMap::createGui(View::MapDocumentWPtr document, Renderer::RenderResources& renderResources) {
            m_miniMapXYView = new MiniMapXYView(this, document, m_visibleBounds, renderResources, m_renderer);
            m_miniMapZView = new MiniMapZView(this, document, m_visibleBounds, renderResources, m_renderer);
            
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
