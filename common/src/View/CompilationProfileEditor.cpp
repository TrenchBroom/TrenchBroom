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

#include "CompilationProfileEditor.h"

#include "Model/CompilationProfile.h"
#include "View/CompilationTaskList.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {

        CompilationProfileEditor::CompilationProfileEditor(wxWindow* parent) :
        wxPanel(parent),
        m_profile(NULL),
        m_taskView(new CompilationTaskList(this)) {
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_taskView, 1, wxEXPAND);
            SetSizer(sizer);
        }
        
        CompilationProfileEditor::~CompilationProfileEditor() {
            if (m_profile != NULL)
                m_profile->profileDidChange.addObserver(this, &CompilationProfileEditor::profileDidChange);
        }

        void CompilationProfileEditor::setProfile(Model::CompilationProfile* profile) {
            if (m_profile != NULL)
                m_profile->profileDidChange.removeObserver(this, &CompilationProfileEditor::profileDidChange);
            m_profile = profile;
            m_taskView->setProfile(profile);
            if (m_profile != NULL)
                m_profile->profileDidChange.addObserver(this, &CompilationProfileEditor::profileDidChange);
        }

        void CompilationProfileEditor::profileDidChange() {
            refresh();
        }
        
        void CompilationProfileEditor::refresh() {
        }
    }
}
