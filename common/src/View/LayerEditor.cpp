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
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>
#include <kdl/string_compare.h>
#include <kdl/string_format.h>

#include <set>
#include <string>
#include <vector>

#include <QAbstractButton>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QVBoxLayout>

#include "ViewUtils.h"

namespace TrenchBroom {
    namespace View {
        LayerEditor::LayerEditor(std::weak_ptr<MapDocument> document, QWidget *parent) :
        QWidget(parent),
        m_document(document),
        m_layerList(nullptr),
        m_addLayerButton(nullptr),
        m_removeLayerButton(nullptr),
        m_showAllLayersButton(nullptr),
        m_hideAllLayersButton(nullptr),
        m_moveLayerUpButton(nullptr),
        m_moveLayerDownButton(nullptr) {
            createGui();

            updateButtons();
        }

        void LayerEditor::onSetCurrentLayer(Model::Layer* layer) {
            auto document = kdl::mem_lock(m_document);
            if (layer->locked()) {
                document->resetLock(std::vector<Model::Node*>(1, layer));
            }
            if (layer->hidden()) {
                document->resetVisibility(std::vector<Model::Node*>(1, layer));
            }
            document->setCurrentLayer(layer);

            updateButtons();
        }

        void LayerEditor::onLayerRightClick(Model::Layer* layer) {
            QMenu popupMenu;
            QAction* moveSelectionToLayerAction = popupMenu.addAction(tr("Move selection to layer"), this, &LayerEditor::onMoveSelectionToLayer);
            popupMenu.addAction(tr("Select all in layer"), this, &LayerEditor::onSelectAllInLayer);
            popupMenu.addSeparator();
            QAction* toggleLayerVisibleAction = popupMenu.addAction(layer->hidden() ? tr("Show layer") : tr("Hide layer"), this, [this, layer](){
                toggleLayerVisible(layer);
            });
            QAction* isolateLayerAction       = popupMenu.addAction(tr("Isolate layer"), this, [this, layer](){
                isolateLayer(layer);
            });
            QAction* toggleLayerLockedAction = popupMenu.addAction(layer->locked() ? tr("Unlock layer") : tr("Lock layer"), this, [this, layer](){
                toggleLayerLocked(layer);
            });
            popupMenu.addSeparator();
            QAction* moveLayerUpAction = popupMenu.addAction(tr("Move layer up"), this, [this, layer](){
                moveLayer(layer, -1);
            });
            QAction* moveLayerDownAction = popupMenu.addAction(tr("Move layer down"), this, [this, layer](){
                moveLayer(layer, 1);
            });
            popupMenu.addSeparator();
            QAction* renameLayerAction = popupMenu.addAction(tr("Rename layer"), this, &LayerEditor::onRenameLayer);
            QAction* removeLayerAction = popupMenu.addAction(tr("Remove layer"), this, &LayerEditor::onRemoveLayer);

            moveSelectionToLayerAction->setEnabled(canMoveSelectionToLayer());
            toggleLayerVisibleAction->setEnabled(canToggleLayerVisible());
            toggleLayerLockedAction->setEnabled(canToggleLayerLocked());
            moveLayerUpAction->setEnabled(canMoveLayer(-1));
            moveLayerDownAction->setEnabled(canMoveLayer(1));
            renameLayerAction->setEnabled(canRenameLayer());
            removeLayerAction->setEnabled(canRemoveLayer());

            popupMenu.exec(QCursor::pos());
        }

        bool LayerEditor::canToggleLayerVisible() const {
            auto* layer = m_layerList->selectedLayer();
            return layer != nullptr;
        }

        void LayerEditor::toggleLayerVisible(Model::Layer* layer) {
            ensure(layer != nullptr, "layer is null");
            auto document = kdl::mem_lock(m_document);
            if (!layer->hidden()) {
                document->hide(std::vector<Model::Node*>(1, layer));
            } else {
                document->resetVisibility(std::vector<Model::Node*>(1, layer));
            }
        }

        bool LayerEditor::canToggleLayerLocked() const {
            auto* layer = m_layerList->selectedLayer();
            if (layer == nullptr) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            if (!layer->locked() && layer == document->currentLayer()) {
                return false;
            }

            return true;
        }

        void LayerEditor::toggleLayerLocked(Model::Layer* layer) {
            ensure(layer != nullptr, "layer is null");
            auto document = kdl::mem_lock(m_document);
            if (!layer->locked()) {
                document->lock(std::vector<Model::Node*>(1, layer));
            } else {
                document->resetLock(std::vector<Model::Node*>(1, layer));
            }
        }

