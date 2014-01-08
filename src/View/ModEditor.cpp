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

#include "ModEditor.h"

#include "Notifier.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Entity.h"
#include "Model/Game.h"
#include "Model/Object.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/ControllerFacade.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/ViewUtils.h"

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ModEditor::ModEditor(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_availableModList(NULL),
        m_enabledModList(NULL),
        m_filterBox(NULL),
        m_addModsButton(NULL),
        m_removeModsButton(NULL),
        m_moveModUpButton(NULL),
        m_moveModDownButton(NULL),
        m_ignoreNotifier(false) {
            createGui();
            bindEvents();
            bindObservers();
        }

        ModEditor::~ModEditor() {
            unbindObservers();
        }

        void ModEditor::OnAddModClicked(wxCommandEvent& event) {
            SetBool ignoreNotifier(m_ignoreNotifier);

            wxArrayInt selections;
            m_availableModList->GetSelections(selections);
            assert(!selections.empty());
            
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            StringList mods = document->mods();
            for (size_t i = 0; i < selections.size(); ++i) {
                const unsigned int index = selections[i] - i;
                const wxString item = m_availableModList->GetString(index);
                m_availableModList->Delete(index);
                m_enabledModList->Append(item);

                mods.push_back(item.ToStdString());
            }
            
            m_availableModList->DeselectAll();
            m_enabledModList->DeselectAll();

            controller->setMods(mods);
        }
        
        void ModEditor::OnRemoveModClicked(wxCommandEvent& event) {
            wxArrayInt selections;
            m_enabledModList->GetSelections(selections);
            assert(!selections.empty());
            
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            StringList mods = document->mods();
            std::sort(selections.begin(), selections.end());
            
            wxArrayInt::const_reverse_iterator it, end;
            for (it = selections.rbegin(), end = selections.rend(); it != end; ++it) {
                const String mod = mods[*it];
                mods.erase(mods.begin() + *it);
            }

            controller->setMods(mods);
        }
        
        void ModEditor::OnMoveModUpClicked(wxCommandEvent& event) {
            wxArrayInt selections;
            m_enabledModList->GetSelections(selections);
            assert(selections.size() == 1);
            
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            StringList mods = document->mods();

            const size_t index = static_cast<size_t>(selections.front());
            assert(index > 0 && index < mods.size());
            
            using std::swap;
            swap(mods[index - 1], mods[index]);
            controller->setMods(mods);

            m_enabledModList->DeselectAll();
            m_enabledModList->SetSelection(index - 1);
        }
        
        void ModEditor::OnMoveModDownClicked(wxCommandEvent& event) {
            wxArrayInt selections;
            m_enabledModList->GetSelections(selections);
            assert(selections.size() == 1);
            
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            StringList mods = document->mods();
            
            const size_t index = static_cast<size_t>(selections.front());
            assert(index < mods.size() - 1);
            
            using std::swap;
            swap(mods[index + 1], mods[index]);
            controller->setMods(mods);
            
            m_enabledModList->DeselectAll();
            m_enabledModList->SetSelection(index + 1);
        }

        void ModEditor::OnUpdateButtonUI(wxUpdateUIEvent& event) {
            wxArrayInt selections;
            if (event.GetEventObject() == m_addModsButton) {
                event.Enable(m_availableModList->GetSelections(selections) > 0);
            } else if (event.GetEventObject() == m_removeModsButton) {
                event.Enable(m_enabledModList->GetSelections(selections) > 0);
            } else {
                if (m_enabledModList->GetSelections(selections) == 1) {
                    if (event.GetEventObject() == m_moveModUpButton)
                        event.Enable(selections.front() > 0);
                    else
                        event.Enable(selections.front() < static_cast<int>(m_enabledModList->GetCount()) - 1);
                } else {
                    event.Enable(false);
                }
            }
        }

        void ModEditor::OnFilterBoxChanged(wxCommandEvent& event) {
            updateMods();
        }

        void ModEditor::createGui() {
            wxStaticText* availableModListTitle = new wxStaticText(this, wxID_ANY, _("Available"));
            wxStaticText* enabledModListTitle = new wxStaticText(this, wxID_ANY, _("Enabled"));
#if defined __APPLE__
            availableModListTitle->SetFont(*wxSMALL_FONT);
            enabledModListTitle->SetFont(*wxSMALL_FONT);
#endif
            
            m_availableModList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
            m_enabledModList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
            
            m_filterBox = new wxSearchCtrl(this, wxID_ANY);
            m_filterBox->SetToolTip(_("Filter the list of available mods"));
            
            const wxBitmap addBitmap = IO::loadImageResource(IO::Path("images/Add.png"));
            const wxBitmap removeBitmap = IO::loadImageResource(IO::Path("images/Remove.png"));
            const wxBitmap upBitmap = IO::loadImageResource(IO::Path("images/Up.png"));
            const wxBitmap downBitmap = IO::loadImageResource(IO::Path("images/Down.png"));
            
            m_addModsButton = new wxBitmapButton(this, wxID_ANY, addBitmap);
            m_addModsButton->SetToolTip(_("Add the selected available mods to the list of enabled mods"));
            m_removeModsButton = new wxBitmapButton(this, wxID_ANY, removeBitmap);
            m_removeModsButton->SetToolTip(_("Remove the selected items from the list of enabled mods"));
            m_moveModUpButton = new wxBitmapButton(this, wxID_ANY, upBitmap);
            m_moveModUpButton->SetToolTip(_("Move the selected mod up in the list of enabled mods"));
            m_moveModDownButton = new wxBitmapButton(this, wxID_ANY, downBitmap);
            m_moveModDownButton->SetToolTip(_("Move the selected mod down in the list of enabled mods"));
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(m_addModsButton);
            buttonSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin / 2);
            buttonSizer->Add(m_removeModsButton);
            buttonSizer->AddStretchSpacer();
            buttonSizer->Add(m_moveModUpButton);
            buttonSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin / 2);
            buttonSizer->Add(m_moveModDownButton);
            
            wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::ControlVerticalMargin, LayoutConstants::ControlHorizontalMargin);
            sizer->Add(availableModListTitle, wxGBPosition(0, 0));
            sizer->Add(enabledModListTitle, wxGBPosition(0, 1));
            sizer->Add(m_availableModList, wxGBPosition(1, 0), wxDefaultSpan, wxEXPAND);
            sizer->Add(m_enabledModList, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
            sizer->Add(m_filterBox, wxGBPosition(2, 0), wxDefaultSpan, wxEXPAND);
            sizer->Add(buttonSizer, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);
            sizer->SetItemMinSize(m_availableModList, 100, 100);
            sizer->SetItemMinSize(m_enabledModList, 100, 100);
            sizer->AddGrowableCol(0);
            sizer->AddGrowableCol(1);
            sizer->AddGrowableRow(1);
            SetSizerAndFit(sizer);
        }
        
        void ModEditor::bindEvents() {
            m_filterBox->Bind(wxEVT_TEXT, &ModEditor::OnFilterBoxChanged, this);
            m_addModsButton->Bind(wxEVT_BUTTON, &ModEditor::OnAddModClicked, this);
            m_removeModsButton->Bind(wxEVT_BUTTON, &ModEditor::OnRemoveModClicked, this);
            m_moveModUpButton->Bind(wxEVT_BUTTON, &ModEditor::OnMoveModUpClicked, this);
            m_moveModDownButton->Bind(wxEVT_BUTTON, &ModEditor::OnMoveModDownClicked, this);
            m_addModsButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateButtonUI, this);
            m_removeModsButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateButtonUI, this);
            m_moveModUpButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateButtonUI, this);
            m_moveModDownButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateButtonUI, this);
        }

        void ModEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &ModEditor::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &ModEditor::documentWasLoaded);
            document->modsDidChangeNotifier.addObserver(this, &ModEditor::modsDidChange);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &ModEditor::preferenceDidChange);
        }
        
        void ModEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &ModEditor::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &ModEditor::documentWasLoaded);
                document->modsDidChangeNotifier.removeObserver(this, &ModEditor::modsDidChange);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &ModEditor::preferenceDidChange);
        }
        
        void ModEditor::documentWasNewed() {
            updateAvailableMods();
            updateMods();
        }
        
        void ModEditor::documentWasLoaded() {
            updateAvailableMods();
            updateMods();
        }
        
        void ModEditor::modsDidChange() {
            if (!m_ignoreNotifier)
                updateMods();
        }

        void ModEditor::preferenceDidChange(const IO::Path& path) {
            MapDocumentSPtr document = lock(m_document);
            if (document->isGamePathPreference(path)) {
                updateAvailableMods();
                updateMods();
            }
        }

        void ModEditor::updateAvailableMods() {
            MapDocumentSPtr document = lock(m_document);
            StringList availableMods = document->game()->availableMods();
            StringUtils::sortCaseInsensitive(availableMods);

            m_availableMods.clear();
            m_availableMods.reserve(availableMods.size());
            for (size_t i = 0; i < availableMods.size(); ++i)
                m_availableMods.push_back(availableMods[i]);
        }

        void ModEditor::updateMods() {
            const String pattern = m_filterBox->GetValue().ToStdString();
            
            MapDocumentSPtr document = lock(m_document);
            StringList enabledMods = document->mods();
            
            wxArrayString availableModItems;
            for (size_t i = 0; i < m_availableMods.size(); ++i) {
                const String& mod = m_availableMods[i];
                if (StringUtils::containsCaseInsensitive(mod, pattern) &&
                    !VectorUtils::contains(enabledMods, mod))
                    availableModItems.Add(mod);
            }

            wxArrayString enabledModItems;
            for (size_t i = 0; i < enabledMods.size(); ++i)
                if (StringUtils::containsCaseInsensitive(enabledMods[i], pattern))
                    enabledModItems.Add(enabledMods[i]);
            
            m_availableModList->Set(availableModItems);
            m_enabledModList->Set(enabledModItems);
        }
    }
}
