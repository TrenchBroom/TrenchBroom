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
#include <wx/stattext.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        DirectoryTextureCollectionEditor::DirectoryTextureCollectionEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document),
        m_availableCollectionsList(nullptr),
        m_enabledCollectionsList(nullptr),
        m_ignoreNotifier(false) {
            createGui();
            bindObservers();
            update();
        }
        
        void DirectoryTextureCollectionEditor::OnAddTextureCollections(wxCommandEvent& event) {
            const IO::Path::List availableCollections = availableTextureCollections();
            IO::Path::List enabledCollections = enabledTextureCollections();
            
            wxArrayInt selections;
            m_availableCollectionsList->GetSelections(selections);
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const size_t index = static_cast<size_t>(selections[i]);
                enabledCollections.push_back(availableCollections[index]);
            }
            
            VectorUtils::sortAndRemoveDuplicates(enabledCollections);
            
            MapDocumentSPtr document = lock(m_document);
            document->setEnabledTextureCollections(enabledCollections);
        }
        
        void DirectoryTextureCollectionEditor::OnRemoveTextureCollections(wxCommandEvent& event) {
            const IO::Path::List availableCollections = availableTextureCollections();
            IO::Path::List enabledCollections = enabledTextureCollections();
            
            wxArrayInt selections;
            m_availableCollectionsList->GetSelections(selections);
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const size_t index = static_cast<size_t>(selections[i]);
                VectorUtils::erase(enabledCollections, availableCollections[index]);
            }
            
            MapDocumentSPtr document = lock(m_document);
            document->setEnabledTextureCollections(enabledCollections);
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
            
            m_availableCollectionsList = new wxListBox(availableCollectionsContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_MULTIPLE | wxBORDER_NONE);
            
            wxSizer* availableModContainerSizer = new wxBoxSizer(wxVERTICAL);
            availableModContainerSizer->Add(m_availableCollectionsList, wxSizerFlags().Expand().Proportion(1));
            availableCollectionsContainer->getPanel()->SetSizer(availableModContainerSizer);
        
            TitledPanel* enabledCollectionsContainer = new TitledPanel(this, "Enabled", false);
            enabledCollectionsContainer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            m_enabledCollectionsList = new wxListBox(enabledCollectionsContainer->getPanel(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_MULTIPLE | wxBORDER_NONE);
            
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
            document->textureCollectionsDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
            document->modsDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &DirectoryTextureCollectionEditor::preferenceDidChange);
        }

        void DirectoryTextureCollectionEditor::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::textureCollectionsDidChange);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &DirectoryTextureCollectionEditor::preferenceDidChange);
        }
        
        void DirectoryTextureCollectionEditor::textureCollectionsDidChange() {
            if (!m_ignoreNotifier)
                update();
        }
        
        void DirectoryTextureCollectionEditor::modsDidChange() {
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
            updateListBox(m_availableCollectionsList, availableTextureCollections());
        }
        
        void DirectoryTextureCollectionEditor::updateEnabledTextureCollections() {
            updateListBox(m_enabledCollectionsList, enabledTextureCollections());
        }

        void DirectoryTextureCollectionEditor::updateListBox(wxListBox* box, const IO::Path::List& paths) {
            wxArrayString values;
            values.reserve(paths.size());
            
            for (const IO::Path& path : paths)
                values.push_back(path.asString());
            
            box->Set(values);
        }
        
        IO::Path::List DirectoryTextureCollectionEditor::availableTextureCollections() const {
            MapDocumentSPtr document = lock(m_document);
            IO::Path::List availableCollections = document->availableTextureCollections();
            VectorUtils::eraseAll(availableCollections, document->enabledTextureCollections());
            return availableCollections;
        }

        IO::Path::List DirectoryTextureCollectionEditor::enabledTextureCollections() const {
            MapDocumentSPtr document = lock(m_document);
            return document->enabledTextureCollections();
        }
    }
}
