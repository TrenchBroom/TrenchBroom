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
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>

wxDEFINE_EVENT(LAYER_SELECTED_EVENT, TrenchBroom::View::LayerCommand);
wxDEFINE_EVENT(LAYER_RIGHT_CLICK_EVENT, TrenchBroom::View::LayerCommand);

namespace TrenchBroom {
    namespace View {
        LayerCommand::LayerCommand(const wxEventType commandType, const int id) :
        wxCommandEvent(commandType, id),
        m_layer(NULL) {}
        
        Model::Layer* LayerCommand::layer() const {
            return m_layer;
        }
        
        void LayerCommand::setLayer(Model::Layer* layer) {
            m_layer = layer;
        }
        
        wxEvent* LayerCommand::Clone() const {
            return new LayerCommand(*this);
        }
        
        class LayerEntry : public wxPanel {
        private:
            int m_index;
            Model::Layer* m_layer;
            wxStaticText* m_nameText;
            wxStaticText* m_infoText;
        public:
            LayerEntry(wxPanel* parent, int index, Model::Layer* layer) :
            wxPanel(parent),
            m_index(index),
            m_layer(layer) {
                m_nameText = new wxStaticText(this, wxID_ANY, m_layer->name());
                m_nameText->SetFont(m_nameText->GetFont().Bold());
                m_infoText = new wxStaticText(this, wxID_ANY, "");
                refresh();
                
                wxWindow* visibleButton = createBitmapToggleButton(this, "Invisible.png", "Visible.png", "Show or hide this layer");
                wxWindow* lockButton = createBitmapToggleButton(this, "Unlocked.png", "Locked.png", "Lock or unlock this layer");
                
                bindMouseEvents(this);
                bindMouseEvents(m_nameText);
                bindMouseEvents(m_infoText);
                
                visibleButton->Bind(wxEVT_BUTTON, &LayerEntry::OnToggleVisible, this);
                visibleButton->Bind(wxEVT_UPDATE_UI, &LayerEntry::OnUpdateVisibleButton, this);
                lockButton->Bind(wxEVT_BUTTON, &LayerEntry::OnToggleLocked, this);
                lockButton->Bind(wxEVT_UPDATE_UI, &LayerEntry::OnUpdateLockButton, this);

                wxSizer* itemPanelBottomSizer = new wxBoxSizer(wxHORIZONTAL);
                itemPanelBottomSizer->Add(visibleButton, 0, wxALIGN_CENTRE_VERTICAL);
                itemPanelBottomSizer->Add(lockButton, 0, wxALIGN_CENTRE_VERTICAL);
                itemPanelBottomSizer->Add(m_infoText, 0, wxALIGN_CENTRE_VERTICAL);
                itemPanelBottomSizer->AddStretchSpacer();
                itemPanelBottomSizer->AddSpacer(LayoutConstants::NarrowHMargin);
                
                wxSizer* itemPanelSizer = new wxBoxSizer(wxVERTICAL);
                itemPanelSizer->AddSpacer(LayoutConstants::NarrowVMargin);
                itemPanelSizer->Add(m_nameText, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
                itemPanelSizer->Add(itemPanelBottomSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
                itemPanelSizer->AddSpacer(LayoutConstants::NarrowVMargin);
                SetSizer(itemPanelSizer);
                
                setSelected(false);
            }
            
            int index() const {
                return m_index;
            }
            
            Model::Layer* layer() const {
                return m_layer;
            }

            void refresh() {
                m_nameText->SetLabel(m_layer->name());
                
                wxString info;
                info << m_layer->objects().size() << " objects";
                m_infoText->SetLabel(info);
            }
            
            void bindMouseEvents(wxWindow* window) {
                window->Bind(wxEVT_LEFT_DOWN, &LayerEntry::OnMouse, this);
                window->Bind(wxEVT_RIGHT_DOWN, &LayerEntry::OnMouse, this);
                window->Bind(wxEVT_RIGHT_UP, &LayerEntry::OnMouse, this);
            }
            
            void setSelected(const bool selected) {
                wxColour foreground, background;
                if (selected) {
                    foreground = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT);
                    background = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
                } else {
                    foreground = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
                    background = *wxWHITE;
                }
                
                SetBackgroundColour(background);
                SetForegroundColour(foreground);
                
                const wxWindowList& children = GetChildren();
                for (size_t i = 0; i < children.size(); ++i) {
                    children[i]->SetBackgroundColour(background);
                    children[i]->SetForegroundColour(foreground);
                }
            }
            
            void OnToggleVisible(wxCommandEvent& event) {
                m_layer->setVisible(!m_layer->visible());
            }
            
            void OnUpdateVisibleButton(wxUpdateUIEvent& event) {
                event.Check(m_layer->visible());
            }
            
            void OnToggleLocked(wxCommandEvent& event) {
                m_layer->setLocked(!m_layer->locked());
            }
            
            void OnUpdateLockButton(wxUpdateUIEvent& event) {
                event.Check(m_layer->locked());
            }
            
            void OnMouse(wxMouseEvent& event) {
                wxMouseEvent newEvent(event);
                newEvent.SetEventObject(this);
                ProcessEvent(newEvent);
            }
        };
        
