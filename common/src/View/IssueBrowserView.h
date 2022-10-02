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

#include "Model/IssueType.h"

#include <memory>
#include <vector>

#include <QAbstractItemModel>
#include <QWidget>

class QWidget;
class QTableView;

namespace TrenchBroom {
namespace Model {
class Issue;
class IssueQuickFix;
} // namespace Model

namespace View {
class IssueBrowserModel;
class MapDocument;

class IssueBrowserView : public QWidget {
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  int m_hiddenIssueTypes;
  bool m_showHiddenIssues;

  bool m_valid;

  QTableView* m_tableView;
  IssueBrowserModel* m_tableModel;

public:
  explicit IssueBrowserView(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

private:
  void createGui();

public:
  int hiddenIssueTypes() const;
  void setHiddenIssueTypes(int hiddenIssueTypes);
  void setShowHiddenIssues(bool show);
  void reload();
  void deselectAll();

private:
  void updateIssues();

  std::vector<Model::Issue*> collectIssues(const QList<QModelIndex>& indices) const;
  std::vector<Model::IssueQuickFix*> collectQuickFixes(const QList<QModelIndex>& indices) const;
  Model::IssueType issueTypeMask() const;

  void setIssueVisibility(bool show);

  QList<QModelIndex> getSelection() const;
  void updateSelection();
  void bindEvents();

  void itemRightClicked(const QPoint& pos);
  void itemSelectionChanged();
  void showIssues();
  void hideIssues();
  void applyQuickFix(const Model::IssueQuickFix* quickFix);

private:
  void invalidate();
public slots:
  void validate();
};

/**
 * Trivial QAbstractTableModel subclass, when the issues list changes,
 * it just refreshes the entire list with beginResetModel()/endResetModel().
 */
class IssueBrowserModel : public QAbstractTableModel {
  Q_OBJECT
private:
  std::vector<Model::Issue*> m_issues;

public:
  explicit IssueBrowserModel(QObject* parent);

  void setIssues(std::vector<Model::Issue*> issues);
  const std::vector<Model::Issue*>& issues();

public: // QAbstractTableModel overrides
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};
} // namespace View
} // namespace TrenchBroom
