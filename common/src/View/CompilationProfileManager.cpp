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

#include "CompilationProfileManager.h"

#include "Model/CompilationConfig.h"
#include "View/BorderLine.h"
#include "View/CompilationProfileListBox.h"
#include "View/CompilationProfileEditor.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        CompilationProfileManager::CompilationProfileManager(wxWindow* parent, Model::CompilationConfig& config) :
        wxPanel(parent),
        m_config(config),
        m_listView(new CompilationProfileListBox(this, m_config)),
        m_editor(new CompilationProfileEditor(this)) {
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_listView, 0, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Vertical), 0, wxEXPAND);
            sizer->Add(m_editor, 1, wxEXPAND);
            sizer->SetItemMinSize(m_listView, wxSize(200, 200));
            SetSizer(sizer);
            
            m_listView->Bind(wxEVT_LISTBOX, &CompilationProfileManager::OnProfileSelectionChanged, this);
        }

        void CompilationProfileManager::OnProfileSelectionChanged(wxCommandEvent& event) {
            const int selection = m_listView->GetSelection();
            if (selection != wxNOT_FOUND) {
                Model::CompilationProfile* profile = m_config.profile(static_cast<size_t>(selection));
                m_editor->setProfile(profile);
            } else {
                m_editor->setProfile(NULL);
            }
        }
    }
}
