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

#include "LayerListView.h"

#include "Model/Layer.h"
#include "Model/Map.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>

namespace TrenchBroom {
    namespace View {
        LayerListView::LayerListView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_scrollWindow(NULL) {
            SetBackgroundColour(*wxWHITE);
            createGui();
            bindEvents();
            bindObservers();
        }
        
        LayerListView::~LayerListView() {
            unbindObservers();
        }

        void LayerListView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &LayerListView::documentWasChanged);
            document->documentWasLoadedNotifier.addObserver(this, &LayerListView::documentWasChanged);
            document->layersWereAddedNotifier.addObserver(this, &LayerListView::layersWereAdded);
            document->layersWereRemovedNotifier.addObserver(this, &LayerListView::layersWereRemoved);
        }
        
        void LayerListView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &LayerListView::documentWasChanged);
                document->documentWasLoadedNotifier.removeObserver(this, &LayerListView::documentWasChanged);
                document->layersWereAddedNotifier.removeObserver(this, &LayerListView::layersWereAdded);
                document->layersWereRemovedNotifier.removeObserver(this, &LayerListView::layersWereRemoved);
            }
        }
        
        void LayerListView::documentWasChanged() {
            const Model::Map* map = lock(m_document)->map();
            const Model::LayerList& layers = map->layers();
            Model::LayerList::const_iterator it, end;
            for (it = layers.begin(), end = layers.end(); it != end; ++it) {
                Model::Layer* layer = *it;
                layer->layerDidChangeNotifier.addObserver(this, &LayerListView::layerDidChange);
            }
            reload();
        }
        
        void LayerListView::layersWereAdded(const Model::LayerList& layers) {
            Model::LayerList::const_iterator it, end;
            for (it = layers.begin(), end = layers.end(); it != end; ++it) {
                Model::Layer* layer = *it;
                layer->layerDidChangeNotifier.addObserver(this, &LayerListView::layerDidChange);
            }
            reload();
        }
        
        void LayerListView::layersWereRemoved(const Model::LayerList& layers) {
            Model::LayerList::const_iterator it, end;
            for (it = layers.begin(), end = layers.end(); it != end; ++it) {
                Model::Layer* layer = *it;
                layer->layerDidChangeNotifier.removeObserver(this, &LayerListView::layerDidChange);
            }
            reload();
        }
        
        void LayerListView::layerDidChange(Model::Layer* layer) {
            reload();
        }

        void LayerListView::createGui() {
            m_scrollWindow = new wxScrolledWindow(this);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(m_scrollWindow, 1, wxEXPAND);
            SetSizer(outerSizer);
        }
        
        void LayerListView::bindEvents() {
        }

        void LayerListView::reload() {
            m_scrollWindow->DestroyChildren();
            
            wxSizer* scrollWindowSizer = new wxBoxSizer(wxVERTICAL);
            
            MapDocumentSPtr document = lock(m_document);
            const Model::Map* map = document->map();
            const Model::LayerList& layers = map->layers();
            
            for (size_t i = 0; i < layers.size(); ++i) {
                Model::Layer* layer = layers[i];
                const String& name = layer->name();
                
                wxPanel* itemPanel = new wxPanel(m_scrollWindow);
                
                wxStaticText* nameText = new wxStaticText(itemPanel, wxID_ANY, name);
                nameText->SetFont(nameText->GetFont().Bold());
                
                wxBitmapToggleButton* visibleButton = createBitmapToggleButton(itemPanel, "Visibility.png", "Show or hide this layer");
                wxBitmapToggleButton* lockButton = createBitmapToggleButton(itemPanel, "Locked.png", "Lock or unlock this layer");
                
                wxString info;
                info << layer->objects().size() << " objects";
                wxStaticText* infoText = new wxStaticText(itemPanel, wxID_ANY, info);

                wxSizer* itemPanelBottomSizer = new wxBoxSizer(wxHORIZONTAL);
                itemPanelBottomSizer->Add(visibleButton, 0, wxALIGN_CENTRE_VERTICAL);
                itemPanelBottomSizer->AddSpacer(LayoutConstants::NarrowHMargin);
                itemPanelBottomSizer->Add(lockButton, 0, wxALIGN_CENTRE_VERTICAL);
                itemPanelBottomSizer->AddSpacer(LayoutConstants::NarrowHMargin);
                itemPanelBottomSizer->Add(infoText, 0, wxALIGN_CENTRE_VERTICAL);
                itemPanelBottomSizer->AddStretchSpacer();
                itemPanelBottomSizer->AddSpacer(LayoutConstants::NarrowHMargin);
                
                wxSizer* itemPanelSizer = new wxBoxSizer(wxVERTICAL);
                itemPanelSizer->Add(nameText, 0, wxEXPAND);
                itemPanelSizer->Add(itemPanelBottomSizer, 0, wxEXPAND);
                itemPanel->SetSizer(itemPanelSizer);

                scrollWindowSizer->AddSpacer(LayoutConstants::NarrowVMargin);
                scrollWindowSizer->Add(itemPanel, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
                scrollWindowSizer->AddSpacer(LayoutConstants::NarrowVMargin);
                scrollWindowSizer->Add(new BorderLine(m_scrollWindow), 0, wxEXPAND);
            }
            
            scrollWindowSizer->AddStretchSpacer();
            m_scrollWindow->SetSizer(scrollWindowSizer);
            m_scrollWindow->SetScrollRate(0, 1);
            Layout();
        }
    }
}
