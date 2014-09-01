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
        
        void LayerEditor::OnAddLayerClicked(wxCommandEvent& event) {
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
            }
        }
        
        void LayerEditor::OnRemoveLayerClicked(wxCommandEvent& event) {
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
            
            wxBitmapButton* addLayerButton = createBitmapButton(this, "Add.png", "Add a new layer from the current selection");
            wxBitmapButton* removeLayerButton = createBitmapButton(this, "Remove.png", "Remove the selected layer and move its objects to the default layer");
            
            addLayerButton->Bind(wxEVT_BUTTON, &LayerEditor::OnAddLayerClicked, this);
            removeLayerButton->Bind(wxEVT_BUTTON, &LayerEditor::OnRemoveLayerClicked, this);
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
