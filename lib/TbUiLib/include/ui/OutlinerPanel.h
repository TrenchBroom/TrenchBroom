/*
 Copyright (C) 2026

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

#include "base/NotifierConnection.h"

#include <QWidget>

class QLineEdit;
class QTreeView;
class QSortFilterProxyModel;
class QItemSelection;
class QModelIndex;
class QPoint;

namespace tb::mdl
{
class Node;
}

namespace tb::ui
{
class AppController;
class MapDocument;
class OutlinerModel;

/**
 * A tree browser for the map's node hierarchy (layers, groups, entities, brushes and
 * patches). Selecting a row selects the corresponding node in the document and vice versa;
 * double-clicking a row frames it in the active 3D view.
 */
class OutlinerPanel : public QWidget
{
  Q_OBJECT
private:
  AppController& m_appController;
  MapDocument& m_document;

  QLineEdit* m_filterBox = nullptr;
  QTreeView* m_treeView = nullptr;
  OutlinerModel* m_model = nullptr;
  QSortFilterProxyModel* m_proxyModel = nullptr;

  NotifierConnection m_notifierConnection;

  // Reentrancy guards to avoid feedback loops between the document selection and the
  // tree view's selection.
  bool m_updatingSelectionFromDocument = false;
  bool m_updatingSelectionFromTree = false;

public:
  explicit OutlinerPanel(
    AppController& appController, MapDocument& document, QWidget* parent = nullptr);

private:
  void createGui();
  void connectObservers();

  void updateSelectionFromDocument();
  void treeSelectionChanged();
  void itemDoubleClicked(const QModelIndex& proxyIndex);
  void showContextMenu(const QPoint& pos);

  std::vector<mdl::Node*> selectedNodesInTree() const;
};

} // namespace tb::ui
