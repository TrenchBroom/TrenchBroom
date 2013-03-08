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
#include "Controller/Command.h"
#include "IO/FileManager.h"
#include "Utility/Preferences.h"
#include "Utility/String.h"
#include "View/CommandIds.h"
#include "View/GeneralPreferencePane.h"
#include "View/LayoutConstants.h"

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(PreferencesFrame, wxFrame)
        EVT_BUTTON(wxID_OK, PreferencesFrame::OnOkClicked)
        EVT_BUTTON(wxID_CANCEL, PreferencesFrame::OnCancelClicked)
		EVT_CLOSE(PreferencesFrame::OnClose)
        EVT_MENU(wxID_CLOSE, PreferencesFrame::OnFileExit)
		END_EVENT_TABLE()

        PreferencesFrame::PreferencesFrame() :
        wxFrame(NULL, wxID_ANY, wxT("Preferences")) {
            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();
            wxBitmap general(fileManager.appendPath(resourcePath, "GeneralPreferences.png"), wxBITMAP_TYPE_PNG);
            wxBitmap keyboard(fileManager.appendPath(resourcePath, "KeyboardPreferences.png"), wxBITMAP_TYPE_PNG);

            m_toolBar = CreateToolBar(wxTB_TEXT);
            m_toolBar->AddCheckTool(1, wxT("General"), general, wxNullBitmap);
            m_toolBar->AddCheckTool(2, wxT("Keyboard"), keyboard, wxNullBitmap);
            m_toolBar->Realize();
            SetToolBar(m_toolBar);
            
            wxPanel* panel = new wxPanel(this);
            m_generalPreferencePane = new GeneralPreferencePane(panel);

            wxSizer* innerSizer = new wxBoxSizer(wxVERTICAL);
#ifndef __APPLE__
            innerSizer->Add(m_generalPreferencePane, 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::DialogOuterMargin);

            wxButton* okButton = new wxButton(panel, wxID_OK, wxT("OK"));
            wxButton* cancelButton = new wxButton(panel, wxID_CANCEL, wxT("Cancel"));

            wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
            buttonSizer->SetAffirmativeButton(okButton);
            buttonSizer->SetCancelButton(cancelButton);
            buttonSizer->Realize();

            innerSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, LayoutConstants::DialogButtonMargin);
#else
            innerSizer->Add(m_generalPreferencePane, 0, wxEXPAND | wxALL, LayoutConstants::DialogOuterMargin);
#endif
            innerSizer->SetItemMinSize(m_generalPreferencePane, 600, m_generalPreferencePane->GetSize().y);
            panel->SetSizerAndFit(innerSizer);

            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(panel, 1, wxEXPAND);
            SetSizerAndFit(outerSizer);

#ifdef __APPLE__
            // allow the dialog to be closed using CMD+W
            wxAcceleratorEntry acceleratorEntries[1];
            acceleratorEntries[0].Set(wxACCEL_CMD, static_cast<int>('W'), wxID_CLOSE);
            wxAcceleratorTable accceleratorTable(4, acceleratorEntries);
            SetAcceleratorTable(accceleratorTable);
#endif
        }

        void PreferencesFrame::OnOkClicked(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
			prefs.save();

			Controller::Command invalidateCacheCommand(Controller::Command::InvalidateEntityModelRendererCache);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &invalidateCacheCommand);

            Controller::Command invalidateInstancedRenderersCommand(Controller::Command::InvalidateInstancedRenderers);
            static_cast<TrenchBroomApp*>(wxTheApp)->UpdateAllViews(NULL, &invalidateInstancedRenderersCommand);
            Destroy();
		}

		void PreferencesFrame::OnCancelClicked(wxCommandEvent& event) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
			prefs.discardChanges();
            Destroy();
		}

		void PreferencesFrame::OnClose(wxCloseEvent& event) {
#ifndef __APPLE__
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
			prefs.discardChanges();
#endif
            event.Skip();
		}

        void PreferencesFrame::OnFileExit(wxCommandEvent& event) {
            Destroy();
        }
	}
}
