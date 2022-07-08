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

#include <memory>
#include <string>
#include <vector>

#include <QWidget>

class QTableView;
class QCheckBox;
class QAbstractButton;
class QShortcut;
class QSortFilterProxyModel;

namespace TrenchBroom {
namespace Model {
class EntityNode;
class Node;
} // namespace Model

namespace View {
class EntityPropertyModel;
class EntityPropertyTable;
class MapDocument;
class Selection;

struct PropertyGridSelection {
  std::string propertyKey;
  int column;
};

/**
 * Panel with the entity property table, and the toolbar below it (add/remove icons,
 * "show default properties" checkbox, etc.)
 */
class EntityPropertyGrid : public QWidget {
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  EntityPropertyModel* m_model;
  QSortFilterProxyModel* m_proxyModel;
  EntityPropertyTable* m_table;
  QAbstractButton* m_addProtectedPropertyButton;
  QAbstractButton* m_addPropertyButton;
  QAbstractButton* m_removePropertiesButton;
  QCheckBox* m_showDefaultPropertiesCheckBox;
  std::vector<PropertyGridSelection> m_selectionBackup;

  NotifierConnection m_notifierConnection;

public:
  explicit EntityPropertyGrid(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

private:
  void backupSelection();
  void restoreSelection();

  void addProperty(bool defaultToProtected);
  void removeSelectedProperties();

  bool canRemoveSelectedProperties() const;
  std::vector<int> selectedRowsAndCursorRow() const;

private:
  void createGui(std::weak_ptr<MapDocument> document);

  void connectObservers();

  void documentWasNewed(MapDocument* document);
  void documentWasLoaded(MapDocument* document);
  void nodesDidChange(const std::vector<Model::Node*>& nodes);
  void selectionWillChange();
  void selectionDidChange(const Selection& selection);
  void entityDefinitionsOrModsDidChange();

private:
  void ensureSelectionVisible();
  void updateControls();
  void updateControlsEnabled();

public:
  std::string selectedRowName() const;
signals:
  void currentRowChanged();
};
} // namespace View
} // namespace TrenchBroom
