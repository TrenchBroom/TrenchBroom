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
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/ChoosePathTypeDialog.h"
#include "View/ControllerFacade.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/filedlg.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        TextureCollectionEditor::TextureCollectionEditor(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller) {
            createGui();
            bindEvents();
            bindObservers();
        }
        
        TextureCollectionEditor::~TextureCollectionEditor() {
            unbindObservers();
        }
        
        void TextureCollectionEditor::OnAddTextureCollectionsClicked(wxCommandEvent& event) {
            const wxString pathWxStr = ::wxFileSelector("Load Texture Collection", wxEmptyString, wxEmptyString, wxEmptyString, "", wxFD_OPEN);
            if (pathWxStr.empty())
                return;
            
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            const IO::Path absPath(pathWxStr.ToStdString());
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();

            const IO::Path docPath = document->path();
            const IO::Path gamePath = gameFactory.gamePath(document->game()->gameName());
            
            ChoosePathTypeDialog pathDialog(wxGetTopLevelParent(this), absPath, docPath, gamePath);
            if (pathDialog.ShowModal() != wxID_OK)
                return;
            
            const IO::Path collectionPath = pathDialog.path();
            controller->addTextureCollection(collectionPath.asString());
        }
        
        void TextureCollectionEditor::OnRemoveTextureCollectionsClicked(wxCommandEvent& event) {
            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(!selections.empty());
            
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            const StringList names = document->textureManager().externalCollectionNames();
            StringList removeNames;

            for (size_t i = 0; i < selections.size(); ++i) {
                const size_t index = static_cast<size_t>(selections[i]);
                assert(index < names.size());
                removeNames.push_back(names[index]);
            }
            
            controller->removeTextureCollections(removeNames);
        }
        
        void TextureCollectionEditor::OnMoveTextureCollectionUpClicked(wxCommandEvent& event) {
            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(selections.size() == 1);
            
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            const StringList names = document->textureManager().externalCollectionNames();
            
            const size_t index = static_cast<size_t>(selections.front());
            assert(index > 0 && index < names.size());
            
            controller->moveTextureCollectionUp(names[index]);
            m_collections->SetSelection(static_cast<int>(index - 1));
        }
        
        void TextureCollectionEditor::OnMoveTextureCollectionDownClicked(wxCommandEvent& event) {
            wxArrayInt selections;
            m_collections->GetSelections(selections);
            assert(selections.size() == 1);
            
            MapDocumentSPtr document = lock(m_document);
            ControllerSPtr controller = lock(m_controller);

            const StringList names = document->textureManager().externalCollectionNames();

            const size_t index = static_cast<size_t>(selections.front());
            assert(index < names.size() - 1);
            
            controller->moveTextureCollectionDown(names[index]);
            m_collections->SetSelection(static_cast<int>(index + 1));
        }

        void TextureCollectionEditor::OnUpdateButtonUI(wxUpdateUIEvent& event) {
            if (event.GetEventObject() == m_addTextureCollectionsButton) {
                event.Enable(true);
            } else {
                wxArrayInt selections;
                m_collections->GetSelections(selections);
                if (event.GetEventObject() == m_removeTextureCollectionsButton) {
                    event.Enable(selections.size() > 0);
                } else if (selections.size() == 1) {
                    if (event.GetEventObject() == m_moveTextureCollectionUpButton) {
                        event.Enable(selections.front() > 0);
                    } else if (event.GetEventObject() == m_moveTextureCollectionDownButton) {
                        event.Enable(static_cast<size_t>(selections.front()) < m_collections->GetCount() - 1);
                    }
                } else {
                    event.Enable(false);
                }
            }
        }

        void TextureCollectionEditor::createGui() {
            static const int ListBoxMargin =
#ifdef __APPLE__
            0;
#else
            LayoutConstants::NarrowHMargin;
#endif

            m_collections = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE | wxBORDER_NONE);

            const wxBitmap addBitmap = IO::loadImageResource(IO::Path("images/Add.png"));
            const wxBitmap removeBitmap = IO::loadImageResource(IO::Path("images/Remove.png"));
            const wxBitmap upBitmap = IO::loadImageResource(IO::Path("images/Up.png"));
            const wxBitmap downBitmap = IO::loadImageResource(IO::Path("images/Down.png"));
            
            m_addTextureCollectionsButton = new wxBitmapButton(this, wxID_ANY, addBitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_addTextureCollectionsButton->SetToolTip("Add texture collections from the file system");
            m_addTextureCollectionsButton->SetBackgroundColour(*wxWHITE);
            m_removeTextureCollectionsButton = new wxBitmapButton(this, wxID_ANY, removeBitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_removeTextureCollectionsButton->SetToolTip("Remove the selected texture collection(s)");
            m_removeTextureCollectionsButton->SetBackgroundColour(*wxWHITE);
            m_moveTextureCollectionUpButton = new wxBitmapButton(this, wxID_ANY, upBitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_moveTextureCollectionUpButton->SetToolTip("Move the selected texture collection up in the list");
            m_moveTextureCollectionUpButton->SetBackgroundColour(*wxWHITE);
            m_moveTextureCollectionDownButton = new wxBitmapButton(this, wxID_ANY, downBitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_moveTextureCollectionDownButton->SetToolTip("Move the selected texture collection down in the list");
            m_moveTextureCollectionDownButton->SetBackgroundColour(*wxWHITE);
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(m_addTextureCollectionsButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(m_removeTextureCollectionsButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(m_moveTextureCollectionUpButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(m_moveTextureCollectionDownButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_collections, 1, wxEXPAND | wxLEFT | wxRIGHT, ListBoxMargin);
            sizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            sizer->SetItemMinSize(m_collections, 100, 70);
            
            SetBackgroundColour(*wxWHITE);
            SetSizerAndFit(sizer);
        }
        
        void TextureCollectionEditor::bindEvents() {
            m_addTextureCollectionsButton->Bind(wxEVT_BUTTON, &TextureCollectionEditor::OnAddTextureCollectionsClicked, this);
            m_removeTextureCollectionsButton->Bind(wxEVT_BUTTON, &TextureCollectionEditor::OnRemoveTextureCollectionsClicked, this);
            m_moveTextureCollectionUpButton->Bind(wxEVT_BUTTON, &TextureCollectionEditor::OnMoveTextureCollectionUpClicked, this);
            m_moveTextureCollectionDownButton->Bind(wxEVT_BUTTON, &TextureCollectionEditor::OnMoveTextureCollectionDownClicked, this);
            
            m_addTextureCollectionsButton->Bind(wxEVT_UPDATE_UI, &TextureCollectionEditor::OnUpdateButtonUI, this);
            m_removeTextureCollectionsButton->Bind(wxEVT_UPDATE_UI, &TextureCollectionEditor::OnUpdateButtonUI, this);
            m_moveTextureCollectionUpButton->Bind(wxEVT_UPDATE_UI, &TextureCollectionEditor::OnUpdateButtonUI, this);
            m_moveTextureCollectionDownButton->Bind(wxEVT_UPDATE_UI, &TextureCollectionEditor::OnUpdateButtonUI, this);
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
        
        void TextureCollectionEditor::documentWasNewed() {
            updateControls();
        }
        
        void TextureCollectionEditor::documentWasLoaded() {
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
            const StringList names = document->textureManager().externalCollectionNames();
            StringList::const_iterator it, end;
            for (it = names.begin(), end = names.end(); it != end; ++it) {
                const String& name = *it;
                m_collections->Append(name);
            }
        }
    }
}
