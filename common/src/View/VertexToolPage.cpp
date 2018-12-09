/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "VertexToolPage.h"

#include "PreferenceManager.h"
#include "Preferences.h"

#include "TrenchBroom.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <wx/checkbox.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        VertexToolPage::VertexToolPage(wxWindow *parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            createGui();
            bindObservers();
        }

        VertexToolPage::~VertexToolPage() {
            unbindObservers();
        }

        void VertexToolPage::createGui() {
            MapDocumentSPtr document = lock(m_document);

            m_uvLockCheckBox = new wxCheckBox(this, wxID_ANY, "UV Lock");
            m_uvLockCheckBox->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& event){
                PreferenceManager::instance().set(Preferences::UVLock, !pref(Preferences::UVLock));
                PreferenceManager::instance().saveChanges();
            });

            auto* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_uvLockCheckBox, 0, wxALIGN_CENTER_VERTICAL);

            SetSizer(sizer);

            updateControls();
        }

        void VertexToolPage::bindObservers() {
            PreferenceManager &prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &VertexToolPage::preferenceDidChange);
        }

        void VertexToolPage::unbindObservers() {
            PreferenceManager &prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &VertexToolPage::preferenceDidChange);
        }

        void VertexToolPage::preferenceDidChange(const IO::Path &path) {
            updateControls();
        }

        void VertexToolPage::updateControls() {
            m_uvLockCheckBox->SetValue(pref(Preferences::UVLock));
        }
    }
}