        void LayerEditor::isolateLayer(Model::Layer* layer) {
            auto document = kdl::mem_lock(m_document);
            const auto layers = document->world()->allLayers();

            // FIXME: Move to a MapDocument method
            Transaction transaction(document, "Isolate " + layer->name());
            document->hide(std::vector<Model::Node*>(std::begin(layers), std::end(layers)));
            document->show(std::vector<Model::Node*>{layer});
        }

        void LayerEditor::onMoveSelectionToLayer() {
            auto* layer = m_layerList->selectedLayer();
            ensure(layer != nullptr, "layer is null");

            auto document = kdl::mem_lock(m_document);
            document->moveSelectionToLayer(layer);
        }

        bool LayerEditor::canMoveSelectionToLayer() const {
            const auto* layer = m_layerList->selectedLayer();
            if (layer == nullptr) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            const auto& nodes = document->selectedNodes().nodes();
            if (nodes.empty()) {
                return false;
            }

            for (auto* node : nodes) {
                auto* nodeGroup = Model::findGroup(node);
                if (nodeGroup != nullptr) {
                    return false;
                }
            }

            for (auto* node : nodes) {
                auto* nodeLayer = Model::findLayer(node);
                if (nodeLayer != layer) {
                    return true;
                }
            }

            return true;
        }

        void LayerEditor::onSelectAllInLayer() {
            auto* layer = m_layerList->selectedLayer();
            ensure(layer != nullptr, "layer is null");

            auto document = kdl::mem_lock(m_document);

            Model::CollectSelectableNodesVisitor visitor(document->editorContext());
            layer->recurse(visitor);

            const auto& nodes = visitor.nodes();
            document->deselectAll();
            document->select(nodes);
        }

        void LayerEditor::onAddLayer() {
            const std::string name = queryLayerName("Unnamed");
            if (!name.empty()) {
                auto document = kdl::mem_lock(m_document);
                auto* world = document->world();
                auto* layer = world->createLayer(name);

                // Sort it at the bottom of the list
                const std::vector<Model::Layer*> customLayers = world->customLayersUserSorted();
                if (customLayers.empty()) {
                    layer->setSortIndex(0);
                } else {
                    layer->setSortIndex(customLayers.back()->sortIndex() + 1);
                }

                Transaction transaction(document, "Create Layer " + layer->name());
                document->addNode(layer, world);
                document->setCurrentLayer(layer);
                m_layerList->setSelectedLayer(layer);
            }
        }

        std::string LayerEditor::queryLayerName(const std::string& suggestion) {
            while (true) {
                bool ok = false;
                const std::string name = QInputDialog::getText(this, "Enter a name", "Layer Name", QLineEdit::Normal, QString::fromStdString(suggestion), &ok).toStdString();

                if (!ok) {
                    return "";
                }

                if (kdl::str_is_blank(name)) {
                    if (QMessageBox::warning(this, "Error", "Layer names cannot be blank.", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) != QMessageBox::Ok) {
                        return "";
                    }
                } else if (kdl::ci::str_contains(name, "\"")) {
                    if (QMessageBox::warning(this, "Error", "Layer names cannot contain double quotes.", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) != QMessageBox::Ok) {
                        return "";
                    }
                } else {
                    return name;
                }
            }
        }

