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

#include "ui/OutlinerPanel.h"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QVBoxLayout>

#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Selection.h"
#include "mdl/Node.h"
#include "mdl/Selection.h"
#include "mdl/SelectionChange.h"
#include "ui/AppController.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"
#include "ui/OutlinerModel.h"
#include "ui/SearchBox.h"
#include "ui/ViewConstants.h"

#include "kd/ranges/to.h"
#include "kd/set_temp.h"

#include <algorithm>
#include <ranges>

namespace tb::ui
{

OutlinerPanel::OutlinerPanel(
  AppController& appController, MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_appController{appController}
  , m_document{document}
{
  createGui();
  connectObservers();
  updateSelectionFromDocument();
}

void OutlinerPanel::createGui()
{
  m_model = new OutlinerModel{m_document, this};
  m_proxyModel = new QSortFilterProxyModel{this};
  m_proxyModel->setSourceModel(m_model);
  m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_proxyModel->setFilterKeyColumn(OutlinerModel::NameColumn);
  m_proxyModel->setRecursiveFilteringEnabled(true);

  m_treeView = new QTreeView{};
  m_treeView->setModel(m_proxyModel);
  m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_treeView->setUniformRowHeights(true);
  m_treeView->setAllColumnsShowFocus(true);

  // All columns are user-resizable (Interactive), including Name: with Name in Stretch
  // mode instead, its right edge cannot be dragged at all, which is exactly what made
  // long, deeply-indented group names impossible to read in full (there is no scrollbar
  // to fall back on either). Type/Visible/Locked get narrow, fixed-ish default widths;
  // Name gets whatever space is left over by default, and can be resized independently
  // of the others afterwards.
  auto* header = m_treeView->header();
  header->setSectionResizeMode(QHeaderView::Interactive);
  header->setStretchLastSection(false);
  header->resizeSection(OutlinerModel::TypeColumn, 70);
  header->resizeSection(OutlinerModel::VisibleColumn, 56);
  header->resizeSection(OutlinerModel::LockedColumn, 56);
  header->resizeSection(OutlinerModel::NameColumn, 260);

  m_filterBox = createSearchBox();
  m_filterBox->setObjectName("OutlinerPanel_FilterBox");
  m_filterBox->setPlaceholderText(tr("Filter by name"));

  connect(m_filterBox, &QLineEdit::textChanged, this, [&](const QString& text) {
    m_proxyModel->setFilterFixedString(text);
    if (!text.isEmpty())
    {
      m_treeView->expandAll();
    }
  });

  connect(
    m_treeView->selectionModel(),
    &QItemSelectionModel::selectionChanged,
    this,
    [&](const QItemSelection&, const QItemSelection&) { treeSelectionChanged(); });

  connect(m_treeView, &QTreeView::doubleClicked, this, [&](const QModelIndex& index) {
    itemDoubleClicked(index);
  });

  m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(
    m_treeView,
    &QWidget::customContextMenuRequested,
    this,
    [&](const QPoint& pos) { showContextMenu(pos); });

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(LayoutConstants::NarrowVMargin);
  layout->addWidget(m_treeView, 1);
  layout->addWidget(m_filterBox);
  setLayout(layout);
}

void OutlinerPanel::connectObservers()
{
  m_notifierConnection += m_document.selectionDidChangeNotifier.connect(
    [&](const mdl::SelectionChange&) { updateSelectionFromDocument(); });
  m_notifierConnection +=
    m_document.documentWasLoadedNotifier.connect([&] { updateSelectionFromDocument(); });
}

void OutlinerPanel::updateSelectionFromDocument()
{
  if (m_updatingSelectionFromTree)
  {
    return;
  }

  const auto guard = kdl::set_temp{m_updatingSelectionFromDocument, true};

  auto* selectionModel = m_treeView->selectionModel();
  auto selection = QItemSelection{};

  for (auto* node : m_document.map().selection().nodes)
  {
    const auto sourceIndex = m_model->indexForNode(node);
    if (!sourceIndex.isValid())
    {
      continue;
    }
    const auto proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
    if (proxyIndex.isValid())
    {
      const auto lastColumn = proxyIndex.siblingAtColumn(OutlinerModel::ColumnCount - 1);
      selection.select(proxyIndex, lastColumn.isValid() ? lastColumn : proxyIndex);
    }
  }

  selectionModel->select(
    selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

  if (!selection.indexes().isEmpty())
  {
    m_treeView->scrollTo(selection.indexes().first());
  }
}

std::vector<mdl::Node*> OutlinerPanel::selectedNodesInTree() const
{
  const auto proxyIndexes = m_treeView->selectionModel()->selectedRows();

  return proxyIndexes | std::views::transform([&](const auto& proxyIndex) {
           const auto sourceIndex = m_proxyModel->mapToSource(proxyIndex);
           return m_model->nodeForIndex(sourceIndex);
         })
         | std::views::filter([](auto* node) { return node != nullptr; })
         | kdl::ranges::to<std::vector>();
}

void OutlinerPanel::treeSelectionChanged()
{
  if (m_updatingSelectionFromDocument)
  {
    return;
  }

  const auto guard = kdl::set_temp{m_updatingSelectionFromTree, true};

  auto& map = m_document.map();
  const auto selectedNodes = selectedNodesInTree();

  mdl::deselectAll(map);
  if (!selectedNodes.empty())
  {
    mdl::selectNodes(map, selectedNodes);
  }
}

void OutlinerPanel::itemDoubleClicked(const QModelIndex& proxyIndex)
{
  if (!proxyIndex.isValid())
  {
    return;
  }

  auto* node = m_model->nodeForIndex(m_proxyModel->mapToSource(proxyIndex));
  if (!node)
  {
    return;
  }

  // Opens every closed group the node is nested inside (outermost first) before
  // selecting it, so a brush found via the Outliner can be reached and selected
  // regardless of how many levels of closed groups sit between it and the world -
  // normally each level needs its own double-click in the 3D view.
  mdl::openAncestorGroupsAndSelectNode(m_document.map(), node);

  if (auto* mapWindow = m_appController.mapWindowManager().topMapWindow())
  {
    mapWindow->focusCameraOnSelection();
  }
}

void OutlinerPanel::showContextMenu(const QPoint& pos)
{
  const auto proxyIndex = m_treeView->indexAt(pos);
  auto* node = proxyIndex.isValid()
                 ? m_model->nodeForIndex(m_proxyModel->mapToSource(proxyIndex))
                 : nullptr;
  auto* groupNode = dynamic_cast<mdl::GroupNode*>(node);

  auto* popupMenu = new QMenu{this};

  if (groupNode)
  {
    popupMenu->addAction(tr("Flatten Group"), this, [&, groupNode]() {
      const auto question = tr(
                               R"(This removes "%1" and every group nested inside it, )"
                               R"(keeping its contents in place. This can only be )"
                               R"(undone with Undo. Continue?)")
                               .arg(QString::fromStdString(groupNode->group().name()));
      if (
        QMessageBox::question(this, tr("Flatten Group"), question)
        == QMessageBox::Yes)
      {
        mdl::flattenGroup(m_document.map(), *groupNode);
      }
    });
    popupMenu->addSeparator();
  }

  popupMenu->addAction(tr("Flatten All Groups"), this, [&]() {
    const auto question = tr(
      R"(This removes every group at every nesting level in the entire map, keeping )"
      R"(all brushes, entities and patches in place on their layers. This can only be )"
      R"(undone with Undo. Continue?)");
    if (
      QMessageBox::question(this, tr("Flatten All Groups"), question)
      == QMessageBox::Yes)
    {
      mdl::flattenAllGroups(m_document.map());
    }
  });

  popupMenu->popup(m_treeView->viewport()->mapToGlobal(pos));
}

} // namespace tb::ui
