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

#include "NavBar.h"

#include <wx/dcclient.h>
#include <wx/sizer.h>
#include <wx/srchctrl.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        wxStaticText* NavBar::makeBreadcrump(const wxString& text, bool link) {
            wxStaticText* staticText = new wxStaticText(m_navPanel, wxID_ANY, text);
#ifdef __APPLE__
            staticText->SetFont(*wxSMALL_FONT);
#endif
            if (link) {
                staticText->SetForegroundColour(wxColour(10, 75, 220)); //wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));
                staticText->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            return staticText;
        }
        
        NavBar::NavBar(wxWindow* parent) :
        ContainerBar(parent, wxBOTTOM),
        m_navPanel(new wxPanel(this, wxID_ANY)),
        m_searchBox(new wxSearchCtrl(this, wxID_ANY)) {
#ifdef __APPLE__
            m_searchBox->SetFont(*wxSMALL_FONT);
#endif
            m_searchBox->Bind(wxEVT_COMMAND_TEXT_UPDATED, &NavBar::OnSearchPatternChanged, this);
            
            wxSizer* innerSizer = new wxBoxSizer(wxHORIZONTAL);
            innerSizer->AddSpacer(4);
            innerSizer->Add(m_navPanel, 1, wxEXPAND | wxALIGN_CENTRE_VERTICAL);
            innerSizer->Add(m_searchBox, 0, wxEXPAND | wxALIGN_RIGHT);
#ifdef __APPLE__
            innerSizer->AddSpacer(4);
#endif
            innerSizer->SetItemMinSize(m_searchBox, 200, wxDefaultSize.y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(2);
            outerSizer->Add(innerSizer, 1, wxEXPAND);
            outerSizer->AddSpacer(2);
            SetSizer(outerSizer);
        }
        
        void NavBar::OnSearchPatternChanged(wxCommandEvent& event) {
        }
        
        void NavBar::updateBreadcrump() {
        }
    }
}
