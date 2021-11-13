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

#include "EntityPropertyGrid.h"

#include "Macros.h"
#include "Model/EntityProperties.h"
#include "View/BorderLine.h"
#include "View/EntityPropertyItemDelegate.h"
#include "View/EntityPropertyModel.h"
#include "View/EntityPropertyTable.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/ViewConstants.h"

#include <kdl/memory_utils.h>
#include <kdl/string_format.h>
#include <kdl/vector_set.h>

#include <vector>

#include <QAbstractButton>
#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QKeySequence>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTimer>

#define GRID_LOG(x)

namespace TrenchBroom {
namespace View {
EntityPropertyGrid::EntityPropertyGrid(std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget(parent)
  , m_document(document) {
  createGui(document);
  connectObservers();
}

void EntityPropertyGrid::backupSelection() {
  m_selectionBackup.clear();

  GRID_LOG(qDebug() << "Backup selection");
  for (const QModelIndex& index : m_table->selectionModel()->selectedIndexes()) {
    const QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    const std::string propertyKey = m_model->propertyKey(sourceIndex.row());
    m_selectionBackup.push_back({propertyKey, sourceIndex.column()});

    GRID_LOG(
      qDebug() << "Backup selection: " << QString::fromStdString(propertyKey) << ","
               << sourceIndex.column());
  }
}

void EntityPropertyGrid::restoreSelection() {
  m_table->selectionModel()->clearSelection();

  GRID_LOG(qDebug() << "Restore selection");
  for (const auto& selection : m_selectionBackup) {
    const int row = m_model->rowForPropertyKey(selection.propertyKey);
    if (row == -1) {
      GRID_LOG(
        qDebug() << "Restore selection: couldn't find "
                 << QString::fromStdString(selection.propertyKey));
      continue;
    }
    const QModelIndex sourceIndex = m_model->index(row, selection.column);
    const QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
    m_table->selectionModel()->select(proxyIndex, QItemSelectionModel::Select);
    m_table->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::Current);

    GRID_LOG(
      qDebug() << "Restore selection: " << QString::fromStdString(selection.propertyKey) << ","
               << selection.column);
  }
  GRID_LOG(
    qDebug() << "Restore selection: current is " << QString::fromStdString(selectedRowName()));
}

void EntityPropertyGrid::addProperty(const bool defaultToProtected) {
  auto document = kdl::mem_lock(m_document);
  const std::string newPropertyKey =
    PropertyRow::newPropertyKeyForEntityNodes(document->allSelectedEntityNodes());

  document->setProperty(newPropertyKey, "", defaultToProtected);

  // Force an immediate update to the table rows (by default, updates are delayed - see
  // EntityPropertyGrid::updateControls), so we can select the new row.
  m_model->updateFromMapDocument();

  const int row = m_model->rowForPropertyKey(newPropertyKey);
  ensure(row != -1, "row should have been inserted");

  // Select the newly inserted property key
  const QModelIndex mi =
    m_proxyModel->mapFromSource(m_model->index(row, EntityPropertyModel::ColumnKey));

  m_table->clearSelection();
  m_table->setCurrentIndex(mi);
  m_table->setFocus();
}

void EntityPropertyGrid::removeSelectedProperties() {
  if (!canRemoveSelectedProperties()) {
    return;
  }

  const auto selectedRows = selectedRowsAndCursorRow();

  std::vector<std::string> propertyKeys;
  for (const int row : selectedRows) {
    propertyKeys.push_back(m_model->propertyKey(row));
  }

  const size_t numRows = propertyKeys.size();
  auto document = kdl::mem_lock(m_document);

  {
    Transaction transaction(
      document, kdl::str_plural(numRows, "Remove Property", "Remove Properties"));

    bool success = true;
    for (const std::string& propertyKey : propertyKeys) {
      success = success && document->removeProperty(propertyKey);
    }

    if (!success) {
      transaction.rollback();
    }
  }
}

bool EntityPropertyGrid::canRemoveSelectedProperties() const {
  const auto rows = selectedRowsAndCursorRow();
  if (rows.empty())
    return false;

  for (const int row : rows) {
    if (!m_model->canRemove(row))
      return false;
  }
  return true;
}

/**
 * returns rows indices in the model (not proxy model).
 */
std::vector<int> EntityPropertyGrid::selectedRowsAndCursorRow() const {
  kdl::vector_set<int> result;

  QItemSelectionModel* selection = m_table->selectionModel();

  // current row
  const QModelIndex currentIndexInSource = m_proxyModel->mapToSource(selection->currentIndex());
  if (currentIndexInSource.isValid()) {
    result.insert(currentIndexInSource.row());
  }

  // selected rows
  for (const QModelIndex& index : selection->selectedIndexes()) {
    const QModelIndex indexInSource = m_proxyModel->mapToSource(index);
    if (indexInSource.isValid()) {
      result.insert(indexInSource.row());
    }
  }

  return result.release_data();
}

class EntitySortFilterProxyModel : public QSortFilterProxyModel {
public:
  explicit EntitySortFilterProxyModel(QObject* parent = nullptr)
    : QSortFilterProxyModel(parent) {}

protected:
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const {
    const EntityPropertyModel& source = dynamic_cast<const EntityPropertyModel&>(*sourceModel());

    return source.lessThan(static_cast<size_t>(left.row()), static_cast<size_t>(right.row()));
  }
};

void EntityPropertyGrid::createGui(std::weak_ptr<MapDocument> document) {
  m_table = new EntityPropertyTable();

  m_model = new EntityPropertyModel(document, this);
  m_model->setParent(m_table); // ensure the table takes ownership of the model in setModel //
                               // FIXME: why? this looks unnecessary

  m_proxyModel = new EntitySortFilterProxyModel(this);
  m_proxyModel->setSourceModel(m_model);
  m_proxyModel->sort(0); // NOTE: must be column 0, because EntitySortFilterProxyModel::lessThan
                         // ignores the column part of the QModelIndex
  m_table->setModel(m_proxyModel);

  m_table->setItemDelegate(new EntityPropertyItemDelegate(m_table, m_model, m_proxyModel, m_table));

  autoResizeRows(m_table);

  m_table->verticalHeader()->setVisible(false);
  m_table->horizontalHeader()->setSectionResizeMode(
    EntityPropertyModel::ColumnProtected, QHeaderView::ResizeToContents);
  m_table->horizontalHeader()->setSectionResizeMode(
    EntityPropertyModel::ColumnKey, QHeaderView::ResizeToContents);
  m_table->horizontalHeader()->setSectionResizeMode(
    EntityPropertyModel::ColumnValue, QHeaderView::Stretch);
  m_table->horizontalHeader()->setSectionsClickable(false);
  m_table->setSelectionBehavior(QAbstractItemView::SelectItems);

  m_addPropertyButton = createBitmapButton(
    "Add.svg", tr("Add a new property (%1)").arg(EntityPropertyTable::insertRowShortcutString()),
    this);
  connect(m_addPropertyButton, &QAbstractButton::clicked, this, [=](const bool /* checked */) {
    addProperty(false);
  });

  m_addProtectedPropertyButton =
    createBitmapButton("AddProtected.svg", tr("Add a new protected property"), this);
  connect(
    m_addProtectedPropertyButton, &QAbstractButton::clicked, this, [=](const bool /* checked */) {
      addProperty(true);
    });

  m_removePropertiesButton = createBitmapButton(
    "Remove.svg",
    tr("Remove the selected properties (%1)").arg(EntityPropertyTable::removeRowShortcutString()),
    this);
  connect(m_removePropertiesButton, &QAbstractButton::clicked, this, [=](const bool /* checked */) {
    removeSelectedProperties();
  });

  m_showDefaultPropertiesCheckBox = new QCheckBox(tr("Show default properties"));
  connect(m_showDefaultPropertiesCheckBox, &QCheckBox::stateChanged, this, [=](const int state) {
    m_model->setShowDefaultRows(state == Qt::Checked);
  });
  m_showDefaultPropertiesCheckBox->setChecked(m_model->showDefaultRows());

  connect(m_table, &EntityPropertyTable::addRowShortcutTriggered, this, [=]() {
    addProperty(false);
  });
  connect(m_table, &EntityPropertyTable::removeRowsShortcutTriggered, this, [=]() {
    removeSelectedProperties();
  });

  connect(
    m_table->selectionModel(), &QItemSelectionModel::currentChanged, this,
    [=](const QModelIndex& current, const QModelIndex& previous) {
      unused(current);
      unused(previous);
      // NOTE: when we get this signal, the selection hasn't been updated yet.
      // So selectedRowsAndCursorRow() will return a mix of the new current row and old selection.
      // Because of this, it's important to also call updateControlsEnabled() in response to
      // QItemSelectionModel::selectionChanged as we do below. (#3165)
      GRID_LOG(qDebug() << "current changed form " << previous << " to " << current);
      updateControlsEnabled();
      ensureSelectionVisible();
      emit currentRowChanged();
    });

  connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=]() {
    if (!m_table->selectionModel()->selectedIndexes().empty()) {
      backupSelection();
    }
    updateControlsEnabled();
    emit currentRowChanged();
  });

