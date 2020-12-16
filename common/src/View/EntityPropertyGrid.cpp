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
#include "View/EntityAttributeItemDelegate.h"
#include "View/EntityAttributeModel.h"
#include "View/EntityAttributeTable.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>
#include <kdl/string_format.h>
#include <kdl/vector_set.h>

#include <vector>

#include <QHeaderView>
#include <QTableView>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QAbstractButton>
#include <QShortcut>
#include <QKeySequence>
#include <QDebug>
#include <QKeyEvent>
#include <QSortFilterProxyModel>
#include <QTimer>

#define GRID_LOG(x)

namespace TrenchBroom {
    namespace View {
        EntityAttributeGrid::EntityAttributeGrid(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_document(document) {
            createGui(document);
            bindObservers();
        }

        EntityAttributeGrid::~EntityAttributeGrid() {
            unbindObservers();
        }

        void EntityAttributeGrid::backupSelection() {
            m_selectionBackup.clear();

            GRID_LOG(qDebug() << "Backup selection");
            for (const QModelIndex& index : m_table->selectionModel()->selectedIndexes()) {
                const QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
                const std::string attributeName = m_model->attributeName(sourceIndex.row());
                m_selectionBackup.push_back({ attributeName, sourceIndex.column() });

                GRID_LOG(qDebug() << "Backup selection: " << QString::fromStdString(attributeName) << "," << sourceIndex.column());
            }
        }

        void EntityAttributeGrid::restoreSelection() {
            m_table->selectionModel()->clearSelection();

            GRID_LOG(qDebug() << "Restore selection");
            for (const auto& selection : m_selectionBackup) {
                const int row = m_model->rowForAttributeName(selection.attributeName);
                if (row == -1) {
                    GRID_LOG(qDebug() << "Restore selection: couldn't find " << QString::fromStdString(selection.attributeName));
                    continue;
                }
                const QModelIndex sourceIndex = m_model->index(row, selection.column);
                const QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);
                m_table->selectionModel()->select(proxyIndex, QItemSelectionModel::Select);
                m_table->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::Current);

                GRID_LOG(qDebug() << "Restore selection: " << QString::fromStdString(selection.attributeName) << "," << selection.column);
            }
            GRID_LOG(qDebug() << "Restore selection: current is " << QString::fromStdString(selectedRowName()));
        }

        void EntityAttributeGrid::addAttribute() {
            auto document = kdl::mem_lock(m_document);
            const std::string newAttributeName = AttributeRow::newAttributeNameForAttributableNodes(
                document->allSelectedEntityNodes());

            document->setProperty(newAttributeName, "");

            // Force an immediate update to the table rows (by default, updates are delayed - see EntityAttributeGrid::updateControls),
            // so we can select the new row.
            m_model->updateFromMapDocument();

            const int row = m_model->rowForAttributeName(newAttributeName);
            ensure(row != -1, "row should have been inserted");

            // Select the newly inserted attribute name
            const QModelIndex mi = m_proxyModel->mapFromSource(m_model->index(row, 0));

            m_table->clearSelection();
            m_table->setCurrentIndex(mi);
            m_table->setFocus();
        }

        void EntityAttributeGrid::removeSelectedAttributes() {
            if (!canRemoveSelectedAttributes()) {
                return;
            }

            const auto selectedRows = selectedRowsAndCursorRow();

            std::vector<std::string> attributes;
            for (const int row : selectedRows) {
                attributes.push_back(m_model->attributeName(row));
            }

            const size_t numRows = attributes.size();
            auto document = kdl::mem_lock(m_document);

            {
                Transaction transaction(document, kdl::str_plural(numRows, "Remove Attribute", "Remove Attributes"));

                bool success = true;
                for (const std::string& attribute : attributes) {
                    success = success && document->removeProperty(attribute);
                }

                if (!success) {
                    transaction.rollback();
                }
            }
        }

        bool EntityAttributeGrid::canRemoveSelectedAttributes() const {
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
        std::vector<int> EntityAttributeGrid::selectedRowsAndCursorRow() const {
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
            explicit EntitySortFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}

        protected:
            bool lessThan(const QModelIndex& left, const QModelIndex& right) const {
                const EntityAttributeModel& source = dynamic_cast<const EntityAttributeModel&>(*sourceModel());

                return source.lessThan(static_cast<size_t>(left.row()), static_cast<size_t>(right.row()));
            }
        };

