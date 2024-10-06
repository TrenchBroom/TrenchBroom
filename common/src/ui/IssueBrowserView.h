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

#include <QAbstractItemModel>
#include <QWidget>

#include "mdl/IssueType.h"

#include <memory>
#include <vector>

class QWidget;
class QTableView;

namespace tb
{
namespace mdl
{
class Issue;
class IssueQuickFix;
} // namespace mdl

namespace ui
{
class IssueBrowserModel;
class MapDocument;

class IssueBrowserView : public QWidget
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  int m_hiddenIssueTypes = 0;
  bool m_showHiddenIssues = false;

  bool m_valid = false;

  QTableView* m_tableView = nullptr;
  IssueBrowserModel* m_tableModel = nullptr;

public:
  explicit IssueBrowserView(
    std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);

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

  std::vector<const mdl::Issue*> collectIssues(const QList<QModelIndex>& indices) const;
  std::vector<const mdl::IssueQuickFix*> collectQuickFixes(
    const QList<QModelIndex>& indices) const;
  mdl::IssueType issueTypeMask() const;

  void setIssueVisibility(bool show);

  QList<QModelIndex> getSelection() const;
  void updateSelection();
  void bindEvents();

  void itemRightClicked(const QPoint& pos);
  void itemSelectionChanged();
  void showIssues();
  void hideIssues();
  void applyQuickFix(const mdl::IssueQuickFix& quickFix);

private:
  void invalidate();
public slots:
  void validate();
};

/**
 * Trivial QAbstractTableModel subclass, when the issues list changes,
 * it just refreshes the entire list with beginResetModel()/endResetModel().
 */
class IssueBrowserModel : public QAbstractTableModel
{
  Q_OBJECT
private:
  std::vector<const mdl::Issue*> m_issues;

public:
  explicit IssueBrowserModel(QObject* parent);

  void setIssues(std::vector<const mdl::Issue*> issues);
  const std::vector<const mdl::Issue*>& issues();

public: // QAbstractTableModel overrides
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};
} // namespace ui
} // namespace tb