  // e.g. handles setting a value of a default property so it becomes non-default
  connect(m_proxyModel, &QAbstractItemModel::dataChanged, this, [=]() {
    updateControlsEnabled();
    emit currentRowChanged();
  });

  // e.g. handles deleting 2 rows
  connect(m_proxyModel, &QAbstractItemModel::modelReset, this, [=]() {
    updateControlsEnabled();
    emit currentRowChanged();
  });

  // Shortcuts

  auto* toolBar = createMiniToolBarLayout(
    m_addPropertyButton, m_addProtectedPropertyButton, m_removePropertiesButton,
    LayoutConstants::WideHMargin, m_showDefaultPropertiesCheckBox);

  auto* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_table, 1);
  layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
  layout->addLayout(toolBar, 0);
  setLayout(layout);

  // NOTE: Do not use QAbstractItemView::SelectedClicked.
  // EntityPropertyTable::mousePressEvent() implements its own version.
  // See: https://github.com/TrenchBroom/TrenchBroom/issues/3582
  m_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
}

void EntityPropertyGrid::connectObservers() {
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection +=
    document->documentWasNewedNotifier.connect(this, &EntityPropertyGrid::documentWasNewed);
  m_notifierConnection +=
    document->documentWasLoadedNotifier.connect(this, &EntityPropertyGrid::documentWasLoaded);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &EntityPropertyGrid::nodesDidChange);
  m_notifierConnection +=
    document->selectionWillChangeNotifier.connect(this, &EntityPropertyGrid::selectionWillChange);
  m_notifierConnection +=
    document->selectionDidChangeNotifier.connect(this, &EntityPropertyGrid::selectionDidChange);
}

