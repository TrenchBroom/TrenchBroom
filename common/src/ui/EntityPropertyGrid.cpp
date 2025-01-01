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

#include "EntityPropertyGrid.h"

#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMenu>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTimer>
#include <QToolButton>

#include "Macros.h"
#include "mdl/EntityNodeBase.h" // IWYU pragma: keep
#include "mdl/Node.h"
#include "ui/BorderLine.h"
#include "ui/EntityPropertyItemDelegate.h"
#include "ui/EntityPropertyModel.h"
#include "ui/EntityPropertyTable.h"
#include "ui/MapDocument.h"
#include "ui/QtUtils.h"
#include "ui/Transaction.h"
#include "ui/ViewConstants.h"

#include "kdl/memory_utils.h"
#include "kdl/string_format.h"
#include "kdl/vector_set.h"
#include "kdl/vector_utils.h"

#include <vector>

#define GRID_LOG(x)

namespace tb::ui
{

EntityPropertyGrid::EntityPropertyGrid(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  createGui(m_document);
  connectObservers();
}

void EntityPropertyGrid::backupSelection()
{
  m_selectionBackup.clear();

  GRID_LOG(qDebug() << "Backup selection");
  for (const auto& index : m_table->selectionModel()->selectedIndexes())
  {
    const auto sourceIndex = m_proxyModel->mapToSource(index);
    const auto propertyKey = m_model->propertyKey(sourceIndex.row());
    m_selectionBackup.push_back({propertyKey, sourceIndex.column()});

    GRID_LOG(
      qDebug() << "Backup selection: " << QString::fromStdString(propertyKey) << ","
               << sourceIndex.column());
  }
}

void EntityPropertyGrid::restoreSelection()
{
  m_table->selectionModel()->clearSelection();

  GRID_LOG(qDebug() << "Restore selection");
  for (const auto& selection : m_selectionBackup)
  {
    const auto row = m_model->rowForPropertyKey(selection.propertyKey);
    if (row == -1)
    {
      GRID_LOG(
        qDebug() << "Restore selection: couldn't find "
                 << QString::fromStdString(selection.propertyKey));
      continue;
    }
    const auto sourceIndex = m_model->index(row, selection.column);
    const auto proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
    m_table->selectionModel()->select(proxyIndex, QItemSelectionModel::Select);
    m_table->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::Current);

    GRID_LOG(
      qDebug() << "Restore selection: " << QString::fromStdString(selection.propertyKey)
               << "," << selection.column);
  }
  GRID_LOG(
    qDebug() << "Restore selection: current is "
             << QString::fromStdString(selectedRowName()));
}

void EntityPropertyGrid::addProperty(const bool defaultToProtected)
{
  auto document = kdl::mem_lock(m_document);
  const auto newPropertyKey =
    newPropertyKeyForEntityNodes(document->allSelectedEntityNodes());

  if (!document->setProperty(newPropertyKey, "", defaultToProtected))
  {
    // Setting a property can fail if a linked group update would be inconsistent
    return;
  }

  // Force an immediate update to the table rows (by default, updates are delayed - see
  // EntityPropertyGrid::updateControls), so we can select the new row.
  m_model->updateFromMapDocument();

  const auto row = m_model->rowForPropertyKey(newPropertyKey);
  ensure(row != -1, "row should have been inserted");

  // Select the newly inserted property key
  const auto mappedIndex =
    m_proxyModel->mapFromSource(m_model->index(row, EntityPropertyModel::ColumnKey));

  m_table->clearSelection();
  m_table->setCurrentIndex(mappedIndex);
  m_table->setFocus();
}

void EntityPropertyGrid::removeSelectedProperties()
{
  if (!canRemoveSelectedProperties())
  {
    return;
  }

  const auto selectedRows = selectedRowsAndCursorRow();
  const auto propertyKeys = kdl::vec_transform(
    selectedRows, [&](const auto row) { return m_model->propertyKey(row); });

  const auto numRows = propertyKeys.size();
  auto document = kdl::mem_lock(m_document);

  auto transaction = Transaction{
    document, kdl::str_plural(numRows, "Remove Property", "Remove Properties")};

  for (const auto& propertyKey : propertyKeys)
  {
    if (!document->removeProperty(propertyKey))
    {
      transaction.cancel();
      return;
    }
  }

  transaction.commit();
}

bool EntityPropertyGrid::canRemoveSelectedProperties() const
{
  const auto rows = selectedRowsAndCursorRow();
  return !rows.empty() && std::all_of(rows.begin(), rows.end(), [&](const auto row) {
    return m_model->canRemove(row);
  });
}

/**
 * returns rows indices in the model (not proxy model).
 */
