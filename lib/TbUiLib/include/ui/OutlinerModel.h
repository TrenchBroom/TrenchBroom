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

#include <QAbstractItemModel>

#include <vector>

namespace tb::mdl
{
class Node;
}

namespace tb::ui
{
class MapDocument;

/**
 * A QAbstractItemModel exposing the map's node tree (layers, groups, entities, brushes
 * and patches) so it can be browsed in a QTreeView.
 *
 * This model wraps the live mdl::Node tree directly (nodes are used as the internal
 * pointer for QModelIndex) rather than mirroring it into a parallel structure. It rebuilds
 * itself on structural notifiers (nodes added/removed, document loaded) and emits
 * lightweight dataChanged signals for visibility/lock toggles, so that expand/collapse
 * state survives everything except an actual structural change.
 */
class OutlinerModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  enum Column
  {
    NameColumn = 0,
    TypeColumn = 1,
    VisibleColumn = 2,
    LockedColumn = 3,
    ColumnCount = 4
  };

private:
  MapDocument& m_document;
  NotifierConnection m_notifierConnection;

public:
  explicit OutlinerModel(MapDocument& document, QObject* parent = nullptr);

public: // QAbstractItemModel overrides
  QModelIndex index(
    int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(
    int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

public:
  /**
   * Returns the model index corresponding to the given node, or an invalid index if the
   * node is not represented in this model (this is the case for the world node itself,
   * which is used as an implicit root).
   */
  QModelIndex indexForNode(const mdl::Node* node) const;

  /**
   * Returns the node represented by the given index, or nullptr if the index is invalid.
   */
  mdl::Node* nodeForIndex(const QModelIndex& index) const;

private:
  void connectObservers();

  void reload();
  void refreshAllData();
  void refreshDataForNodes(const std::vector<mdl::Node*>& nodes);

  std::vector<mdl::Node*> topLevelNodes() const;

  static mdl::Node* toNode(const QModelIndex& index);
  void emitDataChangedRecursive(const QModelIndex& parent);
};

} // namespace tb::ui