        LayerListView::LayerListView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_scrollWindow(NULL),
        m_selection(-1) {
            SetBackgroundColour(*wxWHITE);
            createGui();
            bindObservers();
        }
        
        LayerListView::~LayerListView() {
            unbindObservers();
        }

        Model::Layer* LayerListView::selectedLayer() const {
            if (m_selection == -1)
                return NULL;
            
            const Model::Map* map = lock(m_document)->map();
            const Model::LayerList& layers = map->layers();

            const size_t index = static_cast<size_t>(m_selection);
            assert(index < layers.size());
            return layers[index];
        }

        void LayerListView::setSelectedLayer(Model::Layer* layer) {
            m_selection = -1;
            for (size_t i = 0; i < m_entries.size(); ++i) {
                LayerEntry* entry = m_entries[i];
                if (entry->layer() == layer) {
                    entry->setSelected(true);
                    m_selection = static_cast<int>(i);
                } else {
                    entry->setSelected(false);
                }
            }
            Refresh();
        }

        void LayerListView::OnMouseEntryDown(wxMouseEvent& event) {
            const wxVariant* data = static_cast<wxVariant*>(event.GetEventUserData());
            assert(data != NULL);

            LayerEntry* entry = static_cast<LayerEntry*>(data->GetWxObjectPtr());
            assert(entry != NULL);
            
            Model::Layer* layer = entry->layer();
            setSelectedLayer(layer);
            
            LayerCommand* command = new LayerCommand(LAYER_SELECTED_EVENT);
            command->SetId(GetId());
            command->SetEventObject(this);
            command->setLayer(layer);
            QueueEvent(command);
        }
        
        void LayerListView::OnMouseEntryRightUp(wxMouseEvent& event) {
            const wxVariant* data = static_cast<wxVariant*>(event.GetEventUserData());
            assert(data != NULL);
            
            LayerEntry* entry = static_cast<LayerEntry*>(data->GetWxObjectPtr());
            assert(entry != NULL);
            
            Model::Layer* layer = entry->layer();
            
            LayerCommand* command = new LayerCommand(LAYER_RIGHT_CLICK_EVENT);
            command->SetId(GetId());
            command->SetEventObject(this);
            command->setLayer(layer);
            QueueEvent(command);
        }

        void LayerListView::OnMouseVoidDown(wxMouseEvent& event) {
            setSelectedLayer(NULL);
            
            LayerCommand* command = new LayerCommand(LAYER_SELECTED_EVENT);
            command->SetId(GetId());
            command->SetEventObject(this);
            command->setLayer(NULL);
            QueueEvent(command);
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
            refresh();
        }

        void LayerListView::createGui() {
            m_scrollWindow = new wxScrolledWindow(this);
            m_scrollWindow->Bind(wxEVT_LEFT_DOWN, &LayerListView::OnMouseVoidDown, this);
            m_scrollWindow->Bind(wxEVT_RIGHT_DOWN, &LayerListView::OnMouseVoidDown, this);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(m_scrollWindow, 1, wxEXPAND);
            SetSizer(outerSizer);
        }
        
        void LayerListView::reload() {
            m_selection = -1;
            m_scrollWindow->DestroyChildren();
            m_entries.clear();
            
            wxSizer* scrollWindowSizer = new wxBoxSizer(wxVERTICAL);
            
            MapDocumentSPtr document = lock(m_document);
            const Model::Map* map = document->map();
            const Model::LayerList& layers = map->layers();
            
            for (size_t i = 0; i < layers.size(); ++i) {
                Model::Layer* layer = layers[i];
                LayerEntry* entry = new LayerEntry(m_scrollWindow, static_cast<int>(i), layer);
                entry->Bind(wxEVT_LEFT_DOWN, &LayerListView::OnMouseEntryDown, this, wxID_ANY, wxID_ANY, new wxVariant(entry));
                entry->Bind(wxEVT_RIGHT_DOWN, &LayerListView::OnMouseEntryDown, this, wxID_ANY, wxID_ANY, new wxVariant(entry));
                entry->Bind(wxEVT_RIGHT_UP, &LayerListView::OnMouseEntryRightUp, this, wxID_ANY, wxID_ANY, new wxVariant(entry));

                scrollWindowSizer->Add(entry, 0, wxEXPAND);
                scrollWindowSizer->Add(new BorderLine(m_scrollWindow), 0, wxEXPAND);
                m_entries.push_back(entry);
            }
            
            scrollWindowSizer->AddStretchSpacer();
            m_scrollWindow->SetSizer(scrollWindowSizer);
            m_scrollWindow->SetScrollRate(0, 1);
            Layout();
        }

        void LayerListView::refresh() {
            for (size_t i = 0; i < m_entries.size(); ++i)
                m_entries[i]->refresh();
        }
    }
}
