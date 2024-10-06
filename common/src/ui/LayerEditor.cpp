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
#include "mdl/LayerNode.h"
#include "mdl/ModelUtils.h"
#include "mdl/WorldNode.h"
#include "ui/BorderLine.h"
#include "ui/LayerListBox.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"
#include "ui/Transaction.h"

#include "kdl/memory_utils.h"
#include "kdl/vector_utils.h"

#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace tb::ui
{

LayerEditor::LayerEditor(std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  createGui();

  updateButtons();
}

void LayerEditor::onSetCurrentLayer(mdl::LayerNode* layer)
{
  auto document = kdl::mem_lock(m_document);
  document->setCurrentLayer(layer);

  updateButtons();
}

bool LayerEditor::canSetCurrentLayer(mdl::LayerNode* layer) const
{
  auto document = kdl::mem_lock(m_document);
  return document->currentLayer() != layer;
}

void LayerEditor::onLayerRightClick(mdl::LayerNode* layerNode)
{
  auto document = kdl::mem_lock(m_document);

  auto popupMenu = QMenu{};
  auto* makeActiveAction = popupMenu.addAction(
    tr("Make active layer"), this, [this, layerNode]() { onSetCurrentLayer(layerNode); });
  auto* moveSelectionToLayerAction = popupMenu.addAction(
    tr("Move selection to layer"), this, &LayerEditor::onMoveSelectionToLayer);
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
  moveSelectionToLayerAction->setEnabled(canMoveSelectionToLayer());
  selectAllInLayerAction->setEnabled(canSelectAllInLayer());
  toggleLayerVisibleAction->setEnabled(canToggleLayerVisible());
  isolateLayerAction->setEnabled(document->canIsolateLayers({layerNode}));
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

void LayerEditor::toggleLayerVisible(mdl::LayerNode* layer)
{
  ensure(layer != nullptr, "layer is null");
  auto document = kdl::mem_lock(m_document);
  if (!layer->hidden())
  {
    document->hide(std::vector<mdl::Node*>{layer});
  }
  else
  {
    document->resetVisibility(std::vector<mdl::Node*>{layer});
  }
}

bool LayerEditor::canToggleLayerLocked() const
{
  return m_layerList->selectedLayer() != nullptr;
}

void LayerEditor::toggleLayerLocked(mdl::LayerNode* layer)
{
  ensure(layer != nullptr, "layer is null");
  auto document = kdl::mem_lock(m_document);
  if (!layer->locked())
  {
    document->lock(std::vector<mdl::Node*>{layer});
  }
  else
  {
    document->resetLock(std::vector<mdl::Node*>{layer});
  }
}

void LayerEditor::toggleOmitLayerFromExport(mdl::LayerNode* layerNode)
{
  ensure(layerNode != nullptr, "layer is null");
  kdl::mem_lock(m_document)
    ->setOmitLayerFromExport(layerNode, !layerNode->layer().omitFromExport());
}

void LayerEditor::isolateLayer(mdl::LayerNode* layer)
{
  auto document = kdl::mem_lock(m_document);
  document->isolateLayers(std::vector<mdl::LayerNode*>{layer});
}

void LayerEditor::onMoveSelectionToLayer()
{
  auto* layer = m_layerList->selectedLayer();
  ensure(layer != nullptr, "layer is null");

  auto document = kdl::mem_lock(m_document);
  document->moveSelectionToLayer(layer);
}

bool LayerEditor::canMoveSelectionToLayer() const
{
  if (auto* layer = m_layerList->selectedLayer())
  {
    auto document = kdl::mem_lock(m_document);
    return document->canMoveSelectionToLayer(layer);
  }
  return false;
}

void LayerEditor::onSelectAllInLayer()
{
  auto* layer = m_layerList->selectedLayer();
  ensure(layer != nullptr, "layer is null");

  kdl::mem_lock(m_document)->selectAllInLayers({layer});
}

bool LayerEditor::canSelectAllInLayer() const
{
  if (auto* layer = m_layerList->selectedLayer())
  {
    return kdl::mem_lock(m_document)->canSelectAllInLayers({layer});
  }
  return false;
}

void LayerEditor::onAddLayer()
{
  const auto name = queryLayerName(this, "Unnamed");
  if (!name.empty())
  {
    auto document = kdl::mem_lock(m_document);
    auto* world = document->world();

    auto layer = mdl::Layer{name};

    // Sort it at the bottom of the list
    const auto customLayers = world->customLayersUserSorted();
    layer.setSortIndex(
      !customLayers.empty() ? customLayers.back()->layer().sortIndex() + 1 : 0);

    auto* layerNode = new mdl::LayerNode{std::move(layer)};

    auto transaction = Transaction{document, "Create Layer " + layerNode->name()};
    if (document->addNodes({{world, {layerNode}}}).empty())
    {
      transaction.cancel();
      return;
    }

    document->setCurrentLayer(layerNode);
    transaction.commit();

    m_layerList->setSelectedLayer(layerNode);
  }
}

void LayerEditor::onRemoveLayer()
{
  auto* layer = m_layerList->selectedLayer();
  ensure(layer != nullptr, "layer is null");

  auto document = kdl::mem_lock(m_document);
  auto* defaultLayer = document->world()->defaultLayer();

  auto transaction = Transaction{document, "Remove Layer " + layer->name()};
  document->deselectAll();
  if (layer->hasChildren())
  {
    if (!document->reparentNodes({{defaultLayer, layer->children()}}))
    {
      transaction.cancel();
      return;
    }
  }

  if (document->currentLayer() == layer)
  {
    document->setCurrentLayer(defaultLayer);
  }
  document->removeNodes({layer});
  transaction.commit();
}

bool LayerEditor::canRemoveLayer() const
{
  if (const auto* layer = m_layerList->selectedLayer();
      layer && findVisibleAndUnlockedLayer(layer))
  {
    auto document = kdl::mem_lock(m_document);
    return (layer != document->world()->defaultLayer());
  }

  return false;
}

void LayerEditor::onRenameLayer()
{
  if (canRenameLayer())
  {
    auto document = kdl::mem_lock(m_document);
    auto* layer = m_layerList->selectedLayer();

    if (const auto name = queryLayerName(this, layer->name()); !name.empty())
    {
      document->renameLayer(layer, name);
    }
  }
}

bool LayerEditor::canRenameLayer() const
{
  if (const auto* layer = m_layerList->selectedLayer())
  {
    auto document = kdl::mem_lock(m_document);
    return (layer != document->world()->defaultLayer());
  }
  return false;
}

bool LayerEditor::canMoveLayer(const int direction) const
{
  if (auto* layer = m_layerList->selectedLayer(); layer && direction != 0)
  {
    auto document = kdl::mem_lock(m_document);
    return document->canMoveLayer(layer, direction);
  }
  return false;
}

void LayerEditor::moveLayer(mdl::LayerNode* layer, int direction)
{
  if (direction != 0)
  {
    ensure(layer != nullptr, "layer is null");
    auto document = kdl::mem_lock(m_document);
    document->moveLayer(layer, direction);
  }
}

void LayerEditor::onShowAllLayers()
{
  auto document = kdl::mem_lock(m_document);
  const auto layers = document->world()->allLayers();
  document->resetVisibility(
    std::vector<mdl::Node*>(std::begin(layers), std::end(layers)));
}

bool LayerEditor::canShowAllLayers() const
{
  const auto layers = kdl::mem_lock(m_document)->world()->allLayers();
  return std::ranges::any_of(layers, [](const auto* layer) { return !layer->visible(); });
}

void LayerEditor::onHideAllLayers()
{
  auto document = kdl::mem_lock(m_document);
  const auto layers = document->world()->allLayers();
  document->hide(std::vector<mdl::Node*>{std::begin(layers), std::end(layers)});
}

bool LayerEditor::canHideAllLayers() const
{
  const auto layers = kdl::mem_lock(m_document)->world()->allLayers();
  return std::ranges::any_of(layers, [](const auto* layer) { return layer->visible(); });
}

void LayerEditor::onLockAllLayers()
{
  auto document = kdl::mem_lock(m_document);
  const auto nodes = kdl::vec_static_cast<mdl::Node*>(document->world()->allLayers());
  document->lock(nodes);
}

bool LayerEditor::canLockAllLayers() const
{
  const auto layers = kdl::mem_lock(m_document)->world()->allLayers();
  return std::ranges::any_of(layers, [](const auto* layer) { return !layer->locked(); });
}

void LayerEditor::onUnlockAllLayers()
{
  auto document = kdl::mem_lock(m_document);
  const auto nodes = kdl::vec_static_cast<mdl::Node*>(document->world()->allLayers());
  document->resetLock(nodes);
}

bool LayerEditor::canUnlockAllLayers() const
{
  const auto layers = kdl::mem_lock(m_document)->world()->allLayers();
  return std::ranges::any_of(layers, [](const auto* layer) { return layer->locked(); });
}

mdl::LayerNode* LayerEditor::findVisibleAndUnlockedLayer(
  const mdl::LayerNode* except) const
{
  auto document = kdl::mem_lock(m_document);
  if (
    !document->world()->defaultLayer()->locked()
    && !document->world()->defaultLayer()->hidden())
  {
    return document->world()->defaultLayer();
  }

  const auto& layers = document->world()->customLayers();
  for (auto* layer : layers)
  {
    if (layer != except && !layer->locked() && !layer->hidden())
    {
      return layer;
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
