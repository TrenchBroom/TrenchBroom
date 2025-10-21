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

#include <QWidget>

class QAbstractButton;

namespace tb::mdl
{
class LayerNode;
class Map;
} // namespace tb::mdl

namespace tb::ui
{
class LayerListBox;

class LayerEditor : public QWidget
{
  Q_OBJECT
private:
  mdl::Map& m_map;
  LayerListBox* m_layerList = nullptr;

  QAbstractButton* m_addLayerButton = nullptr;
  QAbstractButton* m_removeLayerButton = nullptr;
  QAbstractButton* m_moveLayerUpButton = nullptr;
  QAbstractButton* m_moveLayerDownButton = nullptr;

public:
  explicit LayerEditor(mdl::Map& map, QWidget* parent = nullptr);

private:
  void onSetCurrentLayer(mdl::LayerNode* layer);
  bool canSetCurrentLayer(mdl::LayerNode* layer) const;

  void onLayerRightClick(mdl::LayerNode* layer);

  void onMoveSelectedNodesToLayer();
  bool canMoveSelectedNodesToLayer() const;

  bool canToggleLayerVisible() const;
  void toggleLayerVisible(mdl::LayerNode* layer);

  bool canToggleLayerLocked() const;
  void toggleLayerLocked(mdl::LayerNode* layer);

  void toggleOmitLayerFromExport(mdl::LayerNode* layer);

  void isolateLayer(mdl::LayerNode* layer);

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
  void moveLayer(mdl::LayerNode* layer, int direction);

private:
  mdl::LayerNode* findVisibleAndUnlockedLayer(const mdl::LayerNode* except) const;
  void createGui();
private slots:
  void updateButtons();
};

} // namespace tb::ui
