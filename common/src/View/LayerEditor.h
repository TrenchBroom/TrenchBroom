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

#include <memory>
#include <string>

#include <QWidget>

class QAbstractButton;

namespace TrenchBroom {
namespace Model {
class LayerNode;
}

namespace View {
class LayerListBox;
class MapDocument;

class LayerEditor : public QWidget {
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;
  LayerListBox* m_layerList;

  QAbstractButton* m_addLayerButton;
  QAbstractButton* m_removeLayerButton;
  QAbstractButton* m_moveLayerUpButton;
  QAbstractButton* m_moveLayerDownButton;

public:
  explicit LayerEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

private:
  void onSetCurrentLayer(Model::LayerNode* layer);
  bool canSetCurrentLayer(Model::LayerNode* layer) const;

  void onLayerRightClick(Model::LayerNode* layer);

  void onMoveSelectionToLayer();
  bool canMoveSelectionToLayer() const;

  bool canToggleLayerVisible() const;
  void toggleLayerVisible(Model::LayerNode* layer);

  bool canToggleLayerLocked() const;
  void toggleLayerLocked(Model::LayerNode* layer);

  void toggleOmitLayerFromExport(Model::LayerNode* layer);

  void isolateLayer(Model::LayerNode* layer);

  void onSelectAllInLayer();
  bool canSelectAllInLayer() const;

  void onAddLayer();

  void onRemoveLayer();
  bool canRemoveLayer() const;

  void onRenameLayer();
  bool canRenameLayer() const;

  void onShowAllLayers();
  bool canShowAllLayers() const;

  void onHideAllLayers();
  bool canHideAllLayers() const;

  void onLockAllLayers();
  bool canLockAllLayers() const;

  void onUnlockAllLayers();
  bool canUnlockAllLayers() const;

  bool canMoveLayer(int direction) const;
  void moveLayer(Model::LayerNode* layer, int direction);

private:
  Model::LayerNode* findVisibleAndUnlockedLayer(const Model::LayerNode* except) const;
  void createGui();
private slots:
  void updateButtons();
};
} // namespace View
} // namespace TrenchBroom
