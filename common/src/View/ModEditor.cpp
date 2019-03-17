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

#include "ModEditor.h"

#include "Notifier.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Model/Entity.h"
#include "Model/Game.h"
#include "Model/Object.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <wx/bmpbuttn.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/srchctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ModEditor::ModEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document),
        m_availableModList(nullptr),
        m_enabledModList(nullptr),
        m_filterBox(nullptr),
        m_ignoreNotifier(false) {
            InheritAttributes();
            createGui();
            bindObservers();
        }

        ModEditor::~ModEditor() {
            unbindObservers();
        }

        bool ModEditor::ShouldInheritColours() const {
            return true;
        }

        void ModEditor::OnAddModClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            TemporarilySetBool ignoreNotifier(m_ignoreNotifier);

            wxArrayInt selections;
            m_availableModList->GetSelections(selections);
            if (selections.empty()) {
                return;
            }

            auto document = lock(m_document);

            auto mods = document->mods();
            for (size_t i = 0; i < selections.size(); ++i) {
                const auto index = selections[i] - static_cast<int>(i);
                const auto item = m_availableModList->GetString(static_cast<unsigned int>(index));
                m_availableModList->Delete(static_cast<unsigned int>(index));
                m_enabledModList->Append(item);

                mods.push_back(item.ToStdString());
            }

            m_availableModList->DeselectAll();
            m_enabledModList->DeselectAll();

            document->setMods(mods);
        }

        void ModEditor::OnRemoveModClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            m_enabledModList->GetSelections(selections);
            if (selections.empty()) {
                return;
            }

            auto document = lock(m_document);
            auto mods = document->mods();
            std::sort(std::begin(selections), std::end(selections));

            for (auto it = selections.rbegin(), end = selections.rend(); it != end; ++it) {
                const auto index = static_cast<size_t>(*it);
                const auto mod = mods[index];
                mods.erase(std::begin(mods) + *it);
            }

            document->setMods(mods);
        }

        void ModEditor::OnMoveModUpClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            m_enabledModList->GetSelections(selections);
            assert(selections.size() == 1);

            auto document = lock(m_document);
            auto mods = document->mods();

            const auto index = static_cast<size_t>(selections.front());
            ensure(index > 0 && index < mods.size(), "index out of range");

            using std::swap;
            swap(mods[index - 1], mods[index]);
            document->setMods(mods);

            m_enabledModList->DeselectAll();
            m_enabledModList->SetSelection(static_cast<int>(index - 1));
        }

        void ModEditor::OnMoveModDownClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            m_enabledModList->GetSelections(selections);
            assert(selections.size() == 1);

            auto document = lock(m_document);
            auto mods = document->mods();

            const auto index = static_cast<size_t>(selections.front());
            ensure(index < mods.size() - 1, "index out of range");

            using std::swap;
            swap(mods[index + 1], mods[index]);
            document->setMods(mods);

            m_enabledModList->DeselectAll();
            m_enabledModList->SetSelection(static_cast<int>(index + 1));
        }

        void ModEditor::OnUpdateAddButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            event.Enable(m_availableModList->GetSelections(selections) > 0);
        }

        void ModEditor::OnUpdateRemoveButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            event.Enable(m_enabledModList->GetSelections(selections) > 0);
        }

        void ModEditor::OnUpdateMoveUpButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            event.Enable(m_enabledModList->GetSelections(selections) == 1 && selections.front() > 0);
        }

        void ModEditor::OnUpdateMoveDownButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            const auto enabledModCount = static_cast<int>(m_enabledModList->GetCount());
            wxArrayInt selections;
            event.Enable(m_enabledModList->GetSelections(selections) == 1 && selections.front() < enabledModCount - 1);
        }

        void ModEditor::OnFilterBoxChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            updateMods();
        }

        void ModEditor::createGui() {
            auto* availableModContainer = new TitledPanel(this, "Available", false);
            availableModContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_availableModList = new wxListBox(availableModContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_MULTIPLE | wxBORDER_NONE);

            auto* availableModContainerSizer = new wxBoxSizer(wxVERTICAL);
            availableModContainerSizer->Add(m_availableModList, wxSizerFlags().Expand().Proportion(1));
            availableModContainer->getPanel()->SetSizer(availableModContainerSizer);

            m_filterBox = new wxSearchCtrl(this, wxID_ANY);
            m_filterBox->SetToolTip("Filter the list of available mods");
            m_filterBox->SetFont(m_availableModList->GetFont());

            auto* filterBoxSizer = new wxBoxSizer(wxVERTICAL);
            filterBoxSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            filterBoxSizer->Add(m_filterBox, wxSizerFlags().Expand());
            filterBoxSizer->AddSpacer(LayoutConstants::NarrowVMargin);

            auto* enabledModContainer = new TitledPanel(this, "Enabled", false);
            enabledModContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_enabledModList = new wxListBox(enabledModContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_MULTIPLE | wxBORDER_NONE);

            auto* enabledModContainerSizer = new wxBoxSizer(wxVERTICAL);
            enabledModContainerSizer->Add(m_enabledModList, wxSizerFlags().Expand().Proportion(1));
            enabledModContainer->getPanel()->SetSizer(enabledModContainerSizer);

            auto* addModsButton = createBitmapButton(this, "Add.png", "Enable the selected mods");
            auto* removeModsButton = createBitmapButton(this, "Remove.png", "Disable the selected mods");
            auto* moveModUpButton = createBitmapButton(this, "Up.png", "Move the selected mod up");
            auto* moveModDownButton = createBitmapButton(this, "Down.png", "Move the selected mod down");

            auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addModsButton, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->Add(removeModsButton, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(moveModUpButton, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->Add(moveModDownButton, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->AddStretchSpacer();

            auto* sizer = new wxGridBagSizer(0, 0);
            sizer->Add(availableModContainer,                                   wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Vertical),    wxGBPosition(0, 1), wxGBSpan(3, 1), wxEXPAND);
            sizer->Add(enabledModContainer,                                     wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal),  wxGBPosition(1, 0), wxGBSpan(1, 3), wxEXPAND);
            sizer->Add(filterBoxSizer,                                          wxGBPosition(2, 0), wxDefaultSpan, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            sizer->Add(buttonSizer,                                             wxGBPosition(2, 2), wxDefaultSpan, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            sizer->SetItemMinSize(availableModContainer, 100, 100);
            sizer->SetItemMinSize(enabledModContainer, 100, 100);
            sizer->AddGrowableCol(0);
            sizer->AddGrowableCol(2);
            sizer->AddGrowableRow(1);

            SetSizerAndFit(sizer);

            m_availableModList->Bind(wxEVT_LISTBOX_DCLICK, &ModEditor::OnAddModClicked, this);
            m_enabledModList->Bind(wxEVT_LISTBOX_DCLICK, &ModEditor::OnRemoveModClicked, this);
            m_filterBox->Bind(wxEVT_TEXT, &ModEditor::OnFilterBoxChanged, this);
            addModsButton->Bind(wxEVT_BUTTON, &ModEditor::OnAddModClicked, this);
            removeModsButton->Bind(wxEVT_BUTTON, &ModEditor::OnRemoveModClicked, this);
            moveModUpButton->Bind(wxEVT_BUTTON, &ModEditor::OnMoveModUpClicked, this);
            moveModDownButton->Bind(wxEVT_BUTTON, &ModEditor::OnMoveModDownClicked, this);
            addModsButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateAddButtonUI, this);
            removeModsButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateRemoveButtonUI, this);
            moveModUpButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateMoveUpButtonUI, this);
            moveModDownButton->Bind(wxEVT_UPDATE_UI, &ModEditor::OnUpdateMoveDownButtonUI, this);

        }

        void ModEditor::bindObservers() {
            auto document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &ModEditor::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &ModEditor::documentWasLoaded);
            document->modsDidChangeNotifier.addObserver(this, &ModEditor::modsDidChange);

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &ModEditor::preferenceDidChange);
        }

        void ModEditor::unbindObservers() {
            if (!expired(m_document)) {
                auto document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &ModEditor::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &ModEditor::documentWasLoaded);
                document->modsDidChangeNotifier.removeObserver(this, &ModEditor::modsDidChange);
            }

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &ModEditor::preferenceDidChange);
        }

        void ModEditor::documentWasNewed(MapDocument* document) {
            updateAvailableMods();
            updateMods();
        }

        void ModEditor::documentWasLoaded(MapDocument* document) {
            updateAvailableMods();
            updateMods();
        }

        void ModEditor::modsDidChange() {
            if (!m_ignoreNotifier)
                updateMods();
        }

        void ModEditor::preferenceDidChange(const IO::Path& path) {
            auto document = lock(m_document);
            if (document->isGamePathPreference(path)) {
                updateAvailableMods();
                updateMods();
            }
        }

        void ModEditor::updateAvailableMods() {
            auto document = lock(m_document);
            StringList availableMods = document->game()->availableMods();
            StringUtils::sortCaseInsensitive(availableMods);

            m_availableMods.clear();
            m_availableMods.reserve(availableMods.size());
            for (size_t i = 0; i < availableMods.size(); ++i) {
                m_availableMods.push_back(availableMods[i]);
            }
        }

        void ModEditor::updateMods() {
            const auto pattern = m_filterBox->GetValue().ToStdString();

            auto document = lock(m_document);
            auto enabledMods = document->mods();

            wxArrayString availableModItems;
            for (size_t i = 0; i < m_availableMods.size(); ++i) {
                const auto& mod = m_availableMods[i];
                if (StringUtils::containsCaseInsensitive(mod, pattern) &&
                    !VectorUtils::contains(enabledMods, mod)) {
                    availableModItems.Add(mod);
                }
            }

            wxArrayString enabledModItems;
            for (size_t i = 0; i < enabledMods.size(); ++i) {
                if (StringUtils::containsCaseInsensitive(enabledMods[i], pattern)) {
                    enabledModItems.Add(enabledMods[i]);
                }
            }

            m_availableModList->Set(availableModItems);
            m_enabledModList->Set(enabledModItems);
        }
    }
}
