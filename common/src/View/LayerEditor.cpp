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
#include "Model/FindGroupVisitor.h"
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

        void LayerEditor::OnSetCurrentLayer(LayerCommand& event) {
            if (IsBeingDeleted()) return;

            Model::Layer* layer = event.layer();
            MapDocumentSPtr document = lock(m_document);
            if (layer->locked())
                document->resetLock(Model::NodeList(1, layer));
            if (layer->hidden())
                document->resetVisibility(Model::NodeList(1, layer));
            document->setCurrentLayer(event.layer());
        }

        void LayerEditor::OnLayerRightClick(LayerCommand& event) {
            if (IsBeingDeleted()) return;

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
            popupMenu.Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateToggleLayerVisibleUI, this, ToggleLayerVisibleCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnToggleLayerLockedFromMenu, this, ToggleLayerLockedCommandId);
            popupMenu.Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateToggleLayerLockedUI, this, ToggleLayerLockedCommandId);
            popupMenu.Bind(wxEVT_MENU, &LayerEditor::OnRemoveLayer, this, RemoveLayerCommandId);
            popupMenu.Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateRemoveLayerUI, this, RemoveLayerCommandId);
            
            PopupMenu(&popupMenu);
        }

        void LayerEditor::OnToggleLayerVisibleFromMenu(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            toggleLayerVisible(m_layerList->selectedLayer());
        }
        
        void LayerEditor::OnToggleLayerVisibleFromList(LayerCommand& event) {
            if (IsBeingDeleted()) return;

            toggleLayerVisible(event.layer());
        }

        void LayerEditor::OnUpdateToggleLayerVisibleUI(wxUpdateUIEvent& event) {
            Model::Layer* layer = m_layerList->selectedLayer();
            if (layer == NULL) {
                event.Enable(false);
                return;
            }
            
            MapDocumentSPtr document = lock(m_document);
            if (!layer->hidden() && layer == document->currentLayer()) {
                event.Enable(false);
                return;
            }
            
            event.Enable(true);
        }

        void LayerEditor::toggleLayerVisible(Model::Layer* layer) {
            assert(layer != NULL);
            MapDocumentSPtr document = lock(m_document);
            if (!layer->hidden())
                document->hide(Model::NodeList(1, layer));
            else
                document->resetVisibility(Model::NodeList(1, layer));
        }

        void LayerEditor::OnToggleLayerLockedFromMenu(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            toggleLayerLocked(m_layerList->selectedLayer());
        }
        
        void LayerEditor::OnToggleLayerLockedFromList(LayerCommand& event) {
            if (IsBeingDeleted()) return;

            toggleLayerLocked(event.layer());
        }

        void LayerEditor::OnUpdateToggleLayerLockedUI(wxUpdateUIEvent& event) {
            Model::Layer* layer = m_layerList->selectedLayer();
            if (layer == NULL) {
                event.Enable(false);
                return;
            }
            
            MapDocumentSPtr document = lock(m_document);
            if (!layer->locked() && layer == document->currentLayer()) {
                event.Enable(false);
                return;
            }
            
            event.Enable(true);
        }

        void LayerEditor::toggleLayerLocked(Model::Layer* layer) {
            assert(layer != NULL);
            MapDocumentSPtr document = lock(m_document);
            if (!layer->locked())
                document->lock(Model::NodeList(1, layer));
            else
                document->resetLock(Model::NodeList(1, layer));
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

                if (group->group() == NULL) {
                    m_moveNodes.insert(group);
                    m_selectNodes.insert(group);
                }
            }
            
            void doVisit(Model::Entity* entity) {
                assert(entity->selected());
                
                if (entity->group() == NULL) {
                    m_moveNodes.insert(entity);
                    m_selectNodes.insert(entity);
                }
            }
            
            void doVisit(Model::Brush* brush)   {
                assert(brush->selected());
                if (brush->group() == NULL) {
                    Model::AttributableNode* entity = brush->entity();
                    if (entity == m_world) {
                        m_moveNodes.insert(brush);
                        m_selectNodes.insert(brush);
                    } else {
                        if (m_moveNodes.insert(entity).second) {
                            const Model::NodeList& siblings = entity->children();
                            m_selectNodes.insert(siblings.begin(), siblings.end());
                        }
                    }
                }
            }
        };
        
        void LayerEditor::OnMoveSelectionToLayer(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            Model::Layer* layer = m_layerList->selectedLayer();
            assert(layer != NULL);

            MapDocumentSPtr document = lock(m_document);
            Transaction transaction(document, "Move Nodes to " + layer->name());
            moveSelectedNodesToLayer(document, layer);
        }
        
        void LayerEditor::OnUpdateMoveSelectionToLayerUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

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
                Model::Group* nodeGroup = Model::findGroup(node);
                if (nodeGroup != NULL) {
                    event.Enable(false);
                    return;
                }
            }

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
            if (IsBeingDeleted()) return;

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
            if (IsBeingDeleted()) return;

            const String name = queryLayerName();
            if (!name.empty()) {
                MapDocumentSPtr document = lock(m_document);
                Model::World* world = document->world();
                Model::Layer* layer = world->createLayer(name, document->worldBounds());
                
                Transaction transaction(document, "Create Layer " + layer->name());
                document->addNode(layer, world);
                document->setCurrentLayer(layer);
                m_layerList->setSelectedLayer(layer);
            }
        }
        
        String LayerEditor::queryLayerName() {
            while (true) {
                wxTextEntryDialog dialog(this, "Enter a name", "Layer Name", "Unnamed");
                dialog.CentreOnParent();
                if (dialog.ShowModal() != wxID_OK)
                    return "";
                
                const String name = dialog.GetValue().ToStdString();
                if (StringUtils::isBlank(name)) {
                    if (wxMessageBox("Layer names cannot be blank.", "Error", wxOK | wxCANCEL | wxCENTRE, this) != wxOK)
                        return "";
                } else if (StringUtils::containsCaseInsensitive(name, "\"")) {
                    if (wxMessageBox("Layer names cannot contain double quotes.", "Error", wxOK | wxCANCEL | wxCENTRE, this) != wxOK)
                        return "";
                } else {
                    return name;
                }
            }
        }

        void LayerEditor::OnRemoveLayer(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            Model::Layer* layer = m_layerList->selectedLayer();
            assert(layer != NULL);
            
            Model::Layer* newCurrentLayer = findVisibleAndUnlockedLayer(layer);
            assert(newCurrentLayer != NULL);

            MapDocumentSPtr document = lock(m_document);
            Model::Layer* defaultLayer = document->world()->defaultLayer();
            
            Model::CollectSelectableNodesVisitor collectSelectableNodes(document->editorContext());
            layer->recurse(collectSelectableNodes);
            
            Transaction transaction(document, "Remove Layer " + layer->name());
            document->deselectAll();
            document->select(collectSelectableNodes.nodes());
            if (layer->hasChildren())
                document->reparentNodes(defaultLayer, layer->children());
            if (document->currentLayer() == layer)
                document->setCurrentLayer(newCurrentLayer);
            document->removeNode(layer);
        }
        
        void LayerEditor::OnUpdateRemoveLayerUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            const Model::Layer* layer = m_layerList->selectedLayer();
            if (layer == NULL) {
                event.Enable(false);
                return;
            }

            if (findVisibleAndUnlockedLayer(layer) == NULL) {
                event.Enable(false);
                return;
            }
            
            MapDocumentSPtr document = lock(m_document);
            event.Enable(layer != document->world()->defaultLayer());
        }

        void LayerEditor::OnShowAllLayers(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            MapDocumentSPtr document = lock(m_document);
            const Model::LayerList& layers = document->world()->allLayers();
            document->resetVisibility(Model::NodeList(layers.begin(), layers.end()));
        }

        Model::Layer* LayerEditor::findVisibleAndUnlockedLayer(const Model::Layer* except) const {
            MapDocumentSPtr document = lock(m_document);
            if (!document->world()->defaultLayer()->locked() && !document->world()->defaultLayer()->hidden())
                return document->world()->defaultLayer();
            
            const Model::LayerList& layers = document->world()->customLayers();
            Model::LayerList::const_iterator it, end;
            for (it = layers.begin(), end = layers.end(); it != end; ++it) {
                Model::Layer* layer = *it;
                if (layer != except && !layer->locked() && !layer->hidden())
                    return layer;
            }
            
            return NULL;
        }

        void LayerEditor::moveSelectedNodesToLayer(MapDocumentSPtr document, Model::Layer* layer) {
            const Model::NodeList& selectedNodes = document->selectedNodes().nodes();
            
            CollectMoveableNodes visitor(document->world());
            Model::Node::accept(selectedNodes.begin(), selectedNodes.end(), visitor);
            
            const Model::NodeList moveNodes = visitor.moveNodes();
            if (!moveNodes.empty()) {
                const Model::NodeList selectNodes = visitor.selectNodes();
                document->deselectAll();
                document->reparentNodes(layer, visitor.moveNodes());
                if (!layer->hidden() && !layer->locked())
                    document->select(visitor.selectNodes());
            }
        }

        void LayerEditor::createGui() {
            SetBackgroundColour(*wxWHITE);

            m_layerList = new LayerListView(this, m_document);
            m_layerList->Bind(LAYER_SET_CURRENT_EVENT, &LayerEditor::OnSetCurrentLayer, this);
            m_layerList->Bind(LAYER_RIGHT_CLICK_EVENT, &LayerEditor::OnLayerRightClick, this);
            m_layerList->Bind(LAYER_TOGGLE_VISIBLE_EVENT, &LayerEditor::OnToggleLayerVisibleFromList, this);
            m_layerList->Bind(LAYER_TOGGLE_LOCKED_EVENT, &LayerEditor::OnToggleLayerLockedFromList, this);

            wxWindow* addLayerButton = createBitmapButton(this, "Add.png", "Add a new layer from the current selection");
            wxWindow* removeLayerButton = createBitmapButton(this, "Remove.png", "Remove the selected layer and move its objects to the default layer");
            wxWindow* showAllLayersButton = createBitmapButton(this, "Visible.png", "Show all layers");
            
            addLayerButton->Bind(wxEVT_BUTTON, &LayerEditor::OnAddLayer, this);
            removeLayerButton->Bind(wxEVT_BUTTON, &LayerEditor::OnRemoveLayer, this);
            removeLayerButton->Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateRemoveLayerUI, this);
            showAllLayersButton->Bind(wxEVT_BUTTON, &LayerEditor::OnShowAllLayers, this);
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addLayerButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(removeLayerButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(showAllLayersButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_layerList, 1, wxEXPAND);
            sizer->Add(buttonSizer, 0, wxEXPAND);
            SetSizer(sizer);
        }
    }
}
