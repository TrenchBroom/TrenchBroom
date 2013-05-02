/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PreferencesFrame.h"

#include <wx/button.h>
#include <wx/settings.h>
#include <wx/toolbar.h>

#include "TrenchBroomApp.h"
#include "Controller/PreferenceChangeEvent.h"
#include "IO/FileManager.h"
#include "Utility/Preferences.h"
#include "Utility/String.h"
#include "View/CommandIds.h"
#include "View/GeneralPreferencePane.h"
#include "View/KeyboardPreferencePane.h"
#include "View/LayoutConstants.h"

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(PreferencesFrame, wxFrame)

        BEGIN_EVENT_TABLE(PreferencesFrame, wxFrame)
        EVT_TOOL_RANGE(PPGeneral, PPKeyboard, PreferencesFrame::OnToolClicked)
        EVT_BUTTON(wxID_OK, PreferencesFrame::OnOkClicked)
        EVT_BUTTON(wxID_CANCEL, PreferencesFrame::OnCancelClicked)
		EVT_CLOSE(PreferencesFrame::OnClose)
        EVT_MENU(wxID_CLOSE, PreferencesFrame::OnFileExit)
		END_EVENT_TABLE()

        void PreferencesFrame::switchToPane(PrefPane pane) {
            if (m_pane != NULL && !m_pane->validate()) {
                m_toolBar->ToggleTool(PPGeneral, m_currentPane == PPGeneral);
                m_toolBar->ToggleTool(PPKeyboard, m_currentPane == PPKeyboard);
                return;
            }

            if (m_panel != NULL) {
                m_panel->Destroy();
                m_panel = NULL;
            }

            m_pane = NULL;
            m_panel = new wxPanel(this);

            m_toolBar->ToggleTool(PPGeneral, pane == PPGeneral);
            m_toolBar->ToggleTool(PPKeyboard, pane == PPKeyboard);

            switch (pane) {
                case PPKeyboard:
                    m_pane = new KeyboardPreferencePane(m_panel);
                    break;
                default:
                    m_pane = new GeneralPreferencePane(m_panel);
                    break;
            }

            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
#ifndef __APPLE__
            innerSizer->Add(m_pane, 1, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::DialogOuterMargin);

            wxButton* okButton = new wxButton(m_panel, wxID_OK, wxT("OK"));
            wxButton* cancelButton = new wxButton(m_panel, wxID_CANCEL, wxT("Cancel"));

            wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
            buttonSizer->SetAffirmativeButton(okButton);
            buttonSizer->SetCancelButton(cancelButton);
            buttonSizer->Realize();

            innerSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, LayoutConstants::DialogButtonMargin);
#else
            innerSizer->Add(m_pane, 1, wxEXPAND | wxALL, LayoutConstants::DialogOuterMargin);
#endif
            m_panel->SetSizerAndFit(innerSizer);

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(m_panel, 1, wxEXPAND);
            SetSizerAndFit(outerSizer);

#ifdef __APPLE__
            // allow the dialog to be closed using CMD+W
            // but only if the keyboard preference pane is not active
            if (m_currentPane == PPKeyboard && pane != PPKeyboard) {
                wxAcceleratorEntry acceleratorEntries[1];
                acceleratorEntries[0].Set(wxACCEL_CMD, static_cast<int>('W'), wxID_CLOSE);
                wxAcceleratorTable accceleratorTable(1, acceleratorEntries);
                SetAcceleratorTable(accceleratorTable);
            } else if (m_currentPane != PPKeyboard && pane == PPKeyboard) {
                wxAcceleratorTable accceleratorTable(0, NULL);
                SetAcceleratorTable(accceleratorTable);
            }
#endif

            m_currentPane = pane;
        }

        PreferencesFrame::PreferencesFrame() :
        wxFrame(NULL, wxID_ANY, wxT("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER),
        m_toolBar(NULL),
        m_panel(NULL),
        m_pane(NULL) {
            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();
            wxBitmap general(fileManager.appendPath(resourcePath, "GeneralPreferences.png"), wxBITMAP_TYPE_PNG);
            wxBitmap keyboard(fileManager.appendPath(resourcePath, "KeyboardPreferences.png"), wxBITMAP_TYPE_PNG);

            m_toolBar = CreateToolBar(wxTB_TEXT);
            m_toolBar->AddCheckTool(PPGeneral, wxT("General"), general, wxNullBitmap);
            m_toolBar->AddCheckTool(PPKeyboard, wxT("Keyboard"), keyboard, wxNullBitmap);
            m_toolBar->Realize();
            SetToolBar(m_toolBar);

            switchToPane(PPGeneral);

            static_cast<AbstractApp*>(wxTheApp)->setPreferencesFrame(this);
        }

        void PreferencesFrame::OnToolClicked(wxCommandEvent& event) {
            switchToPane(static_cast<PrefPane>(event.GetId()));
        }

        void PreferencesFrame::OnOkClicked(wxCommandEvent& event) {
            if (!m_pane->validate()) {
                event.Skip();
                return;
            }

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Preferences::PreferenceBase::Set changedPreferences = prefs.saveChanges();
            
            Controller::PreferenceChangeEvent preferenceChangeEvent(changedPreferences);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &preferenceChangeEvent);

            static_cast<AbstractApp*>(wxTheApp)->setPreferencesFrame(NULL);
            Destroy();
		}

		void PreferencesFrame::OnCancelClicked(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Preferences::PreferenceBase::Set changedPreferences = prefs.discardChanges();
            
            Controller::PreferenceChangeEvent preferenceChangeEvent(changedPreferences);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &preferenceChangeEvent);

            static_cast<AbstractApp*>(wxTheApp)->setPreferencesFrame(NULL);
            Destroy();
		}

		void PreferencesFrame::OnClose(wxCloseEvent& event) {
            if (!m_pane->validate()) {
                event.Veto();
                return;
            }

#ifndef __APPLE__
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Preferences::PreferenceBase::Set changedPreferences = prefs.discardChanges();
            
            Controller::PreferenceChangeEvent preferenceChangeEvent(changedPreferences);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &preferenceChangeEvent);
#endif

            static_cast<AbstractApp*>(wxTheApp)->setPreferencesFrame(NULL);
            event.Skip();
		}

        void PreferencesFrame::OnFileExit(wxCommandEvent& event) {
            if (!m_pane->validate()) {
                event.Skip();
                return;
            }

#ifndef __APPLE__
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Preferences::PreferenceBase::Set changedPreferences = prefs.discardChanges();
            
            Controller::PreferenceChangeEvent preferenceChangeEvent(changedPreferences);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &preferenceChangeEvent);
#endif

            static_cast<AbstractApp*>(wxTheApp)->setPreferencesFrame(NULL);
            Destroy();
        }
	}
}
