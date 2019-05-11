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

#include "LayerEditor.h"

#include "Model/Brush.h"
#include "Model/CollectSelectableNodesVisitor.h"
#include "Model/Entity.h"
#include "Model/FindGroupVisitor.h"
#include "Model/FindLayerVisitor.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"
#include "View/BorderLine.h"
#include "View/LayerListBox.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/bmpbuttn.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/textdlg.h>

#include <set>

namespace TrenchBroom {
    namespace View {
        LayerEditor::LayerEditor(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_document(document),
        m_layerList(nullptr) {
            createGui();
        }

        void LayerEditor::OnSetCurrentLayer(LayerCommand& event) {
            if (IsBeingDeleted()) return;

            auto* layer = event.layer();
            auto document = lock(m_document);
            if (layer->locked()) {
                document->resetLock(Model::NodeList(1, layer));
            }
            if (layer->hidden()) {
                document->resetVisibility(Model::NodeList(1, layer));
            }
            document->setCurrentLayer(event.layer());
        }

        void LayerEditor::OnLayerRightClick(LayerCommand& event) {
            if (IsBeingDeleted()) return;

            const auto* layer = event.layer();

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
            auto* layer = m_layerList->selectedLayer();
            if (layer == nullptr) {
                event.Enable(false);
                return;
            }

            auto document = lock(m_document);
            if (!layer->hidden() && layer == document->currentLayer()) {
                event.Enable(false);
                return;
            }

            event.Enable(true);
        }

        void LayerEditor::toggleLayerVisible(Model::Layer* layer) {
            ensure(layer != nullptr, "layer is null");
            auto document = lock(m_document);
            if (!layer->hidden()) {
                document->hide(Model::NodeList(1, layer));
            } else {
                document->resetVisibility(Model::NodeList(1, layer));
            }
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
            auto* layer = m_layerList->selectedLayer();
            if (layer == nullptr) {
                event.Enable(false);
                return;
            }

            auto document = lock(m_document);
            if (!layer->locked() && layer == document->currentLayer()) {
                event.Enable(false);
                return;
            }

            event.Enable(true);
        }

        void LayerEditor::toggleLayerLocked(Model::Layer* layer) {
            ensure(layer != nullptr, "layer is null");
            auto document = lock(m_document);
            if (!layer->locked()) {
                document->lock(Model::NodeList(1, layer));
            } else {
                document->resetLock(Model::NodeList(1, layer));
            }
        }

        class LayerEditor::CollectMoveableNodes : public Model::NodeVisitor {
        private:
            Model::World* m_world;
            Model::NodeSet m_selectNodes;
            Model::NodeSet m_moveNodes;
        public:
            CollectMoveableNodes(Model::World* world) : m_world(world) {}

            const Model::NodeList selectNodes() const {
                return Model::NodeList(std::begin(m_selectNodes), std::end(m_selectNodes));
            }

            const Model::NodeList moveNodes() const {
                return Model::NodeList(std::begin(m_moveNodes), std::end(m_moveNodes));
            }
        private:
            void doVisit(Model::World* world) override   {}
            void doVisit(Model::Layer* layer) override   {}

            void doVisit(Model::Group* group) override   {
                assert(group->selected());

                if (!group->grouped()) {
                    m_moveNodes.insert(group);
                    m_selectNodes.insert(group);
                }
            }

            void doVisit(Model::Entity* entity) override {
                assert(entity->selected());

                if (!entity->grouped()) {
                    m_moveNodes.insert(entity);
                    m_selectNodes.insert(entity);
                }
            }

            void doVisit(Model::Brush* brush) override   {
                assert(brush->selected());
                if (!brush->grouped()) {
                    auto* entity = brush->entity();
                    if (entity == m_world) {
                        m_moveNodes.insert(brush);
                        m_selectNodes.insert(brush);
                    } else {
                        if (m_moveNodes.insert(entity).second) {
                            const Model::NodeList& siblings = entity->children();
                            m_selectNodes.insert(std::begin(siblings), std::end(siblings));
                        }
                    }
                }
            }
        };

        void LayerEditor::OnMoveSelectionToLayer(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto* layer = m_layerList->selectedLayer();
            ensure(layer != nullptr, "layer is null");

            auto document = lock(m_document);
            Transaction transaction(document, "Move Nodes to " + layer->name());
            moveSelectedNodesToLayer(document, layer);
        }

        void LayerEditor::OnUpdateMoveSelectionToLayerUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            const auto* layer = m_layerList->selectedLayer();
            if (layer == nullptr) {
                event.Enable(false);
                return;
            }

            auto document = lock(m_document);
            const auto& nodes = document->selectedNodes().nodes();
            if (nodes.empty()) {
                event.Enable(false);
                return;
            }

            for (auto* node : nodes) {
                auto* nodeGroup = Model::findGroup(node);
                if (nodeGroup != nullptr) {
                    event.Enable(false);
                    return;
                }
            }

            for (auto* node : nodes) {
                auto* nodeLayer = Model::findLayer(node);
                if (nodeLayer != layer) {
                    event.Enable(true);
                    return;
                }
            }

            event.Enable(false);
        }

