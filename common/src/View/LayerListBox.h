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

#pragma once

#include "NotifierConnection.h"
#include "View/ControlListBox.h"

#include <memory>
#include <vector>

class QLabel;
class QAbstractButton;
class QListWidget;

namespace TrenchBroom {
namespace Model {
class LayerNode;
class Node;
} // namespace Model

namespace View {
class MapDocument;

class LayerListBoxWidget : public ControlListBoxItemRenderer {
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;
  Model::LayerNode* m_layer;
  QAbstractButton* m_activeButton;
  QLabel* m_nameText;
  QLabel* m_infoText;
  QAbstractButton* m_omitFromExportButton;
  QAbstractButton* m_hiddenButton;
  QAbstractButton* m_lockButton;

public:
  LayerListBoxWidget(
    std::weak_ptr<MapDocument> document, Model::LayerNode* layer, QWidget* parent = nullptr);

  void updateItem() override;

private:
  void updateLayerItem();

public:
  Model::LayerNode* layer() const;

private:
  bool eventFilter(QObject* target, QEvent* event) override;
signals:
  void layerActiveClicked(Model::LayerNode* layer);
  void layerOmitFromExportToggled(Model::LayerNode* layer);
  void layerVisibilityToggled(Model::LayerNode* layer);
  void layerLockToggled(Model::LayerNode* layer);
  void layerDoubleClicked(Model::LayerNode* layer);
  void layerRightClicked(Model::LayerNode* layer);
};

class LayerListBox : public ControlListBox {
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  NotifierConnection m_notifierConnection;

public:
  explicit LayerListBox(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

  Model::LayerNode* selectedLayer() const;
  void setSelectedLayer(Model::LayerNode* layer);

private:
  size_t itemCount() const override;

  ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) override;

  void selectedRowChanged(int index) override;

private:
  void connectObservers();

  void documentDidChange(MapDocument* document);
  void nodesDidChange(const std::vector<Model::Node*>& nodes);
  void currentLayerDidChange(const Model::LayerNode* layer);

  const LayerListBoxWidget* widgetAtRow(int row) const;
  Model::LayerNode* layerForRow(int row) const;

  std::vector<Model::LayerNode*> layers() const;
signals:
  void layerSelected(Model::LayerNode* layer);
  void layerSetCurrent(Model::LayerNode* layer);
  void layerRightClicked(Model::LayerNode* layer);
  void layerOmitFromExportToggled(Model::LayerNode* layer);
  void layerVisibilityToggled(Model::LayerNode* layer);
  void layerLockToggled(Model::LayerNode* layer);
};
} // namespace View
} // namespace TrenchBroom
