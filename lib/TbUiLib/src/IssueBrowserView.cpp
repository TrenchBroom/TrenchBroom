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

#include "ui/IssueBrowserView.h"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMenu>
#include <QStackedLayout>
#include <QStandardItemModel>
#include <QTableView>

#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"
#include "ui/SignalDelayer.h"

#include "kd/overload.h"
#include "kd/ranges/to.h"
#include "kd/vector_utils.h"

#include <fmt/format.h>

#include <chrono> // IWYU pragma: keep
#include <vector>

namespace tb::ui
{
namespace
{

/**
 * Measures the height of a single row of unwrapped text and sets it as a fixed
 * row height for all rows. Letting Qt resize every row to its contents becomes
 * pathologically slow once the table holds a lot of issues.
 */
void fixRowHeightToContents(QTableView& tableView)
{
  auto measurementModel = QStandardItemModel{1, 2};
  measurementModel.setItem(0, 0, new QStandardItem{"0"});
  measurementModel.setItem(0, 1, new QStandardItem{"Issue"});

  auto* previousModel = tableView.model();

  tableView.setModel(&measurementModel);
  tableView.verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  tableView.resizeRowsToContents();
  tableView.verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  tableView.verticalHeader()->setDefaultSectionSize(
    tableView.verticalHeader()->sectionSize(0));

  tableView.setModel(previousModel);
}

} // namespace

using namespace std::chrono_literals;

IssueBrowserView::IssueBrowserView(MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
  , m_validateSignalDelayer{new SignalDelayer{500ms, this}}
{
  createGui();
  bindEvents();
}

void IssueBrowserView::createGui()
{
  m_tableModel = new IssueBrowserModel{this};

  m_tableView = new QTableView{};
  m_tableView->setModel(m_tableModel);
  m_tableView->verticalHeader()->setVisible(false);
  m_tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  m_tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_tableView->horizontalHeader()->setSectionsClickable(false);
  m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

  fixRowHeightToContents(*m_tableView);

  m_validatingLabel = new QLabel{tr("Validating, please wait...")};
  m_validatingLabel->setAlignment(Qt::AlignCenter);

  auto font = m_validatingLabel->font();
  font.setPointSize(font.pointSize() + 8);
  m_validatingLabel->setFont(font);

  auto palette = m_validatingLabel->palette();
  palette.setColor(QPalette::WindowText, Qt::gray);
  m_validatingLabel->setPalette(palette);

  m_stackedLayout = new QStackedLayout{};
  m_stackedLayout->setContentsMargins(0, 0, 0, 0);
  m_stackedLayout->addWidget(m_tableView);
  m_stackedLayout->addWidget(m_validatingLabel);
  setLayout(m_stackedLayout);
}

int IssueBrowserView::hiddenIssueTypes() const
{
  return m_hiddenIssueTypes;
}

void IssueBrowserView::setHiddenIssueTypes(const int hiddenIssueTypes)
{
  if (hiddenIssueTypes != m_hiddenIssueTypes)
  {
    m_hiddenIssueTypes = hiddenIssueTypes;
    invalidate();
  }
}

void IssueBrowserView::setShowHiddenIssues(const bool show)
{
  m_showHiddenIssues = show;
  invalidate();
}

void IssueBrowserView::reload()
{
  invalidate();
}

void IssueBrowserView::deselectAll()
{
  m_tableView->clearSelection();
}

/**
 * Updates the MapDocument selection to match the table view
 */
void IssueBrowserView::updateSelection()
{
  auto& map = m_document.map();

  auto nodes = std::vector<mdl::Node*>{};
  for (const auto* issue : collectIssues(getSelection()))
  {
    if (!issue->addSelectableNodes(nodes))
    {
      nodes.clear();
      break;
    }
  }

  mdl::deselectAll(map);
  selectNodes(map, nodes);
}

void IssueBrowserView::updateIssues()
{
  auto& map = m_document.map();
  const auto validators = map.worldNode().registeredValidators();

  auto issues = std::vector<const mdl::Issue*>{};
  const auto collectIssues = [&](auto& node) {
    for (auto* issue : node.issues(validators))
    {
      if (
        m_showHiddenIssues
        || (!issue->hidden() && (issue->type() & m_hiddenIssueTypes) == 0))
      {
        issues.push_back(issue);
      }
    }
  };

  map.worldNode().accept(kdl::overload(
    [&](auto&& thisLambda, mdl::WorldNode& worldNode) {
      collectIssues(worldNode);
      worldNode.visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::LayerNode& layerNode) {
      collectIssues(layerNode);
      layerNode.visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::GroupNode& groupNode) {
      collectIssues(groupNode);
      groupNode.visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::EntityNode& entityNode) {
      collectIssues(entityNode);
      entityNode.visitChildren(thisLambda);
    },
    [&](mdl::BrushNode& brushNode) { collectIssues(brushNode); },
    [&](mdl::PatchNode& patchNode) { collectIssues(patchNode); }));

  issues = kdl::vec_sort(std::move(issues), [](const auto* lhs, const auto* rhs) {
    return lhs->seqId() > rhs->seqId();
  });
  m_tableModel->setIssues(std::move(issues));
}

void IssueBrowserView::applyQuickFix(const mdl::IssueQuickFix& quickFix)
{
  auto& map = m_document.map();
  const auto issues = collectIssues(getSelection());

  auto transaction =
    mdl::Transaction{map, fmt::format("Apply Quick Fix ({})", quickFix.description())};
  updateSelection();
  quickFix.apply(map, issues);
  transaction.commit();
}

std::vector<const mdl::Issue*> IssueBrowserView::collectIssues(
  const QList<QModelIndex>& indices) const
{
  auto issues = indices | std::views::filter(&QModelIndex::isValid)
                | std::views::transform([&](const auto& index) {
                    const auto row = static_cast<size_t>(index.row());
                    return m_tableModel->issues().at(row);
                  })
                | kdl::ranges::to<std::vector>();

  // The QModelIndex list returned by getSelection() contains duplicates
  // (not sure why, current row and selected row?)
  kdl::vec_sort_and_remove_duplicates(issues);
  return issues;
}

std::vector<const mdl::IssueQuickFix*> IssueBrowserView::collectQuickFixes(
  const QList<QModelIndex>& indices) const
{
  if (indices.empty())
  {
    return {};
  }

  auto issueTypes = ~static_cast<mdl::IssueType>(0);
  for (const auto& index : indices)
  {
    if (!index.isValid())
    {
      continue;
    }
    const auto* issue = m_tableModel->issues().at(static_cast<size_t>(index.row()));
    issueTypes &= issue->type();
  }

  return m_document.map().worldNode().quickFixes(issueTypes);
}

mdl::IssueType IssueBrowserView::issueTypeMask() const
{
  auto result = ~static_cast<mdl::IssueType>(0);
  for (const auto* issue : collectIssues(getSelection()))
  {
    result &= issue->type();
  }
  return result;
}

void IssueBrowserView::setIssueVisibility(const bool show)
{
  auto& map = m_document.map();
  for (const auto* issue : collectIssues(getSelection()))
  {
    map.setIssueHidden(*issue, !show);
  }

  invalidate();
}

QList<QModelIndex> IssueBrowserView::getSelection() const
{
  return m_tableView->selectionModel()->selectedIndexes();
}

void IssueBrowserView::bindEvents()
{
  m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(
    m_tableView,
    &QWidget::customContextMenuRequested,
    this,
    &IssueBrowserView::itemRightClicked);

  connect(
    m_tableView->selectionModel(),
    &QItemSelectionModel::selectionChanged,
    this,
    &IssueBrowserView::itemSelectionChanged);

  connect(
    m_validateSignalDelayer,
    &SignalDelayer::processSignal,
    this,
    &IssueBrowserView::validate);
}

void IssueBrowserView::itemRightClicked(const QPoint& pos)
{
  const auto selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
  if (selectedIndexes.empty())
  {
    return;
  }

  auto* popupMenu = new QMenu{this};
  popupMenu->addAction(tr("Show"), this, &IssueBrowserView::showIssues);
  popupMenu->addAction(tr("Hide"), this, &IssueBrowserView::hideIssues);

  const auto quickFixes = collectQuickFixes(selectedIndexes);
  if (!quickFixes.empty())
  {
    auto* quickFixMenu = new QMenu{};
    quickFixMenu->setTitle(tr("Fix"));

    for (const auto* quickFix : quickFixes)
    {
      quickFixMenu->addAction(
        QString::fromStdString(quickFix->description()), this, [&, quickFix]() {
          applyQuickFix(*quickFix);
        });
    }

    popupMenu->addSeparator();
    popupMenu->addMenu(quickFixMenu);
  }

  // `pos` is in m_tableView->viewport() coordinates as per:
  // http://doc.qt.io/qt-5/qwidget.html#customContextMenuRequested
  popupMenu->popup(m_tableView->viewport()->mapToGlobal(pos));
}

void IssueBrowserView::itemSelectionChanged()
{
  updateSelection();
}

void IssueBrowserView::showIssues()
{
  setIssueVisibility(true);
}

void IssueBrowserView::hideIssues()
{
  setIssueVisibility(false);
}

void IssueBrowserView::invalidate()
{
  m_valid = false;
  m_tableView->setEnabled(false);
  m_tableView->setUpdatesEnabled(false);
  m_stackedLayout->setCurrentWidget(m_validatingLabel);

  m_validateSignalDelayer->queueSignal();
}

void IssueBrowserView::validate()
{
  if (!m_valid)
  {
    updateIssues();
    m_valid = true;
    m_tableView->setEnabled(true);
    m_tableView->setUpdatesEnabled(true);
    m_stackedLayout->setCurrentWidget(m_tableView);
  }
}

// IssueBrowserModel

IssueBrowserModel::IssueBrowserModel(QObject* parent)
  : QAbstractTableModel{parent}
{
}

void IssueBrowserModel::setIssues(std::vector<const mdl::Issue*> issues)
{
  beginResetModel();
  m_issues = std::move(issues);
  endResetModel();
}

const std::vector<const mdl::Issue*>& IssueBrowserModel::issues()
{
  return m_issues;
}

int IssueBrowserModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : static_cast<int>(m_issues.size());
}

int IssueBrowserModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : 2;
}

