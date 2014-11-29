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

#include "Model/Brush.h"
#include "Model/CollectSelectableNodesVisitor.h"
#include "Model/Entity.h"
#include "Model/FindLayerVisitor.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"
#include "View/LayerListView.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/bmpbuttn.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/textdlg.h>

#include <set>

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
            popupMenu.Append(ToggleLayerVisibleCommandId, layer->hidden() ? "Show layer" : "Hide layer");
            popupMenu.Append(ToggleLayerLockedCommandId, layer->locked() ? "Unlock layer" : "Lock layer");
            popupMenu.AppendSeparator();
            popupMenu.Append(RemoveLayerCommandId, "Remove layer");
            
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnMoveSelectionToLayer, this, MoveSelectionToLayerCommandId);
            popupMenu.Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateMoveSelectionToLayerUI, this, MoveSelectionToLayerCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnSelectAllInLayer, this, SelectAllInLayerCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnToggleLayerVisibleFromMenu, this, ToggleLayerVisibleCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnToggleLayerLockedFromMenu, this, ToggleLayerLockedCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnRemoveLayer, this, RemoveLayerCommandId);
            popupMenu.Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateRemoveLayerUI, this, RemoveLayerCommandId);
            
            PopupMenu(&popupMenu);
        }

        void LayerEditor::OnToggleLayerVisibleFromMenu(wxCommandEvent& event) {
            toggleLayerVisible(m_layerList->selectedLayer());
        }
        
        void LayerEditor::OnToggleLayerVisibleFromList(LayerCommand& event) {
            toggleLayerVisible(event.layer());
        }

        void LayerEditor::toggleLayerVisible(Model::Layer* layer) {
            assert(layer != NULL);
            MapDocumentSPtr document = lock(m_document);
            document->setLayerHidden(layer, !layer->hidden());
        }

        void LayerEditor::OnToggleLayerLockedFromMenu(wxCommandEvent& event) {
            toggleLayerLocked(m_layerList->selectedLayer());
        }
        
        void LayerEditor::OnToggleLayerLockedFromList(LayerCommand& event) {
            toggleLayerLocked(event.layer());
        }

        void LayerEditor::toggleLayerLocked(Model::Layer* layer) {
            assert(layer != NULL);
            MapDocumentSPtr document = lock(m_document);
            document->setLayerLocked(layer, !layer->locked());
        }

        class LayerEditor::CollectMoveableNodes : public Model::NodeVisitor {
        private:
            Model::World* m_world;
            Model::NodeSet m_selectNodes;
            Model::NodeSet m_moveNodes;
        public:
            CollectMoveableNodes(Model::World* world) : m_world(world) {}
            
            const Model::NodeList selectNodes() const {
                return Model::NodeList(m_selectNodes.begin(), m_selectNodes.end());
            }
            
            const Model::NodeList moveNodes() const {
                return Model::NodeList(m_moveNodes.begin(), m_moveNodes.end());
            }
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            
            void doVisit(Model::Group* group)   {
                assert(group->selected());
                m_moveNodes.insert(group);
            }
            
            void doVisit(Model::Entity* entity) {
                assert(entity->selected());
                m_moveNodes.insert(entity);
            }
            
            void doVisit(Model::Brush* brush)   {
                assert(brush->selected());
                Model::Attributable* entity = brush->entity();
                if (entity == m_world) {
                    m_moveNodes.insert(brush);
                } else {
                    if (m_moveNodes.insert(entity).second) {
                        const Model::NodeList& siblings = entity->children();
                        Model::NodeList::const_iterator it, end;
                        for (it = siblings.begin(), end = siblings.end(); it != end; ++it) {
                            Model::Node* sibling = *it;
                            if (!sibling->selected())
                                m_selectNodes.insert(sibling);
                        }
                    }
                }
            }
        };
        
        void LayerEditor::OnMoveSelectionToLayer(wxCommandEvent& event) {
            Model::Layer* layer = m_layerList->selectedLayer();
            assert(layer != NULL);

            MapDocumentSPtr document = lock(m_document);
            Transaction transaction(document, "Move Nodes to " + layer->name());
            moveSelectedNodesToLayer(document, layer);
        }
        
        void LayerEditor::OnUpdateMoveSelectionToLayerUI(wxUpdateUIEvent& event) {
            const Model::Layer* layer = m_layerList->selectedLayer();
            if (layer == NULL) {
                event.Enable(false);
                return;
            }

            MapDocumentSPtr document = lock(m_document);
            const Model::NodeList& nodes = document->selectedNodes().nodes();
            if (nodes.empty()) {
                event.Enable(false);
                return;
            }

            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                Model::Layer* nodeLayer = Model::findLayer(node);
                if (nodeLayer != layer) {
                    event.Enable(true);
                    return;
                }
            }
            
            event.Enable(false);
        }
        
        void LayerEditor::OnSelectAllInLayer(wxCommandEvent& event) {
            Model::Layer* layer = m_layerList->selectedLayer();
            assert(layer != NULL);
            
            MapDocumentSPtr document = lock(m_document);
            
            Model::CollectSelectableNodesVisitor visitor(document->editorContext());
            layer->recurse(visitor);
            
            const Model::NodeList& nodes = visitor.nodes();
            document->deselectAll();
            document->select(nodes);
        }

        void LayerEditor::OnAddLayer(wxCommandEvent& event) {
            wxTextEntryDialog dialog(this, "Enter a name for the new layer", "New Layer Name", "Unnamed");
            dialog.CentreOnParent();
            dialog.SetTextValidator(wxFILTER_EMPTY);
            if (dialog.ShowModal() == wxID_OK) {
                const String name = dialog.GetValue().ToStdString();

                MapDocumentSPtr document = lock(m_document);
                Model::World* world = document->world();
                Model::Layer* layer = world->createLayer(name);
                
                Transaction transaction(document, "Create Layer " + layer->name());
                document->addNode(layer, world);
                if (document->hasSelectedNodes())
                    moveSelectedNodesToLayer(document, layer);
                m_layerList->setSelectedLayer(layer);
            }
        }
        
        void LayerEditor::OnRemoveLayer(wxCommandEvent& event) {
            Model::Layer* layer = m_layerList->selectedLayer();
            assert(layer != NULL);
            
            MapDocumentSPtr document = lock(m_document);
            Model::Layer* defaultLayer = document->world()->defaultLayer();
            
            Model::CollectSelectableNodesVisitor collectSelectableNodes(document->editorContext());
            layer->recurse(collectSelectableNodes);
            
            Transaction transaction(document, "Remove Layer " + layer->name());
            document->deselectAll();
            document->select(collectSelectableNodes.nodes());
            document->reparentNodes(defaultLayer, layer->children());
            document->removeNode(layer);
        }
        
        void LayerEditor::OnUpdateRemoveLayerUI(wxUpdateUIEvent& event) {
            const Model::Layer* layer = m_layerList->selectedLayer();
            if (layer == NULL) {
                event.Enable(false);
                return;
            }
            
            MapDocumentSPtr document = lock(m_document);
            event.Enable(layer != document->world()->defaultLayer());
        }

        void LayerEditor::moveSelectedNodesToLayer(MapDocumentSPtr document, Model::Layer* layer) {
            const Model::NodeList& selectedNodes = document->selectedNodes().nodes();
            
            CollectMoveableNodes visitor(document->world());
            Model::Node::accept(selectedNodes.begin(), selectedNodes.end(), visitor);
            
            const Model::NodeList moveNodes = visitor.moveNodes();
            if (!moveNodes.empty()) {
                const Model::NodeList selectNodes = visitor.selectNodes();
                document->deselectAll();
                document->select(visitor.selectNodes());
                document->reparentNodes(layer, visitor.moveNodes());
            }
        }

        void LayerEditor::createGui() {
            SetBackgroundColour(*wxWHITE);

            m_layerList = new LayerListView(this, m_document);
            m_layerList->Bind(LAYER_SELECTED_EVENT, &LayerEditor::OnCurrentLayerSelected, this);
            m_layerList->Bind(LAYER_RIGHT_CLICK_EVENT, &LayerEditor::OnLayerRightClick, this);
            m_layerList->Bind(LAYER_TOGGLE_VISIBLE_EVENT, &LayerEditor::OnToggleLayerVisibleFromList, this);
            m_layerList->Bind(LAYER_TOGGLE_LOCKED_EVENT, &LayerEditor::OnToggleLayerLockedFromList, this);

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
