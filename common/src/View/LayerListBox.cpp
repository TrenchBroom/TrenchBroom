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

#include "LayerListBox.h"

#include "Model/Layer.h"
#include "Model/World.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/scrolwin.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>
#include <wx/wupdlock.h>

wxDEFINE_EVENT(LAYER_SELECTED_EVENT, TrenchBroom::View::LayerCommand);
wxDEFINE_EVENT(LAYER_SET_CURRENT_EVENT, TrenchBroom::View::LayerCommand);
wxDEFINE_EVENT(LAYER_RIGHT_CLICK_EVENT, TrenchBroom::View::LayerCommand);
wxDEFINE_EVENT(LAYER_TOGGLE_VISIBLE_EVENT, TrenchBroom::View::LayerCommand);
wxDEFINE_EVENT(LAYER_TOGGLE_LOCKED_EVENT, TrenchBroom::View::LayerCommand);

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

        class LayerListBox::LayerItem : public Item {
        private:
            MapDocumentWPtr m_document;
            Model::Layer* m_layer;
            wxStaticText* m_nameText;
            wxStaticText* m_infoText;
        public:
            LayerItem(wxWindow* parent, MapDocumentWPtr document, Model::Layer* layer, const wxSize& margins) :
            Item(parent),
            m_document(document),
            m_layer(layer) {
                m_nameText = new wxStaticText(this, wxID_ANY, m_layer->name());
                m_infoText = new wxStaticText(this, wxID_ANY, "");
                m_infoText->SetForegroundColour(makeLighter(m_infoText->GetForegroundColour()));
                refresh();

                wxWindow* hiddenText = new wxStaticText(this, wxID_ANY, "yGp"); // this is just for keeping the correct height of the name text
                hiddenText->SetFont(GetFont().Bold());
                hiddenText->Hide();
                
                wxWindow* hiddenButton = createBitmapToggleButton(this, "Visible.png", "Invisible.png", "");
                wxWindow* lockButton = createBitmapToggleButton(this, "Unlocked.png", "Locked.png", "");

                MapDocumentSPtr documentS = lock(m_document);
                hiddenButton->Enable(m_layer->hidden() || m_layer != documentS->currentLayer());
                lockButton->Enable(m_layer->locked() || m_layer != documentS->currentLayer());

                hiddenButton->Bind(wxEVT_BUTTON, &LayerItem::OnToggleVisible, this);
                hiddenButton->Bind(wxEVT_UPDATE_UI, &LayerItem::OnUpdateVisibleButton, this);
                lockButton->Bind(wxEVT_BUTTON, &LayerItem::OnToggleLocked, this);
                lockButton->Bind(wxEVT_UPDATE_UI, &LayerItem::OnUpdateLockButton, this);

                wxSizer* itemPanelTopSizer = new wxBoxSizer(wxHORIZONTAL);
                itemPanelTopSizer->Add(m_nameText, 0, wxALIGN_BOTTOM);
                itemPanelTopSizer->Add(hiddenText, 0, wxALIGN_BOTTOM | wxRESERVE_SPACE_EVEN_IF_HIDDEN);
                
                wxSizer* itemPanelBottomSizer = new wxBoxSizer(wxHORIZONTAL);
                itemPanelBottomSizer->Add(hiddenButton, 0, wxALIGN_CENTRE_VERTICAL);
                itemPanelBottomSizer->Add(lockButton, 0, wxALIGN_CENTRE_VERTICAL);
                itemPanelBottomSizer->Add(m_infoText, 0, wxALIGN_CENTRE_VERTICAL);
                itemPanelBottomSizer->AddStretchSpacer();
                itemPanelBottomSizer->AddSpacer(LayoutConstants::NarrowHMargin);

                wxSizer* itemPanelSizer = new wxBoxSizer(wxVERTICAL);
                itemPanelSizer->AddSpacer(LayoutConstants::NarrowVMargin);
                itemPanelSizer->Add(itemPanelTopSizer,    0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
                itemPanelSizer->Add(itemPanelBottomSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::NarrowHMargin);
                itemPanelSizer->AddSpacer(LayoutConstants::NarrowVMargin);
                SetSizer(itemPanelSizer);
            }

            Model::Layer* layer() const {
                return m_layer;
            }

            void refresh() {
                wxWindowUpdateLocker locker(this);
                
                m_nameText->SetLabel(m_layer->name());
                if (lock(m_document)->currentLayer() == m_layer)
                    m_nameText->SetFont(GetFont().Bold());
                else
                    m_nameText->SetFont(GetFont());

                wxString info;
                info << m_layer->childCount() << " " << StringUtils::safePlural(m_layer->childCount(), "object", "objects");
                m_infoText->SetLabel(info);
                m_infoText->SetFont(GetFont());
                
                Layout();
            }

            void OnToggleVisible(wxCommandEvent& event) {
                LayerCommand* command = new LayerCommand(LAYER_TOGGLE_VISIBLE_EVENT);
                command->SetId(GetId());
                command->SetEventObject(this);
                command->setLayer(m_layer);
                QueueEvent(command);
            }

            void OnUpdateVisibleButton(wxUpdateUIEvent& event) {
                event.Check(m_layer->hidden());

                MapDocumentSPtr document = lock(m_document);
                event.Enable(m_layer->hidden() || m_layer != document->currentLayer());
            }

            void OnToggleLocked(wxCommandEvent& event) {
                LayerCommand* command = new LayerCommand(LAYER_TOGGLE_LOCKED_EVENT);
                command->SetId(GetId());
                command->SetEventObject(this);
                command->setLayer(m_layer);
                QueueEvent(command);
            }

            void OnUpdateLockButton(wxUpdateUIEvent& event) {
                event.Check(m_layer->locked());

                MapDocumentSPtr document = lock(m_document);
                event.Enable(m_layer->locked() || m_layer != document->currentLayer());
            }
        private:
            void setDefaultColours(const wxColour& foreground, const wxColour& background) {
                Item::setDefaultColours(foreground, background);
                m_infoText->SetForegroundColour(makeLighter(m_infoText->GetForegroundColour()));
            }
        };

        LayerListBox::LayerListBox(wxWindow* parent, MapDocumentWPtr document) :
        ControlListBox(parent, true),
        m_document(document) {
            bindObservers();
            bindEvents();
        }

        LayerListBox::~LayerListBox() {
            unbindObservers();
        }

        Model::Layer* LayerListBox::selectedLayer() const {
            if (GetSelection() == wxNOT_FOUND)
                return NULL;

            const Model::World* world = lock(m_document)->world();
            const Model::LayerList layers = world->allLayers();

            const size_t index = static_cast<size_t>(GetSelection());
            ensure(index < layers.size(), "index out of range");
            return layers[index];
        }

        void LayerListBox::setSelectedLayer(Model::Layer* layer) {
            SetSelection(wxNOT_FOUND);
            for (size_t i = 0; i < m_items.size(); ++i) {
                LayerItem* item = static_cast<LayerItem*>(m_items[i]);
                if (item->layer() == layer) {
                    SetSelection(static_cast<int>(i));
                    break;
                }
            }
            Refresh();
        }

        void LayerListBox::OnSelectionChanged(wxCommandEvent& event) {
            LayerCommand* command = new LayerCommand(LAYER_SELECTED_EVENT);
            command->SetId(GetId());
            command->SetEventObject(this);
            command->setLayer(selectedLayer());
            QueueEvent(command);
        }
        
        void LayerListBox::OnDoubleClick(wxCommandEvent& event) {
            LayerCommand* command = new LayerCommand(LAYER_SET_CURRENT_EVENT);
            command->SetId(GetId());
            command->SetEventObject(this);
            command->setLayer(selectedLayer());
            QueueEvent(command);
        }

        void LayerListBox::OnRightClick(wxCommandEvent& event) {
            Model::Layer* layer = selectedLayer();
            if (layer != NULL) {
                LayerCommand* command = new LayerCommand(LAYER_RIGHT_CLICK_EVENT);
                command->SetId(GetId());
                command->SetEventObject(this);
                command->setLayer(layer);
                QueueEvent(command);
            }
        }

        void LayerListBox::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->documentWasLoadedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->documentWasClearedNotifier.addObserver(this, &LayerListBox::documentDidChange);
            document->currentLayerDidChangeNotifier.addObserver(this, &LayerListBox::currentLayerDidChange);
            document->nodesWereAddedNotifier.addObserver(this, &LayerListBox::nodesDidChange);
            document->nodesWereRemovedNotifier.addObserver(this, &LayerListBox::nodesDidChange);
            document->nodesDidChangeNotifier.addObserver(this, &LayerListBox::nodesDidChange);
        }

        void LayerListBox::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->documentWasLoadedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->documentWasClearedNotifier.removeObserver(this, &LayerListBox::documentDidChange);
                document->currentLayerDidChangeNotifier.removeObserver(this, &LayerListBox::currentLayerDidChange);
                document->nodesWereAddedNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodesWereRemovedNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
                document->nodesDidChangeNotifier.removeObserver(this, &LayerListBox::nodesDidChange);
            }
        }

        void LayerListBox::documentDidChange(MapDocument* document) {
            const Model::World* world = document->world();
            if (world != NULL) {
                SetItemCount(world->allLayers().size());
            } else {
                SetItemCount(0);
            }
        }

        void LayerListBox::nodesDidChange(const Model::NodeList& nodes) {
            MapDocumentSPtr document = lock(m_document);
            const Model::World* world = document->world();
            if (world != NULL) {
                SetItemCount(world->allLayers().size());
            } else {
                SetItemCount(0);
            }
        }

        void LayerListBox::currentLayerDidChange() {
            MapDocumentSPtr document = lock(m_document);
            const Model::World* world = document->world();
            if (world != NULL) {
                SetItemCount(world->allLayers().size());
            } else {
                SetItemCount(0);
            }
        }

        void LayerListBox::bindEvents() {
            Bind(wxEVT_LISTBOX, &LayerListBox::OnSelectionChanged, this);
            Bind(wxEVT_LISTBOX_DCLICK, &LayerListBox::OnDoubleClick, this);
            Bind(wxEVT_LISTBOX_RCLICK, &LayerListBox::OnRightClick, this);
        }
        
        ControlListBox::Item* LayerListBox::createItem(wxWindow* parent, const wxSize& margins, const size_t index) {
            MapDocumentSPtr document = lock(m_document);
            const Model::World* world = document->world();
            ensure(world != NULL, "world is null");
            
            const Model::LayerList layers = world->allLayers();
            ensure(index < layers.size(), "index out of range");
            
            return new LayerItem(parent, document, layers[index], margins);
        }
    }
}
