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

#include "LayerEditor.h"

#include "Model/Layer.h"
#include "Model/Map.h"
#include "View/ControllerFacade.h"
#include "View/LayerListView.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/bmpbuttn.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/textdlg.h>

namespace TrenchBroom {
    namespace View {
        LayerEditor::LayerEditor(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_layerList(NULL) {
            createGui();
        }
        
        void LayerEditor::OnCurrentLayerSelected(wxListEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Model::Map* map = document->map();
            const Model::LayerList& layers = map->layers();
            
            const size_t index = static_cast<size_t>(event.GetIndex());
            assert(index < layers.size());
            
            document->setCurrentLayer(layers[index]);
        }
        
        void LayerEditor::OnCurrentLayerDeselected(wxListEvent& event) {
            lock(m_document)->setCurrentLayer(NULL);
        }

        void LayerEditor::OnLayerRightClick(wxListEvent& event) {
            if (event.GetIndex() < 0)
                return;
            
            wxMenu popupMenu;
            popupMenu.Append(MoveSelectionToLayerCommandId, "Move selection to layer");
            popupMenu.Append(SelectAllInLayerCommandId, "Select all in layer");
            popupMenu.AppendSeparator();
            popupMenu.Append(RemoveLayerCommandId, "Remove layer");
            
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnMoveSelectionToLayer, this, MoveSelectionToLayerCommandId);
            popupMenu.Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateMoveSelectionToLayerUI, this, MoveSelectionToLayerCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnSelectAllInLayer, this, SelectAllInLayerCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnRemoveLayer, this, RemoveLayerCommandId);
            popupMenu.Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateRemoveLayerUI, this, RemoveLayerCommandId);
            
            PopupMenu(&popupMenu);
        }

        void LayerEditor::OnMoveSelectionToLayer(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Model::Map* map = document->map();
            const Model::LayerList& layers = map->layers();
            
            const size_t index = m_layerList->getSelection();
            assert(index < layers.size());
            Model::Layer* layer = layers[index];

            const Model::ObjectList& objects = document->selectedObjects();
            assert(!objects.empty());
            
            ControllerSPtr controller = lock(m_controller);
            controller->moveSelectionToLayer(layer);
        }
        
        void LayerEditor::OnUpdateMoveSelectionToLayerUI(wxUpdateUIEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty()) {
                event.Enable(false);
                return;
            }

            Model::Map* map = document->map();
            const Model::LayerList& layers = map->layers();
            
            const size_t index = m_layerList->getSelection();
            assert(index < layers.size());
            const Model::Layer* layer = layers[index];

            Model::ObjectList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                const Model::Object* object = *it;
                if (object->layer() != layer) {
                    event.Enable(true);
                    return;
                }
            }
            
            event.Enable(false);
        }
        
        void LayerEditor::OnSelectAllInLayer(wxCommandEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            Model::Map* map = document->map();
            const Model::LayerList& layers = map->layers();
            
            const size_t index = m_layerList->getSelection();
            assert(index < layers.size());
            Model::Layer* layer = layers[index];
            const Model::ObjectList& objects = layer->objects();
            
            ControllerSPtr controller = lock(m_controller);
            controller->deselectAllAndSelectObjects(objects);
        }

        void LayerEditor::OnAddLayer(wxCommandEvent& event) {
            wxTextEntryDialog dialog(this, "Enter a name for the new layer", "New Layer Name", "Unnamed");
            dialog.CentreOnParent();
            dialog.SetTextValidator(wxFILTER_EMPTY);
            if (dialog.ShowModal() == wxID_OK) {
                const String name = dialog.GetValue().ToStdString();

                MapDocumentSPtr document = lock(m_document);
                Model::Map* map = document->map();
                Model::Layer* layer = map->createLayer(name);
                
                const Model::ObjectList& objects = document->selectedObjects();
                
                ControllerSPtr controller = lock(m_controller);
                
                UndoableCommandGroup group(controller);
                controller->addLayer(layer);
                if (!objects.empty())
                    controller->moveSelectionToLayer(layer);
                
                const long index = static_cast<long>(map->layers().size() - 1);
                m_layerList->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
        }
        
        void LayerEditor::OnRemoveLayer(wxCommandEvent& event) {
            assert(m_layerList->GetSelectedItemCount() > 0);
            const size_t index = m_layerList->getSelection();
            
            MapDocumentSPtr document = lock(m_document);
            Model::Map* map = document->map();
            const Model::LayerList& layers = map->layers();
            assert(index < layers.size());
            
            Model::Layer* layer = layers[index];
            const Model::ObjectList& objects = layer->objects();
            
            ControllerSPtr controller = lock(m_controller);
            UndoableCommandGroup group(controller, "Remove Layer");
            if (!objects.empty())
                controller->moveObjectsToLayer(objects, map->defaultLayer());
            controller->removeLayer(layer);
        }
        
        void LayerEditor::OnUpdateRemoveLayerUI(wxUpdateUIEvent& event) {
            if (m_layerList->GetSelectedItemCount() == 0) {
                event.Enable(false);
                return;
            }
            
            MapDocumentSPtr document = lock(m_document);
            Model::Map* map = document->map();
            
            const size_t index = m_layerList->getSelection();
            const Model::Layer* layer = map->layers()[index];
            event.Enable(map->canRemoveLayer(layer));
        }

        void LayerEditor::createGui() {
            SetBackgroundColour(*wxWHITE);

            m_layerList = new LayerListView(this, m_document, m_controller);
            m_layerList->Bind(wxEVT_LIST_ITEM_SELECTED, &LayerEditor::OnCurrentLayerSelected, this);
            m_layerList->Bind(wxEVT_LIST_ITEM_DESELECTED, &LayerEditor::OnCurrentLayerDeselected, this);
            m_layerList->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &LayerEditor::OnLayerRightClick, this);

            wxBitmapButton* addLayerButton = createBitmapButton(this, "Add.png", "Add a new layer from the current selection");
            wxBitmapButton* removeLayerButton = createBitmapButton(this, "Remove.png", "Remove the selected layer and move its objects to the default layer");
            
            addLayerButton->Bind(wxEVT_BUTTON, &LayerEditor::OnAddLayer, this);
            removeLayerButton->Bind(wxEVT_BUTTON, &LayerEditor::OnRemoveLayer, this);
            removeLayerButton->Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateRemoveLayerUI, this);
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addLayerButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(removeLayerButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_layerList, 1, wxEXPAND);
            sizer->Add(buttonSizer, 0, wxEXPAND);
            SetSizer(sizer);
        }
    }
}
