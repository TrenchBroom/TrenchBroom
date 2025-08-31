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

#include "LayerListBox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QRadioButton>
#include <QToolButton>
#include <QtGlobal>

#include "mdl/EditorContext.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"
#include "ui/ViewConstants.h"

#include "kdl/range_to_vector.h"

namespace tb::ui
{
// LayerListBoxWidget

LayerListBoxWidget::LayerListBoxWidget(
  MapDocument& document, mdl::LayerNode* layer, QWidget* parent)
  : ControlListBoxItemRenderer(parent)
  , m_document{document}
  , m_layer{layer}
  , m_activeButton{new QRadioButton{}}
  , m_nameText{new QLabel{QString::fromStdString(m_layer->name())}}
  , m_infoText{new QLabel{}}
  , m_omitFromExportButton{createBitmapToggleButton(
      "OmitFromExport.svg", tr("Toggle omit from export"))}
  , m_hiddenButton{createBitmapToggleButton("Hidden.svg", tr("Toggle hidden state"))}
  , m_lockButton{createBitmapToggleButton("Lock.svg", tr("Toggle locked state"))}
{
  // Ignore the label's minimum width, this prevents a horizontal scroll bar from
  // appearing on the list widget, and instead just cuts off the label for long layer
  // names.
  m_nameText->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  makeInfo(m_infoText);

  connect(m_omitFromExportButton, &QAbstractButton::clicked, this, [&]() {
    emit layerOmitFromExportToggled(m_layer);
  });
  connect(m_activeButton, &QAbstractButton::clicked, this, [&]() {
    emit layerActiveClicked(m_layer);
  });
  connect(m_hiddenButton, &QAbstractButton::clicked, this, [&]() {
    emit layerVisibilityToggled(m_layer);
  });
  connect(m_lockButton, &QAbstractButton::clicked, this, [&]() {
    emit layerLockToggled(m_layer);
  });

  installEventFilter(this);
  m_nameText->installEventFilter(this);
  m_infoText->installEventFilter(this);

  auto* textLayout = new QVBoxLayout{};
  textLayout->setContentsMargins(
    0, LayoutConstants::NarrowVMargin, 0, LayoutConstants::NarrowVMargin);
  textLayout->setSpacing(LayoutConstants::NarrowVMargin);
  textLayout->addWidget(m_nameText, 1);
  textLayout->addWidget(m_infoText, 1);

  auto* itemPanelLayout = new QHBoxLayout{};
  itemPanelLayout->setContentsMargins(0, 0, 0, 0);
  itemPanelLayout->setSpacing(LayoutConstants::MediumHMargin);

  itemPanelLayout->addWidget(m_activeButton);
  itemPanelLayout->addSpacing(LayoutConstants::NarrowHMargin);
  itemPanelLayout->addLayout(textLayout, 1);
  itemPanelLayout->addWidget(m_omitFromExportButton);
  itemPanelLayout->addWidget(m_hiddenButton);
  itemPanelLayout->addWidget(m_lockButton);
  setLayout(itemPanelLayout);

  updateLayerItem();
}

void LayerListBoxWidget::updateItem()
{
  updateLayerItem();
}

/**
 * This is factored out from updateItem() so the constructor can call it without doing a
 * virtual function call
 */
void LayerListBoxWidget::updateLayerItem()
{
  auto& map = m_document.map();
  const auto& editorContext = map.editorContext();

  // Update labels
  m_nameText->setText(tr("%1").arg(QString::fromStdString(m_layer->name())));
  if (editorContext.currentLayer() == m_layer)
  {
    makeEmphasized(m_nameText);
  }
  else
  {
    makeUnemphasized(m_nameText);
  }

  const auto info = tr("%1 %2")
                      .arg(m_layer->childCount())
                      .arg(m_layer->childCount() == 1 ? "object" : "objects");
  m_infoText->setText(info);

  // Update buttons
  m_activeButton->setChecked(editorContext.currentLayer() == m_layer);
  m_lockButton->setChecked(m_layer->locked());
  m_hiddenButton->setChecked(m_layer->hidden());
  m_omitFromExportButton->setChecked(m_layer->layer().omitFromExport());
}

mdl::LayerNode* LayerListBoxWidget::layer() const
{
  return m_layer;
}

bool LayerListBoxWidget::eventFilter(QObject* target, QEvent* event)
{
  if (event->type() == QEvent::MouseButtonDblClick)
  {
    auto* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->button() == Qt::LeftButton)
    {
      emit layerDoubleClicked(m_layer);
      return true;
    }
  }
  else if (event->type() == QEvent::MouseButtonRelease)
  {
    auto* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->button() == Qt::RightButton)
    {
      emit layerRightClicked(m_layer);
      return true;
    }
  }
  return QObject::eventFilter(target, event);
}

// LayerListBox

LayerListBox::LayerListBox(MapDocument& document, QWidget* parent)
  : ControlListBox{"", true, parent}
  , m_document{document}
{
  connectObservers();
}