        void LayerEditor::onRemoveLayer() {
            auto* layer = m_layerList->selectedLayer();
            ensure(layer != nullptr, "layer is null");

            auto document = kdl::mem_lock(m_document);
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

        bool LayerEditor::canRemoveLayer() const {
            const auto* layer = m_layerList->selectedLayer();
            if (layer == nullptr) {
                return false;
            }

            if (findVisibleAndUnlockedLayer(layer) == nullptr) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            return (layer != document->world()->defaultLayer());
        }

        void LayerEditor::onRenameLayer() {
            if (canRenameLayer()) {
                auto document = kdl::mem_lock(m_document);
                Model::Layer* layer = m_layerList->selectedLayer();

                const std::string name = queryLayerName(layer->name());
                if (!name.empty()) {                    
                    document->renameLayer(layer, name);
                }
            }
        }

        bool LayerEditor::canRenameLayer() const {
            const auto* layer = m_layerList->selectedLayer();
            if (layer == nullptr) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            return (layer != document->world()->defaultLayer());
        }

        bool LayerEditor::canMoveLayer(int direction) const {
            if (direction == 0) {
                return false;
            }

            const auto* layer = m_layerList->selectedLayer();
            if (layer == nullptr) {
                return false;
            }

            auto document = kdl::mem_lock(m_document);
            Model::World* world = document->world();

            if (layer == world->defaultLayer()) {
                return false;
            }

            const std::vector<Model::Layer*> sorted = world->customLayersUserSorted();
            if (sorted.empty()) {
                return false;
            }

            if (direction > 0) {
                return layer != sorted.back();
            }
            else if (direction < 0) {
                return layer != sorted.front();
            }

            return false;
        }

        void LayerEditor::moveLayer(Model::Layer* layer, int direction) {
            if (direction == 0) {
                return;
            }

            ensure(layer != nullptr, "layer is null");
            auto document = kdl::mem_lock(m_document);
            document->moveLayer(layer, direction);
        }

        void LayerEditor::onShowAllLayers() {
            auto document = kdl::mem_lock(m_document);
            const auto layers = document->world()->allLayers();
            document->resetVisibility(std::vector<Model::Node*>(std::begin(layers), std::end(layers)));
        }

        void LayerEditor::onHideAllLayers() {
            auto document = kdl::mem_lock(m_document);
            const auto layers = document->world()->allLayers();
            document->hide(std::vector<Model::Node*>(std::begin(layers), std::end(layers)));
        }

        Model::Layer* LayerEditor::findVisibleAndUnlockedLayer(const Model::Layer* except) const {
            auto document = kdl::mem_lock(m_document);
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

        void LayerEditor::createGui() {
            m_layerList = new LayerListBox(m_document, this);
            connect(m_layerList, &LayerListBox::layerSetCurrent, this, &LayerEditor::onSetCurrentLayer);
            connect(m_layerList, &LayerListBox::layerRightClicked, this, &LayerEditor::onLayerRightClick);
            connect(m_layerList, &LayerListBox::layerVisibilityToggled, this, [this](Model::Layer* layer){
                toggleLayerVisible(layer);
            });
            connect(m_layerList, &LayerListBox::layerLockToggled, this, [this](Model::Layer* layer){
                toggleLayerLocked(layer);
            });
            connect(m_layerList, &LayerListBox::layerMovedUp, this, [this](Model::Layer* layer){
                moveLayer(layer, -1);
            });
            connect(m_layerList, &LayerListBox::layerMovedDown, this, [this](Model::Layer* layer){
                moveLayer(layer, 1);
            });
            connect(m_layerList, &LayerListBox::itemSelectionChanged, this, &LayerEditor::updateButtons);

            m_addLayerButton = createBitmapButton("Add.png", tr("Add a new layer from the current selection"));
            m_removeLayerButton = createBitmapButton("Remove.png", tr("Remove the selected layer and move its objects to the default layer"));
            m_showAllLayersButton = createBitmapButton("Hidden_off.png", tr("Show all layers"));
            m_hideAllLayersButton = createBitmapButton("Hidden_on.png", tr("Hide all layers"));
            m_moveLayerUpButton = createBitmapButton("Up.png", "Move the selected layer up");
            m_moveLayerDownButton = createBitmapButton("Down.png", "Move the selected layer down");

            connect(m_addLayerButton, &QAbstractButton::pressed, this, &LayerEditor::onAddLayer);
            connect(m_removeLayerButton, &QAbstractButton::pressed, this, &LayerEditor::onRemoveLayer);
            connect(m_showAllLayersButton, &QAbstractButton::pressed, this, &LayerEditor::onShowAllLayers);
            connect(m_hideAllLayersButton, &QAbstractButton::pressed, this, &LayerEditor::onHideAllLayers);
            connect(m_moveLayerUpButton, &QAbstractButton::pressed, this, [=](){
                Model::Layer* layer = m_layerList->selectedLayer();
                moveLayer(layer, -1);
            });
            connect(m_moveLayerDownButton, &QAbstractButton::pressed, this, [=](){
                Model::Layer* layer = m_layerList->selectedLayer();
                moveLayer(layer, 1);
            });

            auto* buttonSizer = new QHBoxLayout();
            buttonSizer->addWidget(m_addLayerButton);
            buttonSizer->addWidget(m_removeLayerButton);
            buttonSizer->addWidget(m_moveLayerUpButton);
            buttonSizer->addWidget(m_moveLayerDownButton);
            buttonSizer->addStretch(1);
            buttonSizer->addWidget(m_showAllLayersButton);
            buttonSizer->addWidget(m_hideAllLayersButton);

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->setSpacing(0);
            sizer->addWidget(m_layerList, 1);
            sizer->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
            sizer->addLayout(buttonSizer, 0);
            setLayout(sizer);
        }

        void LayerEditor::updateButtons() {
            m_removeLayerButton->setEnabled(canRemoveLayer());
            m_moveLayerUpButton->setEnabled(canMoveLayer(-1));
            m_moveLayerDownButton->setEnabled(canMoveLayer(1));
        }
    }
}
