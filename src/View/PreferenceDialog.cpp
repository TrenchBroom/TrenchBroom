/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "PreferenceDialog.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "IO/FileSystem.h"
#include "View/GeneralPreferencePane.h"
#include "View/KeyboardPreferencePane.h"
#include "View/LayoutConstants.h"
#include "View/PreferencePane.h"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/toolbar.h>

namespace TrenchBroom {
    namespace View {
        PreferenceDialog::PreferenceDialog() {
            Create();
        }
        
        bool PreferenceDialog::Create() {
            if (!wxDialog::Create(NULL, wxID_ANY, _("Preferences")))
                return false;
            
            createGui();
            bindEvents();
            return true;
        }

        void PreferenceDialog::OnToolClicked(wxCommandEvent& event) {
            switchToPane(static_cast<PrefPane>(event.GetId()));
        }

        void PreferenceDialog::OnApplyClicked(wxCommandEvent& event) {
#if !defined __APPLE__
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.saveChanges();
#endif
        }

        void PreferenceDialog::OnClose(wxCloseEvent& event) {
            if (!m_pane->validate() && event.CanVeto()) {
                event.Veto();
                return;
            }
            
#if !defined __APPLE__
            PreferenceManager& prefs = PreferenceManager::instance();
            switch (event.GetId()) {
                case wxID_OK:
                    prefs.saveChanges();
                    break;
                default:
                    prefs.discardChanges();
                    break;
                    
            }
#endif
        }

        void PreferenceDialog::createGui() {
            m_pane = NULL;
            
            IO::FileSystem fs;
            const IO::Path resourcePath = fs.resourceDirectory();
            const IO::Path generalPath = resourcePath + IO::Path("images/GeneralPreferences.png");
            const IO::Path keyboardPath = resourcePath + IO::Path("images/KeyboardPreferences.png");
            const wxBitmap general(generalPath.asString(), wxBITMAP_TYPE_PNG);
            const wxBitmap keyboard(keyboardPath.asString(), wxBITMAP_TYPE_PNG);
            
            m_toolBar = new wxToolBar(this, wxID_ANY);
            m_toolBar->AddCheckTool(PPGeneral, wxT("General"), general, wxNullBitmap);
            m_toolBar->AddCheckTool(PPKeyboard, wxT("Keyboard"), keyboard, wxNullBitmap);
            m_toolBar->Realize();
            
            m_paneContainer = new wxPanel(this);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_toolBar, 0, wxEXPAND);
            sizer->Add(m_paneContainer, 1, wxEXPAND);
            
#if not defined __APPLE__
            wxSizer* stdButtonsSizer = CreateSeparatedButtonSizer(wxOK | wxAPPLY | wxCANCEL);
            sizer->Add(stdButtonsSizer, 0, wxEXPAND);
#endif
            
            SetSizer(sizer);

            switchToPane(PPGeneral);
        }
        
        void PreferenceDialog::bindEvents() {
            Bind(wxEVT_BUTTON, &PreferenceDialog::OnApplyClicked, this, wxID_APPLY);
            Bind(wxEVT_TOOL, &PreferenceDialog::OnToolClicked, this, PPGeneral, PPKeyboard);
        }

        void PreferenceDialog::switchToPane(const PrefPane pane) {
            if (m_pane != NULL && !m_pane->validate()) {
                m_toolBar->ToggleTool(PPGeneral, m_currentPane == PPGeneral);
                m_toolBar->ToggleTool(PPKeyboard, m_currentPane == PPKeyboard);
                return;
            }
            
            if (m_pane != NULL)
                m_pane->Destroy();

            m_toolBar->ToggleTool(PPGeneral, pane == PPGeneral);
            m_toolBar->ToggleTool(PPKeyboard, pane == PPKeyboard);
            
            switch (pane) {
                case PPKeyboard:
                    m_pane = new KeyboardPreferencePane(m_paneContainer);
                    break;
                default:
                    m_pane = new GeneralPreferencePane(m_paneContainer);
                    break;
            }
            
            wxBoxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->Add(m_pane, 1, wxEXPAND | wxALL, LayoutConstants::DialogOuterMargin);
            m_paneContainer->SetSizerAndFit(containerSizer);
            
            Fit();
        }
    }
}