        void LayerEditor::OnSelectAllInLayer(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto* layer = m_layerList->selectedLayer();
            ensure(layer != nullptr, "layer is null");

            auto document = lock(m_document);

            Model::CollectSelectableNodesVisitor visitor(document->editorContext());
            layer->recurse(visitor);

            const auto& nodes = visitor.nodes();
            document->deselectAll();
            document->select(nodes);
        }

        void LayerEditor::OnAddLayer(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const String name = queryLayerName();
            if (!name.empty()) {
                auto document = lock(m_document);
                auto* world = document->world();
                auto* layer = world->createLayer(name, document->worldBounds());

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
                if (dialog.ShowModal() != wxID_OK) {
                    return "";
                }

                const String name = dialog.GetValue().ToStdString();
                if (StringUtils::isBlank(name)) {
                    if (wxMessageBox("Layer names cannot be blank.", "Error", wxOK | wxCANCEL | wxCENTRE, this) != wxOK) {
                        return "";
                    }
                } else if (StringUtils::containsCaseInsensitive(name, "\"")) {
                    if (wxMessageBox("Layer names cannot contain double quotes.", "Error", wxOK | wxCANCEL | wxCENTRE, this) != wxOK) {
                        return "";
                    }
                } else {
                    return name;
                }
            }
        }

        void LayerEditor::OnRemoveLayer(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto* layer = m_layerList->selectedLayer();
            ensure(layer != nullptr, "layer is null");

            auto document = lock(m_document);
            auto* defaultLayer = document->world()->defaultLayer();

            Transaction transaction(document, "Remove Layer " + layer->name());
            document->deselectAll();
            if (layer->hasChildren()) {
                document->reparentNodes(defaultLayer, layer->children());
            }
            if (document->currentLayer() == layer) {
                document->setCurrentLayer(defaultLayer);
            }
            document->removeNode(layer);
        }

        void LayerEditor::OnUpdateRemoveLayerUI(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            const auto* layer = m_layerList->selectedLayer();
            if (layer == nullptr) {
                event.Enable(false);
                return;
            }

            if (findVisibleAndUnlockedLayer(layer) == nullptr) {
                event.Enable(false);
                return;
            }

            auto document = lock(m_document);
            event.Enable(layer != document->world()->defaultLayer());
        }

        void LayerEditor::OnShowAllLayers(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto document = lock(m_document);
            const auto& layers = document->world()->allLayers();
            document->resetVisibility(Model::NodeList(std::begin(layers), std::end(layers)));
        }

        Model::Layer* LayerEditor::findVisibleAndUnlockedLayer(const Model::Layer* except) const {
            auto document = lock(m_document);
            if (!document->world()->defaultLayer()->locked() && !document->world()->defaultLayer()->hidden()) {
                return document->world()->defaultLayer();
            }

            const auto& layers = document->world()->customLayers();
            for (auto* layer : layers) {
                if (layer != except && !layer->locked() && !layer->hidden()) {
                    return layer;
                }
            }

            return nullptr;
        }

        void LayerEditor::moveSelectedNodesToLayer(MapDocumentSPtr document, Model::Layer* layer) {
            const auto& selectedNodes = document->selectedNodes().nodes();

            CollectMoveableNodes visitor(document->world());
            Model::Node::accept(std::begin(selectedNodes), std::end(selectedNodes), visitor);

            const auto moveNodes = visitor.moveNodes();
            if (!moveNodes.empty()) {
                document->deselectAll();
                document->reparentNodes(layer, visitor.moveNodes());
                if (!layer->hidden() && !layer->locked()) {
                    document->select(visitor.selectNodes());
                }
            }
        }

        void LayerEditor::createGui() {
            m_layerList = new LayerListBox(this, m_document);
            m_layerList->Bind(LAYER_SET_CURRENT_EVENT, &LayerEditor::OnSetCurrentLayer, this);
            m_layerList->Bind(LAYER_RIGHT_CLICK_EVENT, &LayerEditor::OnLayerRightClick, this);
            m_layerList->Bind(LAYER_TOGGLE_VISIBLE_EVENT, &LayerEditor::OnToggleLayerVisibleFromList, this);
            m_layerList->Bind(LAYER_TOGGLE_LOCKED_EVENT, &LayerEditor::OnToggleLayerLockedFromList, this);

            auto* addLayerButton = createBitmapButton(this, "Add.png", "Add a new layer from the current selection");
            auto* removeLayerButton = createBitmapButton(this, "Remove.png", "Remove the selected layer and move its objects to the default layer");
            auto* showAllLayersButton = createBitmapButton(this, "Visible.png", "Show all layers");

            addLayerButton->Bind(wxEVT_BUTTON, &LayerEditor::OnAddLayer, this);
            removeLayerButton->Bind(wxEVT_BUTTON, &LayerEditor::OnRemoveLayer, this);
            removeLayerButton->Bind(wxEVT_UPDATE_UI, &LayerEditor::OnUpdateRemoveLayerUI, this);
            showAllLayersButton->Bind(wxEVT_BUTTON, &LayerEditor::OnShowAllLayers, this);

            auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addLayerButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(removeLayerButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(showAllLayersButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();

            auto* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_layerList, 1, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Horizontal), 0, wxEXPAND);
            sizer->Add(buttonSizer, 0, wxEXPAND);
            SetSizer(sizer);
        }
    }
}
