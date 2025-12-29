/*
 Copyright (C) 2010 Kristian Duske

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

#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>

#include "ViewUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Layers.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_NodeVisibility.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "ui/BitmapButton.h"
#include "ui/BorderLine.h"
#include "ui/LayerListBox.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"
#include "kd/vector_utils.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace tb::ui
{

LayerEditor::LayerEditor(MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
{
  createGui();

  updateButtons();
}

void LayerEditor::onSetCurrentLayer(mdl::LayerNode* layerNode)
{
  setCurrentLayer(m_document.map(), layerNode);

  updateButtons();
}

bool LayerEditor::canSetCurrentLayer(mdl::LayerNode* layerNode) const
{
  return mdl::canSetCurrentLayer(m_document.map(), layerNode);
}

void LayerEditor::onLayerRightClick(mdl::LayerNode* layerNode)
{
  auto popupMenu = QMenu{};
  auto* makeActiveAction = popupMenu.addAction(
    tr("Make active layer"), this, [this, layerNode]() { onSetCurrentLayer(layerNode); });
  auto* moveSelectionToLayerAction = popupMenu.addAction(
    tr("Move selection to layer"), this, &LayerEditor::onMoveSelectedNodesToLayer);
  auto* selectAllInLayerAction = popupMenu.addAction(
    tr("Select all in layer"), this, &LayerEditor::onSelectAllInLayer);
  popupMenu.addSeparator();
  auto* toggleLayerVisibleAction = popupMenu.addAction(
    layerNode->hidden() ? tr("Show layer") : tr("Hide layer"), this, [this, layerNode]() {
      toggleLayerVisible(layerNode);
    });
  auto* isolateLayerAction = popupMenu.addAction(
    tr("Isolate layer"), this, [this, layerNode]() { isolateLayer(layerNode); });
  auto* toggleLayerLockedAction = popupMenu.addAction(
    layerNode->locked() ? tr("Unlock layer") : tr("Lock layer"),
    this,
    [this, layerNode]() { toggleLayerLocked(layerNode); });
  auto* toggleLayerOmitFromExportAction =
    popupMenu.addAction(tr("Omit From Export"), this, [this, layerNode]() {
      toggleOmitLayerFromExport(layerNode);
    });
  popupMenu.addSeparator();
  auto* showAllLayersAction =
    popupMenu.addAction(tr("Show All Layers"), this, &LayerEditor::onShowAllLayers);
  auto* hideAllLayersAction =
    popupMenu.addAction(tr("Hide All Layers"), this, &LayerEditor::onHideAllLayers);
  popupMenu.addSeparator();
  auto* unlockAllLayersAction =
    popupMenu.addAction(tr("Unlock All Layers"), this, &LayerEditor::onUnlockAllLayers);
  auto* lockAllLayersAction =
    popupMenu.addAction(tr("Lock All Layers"), this, &LayerEditor::onLockAllLayers);
  popupMenu.addSeparator();
  auto* renameLayerAction =
    popupMenu.addAction(tr("Rename Layer"), this, &LayerEditor::onRenameLayer);
  auto* removeLayerAction =
    popupMenu.addAction(tr("Remove Layer"), this, &LayerEditor::onRemoveLayer);

  makeActiveAction->setEnabled(canSetCurrentLayer(layerNode));
  moveSelectionToLayerAction->setEnabled(canMoveSelectedNodesToLayer());
  selectAllInLayerAction->setEnabled(canSelectAllInLayer());
  toggleLayerVisibleAction->setEnabled(canToggleLayerVisible());
  isolateLayerAction->setEnabled(canIsolateLayers(m_document.map(), {layerNode}));
  toggleLayerOmitFromExportAction->setCheckable(true);
  toggleLayerOmitFromExportAction->setChecked(layerNode->layer().omitFromExport());

  toggleLayerLockedAction->setEnabled(canToggleLayerLocked());
  showAllLayersAction->setEnabled(canShowAllLayers());
  hideAllLayersAction->setEnabled(canHideAllLayers());
  unlockAllLayersAction->setEnabled(canUnlockAllLayers());
  lockAllLayersAction->setEnabled(canLockAllLayers());
  renameLayerAction->setEnabled(canRenameLayer());
  removeLayerAction->setEnabled(canRemoveLayer());

  popupMenu.exec(QCursor::pos());
}

bool LayerEditor::canToggleLayerVisible() const
{
  return m_layerList->selectedLayer() != nullptr;
}

void LayerEditor::toggleLayerVisible(mdl::LayerNode* layerNode)
{
  contract_pre(layerNode != nullptr);

  auto& map = m_document.map();
  if (!layerNode->hidden())
  {
    hideNodes(map, std::vector<mdl::Node*>{layerNode});
  }
  else
  {
    resetNodeVisibility(map, std::vector<mdl::Node*>{layerNode});
  }
}

bool LayerEditor::canToggleLayerLocked() const
{
  return m_layerList->selectedLayer() != nullptr;
}

void LayerEditor::toggleLayerLocked(mdl::LayerNode* layerNode)
{
  contract_pre(layerNode != nullptr);

  auto& map = m_document.map();
  if (!layerNode->locked())
  {
    lockNodes(map, std::vector<mdl::Node*>{layerNode});
  }
  else
  {
    resetNodeLockingState(map, std::vector<mdl::Node*>{layerNode});
  }
}

void LayerEditor::toggleOmitLayerFromExport(mdl::LayerNode* layerNode)
{
  contract_pre(layerNode != nullptr);

  setOmitLayerFromExport(
    m_document.map(), layerNode, !layerNode->layer().omitFromExport());
}

void LayerEditor::isolateLayer(mdl::LayerNode* layer)
{
  isolateLayers(m_document.map(), std::vector<mdl::LayerNode*>{layer});
}

void LayerEditor::onMoveSelectedNodesToLayer()
{
  auto* layerNode = m_layerList->selectedLayer();
  contract_assert(layerNode != nullptr);

  moveSelectedNodesToLayer(m_document.map(), layerNode);
}

bool LayerEditor::canMoveSelectedNodesToLayer() const
{
  if (auto* layerNode = m_layerList->selectedLayer())
  {
    return mdl::canMoveSelectedNodesToLayer(m_document.map(), layerNode);
  }
  return false;
}

void LayerEditor::onSelectAllInLayer()
{
  auto* layerNode = m_layerList->selectedLayer();
  contract_assert(layerNode != nullptr);

  selectAllInLayers(m_document.map(), {layerNode});
}

bool LayerEditor::canSelectAllInLayer() const
{
  if (auto* layerNode = m_layerList->selectedLayer())
  {
    return canSelectAllInLayers(m_document.map(), {layerNode});
  }
  return false;
}

void LayerEditor::onAddLayer()
{
  auto& map = m_document.map();

  const auto name = queryLayerName(this, "Unnamed");
  if (!name.empty())
  {
    auto layer = mdl::Layer{name};

    // Sort it at the bottom of the list
    const auto customLayers = map.worldNode().customLayersUserSorted();
    layer.setSortIndex(
      !customLayers.empty() ? customLayers.back()->layer().sortIndex() + 1 : 0);

    auto* layerNode = new mdl::LayerNode{std::move(layer)};

    auto transaction = mdl::Transaction{map, "Create Layer " + layerNode->name()};
    if (addNodes(map, {{&map.worldNode(), {layerNode}}}).empty())
    {
      transaction.cancel();
      return;
    }

    setCurrentLayer(map, layerNode);
    transaction.commit();

    m_layerList->setSelectedLayer(layerNode);

    updateButtons();
  }
}

void LayerEditor::onRemoveLayer()
{
  auto* layerNode = m_layerList->selectedLayer();
  contract_assert(layerNode != nullptr);

  auto& map = m_document.map();
  auto* defaultLayerNode = map.worldNode().defaultLayer();

  auto transaction = mdl::Transaction{map, "Remove Layer " + layerNode->name()};
  deselectAll(map);
  if (layerNode->hasChildren())
  {
    if (!reparentNodes(map, {{defaultLayerNode, layerNode->children()}}))
    {
      transaction.cancel();
      return;
    }
  }

  if (map.editorContext().currentLayer() == layerNode)
  {
    setCurrentLayer(map, defaultLayerNode);
  }

  m_layerList->updateSelectionForRemoval();
  removeNodes(map, {layerNode});
  transaction.commit();

  updateButtons();
}

bool LayerEditor::canRemoveLayer() const
{
  if (const auto* layerNode = m_layerList->selectedLayer();
      layerNode && findVisibleAndUnlockedLayer(layerNode))
  {
    return (layerNode != m_document.map().worldNode().defaultLayer());
  }

  return false;
}

void LayerEditor::onRenameLayer()
{
  if (canRenameLayer())
  {
    auto* layerNode = m_layerList->selectedLayer();

    if (const auto name = queryLayerName(this, layerNode->name()); !name.empty())
    {
      renameLayer(m_document.map(), layerNode, name);
    }
  }
}

bool LayerEditor::canRenameLayer() const
{
  if (const auto* layerNode = m_layerList->selectedLayer())
  {
    return (layerNode != m_document.map().worldNode().defaultLayer());
  }
  return false;
}

bool LayerEditor::canMoveLayer(const int direction) const
{
  if (auto* layerNode = m_layerList->selectedLayer(); layerNode && direction != 0)
  {
    return mdl::canMoveLayer(m_document.map(), layerNode, direction);
  }
  return false;
}

void LayerEditor::moveLayer(mdl::LayerNode* layerNode, const int direction)
{
  contract_pre(direction == 0 || layerNode != nullptr);

  if (direction != 0)
  {
    mdl::moveLayer(m_document.map(), layerNode, direction);
  }
}

void LayerEditor::onShowAllLayers()
{
  auto& map = m_document.map();

  const auto layers = map.worldNode().allLayers();
  resetNodeVisibility(map, kdl::vec_static_cast<mdl::Node*>(layers));
}

bool LayerEditor::canShowAllLayers() const
{
  const auto layers = m_document.map().worldNode().allLayers();
  return std::ranges::any_of(
    layers, [](const auto* layerNode) { return !layerNode->visible(); });
}

void LayerEditor::onHideAllLayers()
{
  auto& map = m_document.map();

  const auto layers = map.worldNode().allLayers();
  hideNodes(map, kdl::vec_static_cast<mdl::Node*>(layers));
}

bool LayerEditor::canHideAllLayers() const
{
  const auto layers = m_document.map().worldNode().allLayers();
  return std::ranges::any_of(layers, [](const auto* layer) { return layer->visible(); });
}

void LayerEditor::onLockAllLayers()
{
  auto& map = m_document.map();

  const auto layers = map.worldNode().allLayers();
  lockNodes(map, kdl::vec_static_cast<mdl::Node*>(layers));
}

bool LayerEditor::canLockAllLayers() const
{
  const auto layers = m_document.map().worldNode().allLayers();
  return std::ranges::any_of(layers, [](const auto* layer) { return !layer->locked(); });
}

void LayerEditor::onUnlockAllLayers()
{
  auto& map = m_document.map();

  const auto layers = map.worldNode().allLayers();
  resetNodeLockingState(map, kdl::vec_static_cast<mdl::Node*>(layers));
}

bool LayerEditor::canUnlockAllLayers() const
{
  const auto layers = m_document.map().worldNode().allLayers();
  return std::ranges::any_of(layers, [](const auto* layer) { return layer->locked(); });
}

mdl::LayerNode* LayerEditor::findVisibleAndUnlockedLayer(
  const mdl::LayerNode* except) const
{
  auto& map = m_document.map();

  if (
    !map.worldNode().defaultLayer()->locked()
    && !map.worldNode().defaultLayer()->hidden())
  {
    return map.worldNode().defaultLayer();
  }

  const auto& layers = map.worldNode().customLayers();
  for (auto* layerNode : layers)
  {
    if (layerNode != except && !layerNode->locked() && !layerNode->hidden())
    {
      return layerNode;
    }
  }

  return nullptr;
}

void LayerEditor::createGui()
{
  m_layerList = new LayerListBox{m_document, this};
  connect(
    m_layerList, &LayerListBox::layerSetCurrent, this, &LayerEditor::onSetCurrentLayer);
  connect(
    m_layerList, &LayerListBox::layerRightClicked, this, &LayerEditor::onLayerRightClick);
  connect(
    m_layerList,
    &LayerListBox::layerOmitFromExportToggled,
    this,
    &LayerEditor::toggleOmitLayerFromExport);
  connect(m_layerList, &LayerListBox::layerVisibilityToggled, this, [&](auto* layer) {
    toggleLayerVisible(layer);
  });
  connect(m_layerList, &LayerListBox::layerLockToggled, this, [&](auto* layer) {
    toggleLayerLocked(layer);
  });
  connect(
    m_layerList, &LayerListBox::itemSelectionChanged, this, &LayerEditor::updateButtons);

  m_addLayerButton =
    createBitmapButton("Add.svg", tr("Add a new layer from the current selection"));
  m_removeLayerButton = createBitmapButton(
    "Remove.svg",
    tr("Remove the selected layer and move its objects to the default layer"));
  m_moveLayerUpButton = createBitmapButton("Up.svg", "Move the selected layer up");
  m_moveLayerDownButton = createBitmapButton("Down.svg", "Move the selected layer down");

  connect(m_addLayerButton, &QAbstractButton::pressed, this, &LayerEditor::onAddLayer);
  connect(
    m_removeLayerButton, &QAbstractButton::pressed, this, &LayerEditor::onRemoveLayer);
  connect(m_moveLayerUpButton, &QAbstractButton::pressed, this, [&]() {
    moveLayer(m_layerList->selectedLayer(), -1);
  });
  connect(m_moveLayerDownButton, &QAbstractButton::pressed, this, [&]() {
    moveLayer(m_layerList->selectedLayer(), 1);
  });

  auto* buttonSizer = new QHBoxLayout{};
  buttonSizer->addWidget(m_addLayerButton);
  buttonSizer->addWidget(m_removeLayerButton);
  buttonSizer->addWidget(m_moveLayerUpButton);
  buttonSizer->addWidget(m_moveLayerDownButton);
  buttonSizer->addStretch(1);

  auto* sizer = new QVBoxLayout{};
  sizer->setContentsMargins(0, 0, 0, 0);
  sizer->setSpacing(0);
  sizer->addWidget(m_layerList, 1);
  sizer->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
  sizer->addLayout(buttonSizer, 0);
  setLayout(sizer);
}

void LayerEditor::updateButtons()
{
  m_removeLayerButton->setEnabled(canRemoveLayer());
  m_moveLayerUpButton->setEnabled(canMoveLayer(-1));
  m_moveLayerDownButton->setEnabled(canMoveLayer(1));
}

} // namespace tb::ui