void EntityPropertyGrid::documentWasNewed(MapDocument*) {
  updateControls();
}

void EntityPropertyGrid::documentWasLoaded(MapDocument*) {
  updateControls();
}

void EntityPropertyGrid::nodesDidChange(const std::vector<Model::Node*>&) {
  updateControls();
}

void EntityPropertyGrid::selectionWillChange() {}

void EntityPropertyGrid::selectionDidChange(const Selection&) {
  updateControls();
}

void EntityPropertyGrid::updateControls() {
  // When you change the selected entity in the map, there's a brief intermediate state where
  // worldspawn is selected. If we call this directly, it'll cause the table to be rebuilt based on
  // that intermediate state. Everything is fine except you lose the selected row in the table,
  // unless it's a key name that exists in worldspawn. To avoid that problem, make a delayed call to
  // update the table.
  QTimer::singleShot(0, this, [&]() {
    m_model->updateFromMapDocument();

    if (m_table->selectionModel()->selectedIndexes().empty()) {
      restoreSelection();
    }
    ensureSelectionVisible();

    const auto shouldShowProtectedProperties = m_model->shouldShowProtectedProperties();
    m_table->setColumnHidden(EntityPropertyModel::ColumnProtected, !shouldShowProtectedProperties);
    m_addProtectedPropertyButton->setHidden(!shouldShowProtectedProperties);
  });
  updateControlsEnabled();
}

void EntityPropertyGrid::ensureSelectionVisible() {
  m_table->scrollTo(m_table->currentIndex());
}

void EntityPropertyGrid::updateControlsEnabled() {
  auto document = kdl::mem_lock(m_document);
  const auto nodes = document->allSelectedEntityNodes();
  m_table->setEnabled(!nodes.empty());
  m_addPropertyButton->setEnabled(!nodes.empty());
  m_removePropertiesButton->setEnabled(!nodes.empty() && canRemoveSelectedProperties());
  m_showDefaultPropertiesCheckBox->setChecked(m_model->showDefaultRows());
}

std::string EntityPropertyGrid::selectedRowName() const {
  QModelIndex current = m_proxyModel->mapToSource(m_table->currentIndex());
  const PropertyRow* rowModel = m_model->dataForModelIndex(current);
  if (rowModel == nullptr) {
    return "";
  }

  return rowModel->key();
}
} // namespace View
} // namespace TrenchBroom
