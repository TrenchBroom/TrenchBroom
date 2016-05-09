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

#include "DirectoryTextureCollectionEditor.h"

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
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        DirectoryTextureCollectionEditor::DirectoryTextureCollectionEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document),
        m_availableCollectionsList(NULL),
        m_enabledCollectionsList(NULL),
        m_ignoreNotifier(false) {
            createGui();
            bindObservers();
            update();
        }
        
        void DirectoryTextureCollectionEditor::OnAddTextureCollections(wxCommandEvent& event) {
        }
        
        void DirectoryTextureCollectionEditor::OnRemoveTextureCollections(wxCommandEvent& event) {
            
        }

        void DirectoryTextureCollectionEditor::OnUpdateAddTextureCollections(wxUpdateUIEvent& event) {
            wxArrayInt selections;
            event.Enable(m_availableCollectionsList->GetSelections(selections) > 0);
        }
        
        void DirectoryTextureCollectionEditor::OnUpdateRemoveTextureCollections(wxUpdateUIEvent& event) {
            wxArrayInt selections;
            event.Enable(m_enabledCollectionsList->GetSelections(selections) > 0);
        }

        void DirectoryTextureCollectionEditor::createGui() {
            TitledPanel* availableCollectionsContainer = new TitledPanel(this, "Available", false);
            availableCollectionsContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            
            m_availableCollectionsList = new wxListBox(availableCollectionsContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE | wxBORDER_NONE);
            
            wxSizer* availableModContainerSizer = new wxBoxSizer(wxVERTICAL);
            availableModContainerSizer->Add(m_availableCollectionsList, wxSizerFlags().Expand().Proportion(1));
            availableCollectionsContainer->getPanel()->SetSizer(availableModContainerSizer);
        
            TitledPanel* enabledCollectionsContainer = new TitledPanel(this, "Enabled", false);
            enabledCollectionsContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_enabledCollectionsList = new wxListBox(enabledCollectionsContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE | wxBORDER_NONE);
            
            wxSizer* enabledCollectionsContainerSizer = new wxBoxSizer(wxVERTICAL);
            enabledCollectionsContainerSizer->Add(m_enabledCollectionsList, wxSizerFlags().Expand().Proportion(1));
            enabledCollectionsContainer->getPanel()->SetSizer(enabledCollectionsContainerSizer);
            
            wxWindow* addCollectionsButton = createBitmapButton(this, "Add.png", "Enable the selected texture collections");
            wxWindow* removeCollectionsButton = createBitmapButton(this, "Remove.png", "Disable the selected texture collections");
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addCollectionsButton, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->Add(removeCollectionsButton, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin));
            buttonSizer->AddStretchSpacer();
            
            wxGridBagSizer* sizer = new wxGridBagSizer(0, 0);
            sizer->Add(availableCollectionsContainer,                           wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Vertical),    wxGBPosition(0, 1), wxGBSpan(3, 1), wxEXPAND);
            sizer->Add(enabledCollectionsContainer,                             wxGBPosition(0, 2), wxDefaultSpan, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal),  wxGBPosition(1, 0), wxGBSpan(1, 3), wxEXPAND);
            sizer->Add(buttonSizer,                                             wxGBPosition(2, 2), wxDefaultSpan, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            sizer->SetItemMinSize(availableCollectionsContainer, 100, 100);
            sizer->SetItemMinSize(enabledCollectionsContainer, 100, 100);
            sizer->AddGrowableCol(0);
            sizer->AddGrowableCol(2);
            sizer->AddGrowableRow(1);
            
            SetSizerAndFit(sizer);
            
            m_availableCollectionsList->Bind(wxEVT_LISTBOX_DCLICK, &DirectoryTextureCollectionEditor::OnAddTextureCollections, this);
            m_enabledCollectionsList->Bind(wxEVT_LISTBOX_DCLICK, &DirectoryTextureCollectionEditor::OnRemoveTextureCollections, this);
            addCollectionsButton->Bind(wxEVT_BUTTON, &DirectoryTextureCollectionEditor::OnAddTextureCollections, this);
            removeCollectionsButton->Bind(wxEVT_BUTTON, &DirectoryTextureCollectionEditor::OnRemoveTextureCollections, this);
            addCollectionsButton->Bind(wxEVT_UPDATE_UI, &DirectoryTextureCollectionEditor::OnUpdateAddTextureCollections, this);
            removeCollectionsButton->Bind(wxEVT_UPDATE_UI, &DirectoryTextureCollectionEditor::OnUpdateRemoveTextureCollections, this);
        }
        
        void DirectoryTextureCollectionEditor::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->modsDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
        }

        void DirectoryTextureCollectionEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->modsDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
            }
        }
        
        void DirectoryTextureCollectionEditor::textureCollectionsDidChange() {
            if (!m_ignoreNotifier)
                update();
        }
        
        void DirectoryTextureCollectionEditor::preferenceDidChange(const IO::Path& path) {
            MapDocumentSPtr document = lock(m_document);
            if (document->isGamePathPreference(path))
                update();
        }
        
        void DirectoryTextureCollectionEditor::update() {
            updateAvailableTextureCollections();
            updateEnabledTextureCollections();
        }
        
        void DirectoryTextureCollectionEditor::updateAvailableTextureCollections() {
            MapDocumentSPtr document = lock(m_document);
            updateListBox(m_availableCollectionsList, document->availableTextureCollections());
        }
        
        void DirectoryTextureCollectionEditor::updateEnabledTextureCollections() {
            MapDocumentSPtr document = lock(m_document);
            updateListBox(m_enabledCollectionsList, document->enabledTextureCollections());
        }

        void DirectoryTextureCollectionEditor::updateListBox(wxListBox* box, const IO::Path::List& paths) {
            box->Clear();
            
            IO::Path::List::const_iterator it, end;
            for (it = paths.begin(), end = paths.end(); it != end; ++it) {
                const IO::Path& path = *it;
                box->Append(path.asString());
            }
        }
    }
}
