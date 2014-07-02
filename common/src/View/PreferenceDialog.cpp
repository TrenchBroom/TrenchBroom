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

#include "PreferenceDialog.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/BorderLine.h"
#include "View/GamesPreferencePane.h"
#include "View/KeyboardPreferencePane.h"
#include "View/CameraPreferencePane.h"
#include "View/ViewPreferencePane.h"
#include "View/ViewConstants.h"
#include "View/PreferencePane.h"
#include "View/wxUtils.h"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/simplebook.h>
#include <wx/toolbar.h>

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(PreferenceDialog, wxDialog)

        PreferenceDialog::PreferenceDialog() :
        m_toolBar(NULL),
        m_book(NULL) {
            Create();
        }
        
        bool PreferenceDialog::Create() {
            if (!wxDialog::Create(NULL, wxID_ANY, "Preferences"))
                return false;
            
            createGui();
            bindEvents();
            switchToPane(PrefPane_First);
            SetClientSize(currentPane()->GetMinSize());
            
            return true;
        }

        void PreferenceDialog::OnToolClicked(wxCommandEvent& event) {
            const PrefPane newPane = static_cast<PrefPane>(event.GetId());
            switchToPane(newPane);
        }

        void PreferenceDialog::OnOKClicked(wxCommandEvent& event) {
            PreferenceManager& prefs = PreferenceManager::instance();
            if (!prefs.saveInstantly())
                prefs.saveChanges();
            EndModal(wxID_OK);
        }

        void PreferenceDialog::OnApplyClicked(wxCommandEvent& event) {
            PreferenceManager& prefs = PreferenceManager::instance();
            if (!prefs.saveInstantly())
                prefs.saveChanges();
        }

        void PreferenceDialog::OnCancelClicked(wxCommandEvent& event) {
            PreferenceManager& prefs = PreferenceManager::instance();
            if (!prefs.saveInstantly())
                prefs.discardChanges();
            EndModal(wxID_CANCEL);
        }

        void PreferenceDialog::OnClose(wxCloseEvent& event) {
            if (!currentPane()->validate() && event.CanVeto()) {
                event.Veto();
                return;
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            if (!prefs.saveInstantly()) {
                switch (event.GetId()) {
                    case wxID_OK:
                        prefs.saveChanges();
                        break;
                    default:
                        prefs.discardChanges();
                        break;
                        
                }
            }
        }

        void PreferenceDialog::OnFileClose(wxCommandEvent& event) {
            if (!currentPane()->validate()) {
                event.Skip();
                return;
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.discardChanges(); // does nothing if the preferences save changes instantly
            Close();
        }

        void PreferenceDialog::createGui() {
            const wxBitmap gamesImage = IO::loadImageResource(IO::Path("images/GeneralPreferences.png"));
            const wxBitmap generalImage = IO::loadImageResource(IO::Path("images/GeneralPreferences.png"));
            const wxBitmap keyboardImage = IO::loadImageResource(IO::Path("images/KeyboardPreferences.png"));
            
            m_toolBar = new wxToolBar(this, wxID_ANY);
            m_toolBar->AddCheckTool(PrefPane_Games, "Games", gamesImage, wxNullBitmap);
            m_toolBar->AddCheckTool(PrefPane_View, "View", generalImage, wxNullBitmap);
            m_toolBar->AddCheckTool(PrefPane_Camera, "Camera", generalImage, wxNullBitmap);
            m_toolBar->AddCheckTool(PrefPane_Keyboard, "Keyboard", keyboardImage, wxNullBitmap);
            m_toolBar->Realize();
            
            m_book = new wxSimplebook(this);
            m_book->AddPage(new GamesPreferencePane(m_book), "Games");
            m_book->AddPage(new ViewPreferencePane(m_book), "View");
            m_book->AddPage(new CameraPreferencePane(m_book), "Camera");
            m_book->AddPage(new KeyboardPreferencePane(m_book), "Keyboard");
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_toolBar, 0, wxEXPAND);
#if !defined __APPLE__
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->SetItemMinSize(1, wxSize(wxDefaultCoord, 1));
#endif
            sizer->Add(m_book, 1, wxEXPAND);
            
#if !defined __APPLE__
            wxSizer* buttonSizer = CreateButtonSizer(wxOK | wxAPPLY | wxCANCEL);
            sizer->Add(wrapDialogButtonSizer(buttonSizer, this), 0, wxEXPAND);
#endif
            
            SetSizer(sizer);
        }
        
        void PreferenceDialog::bindEvents() {
            Bind(wxEVT_COMMAND_MENU_SELECTED, &PreferenceDialog::OnFileClose, this, wxID_CLOSE);
            Bind(wxEVT_BUTTON, &PreferenceDialog::OnOKClicked, this, wxID_OK);
            Bind(wxEVT_BUTTON, &PreferenceDialog::OnApplyClicked, this, wxID_APPLY);
            Bind(wxEVT_BUTTON, &PreferenceDialog::OnCancelClicked, this, wxID_CANCEL);
            Bind(wxEVT_TOOL, &PreferenceDialog::OnToolClicked, this, PrefPane_First, PrefPane_Last);
        }

        void PreferenceDialog::switchToPane(const PrefPane pane) {
            if (currentPaneId() == pane && currentPane() != NULL) {
                toggleTools(currentPaneId());
                return;
            }
            
            if (currentPane() != NULL && !currentPane()->validate()) {
                toggleTools(currentPaneId());
                return;
            }
            
            toggleTools(pane);
            m_book->SetSelection(static_cast<size_t>(pane));
            currentPane()->updateControls();
            
            SetSize(currentPane()->GetMinSize());
#if defined __APPLE__
            updateAcceleratorTable(pane);
#endif
        }

        void PreferenceDialog::toggleTools(const PrefPane pane) {
            for (int i = PrefPane_First + 1; i < PrefPane_Last; ++i)
                m_toolBar->ToggleTool(i, pane == i);
        }
        
        PreferencePane* PreferenceDialog::currentPane() const {
            return static_cast<PreferencePane*>(m_book->GetCurrentPage());
        }
        
        PreferenceDialog::PrefPane PreferenceDialog::currentPaneId() const {
            return static_cast<PrefPane>(m_book->GetSelection());
        }

        void PreferenceDialog::updateAcceleratorTable(const PrefPane pane) {
            // allow the dialog to be closed using CMD+W
            // but only if the keyboard preference pane is not active
            if (pane != PrefPane_Keyboard) {
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
