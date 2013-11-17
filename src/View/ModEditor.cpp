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

#include "ModEditor.h"

#include "Notifier.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Entity.h"
#include "Model/Game.h"
#include "Model/Object.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/LayoutConstants.h"
#include "View/MapDocument.h"

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/gbsizer.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        ModEditor::ModEditor(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller) :
        wxCollapsiblePane(parent, wxID_ANY, _("Mods"), wxDefaultPosition, wxDefaultSize, wxCP_NO_TLW_RESIZE | wxTAB_TRAVERSAL | wxBORDER_NONE),
        m_document(document),
        m_controller(controller) {
            createGui();
            bindEvents();
            bindObservers();
        }

        ModEditor::~ModEditor() {
            unbindObservers();
        }

        void ModEditor::OnPaneChanged(wxCollapsiblePaneEvent& event) {
            GetParent()->Layout();
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
                        event.Enable(selections.front() < static_cast<size_t>(m_enabledModList->GetCount()) - 1);
                } else {
                    event.Enable(false);
                }
            }
        }

        void ModEditor::OnFilterBoxChanged(wxCommandEvent& event) {
            updateMods();
        }

        void ModEditor::createGui() {
            wxStaticText* availableModListTitle = new wxStaticText(GetPane(), wxID_ANY, _("Available"));
            wxStaticText* enabledModListTitle = new wxStaticText(GetPane(), wxID_ANY, _("Enabled"));
#if defined __APPLE__
            availableModListTitle->SetFont(*wxSMALL_FONT);
            enabledModListTitle->SetFont(*wxSMALL_FONT);
#endif
            
            m_availableModList = new wxListBox(GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
            m_enabledModList = new wxListBox(GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE);
            
            m_filterBox = new wxSearchCtrl(GetPane(), wxID_ANY);
            m_filterBox->SetToolTip(_("Filter the list of available mods"));
            
            const wxBitmap addBitmap = IO::loadImageResource(IO::Path("images/Add.png"));
            const wxBitmap removeBitmap = IO::loadImageResource(IO::Path("images/Remove.png"));
            const wxBitmap upBitmap = IO::loadImageResource(IO::Path("images/Up.png"));
            const wxBitmap downBitmap = IO::loadImageResource(IO::Path("images/Down.png"));
            
            m_addModsButton = new wxBitmapButton(GetPane(), wxID_ANY, addBitmap);
            m_addModsButton->SetToolTip(_("Add the selected available mods to the list of enabled mods"));
            m_removeModsButton = new wxBitmapButton(GetPane(), wxID_ANY, removeBitmap);
            m_removeModsButton->SetToolTip(_("Remove the selected items from the list of enabled mods"));
            m_moveModUpButton = new wxBitmapButton(GetPane(), wxID_ANY, upBitmap);
            m_moveModUpButton->SetToolTip(_("Move the selected mod up in the list of enabled mods"));
            m_moveModDownButton = new wxBitmapButton(GetPane(), wxID_ANY, downBitmap);
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
            GetPane()->SetSizerAndFit(sizer);
        }
        
        void ModEditor::bindEvents() {
            Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &ModEditor::OnPaneChanged, this);
            m_filterBox->Bind(wxEVT_TEXT, &ModEditor::OnFilterBoxChanged, this);
            m_addModsButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateButtonUI, this);
            m_removeModsButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateButtonUI, this);
            m_moveModUpButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateButtonUI, this);
            m_moveModDownButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateButtonUI, this);
        }

        void ModEditor::bindObservers() {
            m_document->documentWasNewedNotifier.addObserver(this, &ModEditor::documentWasNewed);
            m_document->documentWasLoadedNotifier.addObserver(this, &ModEditor::documentWasLoaded);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &ModEditor::preferenceDidChange);
        }
        
        void ModEditor::unbindObservers() {
            m_document->documentWasNewedNotifier.removeObserver(this, &ModEditor::documentWasNewed);
            m_document->documentWasLoadedNotifier.removeObserver(this, &ModEditor::documentWasLoaded);
            
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
        
        void ModEditor::objectDidChange(Model::Object* object) {
            if (object->type() == Model::Object::OTEntity) {
                const Model::Entity* entity = static_cast<Model::Entity*>(object);
                if (entity->worldspawn())
                    updateMods();
            }
        }

        void ModEditor::preferenceDidChange(const IO::Path& path) {
            if (m_document->isGamePathPreference(path)) {
                updateAvailableMods();
                updateMods();
            }
        }

        void ModEditor::updateAvailableMods() {
            StringList availableMods = m_document->game()->availableMods();
            StringUtils::sortCaseInsensitive(availableMods);

            m_availableMods.clear();
            m_availableMods.reserve(availableMods.size());
            for (size_t i = 0; i < availableMods.size(); ++i)
                m_availableMods.push_back(availableMods[i]);
        }

        void ModEditor::updateMods() {
            const String pattern = m_filterBox->GetValue().ToStdString();
            
            StringList enabledMods = m_document->mods();
            StringUtils::sortCaseInsensitive(enabledMods);

            
            wxArrayString availableModItems;
            for (size_t i = 0; i < m_availableMods.size(); ++i)
                if (StringUtils::containsCaseInsensitive(m_availableMods[i], pattern))
                    availableModItems.Add(m_availableMods[i]);

            wxArrayString enabledModItems;
            for (size_t i = 0; i < enabledMods.size(); ++i)
                if (StringUtils::containsCaseInsensitive(enabledMods[i], pattern))
                    enabledModItems.Add(enabledMods[i]);
            
            m_availableModList->Set(availableModItems);
            m_enabledModList->Set(enabledModItems);
        }
    }
}
