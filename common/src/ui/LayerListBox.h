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

#pragma once

#include "NotifierConnection.h"
#include "ui/ControlListBox.h"

#include <memory>
#include <vector>

class QLabel;
class QAbstractButton;
class QListWidget;

namespace tb::mdl
{
class LayerNode;
class Map;
class Node;
} // namespace tb::mdl

namespace tb::ui
{
class MapDocument;

class LayerListBoxWidget : public ControlListBoxItemRenderer
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;
  mdl::LayerNode* m_layer = nullptr;
  QAbstractButton* m_activeButton = nullptr;
  QLabel* m_nameText = nullptr;
  QLabel* m_infoText = nullptr;
  QAbstractButton* m_omitFromExportButton = nullptr;
  QAbstractButton* m_hiddenButton = nullptr;
  QAbstractButton* m_lockButton = nullptr;

public:
  LayerListBoxWidget(
    std::weak_ptr<MapDocument> document,
    mdl::LayerNode* layer,
    QWidget* parent = nullptr);

  void updateItem() override;

private:
  void updateLayerItem();

public:
  mdl::LayerNode* layer() const;

private:
  bool eventFilter(QObject* target, QEvent* event) override;
signals:
  void layerActiveClicked(mdl::LayerNode* layer);
  void layerOmitFromExportToggled(mdl::LayerNode* layer);
  void layerVisibilityToggled(mdl::LayerNode* layer);
  void layerLockToggled(mdl::LayerNode* layer);
  void layerDoubleClicked(mdl::LayerNode* layer);
  void layerRightClicked(mdl::LayerNode* layer);
};

class LayerListBox : public ControlListBox
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  NotifierConnection m_notifierConnection;

public:
  explicit LayerListBox(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

  mdl::LayerNode* selectedLayer() const;
  void setSelectedLayer(mdl::LayerNode* layer);
  void updateSelectionForRemoval();

private:
  size_t itemCount() const override;

  ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) override;

  void selectedRowChanged(int index) override;

private:
  void connectObservers();

  void mapDidChange(mdl::Map& map);
  void nodesDidChange(const std::vector<mdl::Node*>& nodes);
  void currentLayerDidChange(const mdl::LayerNode* layer);

  const LayerListBoxWidget* widgetAtRow(int row) const;
  mdl::LayerNode* layerForRow(int row) const;

  std::vector<mdl::LayerNode*> layers() const;
signals:
  void layerSelected(mdl::LayerNode* layer);
  void layerSetCurrent(mdl::LayerNode* layer);
  void layerRightClicked(mdl::LayerNode* layer);
  void layerOmitFromExportToggled(mdl::LayerNode* layer);
  void layerVisibilityToggled(mdl::LayerNode* layer);
  void layerLockToggled(mdl::LayerNode* layer);
};

} // namespace tb::ui
