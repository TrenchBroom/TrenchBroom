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

#include "DirectoryTextureCollectionEditor.h"

#include "PreferenceManager.h"
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
#include <QLabel>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        DirectoryTextureCollectionEditor::DirectoryTextureCollectionEditor(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document),
        m_availableCollectionsList(nullptr),
        m_enabledCollectionsList(nullptr) {
            createGui();
            bindObservers();
            update();
        }

		DirectoryTextureCollectionEditor::~DirectoryTextureCollectionEditor() {
			unbindObservers();
		}

		void DirectoryTextureCollectionEditor::OnAddTextureCollections(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const auto availableCollections = availableTextureCollections();
            auto enabledCollections = enabledTextureCollections();

            wxArrayInt selections;
            m_availableCollectionsList->GetSelections(selections);

            for (size_t i = 0; i < selections.size(); ++i) {
                const auto index = static_cast<size_t>(selections[i]);
                enabledCollections.push_back(availableCollections[index]);
            }

            VectorUtils::sortAndRemoveDuplicates(enabledCollections);

            MapDocumentSPtr document = lock(m_document);
            document->setEnabledTextureCollections(enabledCollections);
        }

        void DirectoryTextureCollectionEditor::OnRemoveTextureCollections(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto enabledCollections = enabledTextureCollections();

            wxArrayInt selections;
            m_enabledCollectionsList->GetSelections(selections);

            // erase back to front
            for (auto sIt = std::rbegin(selections), sEnd = std::rend(selections); sIt != sEnd; ++sIt) {
                const auto index = static_cast<size_t>(*sIt);
                VectorUtils::erase(enabledCollections, index);
            }

            auto document = lock(m_document);
            document->setEnabledTextureCollections(enabledCollections);
        }

        void DirectoryTextureCollectionEditor::OnReloadTextureCollections(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto document = lock(m_document);
            document->reloadTextureCollections();
        }

        void DirectoryTextureCollectionEditor::OnUpdateAddTextureCollections(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            event.Enable(m_availableCollectionsList->GetSelections(selections) > 0);
        }

        void DirectoryTextureCollectionEditor::OnUpdateRemoveTextureCollections(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            event.Enable(m_enabledCollectionsList->GetSelections(selections) > 0);
        }

        void DirectoryTextureCollectionEditor::OnUpdateReloadTextureCollections(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Enable(!m_enabledCollectionsList->IsEmpty());
        }

        void DirectoryTextureCollectionEditor::createGui() {
            auto* availableCollectionsContainer = new TitledPanel(this, "Available", false);
            availableCollectionsContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));

            m_availableCollectionsList = new wxListBox(availableCollectionsContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_MULTIPLE | wxBORDER_NONE);

            auto* availableModContainerSizer = new QVBoxLayout();
            availableModContainerSizer->Add(m_availableCollectionsList, wxSizerFlags().Expand().Proportion(1));
            availableCollectionsContainer->getPanel()->SetSizer(availableModContainerSizer);

            auto* enabledCollectionsContainer = new TitledPanel(this, "Enabled", false);
            enabledCollectionsContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_enabledCollectionsList = new wxListBox(enabledCollectionsContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_MULTIPLE | wxBORDER_NONE);

            auto* enabledCollectionsContainerSizer = new QVBoxLayout();
            enabledCollectionsContainerSizer->Add(m_enabledCollectionsList, wxSizerFlags().Expand().Proportion(1));
            enabledCollectionsContainer->getPanel()->SetSizer(enabledCollectionsContainerSizer);

            auto* addCollectionsButton = createBitmapButton(this, "Add.png", "Enable the selected texture collections");
            auto* removeCollectionsButton = createBitmapButton(this, "Remove.png", "Disable the selected texture collections");
            auto* reloadCollectionsButton = createBitmapButton(this, "Refresh.png", "Reload all enabled texture collections");

            auto* buttonSizer = new QHBoxLayout();
            buttonSizer->Add(addCollectionsButton, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->Add(removeCollectionsButton, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->addSpacing(LayoutConstants::WideHMargin);
            buttonSizer->Add(reloadCollectionsButton, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->AddStretchSpacer();

            auto* sizer = new wxGridBagSizer(0, 0);
            sizer->addWidget(availableCollectionsContainer,                           wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
            sizer->addWidget(new BorderLine(this, BorderLine::Direction_Vertical),    wxGBPosition(0, 1), wxGBSpan(3, 1), wxEXPAND);
            sizer->addWidget(enabledCollectionsContainer,                             wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND);
            sizer->addWidget(new BorderLine(this, BorderLine::Direction_Horizontal),  wxGBPosition(1, 0), wxGBSpan(1, 3), wxEXPAND);
            sizer->addWidget(buttonSizer,                                             wxGBPosition(2, 2), wxDefaultSpan, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            sizer->SetItemMinSize(availableCollectionsContainer, 100, 100);
            sizer->SetItemMinSize(enabledCollectionsContainer, 100, 100);
            sizer->AddGrowableCol(0);
            sizer->AddGrowableCol(2);
            sizer->AddGrowableRow(1);

            setLayout(sizer);

            m_availableCollectionsList->Bind(wxEVT_LISTBOX_DCLICK, &DirectoryTextureCollectionEditor::OnAddTextureCollections, this);
            m_enabledCollectionsList->Bind(wxEVT_LISTBOX_DCLICK, &DirectoryTextureCollectionEditor::OnRemoveTextureCollections, this);
            addCollectionsButton->Bind(wxEVT_BUTTON, &DirectoryTextureCollectionEditor::OnAddTextureCollections, this);
            removeCollectionsButton->Bind(wxEVT_BUTTON, &DirectoryTextureCollectionEditor::OnRemoveTextureCollections, this);
            reloadCollectionsButton->Bind(wxEVT_BUTTON, &DirectoryTextureCollectionEditor::OnReloadTextureCollections, this);
            addCollectionsButton->Bind(wxEVT_UPDATE_UI, &DirectoryTextureCollectionEditor::OnUpdateAddTextureCollections, this);
            removeCollectionsButton->Bind(wxEVT_UPDATE_UI, &DirectoryTextureCollectionEditor::OnUpdateRemoveTextureCollections, this);
            reloadCollectionsButton->Bind(wxEVT_UPDATE_UI, &DirectoryTextureCollectionEditor::OnUpdateReloadTextureCollections, this);
        }

        void DirectoryTextureCollectionEditor::bindObservers() {
            auto document = lock(m_document);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
            document->modsDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::modsDidChange);

            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::preferenceDidChange);
        }

        void DirectoryTextureCollectionEditor::unbindObservers() {
            if (!expired(m_document)) {
                auto document = lock(m_document);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::modsDidChange);
            }

            auto& prefs = PreferenceManager::instance();
            assertResult(prefs.preferenceDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::preferenceDidChange));
        }

        void DirectoryTextureCollectionEditor::textureCollectionsDidChange() {
            update();
        }

        void DirectoryTextureCollectionEditor::modsDidChange() {
            update();
        }

        void DirectoryTextureCollectionEditor::preferenceDidChange(const IO::Path& path) {
            auto document = lock(m_document);
            if (document->isGamePathPreference(path)) {
                update();
            }
        }

        void DirectoryTextureCollectionEditor::update() {
            updateAvailableTextureCollections();
            updateEnabledTextureCollections();
        }

        void DirectoryTextureCollectionEditor::updateAvailableTextureCollections() {
            updateListBox(m_availableCollectionsList, availableTextureCollections());
        }

        void DirectoryTextureCollectionEditor::updateEnabledTextureCollections() {
            updateListBox(m_enabledCollectionsList, enabledTextureCollections());
        }

        void DirectoryTextureCollectionEditor::updateListBox(wxListBox* box, const IO::Path::List& paths) {
            QStringList values;
            values.reserve(paths.size());

            for (const auto& path : paths) {
                values.push_back(path.asString());
            }

            box->Set(values);
        }

        IO::Path::List DirectoryTextureCollectionEditor::availableTextureCollections() const {
            auto document = lock(m_document);
            auto availableCollections = document->availableTextureCollections();
            VectorUtils::eraseAll(availableCollections, document->enabledTextureCollections());
            return availableCollections;
        }

        IO::Path::List DirectoryTextureCollectionEditor::enabledTextureCollections() const {
            auto document = lock(m_document);
            return document->enabledTextureCollections();
        }
    }
}
