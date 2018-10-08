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

#include "FileTextureCollectionEditor.h"

#include "PreferenceManager.h"
#include "Assets/TextureManager.h"
#include "Assets/TextureCollection.h"
#include "View/BorderLine.h"
#include "View/ChoosePathTypeDialog.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <wx/bmpbuttn.h>
#include <wx/filedlg.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        FileTextureCollectionEditor::FileTextureCollectionEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            createGui();
            bindObservers();
            updateControls();
        }
        
        FileTextureCollectionEditor::~FileTextureCollectionEditor() {
            unbindObservers();
        }

        bool FileTextureCollectionEditor::debugUIConsistency() const {
            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();

            assert(m_collections->GetCount() == collections.size());

            wxArrayInt selectedIndices;
            m_collections->GetSelections(selectedIndices);
            for (size_t i = 0; i < selectedIndices.size(); ++i) {
                assert(selectedIndices[i] >= 0);
                assert(static_cast<size_t>(selectedIndices[i]) < collections.size());
            }
            return true;
        }

        bool FileTextureCollectionEditor::canRemoveTextureCollections() const {
            assert(debugUIConsistency());

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            if (selections.empty()) {
                return false;
            }

            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();
            for (size_t i = 0; i < selections.size(); ++i) {
                const auto index = static_cast<size_t>(selections[i]);
                if (index >= collections.size()) {
                    return false;
                }
            }

            return true;
        }

        bool FileTextureCollectionEditor::canMoveTextureCollectionsUp() const {
            assert(debugUIConsistency());

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            if (selections.size() != 1) {
                return false;
            }

            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();

            const auto index = static_cast<size_t>(selections.front());
            return index >= 1 && index < collections.size();
        }

        bool FileTextureCollectionEditor::canMoveTextureCollectionsDown() const {
            assert(debugUIConsistency());

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            if (selections.size() != 1) {
                return false;
            }

            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();

            const auto index = static_cast<size_t>(selections.front());
            return (index + 1) < collections.size();
        }

        void FileTextureCollectionEditor::OnAddTextureCollectionsClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const wxString pathWxStr = ::wxFileSelector("Load Texture Collection", wxEmptyString, wxEmptyString, wxEmptyString, "", wxFD_OPEN);
            if (pathWxStr.empty())
                return;
            
            loadTextureCollection(m_document, this, pathWxStr);
        }
        
        void FileTextureCollectionEditor::OnRemoveTextureCollectionsClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            if (!canRemoveTextureCollections()) {
                return;
            }

            wxArrayInt selections;
            m_collections->GetSelections(selections);

            auto document = lock(m_document);

            auto collections = document->enabledTextureCollections();
            decltype(collections) toRemove;
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const auto index = static_cast<size_t>(selections[i]);
                ensure(index < collections.size(), "index out of range");
                toRemove.push_back(collections[index]);
            }

            VectorUtils::eraseAll(collections, toRemove);
            document->setEnabledTextureCollections(collections);
        }
        
        void FileTextureCollectionEditor::OnMoveTextureCollectionUpClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            if (!canMoveTextureCollectionsUp()) {
                return;
            }

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(selections.size() == 1);

            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();
            
            const auto index = static_cast<size_t>(selections.front());
            VectorUtils::swapPred(collections, index);
            
            document->setEnabledTextureCollections(collections);
            m_collections->SetSelection(static_cast<int>(index - 1));
        }
        
        void FileTextureCollectionEditor::OnMoveTextureCollectionDownClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            if (!canMoveTextureCollectionsDown()) {
                return;
            }

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(selections.size() == 1);

            auto document = lock(m_document);
            auto collections = document->enabledTextureCollections();
            
            const auto index = static_cast<size_t>(selections.front());
            VectorUtils::swapSucc(collections, index);
            
            document->setEnabledTextureCollections(collections);
            m_collections->SetSelection(static_cast<int>(index + 1));
        }

        void FileTextureCollectionEditor::OnReloadTextureCollectionsClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto document = lock(m_document);
            document->reloadTextureCollections();
        }

        void FileTextureCollectionEditor::OnUpdateRemoveButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Enable(canRemoveTextureCollections());
        }
        
        void FileTextureCollectionEditor::OnUpdateMoveUpButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Enable(canMoveTextureCollectionsUp());
        }
        
        void FileTextureCollectionEditor::OnUpdateMoveDownButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Enable(canMoveTextureCollectionsDown());
        }

        void FileTextureCollectionEditor::OnUpdateReloadTextureCollectionsButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Enable(!m_collections->IsEmpty());
        }

        void FileTextureCollectionEditor::createGui() {
            m_collections = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_MULTIPLE | wxBORDER_NONE);

            auto* addTextureCollectionsButton = createBitmapButton(this, "Add.png", "Add texture collections from the file system");
            auto* removeTextureCollectionsButton = createBitmapButton(this, "Remove.png", "Remove the selected texture collections");
            auto* moveTextureCollectionUpButton = createBitmapButton(this, "Up.png", "Move the selected texture collection up");
            auto* moveTextureCollectionDownButton = createBitmapButton(this, "Down.png", "Move the selected texture collection down");
            auto* reloadTextureCollectionsButton = createBitmapButton(this, "Refresh.png", "Reload all texture collections");
            
            addTextureCollectionsButton->Bind(wxEVT_BUTTON, &FileTextureCollectionEditor::OnAddTextureCollectionsClicked, this);
            removeTextureCollectionsButton->Bind(wxEVT_BUTTON, &FileTextureCollectionEditor::OnRemoveTextureCollectionsClicked, this);
            moveTextureCollectionUpButton->Bind(wxEVT_BUTTON, &FileTextureCollectionEditor::OnMoveTextureCollectionUpClicked, this);
            moveTextureCollectionDownButton->Bind(wxEVT_BUTTON, &FileTextureCollectionEditor::OnMoveTextureCollectionDownClicked, this);
            reloadTextureCollectionsButton->Bind(wxEVT_BUTTON, &FileTextureCollectionEditor::OnReloadTextureCollectionsClicked, this);
            removeTextureCollectionsButton->Bind(wxEVT_UPDATE_UI, &FileTextureCollectionEditor::OnUpdateRemoveButtonUI, this);
            moveTextureCollectionUpButton->Bind(wxEVT_UPDATE_UI, &FileTextureCollectionEditor::OnUpdateMoveUpButtonUI, this);
            moveTextureCollectionDownButton->Bind(wxEVT_UPDATE_UI, &FileTextureCollectionEditor::OnUpdateMoveDownButtonUI, this);
            reloadTextureCollectionsButton->Bind(wxEVT_UPDATE_UI, &FileTextureCollectionEditor::OnUpdateReloadTextureCollectionsButtonUI, this);

            auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addTextureCollectionsButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(removeTextureCollectionsButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(moveTextureCollectionUpButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(moveTextureCollectionDownButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(reloadTextureCollectionsButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();
            
            auto* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_collections, 1, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            sizer->SetItemMinSize(m_collections, 100, 70);
            
            SetSizerAndFit(sizer);
        }
        
        void FileTextureCollectionEditor::bindObservers() {
            auto document = lock(m_document);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &FileTextureCollectionEditor::textureCollectionsDidChange);
            
            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &FileTextureCollectionEditor::preferenceDidChange);
        }
        
        void FileTextureCollectionEditor::unbindObservers() {
            if (!expired(m_document)) {
                auto document = lock(m_document);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &FileTextureCollectionEditor::textureCollectionsDidChange);
            }
            
            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &FileTextureCollectionEditor::preferenceDidChange);
        }
        
        void FileTextureCollectionEditor::textureCollectionsDidChange() {
            updateControls();
        }
        
        void FileTextureCollectionEditor::preferenceDidChange(const IO::Path& path) {
            auto document = lock(m_document);
            if (document->isGamePathPreference(path))
                updateControls();
        }

        void FileTextureCollectionEditor::updateControls() {
            m_collections->Clear();
            
            auto document = lock(m_document);
            for (const auto& path : document->enabledTextureCollections())
                m_collections->Append(path.asString());
        }
    }
}
