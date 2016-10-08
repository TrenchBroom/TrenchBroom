/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
        
        void FileTextureCollectionEditor::OnAddTextureCollectionsClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const wxString pathWxStr = ::wxFileSelector("Load Texture Collection", wxEmptyString, wxEmptyString, wxEmptyString, "", wxFD_OPEN);
            if (pathWxStr.empty())
                return;
            
            loadTextureCollection(m_document, this, pathWxStr);
        }
        
        void FileTextureCollectionEditor::OnRemoveTextureCollectionsClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(!selections.empty());
            
            MapDocumentSPtr document = lock(m_document);

            IO::Path::List collections = document->enabledTextureCollections();
            IO::Path::List toRemove;
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const size_t index = static_cast<size_t>(selections[i]);
                ensure(index < collections.size(), "index out of range");
                toRemove.push_back(collections[index]);
            }

            VectorUtils::eraseAll(collections, toRemove);
            document->setEnabledTextureCollections(collections);
        }
        
        void FileTextureCollectionEditor::OnMoveTextureCollectionUpClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(selections.size() == 1);
            
            MapDocumentSPtr document = lock(m_document);
            IO::Path::List collections = document->enabledTextureCollections();
            
            const size_t index = static_cast<size_t>(selections.front());
            VectorUtils::swapPred(collections, index);
            
            document->setEnabledTextureCollections(collections);
            m_collections->SetSelection(static_cast<int>(index - 1));
        }
        
        void FileTextureCollectionEditor::OnMoveTextureCollectionDownClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(selections.size() == 1);
            
            MapDocumentSPtr document = lock(m_document);
            IO::Path::List collections = document->enabledTextureCollections();
            
            const size_t index = static_cast<size_t>(selections.front());
            VectorUtils::swapSucc(collections, index);
            
            document->setEnabledTextureCollections(collections);
            m_collections->SetSelection(static_cast<int>(index + 1));
        }

        void FileTextureCollectionEditor::OnUpdateRemoveButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

                wxArrayInt selections;
                event.Enable(m_collections->GetSelections(selections) > 0);
        }
        
        void FileTextureCollectionEditor::OnUpdateMoveUpButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            event.Enable(m_collections->GetSelections(selections) == 1 && selections.front() > 0);
        }
        
        void FileTextureCollectionEditor::OnUpdateMoveDownButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            const int collectionCount = static_cast<int>(m_collections->GetCount());
            wxArrayInt selections;
            event.Enable(m_collections->GetSelections(selections) == 1 && selections.front() < collectionCount - 1);
        }
        
        void FileTextureCollectionEditor::createGui() {
            m_collections = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE | wxBORDER_NONE);

            wxWindow* addTextureCollectionsButton = createBitmapButton(this, "Add.png", "Add texture collections from the file system");
            wxWindow* removeTextureCollectionsButton = createBitmapButton(this, "Remove.png", "Remove the selected texture collections");
            wxWindow* moveTextureCollectionUpButton = createBitmapButton(this, "Up.png", "Move the selected texture collection up");
            wxWindow* moveTextureCollectionDownButton = createBitmapButton(this, "Down.png", "Move the selected texture collection down");
            
            addTextureCollectionsButton->Bind(wxEVT_BUTTON, &FileTextureCollectionEditor::OnAddTextureCollectionsClicked, this);
            removeTextureCollectionsButton->Bind(wxEVT_BUTTON, &FileTextureCollectionEditor::OnRemoveTextureCollectionsClicked, this);
            moveTextureCollectionUpButton->Bind(wxEVT_BUTTON, &FileTextureCollectionEditor::OnMoveTextureCollectionUpClicked, this);
            moveTextureCollectionDownButton->Bind(wxEVT_BUTTON, &FileTextureCollectionEditor::OnMoveTextureCollectionDownClicked, this);
            removeTextureCollectionsButton->Bind(wxEVT_UPDATE_UI, &FileTextureCollectionEditor::OnUpdateRemoveButtonUI, this);
            moveTextureCollectionUpButton->Bind(wxEVT_UPDATE_UI, &FileTextureCollectionEditor::OnUpdateMoveUpButtonUI, this);
            moveTextureCollectionDownButton->Bind(wxEVT_UPDATE_UI, &FileTextureCollectionEditor::OnUpdateMoveDownButtonUI, this);

            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addTextureCollectionsButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(removeTextureCollectionsButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(moveTextureCollectionUpButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(moveTextureCollectionDownButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_collections, 1, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            sizer->SetItemMinSize(m_collections, 100, 70);
            
            SetSizerAndFit(sizer);
        }
        
        void FileTextureCollectionEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &FileTextureCollectionEditor::textureCollectionsDidChange);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &FileTextureCollectionEditor::preferenceDidChange);
        }
        
        void FileTextureCollectionEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &FileTextureCollectionEditor::textureCollectionsDidChange);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &FileTextureCollectionEditor::preferenceDidChange);
        }
        
        void FileTextureCollectionEditor::textureCollectionsDidChange() {
            updateControls();
        }
        
        void FileTextureCollectionEditor::preferenceDidChange(const IO::Path& path) {
            MapDocumentSPtr document = lock(m_document);
            if (document->isGamePathPreference(path))
                updateControls();
        }

        void FileTextureCollectionEditor::updateControls() {
            m_collections->Clear();
            
            MapDocumentSPtr document = lock(m_document);
            const IO::Path::List collections = document->enabledTextureCollections();
            IO::Path::List::const_iterator it, end;
            for (it = collections.begin(), end = collections.end(); it != end; ++it) {
                const IO::Path& path = *it;
                m_collections->Append(path.asString());
            }
        }
    }
}
