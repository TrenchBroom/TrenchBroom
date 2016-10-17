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

#include "TextureBrowser.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/TextureManager.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/TextureSelectedCommand.h"

#include <wx/choice.h>
#include <wx/event.h>
#include <wx/tglbtn.h>
#include <wx/sizer.h>
#include <wx/srchctrl.h>

namespace TrenchBroom {
    namespace View {
        TextureBrowser::TextureBrowser(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager) :
        wxPanel(parent),
        m_document(document) {
            createGui(contextManager);
            bindEvents();
            bindObservers();
            reload();
        }
        
        TextureBrowser::~TextureBrowser() {
            unbindObservers();
        }

        Assets::Texture* TextureBrowser::selectedTexture() const {
            return m_view->selectedTexture();
        }
        
        void TextureBrowser::setSelectedTexture(Assets::Texture* selectedTexture) {
            m_view->setSelectedTexture(selectedTexture);
        }

        void TextureBrowser::setSortOrder(const TextureBrowserView::SortOrder sortOrder) {
            m_view->setSortOrder(sortOrder);
            switch (sortOrder) {
                case TextureBrowserView::SO_Name:
                    m_sortOrderChoice->SetSelection(0);
                    break;
                case TextureBrowserView::SO_Usage:
                    m_sortOrderChoice->SetSelection(1);
                    break;
                switchDefault()
            }
            
        }
        
        void TextureBrowser::setGroup(const bool group) {
            m_view->setGroup(group);
            m_groupButton->SetValue(group);
        }
        
        void TextureBrowser::setHideUnused(const bool hideUnused) {
            m_view->setHideUnused(hideUnused);
            m_usedButton->SetValue(hideUnused);
        }
        
        void TextureBrowser::setFilterText(const String& filterText) {
            m_view->setFilterText(filterText);
            m_filterBox->ChangeValue(filterText);
        }

        void TextureBrowser::OnSortOrderChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const TextureBrowserView::SortOrder sortOrder = event.GetSelection() == 0 ? TextureBrowserView::SO_Name : TextureBrowserView::SO_Usage;
            m_view->setSortOrder(sortOrder);
        }
        
        void TextureBrowser::OnGroupButtonToggled(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_view->setGroup(m_groupButton->GetValue());
        }
        
        void TextureBrowser::OnUsedButtonToggled(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_view->setHideUnused(m_usedButton->GetValue());
        }
        
