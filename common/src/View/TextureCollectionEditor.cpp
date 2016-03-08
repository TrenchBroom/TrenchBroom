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

#include "TextureCollectionEditor.h"

#include "PreferenceManager.h"
#include "Assets/TextureManager.h"
#include "Assets/TextureCollection.h"
#include "View/ChoosePathTypeDialog.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"

#include <wx/bmpbuttn.h>
#include <wx/filedlg.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        TextureCollectionEditor::TextureCollectionEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document) {
            createGui();
            bindObservers();
        }
        
        TextureCollectionEditor::~TextureCollectionEditor() {
            unbindObservers();
        }
        
        void TextureCollectionEditor::OnAddTextureCollectionsClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const wxString pathWxStr = ::wxFileSelector("Load Texture Collection", wxEmptyString, wxEmptyString, wxEmptyString, "", wxFD_OPEN);
            if (pathWxStr.empty())
                return;
            
            loadTextureCollection(m_document, this, pathWxStr);
        }
        
        void TextureCollectionEditor::OnRemoveTextureCollectionsClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(!selections.empty());
            
            MapDocumentSPtr document = lock(m_document);

            const StringList names = document->externalTextureCollectionNames();
            StringList removeNames;

            for (size_t i = 0; i < selections.size(); ++i) {
                const size_t index = static_cast<size_t>(selections[i]);
                assert(index < names.size());
                removeNames.push_back(names[index]);
            }
            
            document->removeTextureCollections(removeNames);
        }
        
        void TextureCollectionEditor::OnMoveTextureCollectionUpClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(selections.size() == 1);
            
            MapDocumentSPtr document = lock(m_document);

            const StringList names = document->externalTextureCollectionNames();
            
            const size_t index = static_cast<size_t>(selections.front());
            assert(index > 0 && index < names.size());
            
            document->moveTextureCollectionUp(names[index]);
            m_collections->SetSelection(static_cast<int>(index - 1));
        }
        
        void TextureCollectionEditor::OnMoveTextureCollectionDownClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(selections.size() == 1);
            
            MapDocumentSPtr document = lock(m_document);

            const StringList names = document->externalTextureCollectionNames();

            const size_t index = static_cast<size_t>(selections.front());
            assert(index < names.size() - 1);
            
            document->moveTextureCollectionDown(names[index]);
            m_collections->SetSelection(static_cast<int>(index + 1));
        }

        void TextureCollectionEditor::OnUpdateRemoveButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

                wxArrayInt selections;
                event.Enable(m_collections->GetSelections(selections) > 0);
        }
        
        void TextureCollectionEditor::OnUpdateMoveUpButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            wxArrayInt selections;
            event.Enable(m_collections->GetSelections(selections) == 1 && selections.front() > 0);
        }
        
        void TextureCollectionEditor::OnUpdateMoveDownButtonUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            const int collectionCount = static_cast<int>(m_collections->GetCount());
            wxArrayInt selections;
            event.Enable(m_collections->GetSelections(selections) == 1 && selections.front() < collectionCount - 1);
        }
        
        void TextureCollectionEditor::createGui() {
            static const int ListBoxMargin =
#ifdef __APPLE__
            0;
#else
            LayoutConstants::NarrowHMargin;
#endif

            m_collections = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE | wxBORDER_NONE);

            wxWindow* addTextureCollectionsButton = createBitmapButton(this, "Add.png", "Add texture collections from the file system");
            wxWindow* removeTextureCollectionsButton = createBitmapButton(this, "Remove.png", "Remove the selected texture collections");
            wxWindow* moveTextureCollectionUpButton = createBitmapButton(this, "Up.png", "Move the selected texture collection up");
            wxWindow* moveTextureCollectionDownButton = createBitmapButton(this, "Down.png", "Move the selected texture collection down");
            
            addTextureCollectionsButton->Bind(wxEVT_BUTTON, &TextureCollectionEditor::OnAddTextureCollectionsClicked, this);
            removeTextureCollectionsButton->Bind(wxEVT_BUTTON, &TextureCollectionEditor::OnRemoveTextureCollectionsClicked, this);
            moveTextureCollectionUpButton->Bind(wxEVT_BUTTON, &TextureCollectionEditor::OnMoveTextureCollectionUpClicked, this);
            moveTextureCollectionDownButton->Bind(wxEVT_BUTTON, &TextureCollectionEditor::OnMoveTextureCollectionDownClicked, this);
            removeTextureCollectionsButton->Bind(wxEVT_UPDATE_UI, &TextureCollectionEditor::OnUpdateRemoveButtonUI, this);
            moveTextureCollectionUpButton->Bind(wxEVT_UPDATE_UI, &TextureCollectionEditor::OnUpdateMoveUpButtonUI, this);
            moveTextureCollectionDownButton->Bind(wxEVT_UPDATE_UI, &TextureCollectionEditor::OnUpdateMoveDownButtonUI, this);

            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addTextureCollectionsButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(removeTextureCollectionsButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(moveTextureCollectionUpButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(moveTextureCollectionDownButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_collections, 1, wxEXPAND | wxLEFT | wxRIGHT, ListBoxMargin);
            sizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            sizer->SetItemMinSize(m_collections, 100, 70);
            
            SetBackgroundColour(*wxWHITE);
            SetSizerAndFit(sizer);
        }
        
        void TextureCollectionEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &TextureCollectionEditor::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &TextureCollectionEditor::documentWasLoaded);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &TextureCollectionEditor::textureCollectionsDidChange);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &TextureCollectionEditor::preferenceDidChange);
        }
        
        void TextureCollectionEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &TextureCollectionEditor::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &TextureCollectionEditor::documentWasLoaded);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &TextureCollectionEditor::textureCollectionsDidChange);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &TextureCollectionEditor::preferenceDidChange);
        }
        
        void TextureCollectionEditor::documentWasNewed(MapDocument* document) {
            updateControls();
        }
        
        void TextureCollectionEditor::documentWasLoaded(MapDocument* document) {
            updateControls();
        }
        
        void TextureCollectionEditor::textureCollectionsDidChange() {
            updateControls();
        }
        
        void TextureCollectionEditor::preferenceDidChange(const IO::Path& path) {
            MapDocumentSPtr document = lock(m_document);
            if (document->isGamePathPreference(path))
                updateControls();
        }

        void TextureCollectionEditor::updateControls() {
            m_collections->Clear();
            
            MapDocumentSPtr document = lock(m_document);
            const StringList names = document->externalTextureCollectionNames();
            StringList::const_iterator it, end;
            for (it = names.begin(), end = names.end(); it != end; ++it) {
                const String& name = *it;
                m_collections->Append(name);
            }
        }
    }
}