QVariant IssueBrowserModel::data(const QModelIndex& index, const int role) const
{
  if (
    !index.isValid() || index.row() < 0
    || index.row() >= static_cast<int>(m_issues.size()) || index.column() < 0
    || index.column() >= 2)
  {
    return QVariant{};
  }

  const auto* issue = m_issues.at(static_cast<size_t>(index.row()));

  if (role == Qt::DisplayRole)
  {
    if (index.column() == 0)
    {
      if (issue->lineNumber() > 0)
      {
        return QVariant::fromValue<size_t>(issue->lineNumber());
      }
    }
    else
    {
      return QVariant{QString::fromStdString(issue->description())};
    }
  }
  else if (role == Qt::FontRole)
  {
    if (issue->hidden())
    {
      // hidden issues are italic
      auto italicFont = QFont{};
      italicFont.setItalic(true);
      return QVariant{italicFont};
    }
  }

  return QVariant{};
}

QVariant IssueBrowserModel::headerData(
  const int section, const Qt::Orientation orientation, const int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant{};
  }

  if (orientation == Qt::Horizontal)
  {
    if (section == 0)
    {
      return QVariant{tr("Line")};
    }
    if (section == 1)
    {
      return QVariant{tr("Description")};
    }
  }
  return QVariant{};
}

} // namespace tb::ui