std::vector<int> EntityPropertyGrid::selectedRowsAndCursorRow() const
{
  auto result = kdl::vector_set<int>{};

  auto* selection = m_table->selectionModel();

  // current row
  const auto currentIndexInSource = m_proxyModel->mapToSource(selection->currentIndex());
  if (currentIndexInSource.isValid())
  {
    result.insert(currentIndexInSource.row());
  }

  // selected rows
  for (const auto& index : selection->selectedIndexes())
  {
    const auto indexInSource = m_proxyModel->mapToSource(index);
    if (indexInSource.isValid())
    {
      result.insert(indexInSource.row());
    }
  }

  return result.release_data();
}

class EntitySortFilterProxyModel : public QSortFilterProxyModel
{
public:
  explicit EntitySortFilterProxyModel(QObject* parent = nullptr)
    : QSortFilterProxyModel{parent}
  {
  }

protected:
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override
  {
    const auto& source = dynamic_cast<const EntityPropertyModel&>(*sourceModel());
    return source.lessThan(
      static_cast<size_t>(left.row()), static_cast<size_t>(right.row()));
  }
};

void EntityPropertyGrid::createGui(std::weak_ptr<MapDocument> document)
{
  m_table = new EntityPropertyTable{};

  m_model = new EntityPropertyModel{document, this};

  // ensure the table takes ownership of the model in setModel
  // FIXME: why? this looks unnecessary
  m_model->setParent(m_table);

  m_proxyModel = new EntitySortFilterProxyModel{this};
  m_proxyModel->setSourceModel(m_model);

  // NOTE: must be column 0, because EntitySortFilterProxymdl::lessThan ignores the
  // column part of the QModelIndex
  m_proxyModel->sort(0);
  m_table->setModel(m_proxyModel);

  m_table->setItemDelegate(
    new EntityPropertyItemDelegate{m_table, m_model, m_proxyModel, m_table});

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
    "Add.svg",
    tr("Add a new property (%1)").arg(EntityPropertyTable::insertRowShortcutString()),
    this);
  connect(
    m_addPropertyButton, &QAbstractButton::clicked, this, [&](const bool /* checked */) {
      addProperty(false);
    });

  m_addProtectedPropertyButton =
    createBitmapButton("AddProtected.svg", tr("Add a new protected property"), this);
  connect(
    m_addProtectedPropertyButton,
    &QAbstractButton::clicked,
    this,
    [&](const bool /* checked */) { addProperty(true); });

  m_removePropertiesButton = createBitmapButton(
    "Remove.svg",
    tr("Remove the selected properties (%1)")
      .arg(EntityPropertyTable::removeRowShortcutString()),
    this);
  connect(
    m_removePropertiesButton,
    &QAbstractButton::clicked,
    this,
    [&](const bool /* checked */) { removeSelectedProperties(); });

  auto* setDefaultPropertiesMenu = new QMenu{this};
  setDefaultPropertiesMenu->addAction(tr("Set existing default properties"), this, [&]() {
    kdl::mem_lock(m_document)
      ->setDefaultProperties(mdl::SetDefaultPropertyMode::SetExisting);
  });
  setDefaultPropertiesMenu->addAction(tr("Set missing default properties"), this, [&]() {
    kdl::mem_lock(m_document)
      ->setDefaultProperties(mdl::SetDefaultPropertyMode::SetMissing);
  });
  setDefaultPropertiesMenu->addAction(tr("Set all default properties"), this, [&]() {
    kdl::mem_lock(m_document)
      ->setDefaultProperties(mdl::SetDefaultPropertyMode::SetMissing);
  });

  m_setDefaultPropertiesButton =
    createBitmapButton("SetDefaultProperties.svg", tr("Set default properties"), this);
  m_setDefaultPropertiesButton->setPopupMode(QToolButton::InstantPopup);
  m_setDefaultPropertiesButton->setMenu(setDefaultPropertiesMenu);

  m_showDefaultPropertiesCheckBox = new QCheckBox{tr("Show default properties")};
  connect(
    m_showDefaultPropertiesCheckBox,
    &QCheckBox::checkStateChanged,
    this,
    [&](const int state) { m_model->setShowDefaultRows(state == Qt::Checked); });
  m_showDefaultPropertiesCheckBox->setChecked(m_model->showDefaultRows());

  connect(m_table, &EntityPropertyTable::addRowShortcutTriggered, this, [&]() {
    addProperty(false);
  });
  connect(m_table, &EntityPropertyTable::removeRowsShortcutTriggered, this, [&]() {
    removeSelectedProperties();
  });

  connect(
    m_table->selectionModel(),
    &QItemSelectionModel::currentChanged,
    this,
    [&](const QModelIndex& current, const QModelIndex& previous) {
      unused(current);
      unused(previous);
      // NOTE: when we get this signal, the selection hasn't been updated yet.
      // So selectedRowsAndCursorRow() will return a mix of the new current row and old
      // selection. Because of this, it's important to also call updateControlsEnabled()
      // in response to QItemSelectionModel::selectionChanged as we do below. (#3165)
      GRID_LOG(qDebug() << "current changed form " << previous << " to " << current);
      updateControlsEnabled();
      ensureSelectionVisible();
      emit currentRowChanged();
    });

  connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]() {
    if (!m_table->selectionModel()->selectedIndexes().empty())
    {
      backupSelection();
    }
    updateControlsEnabled();
    emit currentRowChanged();
  });

  // e.g. handles setting a value of a default property so it becomes non-default
  connect(m_proxyModel, &QAbstractItemModel::dataChanged, this, [&]() {
    updateControlsEnabled();
    emit currentRowChanged();
  });

  // e.g. handles deleting 2 rows
  connect(m_proxyModel, &QAbstractItemModel::modelReset, this, [&]() {
    updateControlsEnabled();
    emit currentRowChanged();
  });

  // Shortcuts

  auto* toolBar = createMiniToolBarLayout(
    m_addPropertyButton,
    m_addProtectedPropertyButton,
    m_removePropertiesButton,
    LayoutConstants::WideHMargin,
    m_setDefaultPropertiesButton,
    LayoutConstants::WideHMargin,
    m_showDefaultPropertiesCheckBox);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_table, 1);
  layout->addWidget(new BorderLine{}, 0);
  layout->addLayout(toolBar, 0);
  setLayout(layout);

  // NOTE: Do not use QAbstractItemView::SelectedClicked.
  // EntityPropertyTable::mousePressEvent() implements its own version.
  // See: https://github.com/TrenchBroom/TrenchBroom/issues/3582
  m_table->setEditTriggers(
    QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
}