        void TextureBrowser::OnFilterPatternChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            m_view->setFilterText(m_filterBox->GetValue().ToStdString());
        }

        void TextureBrowser::OnTextureSelected(TextureSelectedCommand& event) {
            if (IsBeingDeleted()) return;

            // let the event bubble up to our own listeners
            event.SetEventObject(this);
            event.SetId(GetId());
            ProcessEvent(event);
        }

        void TextureBrowser::createGui(GLContextManager& contextManager) {
            wxPanel* browserPanel = new wxPanel(this);
            m_scrollBar = new wxScrollBar(browserPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
            
            MapDocumentSPtr document = lock(m_document);
            m_view = new TextureBrowserView(browserPanel, m_scrollBar, contextManager, document->textureManager());
            
            wxSizer* browserPanelSizer = new wxBoxSizer(wxHORIZONTAL);
            browserPanelSizer->Add(m_view, 1, wxEXPAND);
            browserPanelSizer->Add(m_scrollBar, 0, wxEXPAND);
            browserPanel->SetSizer(browserPanelSizer);
            
            const wxString sortOrders[2] = { "Name", "Usage" };
            m_sortOrderChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, sortOrders);
            m_sortOrderChoice->SetSelection(0);
            m_sortOrderChoice->SetToolTip("Select ordering criterion");
            
            m_groupButton = new wxToggleButton(this, wxID_ANY, "Group", wxDefaultPosition, wxDefaultSize, LayoutConstants::ToggleButtonStyle | wxBU_EXACTFIT);
            m_groupButton->SetToolTip("Group textures by texture collection");

            m_usedButton = new wxToggleButton(this, wxID_ANY, "Used", wxDefaultPosition, wxDefaultSize, LayoutConstants::ToggleButtonStyle | wxBU_EXACTFIT);
            m_usedButton->SetToolTip("Only show textures currently in use");
            
            m_filterBox = new wxSearchCtrl(this, wxID_ANY);
            m_filterBox->ShowCancelButton(true);
            
            wxSizer* controlSizer = new wxBoxSizer(wxHORIZONTAL);
            controlSizer->AddSpacer(LayoutConstants::ChoiceLeftMargin);
            controlSizer->Add(m_sortOrderChoice, 0, wxTOP, LayoutConstants::ChoiceTopMargin);
            controlSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            controlSizer->Add(m_groupButton);
            controlSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            controlSizer->Add(m_usedButton);
            controlSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            controlSizer->Add(m_filterBox, 1, wxEXPAND);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(browserPanel, 1, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::NarrowVMargin);
            outerSizer->Add(controlSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
            outerSizer->AddSpacer(LayoutConstants::NarrowVMargin);

            SetSizer(outerSizer);
        }
        
        void TextureBrowser::bindEvents() {
            m_sortOrderChoice->Bind(wxEVT_CHOICE, &TextureBrowser::OnSortOrderChanged, this);
            m_groupButton->Bind(wxEVT_TOGGLEBUTTON, &TextureBrowser::OnGroupButtonToggled, this);
            m_usedButton->Bind(wxEVT_TOGGLEBUTTON, &TextureBrowser::OnUsedButtonToggled, this);
            m_filterBox->Bind(wxEVT_TEXT, &TextureBrowser::OnFilterPatternChanged, this);
            m_view->Bind(TEXTURE_SELECTED_EVENT, &TextureBrowser::OnTextureSelected, this);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &TextureBrowser::preferenceDidChange);
        }
        
        void TextureBrowser::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &TextureBrowser::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &TextureBrowser::documentWasLoaded);
            document->nodesWereAddedNotifier.addObserver(this, &TextureBrowser::nodesWereAdded);
            document->nodesWereRemovedNotifier.addObserver(this, &TextureBrowser::nodesWereRemoved);
            document->nodesDidChangeNotifier.addObserver(this, &TextureBrowser::nodesDidChange);
            document->brushFacesDidChangeNotifier.addObserver(this, &TextureBrowser::brushFacesDidChange);
            document->textureCollectionsDidChangeNotifier.addObserver(this, &TextureBrowser::textureCollectionsDidChange);
            document->currentTextureNameDidChangeNotifier.addObserver(this, &TextureBrowser::currentTextureNameDidChange);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &TextureBrowser::preferenceDidChange);
        }
        
        void TextureBrowser::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &TextureBrowser::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &TextureBrowser::documentWasLoaded);
                document->textureCollectionsDidChangeNotifier.removeObserver(this, &TextureBrowser::textureCollectionsDidChange);
                document->nodesWereAddedNotifier.removeObserver(this, &TextureBrowser::nodesWereAdded);
                document->nodesWereRemovedNotifier.removeObserver(this, &TextureBrowser::nodesWereRemoved);
                document->nodesDidChangeNotifier.removeObserver(this, &TextureBrowser::nodesDidChange);
                document->brushFacesDidChangeNotifier.removeObserver(this, &TextureBrowser::brushFacesDidChange);
                document->currentTextureNameDidChangeNotifier.removeObserver(this, &TextureBrowser::currentTextureNameDidChange);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &TextureBrowser::preferenceDidChange);
        }
        
        void TextureBrowser::documentWasNewed(MapDocument* document) {
            reload();
        }
        
        void TextureBrowser::documentWasLoaded(MapDocument* document) {
            reload();
        }

        void TextureBrowser::nodesWereAdded(const Model::NodeList& nodes) {
            reload();
        }
        
        void TextureBrowser::nodesWereRemoved(const Model::NodeList& nodes) {
            reload();
        }
        
        void TextureBrowser::nodesDidChange(const Model::NodeList& nodes) {
            reload();
        }
        
        void TextureBrowser::brushFacesDidChange(const Model::BrushFaceList& faces) {
            reload();
        }

        void TextureBrowser::textureCollectionsDidChange() {
            reload();
        }

        void TextureBrowser::currentTextureNameDidChange(const String& textureName) {
            updateSelectedTexture();
        }

        void TextureBrowser::preferenceDidChange(const IO::Path& path) {
            MapDocumentSPtr document = lock(m_document);
            if (path == Preferences::TextureBrowserIconSize.path() ||
                document->isGamePathPreference(path))
                reload();
            else
                m_view->Refresh();
        }
        
        void TextureBrowser::reload() {
            if (m_view != NULL) {
                updateSelectedTexture();
                m_view->invalidate();
                m_view->Refresh();
            }
        }

        void TextureBrowser::updateSelectedTexture() {
            MapDocumentSPtr document = lock(m_document);
            const String& textureName = document->currentTextureName();
            Assets::Texture* texture = document->textureManager().texture(textureName);
            m_view->setSelectedTexture(texture);
        }
    }
}