        void EntityAttributeGrid::createGui(std::weak_ptr<MapDocument> document) {
            m_table = new EntityAttributeTable();

            m_model = new EntityAttributeModel(document, this);
            m_model->setParent(m_table); // ensure the table takes ownership of the model in setModel // FIXME: why? this looks unnecessary

            m_proxyModel = new EntitySortFilterProxyModel(this);
            m_proxyModel->setSourceModel(m_model);
            m_proxyModel->sort(0);
            m_table->setModel(m_proxyModel);

            m_table->setItemDelegate(new EntityAttributeItemDelegate(m_table, m_model, m_proxyModel, m_table));

            autoResizeRows(m_table);

            m_table->verticalHeader()->setVisible(false);
            m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            m_table->horizontalHeader()->setSectionsClickable(false);
            m_table->setSelectionBehavior(QAbstractItemView::SelectItems);

            m_addAttributeButton = createBitmapButton("Add.svg", tr("Add a new property (%1)").arg(EntityAttributeTable::insertRowShortcutString()), this);
            connect(m_addAttributeButton, &QAbstractButton::clicked, this, [=](const bool /* checked */){
                addAttribute();
            });

            m_removePropertiesButton = createBitmapButton("Remove.svg", tr("Remove the selected properties (%1)").arg(EntityAttributeTable::removeRowShortcutString()), this);
            connect(m_removePropertiesButton, &QAbstractButton::clicked, this, [=](const bool /* checked */){
                removeSelectedAttributes();
            });

            m_showDefaultPropertiesCheckBox = new QCheckBox(tr("Show default properties"));
            connect(m_showDefaultPropertiesCheckBox, &QCheckBox::stateChanged, this, [=](const int state){
                m_model->setShowDefaultRows(state == Qt::Checked);
            });
            m_showDefaultPropertiesCheckBox->setChecked(m_model->showDefaultRows());

            connect(m_table, &EntityAttributeTable::addRowShortcutTriggered, this, [=](){
                addAttribute();
            });
            connect(m_table, &EntityAttributeTable::removeRowsShortcutTriggered, this, [=](){
                removeSelectedAttributes();
            });

            connect(m_table->selectionModel(), &QItemSelectionModel::currentChanged, this, [=](const QModelIndex& current, const QModelIndex& previous){
                unused(current);
                unused(previous);
                // NOTE: when we get this signal, the selection hasn't been updated yet.
                // So selectedRowsAndCursorRow() will return a mix of the new current row and old selection.
                // Because of this, it's important to also call updateControlsEnabled() in response to QItemSelectionModel::selectionChanged
                // as we do below. (#3165)
                GRID_LOG(qDebug() << "current changed form " << previous << " to " << current);
                updateControlsEnabled();
                ensureSelectionVisible();
                emit currentRowChanged();
            });

            connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=](){
                if (!m_table->selectionModel()->selectedIndexes().empty()) {
                    backupSelection();
                }
                updateControlsEnabled();
                emit currentRowChanged();
            });

            // e.g. handles setting a value of a default attribute so it becomes non-default
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
                m_addAttributeButton,
                m_removePropertiesButton,
                LayoutConstants::WideHMargin,
                m_showDefaultPropertiesCheckBox);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(m_table, 1);
            layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 0);
            layout->addLayout(toolBar, 0);
            setLayout(layout);

            // NOTE: Do not use QAbstractItemView::SelectedClicked.
            // EntityAttributeTable::mousePressEvent() implements its own version.
            // See: https://github.com/TrenchBroom/TrenchBroom/issues/3582
            m_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
        }

        void EntityAttributeGrid::bindObservers() {
            auto document = kdl::mem_lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &EntityAttributeGrid::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &EntityAttributeGrid::documentWasLoaded);
            document->nodesDidChangeNotifier.addObserver(this, &EntityAttributeGrid::nodesDidChange);
            document->selectionWillChangeNotifier.addObserver(this, &EntityAttributeGrid::selectionWillChange);
            document->selectionDidChangeNotifier.addObserver(this, &EntityAttributeGrid::selectionDidChange);
        }

        void EntityAttributeGrid::unbindObservers() {
            if (!kdl::mem_expired(m_document)) {
                auto document = kdl::mem_lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &EntityAttributeGrid::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &EntityAttributeGrid::documentWasLoaded);
                document->nodesDidChangeNotifier.removeObserver(this, &EntityAttributeGrid::nodesDidChange);
                document->selectionWillChangeNotifier.removeObserver(this, &EntityAttributeGrid::selectionWillChange);
                document->selectionDidChangeNotifier.removeObserver(this, &EntityAttributeGrid::selectionDidChange);
            }
        }

        void EntityAttributeGrid::documentWasNewed(MapDocument*) {
            updateControls();
        }

        void EntityAttributeGrid::documentWasLoaded(MapDocument*) {
            updateControls();
        }

        void EntityAttributeGrid::nodesDidChange(const std::vector<Model::Node*>&) {
            updateControls();
        }

        void EntityAttributeGrid::selectionWillChange() {
        }

        void EntityAttributeGrid::selectionDidChange(const Selection&) {
            updateControls();
        }

        void EntityAttributeGrid::updateControls() {
            // When you change the selected entity in the map, there's a brief intermediate state where worldspawn
            // is selected. If we call this directly, it'll cause the table to be rebuilt based on that intermediate
            // state. Everything is fine except you lose the selected row in the table, unless it's a key
            // name that exists in worldspawn. To avoid that problem, make a delayed call to update the table.
            QTimer::singleShot(0, this, [&](){
                m_model->updateFromMapDocument();

                if (m_table->selectionModel()->selectedIndexes().empty()) {
                    restoreSelection();
                }
                ensureSelectionVisible();
            });
            updateControlsEnabled();
        }

        void EntityAttributeGrid::ensureSelectionVisible() {
            m_table->scrollTo(m_table->currentIndex());
        }

        void EntityAttributeGrid::updateControlsEnabled() {
            auto document = kdl::mem_lock(m_document);
            const auto nodes = document->allSelectedEntityNodes();
            m_table->setEnabled(!nodes.empty());
            m_addAttributeButton->setEnabled(!nodes.empty());
            m_removePropertiesButton->setEnabled(!nodes.empty() && canRemoveSelectedAttributes());
            m_showDefaultPropertiesCheckBox->setChecked(m_model->showDefaultRows());
        }

        std::string EntityAttributeGrid::selectedRowName() const {
            QModelIndex current = m_proxyModel->mapToSource(m_table->currentIndex());
            const AttributeRow* rowModel = m_model->dataForModelIndex(current);
            if (rowModel == nullptr) {
                return "";
            }

            return rowModel->name();
        }
    }
}