void EntityPropertyGrid::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->documentWasNewedNotifier.connect(
    this, &EntityPropertyGrid::documentWasNewed);
  m_notifierConnection += document->documentWasLoadedNotifier.connect(
    this, &EntityPropertyGrid::documentWasLoaded);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &EntityPropertyGrid::nodesDidChange);
  m_notifierConnection += document->selectionWillChangeNotifier.connect(
    this, &EntityPropertyGrid::selectionWillChange);
  m_notifierConnection += document->selectionDidChangeNotifier.connect(
    this, &EntityPropertyGrid::selectionDidChange);
}

void EntityPropertyGrid::documentWasNewed(MapDocument*)
{
  updateControls();
}

void EntityPropertyGrid::documentWasLoaded(MapDocument*)
{
  updateControls();
}

void EntityPropertyGrid::nodesDidChange(const std::vector<mdl::Node*>&)
{
  updateControls();
}

void EntityPropertyGrid::selectionWillChange() {}

void EntityPropertyGrid::selectionDidChange(const Selection&)
{
  updateControls();
}

void EntityPropertyGrid::updateControls()
{
  // When you change the selected entity in the map, there's a brief intermediate state
  // where worldspawn is selected. If we call this directly, it'll cause the table to be
  // rebuilt based on that intermediate state. Everything is fine except you lose the
  // selected row in the table, unless it's a key name that exists in worldspawn. To avoid
  // that problem, make a delayed call to update the table.
  QTimer::singleShot(0, this, [&]() {
    m_model->updateFromMapDocument();

    if (m_table->selectionModel()->selectedIndexes().empty())
    {
      restoreSelection();
    }
    ensureSelectionVisible();

    const auto shouldShowProtectedProperties = m_model->shouldShowProtectedProperties();
    m_table->setColumnHidden(
      EntityPropertyModel::ColumnProtected, !shouldShowProtectedProperties);
    m_addProtectedPropertyButton->setHidden(!shouldShowProtectedProperties);
  });
  updateControlsEnabled();
}

void EntityPropertyGrid::ensureSelectionVisible()
{
  m_table->scrollTo(m_table->currentIndex());
}

void EntityPropertyGrid::updateControlsEnabled()
{
  auto document = kdl::mem_lock(m_document);
  const auto nodes = document->allSelectedEntityNodes();
  const auto canUpdateLinkedGroups =
    document->canUpdateLinkedGroups(kdl::vec_static_cast<mdl::Node*>(nodes));
  m_table->setEnabled(!nodes.empty() && canUpdateLinkedGroups);
  m_addPropertyButton->setEnabled(!nodes.empty() && canUpdateLinkedGroups);
  m_removePropertiesButton->setEnabled(
    !nodes.empty() && canUpdateLinkedGroups && canRemoveSelectedProperties());
  m_setDefaultPropertiesButton->setEnabled(!nodes.empty() && canUpdateLinkedGroups);
  m_showDefaultPropertiesCheckBox->setChecked(m_model->showDefaultRows());
}

std::string EntityPropertyGrid::selectedRowName() const
{
  const auto current = m_proxyModel->mapToSource(m_table->currentIndex());
  const auto* rowModel = m_model->dataForModelIndex(current);
  return rowModel ? rowModel->key() : "";
}

} // namespace tb::ui
