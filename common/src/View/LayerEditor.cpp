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
        LayerEditor::LayerEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document),
        m_layerList(NULL) {
            createGui();
        }

        void LayerEditor::OnCurrentLayerSelected(LayerCommand& event) {
            MapDocumentSPtr document = lock(m_document);
            document->setCurrentLayer(event.layer());
        }

        void LayerEditor::OnLayerRightClick(LayerCommand& event) {
            const Model::Layer* layer = event.layer();
            
            wxMenu popupMenu;
            popupMenu.Append(MoveSelectionToLayerCommandId, "Move selection to layer");
            popupMenu.Append(SelectAllInLayerCommandId, "Select all in layer");
            popupMenu.AppendSeparator();
            popupMenu.Append(ToggleLayerVisibleCommandId, layer->visible() ? "Hide layer" : "Show layer");
            popupMenu.Append(ToggleLayerLockedCommandId, layer->locked() ? "Unlock layer" : "Lock layer");
            popupMenu.AppendSeparator();
            popupMenu.Append(RemoveLayerCommandId, "Remove layer");
            
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnMoveSelectionToLayer, this, MoveSelectionToLayerCommandId);
            popupMenu.Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateMoveSelectionToLayerUI, this, MoveSelectionToLayerCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnSelectAllInLayer, this, SelectAllInLayerCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnToggleLayerVisible, this, ToggleLayerVisibleCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnToggleLayerLocked, this, ToggleLayerLockedCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnRemoveLayer, this, RemoveLayerCommandId);
            popupMenu.Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateRemoveLayerUI, this, RemoveLayerCommandId);
            
            PopupMenu(&popupMenu);
        }

        void LayerEditor::OnToggleLayerVisible(wxCommandEvent& event) {
            Model::Layer* layer = m_layerList->selectedLayer();
            assert(layer != NULL);
            
            layer->setVisible(!layer->visible());
        }
        
        void LayerEditor::OnToggleLayerLocked(wxCommandEvent& event) {
            Model::Layer* layer = m_layerList->selectedLayer();
            assert(layer != NULL);

            layer->setLocked(!layer->locked());
        }
        
        void LayerEditor::OnMoveSelectionToLayer(wxCommandEvent& event) {
            Model::Layer* layer = m_layerList->selectedLayer();
            assert(layer != NULL);

            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            assert(!objects.empty());
            
            ControllerSPtr controller = lock(m_controller);
            controller->moveSelectionToLayer(layer);
        }
        
        void LayerEditor::OnUpdateMoveSelectionToLayerUI(wxUpdateUIEvent& event) {
            const Model::Layer* layer = m_layerList->selectedLayer();
            if (layer == NULL) {
                event.Enable(false);
                return;
            }

            MapDocumentSPtr document = lock(m_document);
            const Model::ObjectList& objects = document->selectedObjects();
            if (objects.empty()) {
                event.Enable(false);
                return;
            }

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
            Model::Layer* layer = m_layerList->selectedLayer();
            assert(layer != NULL);
            
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
                
                m_layerList->setSelectedLayer(layer);
            }
        }
        
        void LayerEditor::OnRemoveLayer(wxCommandEvent& event) {
            Model::Layer* layer = m_layerList->selectedLayer();
            assert(layer != NULL);

            MapDocumentSPtr document = lock(m_document);
            Model::Map* map = document->map();
            const Model::ObjectList& objects = layer->objects();
            
            ControllerSPtr controller = lock(m_controller);
            UndoableCommandGroup group(controller, "Remove Layer");
            if (!objects.empty())
                controller->moveObjectsToLayer(objects, map->defaultLayer());
            controller->removeLayer(layer);
        }
        
        void LayerEditor::OnUpdateRemoveLayerUI(wxUpdateUIEvent& event) {
            const Model::Layer* layer = m_layerList->selectedLayer();
            if (layer == NULL) {
                event.Enable(false);
                return;
            }
            
            MapDocumentSPtr document = lock(m_document);
            Model::Map* map = document->map();
            event.Enable(map->canRemoveLayer(layer));
        }

        void LayerEditor::createGui() {
            SetBackgroundColour(*wxWHITE);

            m_layerList = new LayerListView(this, m_document, m_controller);
            m_layerList->Bind(LAYER_SELECTED_EVENT, &LayerEditor::OnCurrentLayerSelected, this);
            m_layerList->Bind(LAYER_RIGHT_CLICK_EVENT, &LayerEditor::OnLayerRightClick, this);

            wxWindow* addLayerButton = createBitmapButton(this, "Add.png", "Add a new layer from the current selection");
            wxWindow* removeLayerButton = createBitmapButton(this, "Remove.png", "Remove the selected layer and move its objects to the default layer");
            
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
