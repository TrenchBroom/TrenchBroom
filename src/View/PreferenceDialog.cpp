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
#include "IO/Path.h"
#include "IO/SystemPaths.h"
#include "View/GamesPreferencePane.h"
#include "View/GeneralPreferencePane.h"
#include "View/KeyboardPreferencePane.h"
#include "View/LayoutConstants.h"
#include "View/PreferencePane.h"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/toolbar.h>

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(PreferenceDialog, wxDialog)

        PreferenceDialog::PreferenceDialog() :
        m_toolBar(NULL),
        m_paneContainer(NULL),
        m_pane(NULL) {
            Create();
        }
        
        bool PreferenceDialog::Create() {
            if (!wxDialog::Create(NULL, wxID_ANY, _("Preferences")))
                return false;
            
            createGui();
            bindEvents();
            switchToPane(PPGames);
            return true;
        }

        void PreferenceDialog::OnToolClicked(wxCommandEvent& event) {
            const PrefPane newPane = static_cast<PrefPane>(event.GetId());
            switchToPane(newPane);
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

        void PreferenceDialog::OnFileClose(wxCommandEvent& event) {
            if (!m_pane->validate()) {
                event.Skip();
                return;
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.discardChanges(); // does nothing if the preferences save changes instantly
            Close();
        }

        void PreferenceDialog::createGui() {
            m_pane = NULL;
            
            const IO::Path resourcePath = IO::SystemPaths::resourceDirectory();
            const IO::Path gamesPath = resourcePath + IO::Path("images/GeneralPreferences.png");
            const IO::Path generalPath = resourcePath + IO::Path("images/GeneralPreferences.png");
            const IO::Path keyboardPath = resourcePath + IO::Path("images/KeyboardPreferences.png");
            
            const wxBitmap games(gamesPath.asString(), wxBITMAP_TYPE_PNG);
            const wxBitmap general(generalPath.asString(), wxBITMAP_TYPE_PNG);
            const wxBitmap keyboard(keyboardPath.asString(), wxBITMAP_TYPE_PNG);
            
            m_toolBar = new wxToolBar(this, wxID_ANY);
            m_toolBar->AddCheckTool(PPGames, _("Games"), games, wxNullBitmap);
            m_toolBar->AddCheckTool(PPGeneral, _("General"), general, wxNullBitmap);
            m_toolBar->AddCheckTool(PPKeyboard, _("Keyboard"), keyboard, wxNullBitmap);
            m_toolBar->Realize();
            
            m_paneContainer = new wxPanel(this);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_toolBar, 0, wxEXPAND);
#if !defined __APPLE__
			wxStaticLine* toolBarDivider = new wxStaticLine(this);
            sizer->Add(toolBarDivider, 0, wxEXPAND);
			sizer->SetItemMinSize(toolBarDivider, wxDefaultSize.x, 5);
#endif
            
            sizer->Add(m_paneContainer, 1, wxEXPAND);
            
#if !defined __APPLE__
            wxSizer* stdButtonsSizer = CreateSeparatedButtonSizer(wxOK | wxAPPLY | wxCANCEL);
            sizer->Add(stdButtonsSizer, 0, wxEXPAND);
			sizer->AddSpacer(LayoutConstants::DialogOuterMargin);
#endif
            
            SetSizer(sizer);
        }
        
        void PreferenceDialog::bindEvents() {
            Bind(wxEVT_COMMAND_MENU_SELECTED, &PreferenceDialog::OnFileClose, this, wxID_CLOSE);
            Bind(wxEVT_BUTTON, &PreferenceDialog::OnApplyClicked, this, wxID_APPLY);
            Bind(wxEVT_TOOL, &PreferenceDialog::OnToolClicked, this, PP_First, PP_Last);
        }

        void PreferenceDialog::switchToPane(const PrefPane pane) {
            if (m_currentPane == pane && m_pane != NULL) {
                toggleTools(m_currentPane);
                return;
            }
            
            if (m_pane != NULL && !m_pane->validate()) {
                toggleTools(m_currentPane);
                return;
            }
            
            if (m_pane != NULL)
                m_pane->Destroy();

            toggleTools(pane);
            
            switch (pane) {
                case PPGames:
                    m_pane = new GamesPreferencePane(m_paneContainer);
                    break;
                case PPKeyboard:
                    m_pane = new KeyboardPreferencePane(m_paneContainer);
                    break;
                default:
                    m_pane = new GeneralPreferencePane(m_paneContainer);
                    break;
            }
            m_currentPane = pane;
            
            wxBoxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->Add(m_pane, 1, wxEXPAND | wxALL, LayoutConstants::DialogOuterMargin);
            m_paneContainer->SetSizerAndFit(containerSizer);
            
            Fit();
#if defined __APPLE__
            updateAcceleratorTable(pane);
#endif
        }

        void PreferenceDialog::toggleTools(const PrefPane pane) {
            for (size_t i = PP_First + 1; i < PP_Last; ++i)
                m_toolBar->ToggleTool(i, pane == i);
        }
        
        void PreferenceDialog::updateAcceleratorTable(const PrefPane pane) {
            // allow the dialog to be closed using CMD+W
            // but only if the keyboard preference pane is not active
            if (pane != PPKeyboard) {
                wxAcceleratorEntry acceleratorEntries[1];
                acceleratorEntries[0].Set(wxACCEL_CMD, static_cast<int>('W'), wxID_CLOSE);
                wxAcceleratorTable accceleratorTable(1, acceleratorEntries);
                SetAcceleratorTable(accceleratorTable);
            } else {
                wxAcceleratorTable accceleratorTable(0, NULL);
                SetAcceleratorTable(accceleratorTable);
            }
        }
    }
}