mdl::LayerNode* LayerListBox::selectedLayer() const
{
  return layerForRow(currentRow());
}

void LayerListBox::setSelectedLayer(mdl::LayerNode* layer)
{
  for (int i = 0; i < count(); ++i)
  {
    if (layerForRow(i) == layer)
    {
      setCurrentRow(i);
      return;
    }
  }
  setCurrentRow(-1);
}

void LayerListBox::updateSelectionForRemoval()
{
  const auto currentRow = this->currentRow();
  if (currentRow < count() - 1)
  {
    setCurrentRow(currentRow + 1);
  }
  else if (currentRow > 0)
  {
    setCurrentRow(currentRow - 1);
  }
  else
  {
    setCurrentRow(-1);
  }
}

void LayerListBox::connectObservers()
{
  auto& map = m_document.map();
  m_notifierConnection +=
    map.mapWasCreatedNotifier.connect(this, &LayerListBox::mapDidChange);
  m_notifierConnection +=
    map.mapWasLoadedNotifier.connect(this, &LayerListBox::mapDidChange);
  m_notifierConnection +=
    map.mapWasClearedNotifier.connect(this, &LayerListBox::mapDidChange);
  m_notifierConnection +=
    map.currentLayerDidChangeNotifier.connect(this, &LayerListBox::currentLayerDidChange);
  m_notifierConnection +=
    map.nodesWereAddedNotifier.connect(this, &LayerListBox::nodesDidChange);
  m_notifierConnection +=
    map.nodesWereRemovedNotifier.connect(this, &LayerListBox::nodesDidChange);
  m_notifierConnection +=
    map.nodesDidChangeNotifier.connect(this, &LayerListBox::nodesDidChange);
  m_notifierConnection +=
    map.nodeVisibilityDidChangeNotifier.connect(this, &LayerListBox::nodesDidChange);
  m_notifierConnection +=
    map.nodeLockingDidChangeNotifier.connect(this, &LayerListBox::nodesDidChange);
}

void LayerListBox::mapDidChange(mdl::Map&)
{
  reload();
}

void LayerListBox::nodesDidChange(const std::vector<mdl::Node*>&)
{
  const auto documentLayers = m_document.map().world()->allLayersUserSorted();

  if (layers() != documentLayers)
  {
    // A layer was added or removed or modified, so we need to clear and repopulate the
    // list
    auto* previouslySelectedLayer = selectedLayer();
    reload();
    setSelectedLayer(previouslySelectedLayer);
    return;
  }
  updateItems();
}

void LayerListBox::currentLayerDidChange(const mdl::LayerNode*)
{
  updateItems();
}

size_t LayerListBox::itemCount() const
{
  auto& map = m_document.map();
  if (const auto* worldNode = map.world())
  {
    return worldNode->allLayers().size();
  }
  return 0;
}

ControlListBoxItemRenderer* LayerListBox::createItemRenderer(
  QWidget* parent, const size_t index)
{
  auto& map = m_document.map();
  auto* worldNode = map.world();

  auto* layerNode = index > 0 ? worldNode->customLayersUserSorted().at(index - 1)
                              : worldNode->defaultLayer();

  auto* renderer = new LayerListBoxWidget{m_document, layerNode, parent};
  connect(
    renderer,
    &LayerListBoxWidget::layerActiveClicked,
    this,
    &LayerListBox::layerSetCurrent);
  connect(
    renderer,
    &LayerListBoxWidget::layerDoubleClicked,
    this,
    &LayerListBox::layerSetCurrent);
  connect(
    renderer,
    &LayerListBoxWidget::layerRightClicked,
    this,
    &LayerListBox::layerRightClicked);
  connect(
    renderer,
    &LayerListBoxWidget::layerOmitFromExportToggled,
    this,
    &LayerListBox::layerOmitFromExportToggled);
  connect(
    renderer,
    &LayerListBoxWidget::layerVisibilityToggled,
    this,
    &LayerListBox::layerVisibilityToggled);
  connect(
    renderer,
    &LayerListBoxWidget::layerLockToggled,
    this,
    &LayerListBox::layerLockToggled);
  return renderer;
}

void LayerListBox::selectedRowChanged(const int index)
{
  emit layerSelected(layerForRow(index));
}

const LayerListBoxWidget* LayerListBox::widgetAtRow(const int row) const
{
  return static_cast<const LayerListBoxWidget*>(renderer(row));
}

mdl::LayerNode* LayerListBox::layerForRow(const int row) const
{
  if (const auto* widget = widgetAtRow(row))
  {
    return widget->layer();
  }
  return nullptr;
}

std::vector<mdl::LayerNode*> LayerListBox::layers() const
{
  const auto rowCount = count();

  return std::views::iota(0, rowCount)
         | std::views::transform([&](const auto i) { return layerForRow(i); })
         | kdl::to_vector;
}

} // namespace tb::ui
