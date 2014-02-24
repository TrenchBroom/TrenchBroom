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
#include "View/MiniMapView.h"

#include <wx/sizer.h>
#include <wx/slider.h>

namespace TrenchBroom {
    namespace View {
        MiniMap::MiniMap(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources) :
        wxPanel(parent),
        m_zSlider(NULL),
        m_miniMapView(NULL) {
            createGui(document, renderResources);
            bindEvents();
        }

        void MiniMap::OnZSliderChanged(wxScrollEvent& event) {
            const float position = static_cast<float>(event.GetPosition());
            const float max = static_cast<float>(m_zSlider->GetMax());
            m_miniMapView->setZPosition(position / max);
        }

        void MiniMap::createGui(View::MapDocumentWPtr document, Renderer::RenderResources& renderResources) {
            m_zSlider = new wxSlider(this, wxID_ANY, 2048, 0, 4096, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
            m_miniMapView = new MiniMapView(this, document, renderResources);
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_zSlider, 0, wxEXPAND);
            sizer->Add(m_miniMapView, 1, wxEXPAND);
            
            SetSizer(sizer);
        }
        
        void MiniMap::bindEvents() {
            m_zSlider->Bind(wxEVT_SCROLL_TOP, &MiniMap::OnZSliderChanged, this);
            m_zSlider->Bind(wxEVT_SCROLL_BOTTOM, &MiniMap::OnZSliderChanged, this);
            m_zSlider->Bind(wxEVT_SCROLL_LINEUP, &MiniMap::OnZSliderChanged, this);
            m_zSlider->Bind(wxEVT_SCROLL_LINEDOWN, &MiniMap::OnZSliderChanged, this);
            m_zSlider->Bind(wxEVT_SCROLL_PAGEUP, &MiniMap::OnZSliderChanged, this);
            m_zSlider->Bind(wxEVT_SCROLL_PAGEDOWN, &MiniMap::OnZSliderChanged, this);
            m_zSlider->Bind(wxEVT_SCROLL_THUMBTRACK, &MiniMap::OnZSliderChanged, this);
        }
    }
}
