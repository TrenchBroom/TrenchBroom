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

#include "EntityAttributeGrid.h"

#include "Model/EntityAttributes.h"
#include "View/BorderLine.h"
#include "View/EntityAttributeItemDelegate.h"
#include "View/EntityAttributeModel.h"
#include "View/EntityAttributeTable.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QHeaderView>
#include <QTableView>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QAbstractButton>
#include <QShortcut>
#include <QKeySequence>
#include <QDebug>
#include <QKeyEvent>

namespace TrenchBroom {
    namespace View {
        EntityAttributeGrid::EntityAttributeGrid(MapDocumentWPtr document, QWidget* parent) :
        QWidget(parent),
        m_document(document),
        m_ignoreSelection(false) {
            createGui(document);
            createShortcuts();
            updateShortcuts();
            bindObservers();
        }

        EntityAttributeGrid::~EntityAttributeGrid() {
            unbindObservers();
        }

        void EntityAttributeGrid::addAttribute() {
            MapDocumentSPtr document = lock(m_document);
            const String newAttributeName = AttributeRow::newAttributeNameForAttributableNodes(document->allSelectedAttributableNodes());

            document->setAttribute(newAttributeName, "");

            // Force an immediate update to the table rows (by default, updates are delayed - see EntityAttributeGrid::updateControls),
            // so we can select the new row.
            m_model->updateFromMapDocument();

            const int row = m_model->rowForAttributeName(newAttributeName);
            ensure(row != -1, "row should have been inserted");

            // Select the newly inserted attribute name
            QModelIndex mi = m_model->index(row, 0);
            m_table->setCurrentIndex(mi);
            m_table->setFocus();
        }

        void EntityAttributeGrid::removeSelectedAttributes() {
            qDebug("FIXME: removeSelectedAttributes");

            QItemSelectionModel *s = m_table->selectionModel();
            if (!s->hasSelection()) {
                return;
            }
            // FIXME: support more than 1 row
            // FIXME: current vs selected
            QModelIndex current = s->currentIndex();
            if (!current.isValid()) {
                return;
            }

            const AttributeRow* temp = m_model->dataForModelIndex(current);
            String name = temp->name();

            MapDocumentSPtr document = lock(m_document);

            // FIXME: transaction
            document->removeAttribute(name);


//            assert(canRemoveSelectedAttributes());
//
//            const auto selectedRows = selectedRowsAndCursorRow();
//
//            StringList attributes;
//            for (const int row : selectedRows) {
//                attributes.push_back(m_model->attributeName(row));
//            }
//
//            for (const String& key : attributes) {
//                removeAttribute(key);
//            }
        }

        /**
         * Removes an attribute, and clear the current selection.
         *
         * If this attribute is still in the table after removing, sets the grid cursor on the new row
         */
        void EntityAttributeGrid::removeAttribute(const String& key) {
            qDebug() << "removeAttribute " << QString::fromStdString(key);




//            const int row = m_model->rowForName(key);
//            if (row == -1)
//                return;
//
//            m_table->DeleteRows(row, 1);
//            m_table->ClearSelection();
//
//            const int newRow = m_model->rowForName(key);
//            if (newRow != -1) {
//                m_table->SetGridCursor(newRow, m_table->GetGridCursorCol());
//            }
        }

        bool EntityAttributeGrid::canRemoveSelectedAttributes() const {
            return true;
            /* FIXME:
            const auto rows = selectedRowsAndCursorRow();
            if (rows.empty())
                return false;

            for (const int row : rows) {
                if (!m_model->canRemove(row))
                    return false;
            }
            return true;
             */
        }

        std::set<int> EntityAttributeGrid::selectedRowsAndCursorRow() const {
            std::set<int> result;

            // FIXME:
//            if (m_table->GetGridCursorCol() != -1
//                && m_table->GetGridCursorRow() != -1) {
//                result.insert(m_table->GetGridCursorRow());
//            }
//
//            for (const int row : m_table->GetSelectedRows()) {
//                result.insert(row);
//            }
            return result;
        }
        void EntityAttributeGrid::createGui(MapDocumentWPtr document) {
            m_table = new EntityAttributeTable();
            m_model = new EntityAttributeModel(document, this);
            m_model->setParent(m_table); // ensure the table takes ownership of the model in setModel
            m_table->setModel(m_model);
            m_table->setItemDelegate(new EntityAttributeItemDelegate(m_table, m_model, m_table));

            connect(m_model, &EntityAttributeModel::currentItemChangeRequestedByModel, this, [this](const QModelIndex& index) {
                qDebug() << "setting current to " << index;
                QItemSelectionModel* selectionModel = this->m_table->selectionModel();
                selectionModel->clearSelection();
                selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
            });

            autoResizeRows(m_table);

            m_table->setStyleSheet("QTableView { border: none; }");
            m_table->verticalHeader()->setVisible(false);
            m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            m_table->horizontalHeader()->setSectionsClickable(false);
            m_table->setSelectionBehavior(QAbstractItemView::SelectItems);

//            m_table->Bind(wxEVT_GRID_SELECT_CELL, &EntityAttributeGrid::OnAttributeGridSelectCell, this);

            m_addAttributeButton = createBitmapButton("Add.png", tr("Add a new property"), this);
            connect(m_addAttributeButton, &QAbstractButton::clicked, this, [=](bool checked){
                addAttribute();
            });

            m_removePropertiesButton = createBitmapButton("Remove.png", tr("Remove the selected properties"), this);
            connect(m_removePropertiesButton, &QAbstractButton::clicked, this, [=](bool checked){
                removeSelectedAttributes();
            });

            m_showDefaultPropertiesCheckBox = new QCheckBox(tr("Show default properties"));
            connect(m_showDefaultPropertiesCheckBox, &QCheckBox::stateChanged, this, [=](int state){
                m_model->setShowDefaultRows(state == Qt::Checked);
            });
            m_showDefaultPropertiesCheckBox->setChecked(m_model->showDefaultRows());

            connect(m_table->selectionModel(), &QItemSelectionModel::currentChanged, this, [=](const QModelIndex& current, const QModelIndex& previous){
                qDebug() << "current changed form " << previous << " to " << current;
                emit selectedRow();
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
            layout->addWidget(new BorderLine(BorderLine::Direction_Horizontal), 0);
            layout->addLayout(toolBar, 0);
            setLayout(layout);

            // FIXME: warning in MSVC
            printf("et: %d\n", m_table->editTriggers());

            //m_table->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::AnyKeyPressed);
        }

        void EntityAttributeGrid::createShortcuts() {
            m_insertRowShortcut = new QShortcut(QKeySequence("Ctrl-Return"), this);
            m_insertRowShortcut->setContext(Qt::WidgetWithChildrenShortcut);
            connect(m_insertRowShortcut, &QShortcut::activated, this, [=](){
                addAttribute();
            });

            m_removeRowShortcut = new QShortcut(QKeySequence("Delete"), this);
            m_removeRowShortcut->setContext(Qt::WidgetWithChildrenShortcut);
            connect(m_removeRowShortcut, &QShortcut::activated, this, [=](){
                removeSelectedAttributes();
            });

            m_removeRowAlternateShortcut = new QShortcut(QKeySequence("Backspace"), this);
            m_removeRowAlternateShortcut->setContext(Qt::WidgetWithChildrenShortcut);
            connect(m_removeRowAlternateShortcut, &QShortcut::activated, this, [=](){
                removeSelectedAttributes();
            });
       }

       void EntityAttributeGrid::updateShortcuts() {
           m_insertRowShortcut->setEnabled(true);
           m_removeRowShortcut->setEnabled(canRemoveSelectedAttributes());
           m_removeRowAlternateShortcut->setEnabled(canRemoveSelectedAttributes());
        }

        void EntityAttributeGrid::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &EntityAttributeGrid::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &EntityAttributeGrid::documentWasLoaded);
            document->nodesDidChangeNotifier.addObserver(this, &EntityAttributeGrid::nodesDidChange);
            document->selectionWillChangeNotifier.addObserver(this, &EntityAttributeGrid::selectionWillChange);
            document->selectionDidChangeNotifier.addObserver(this, &EntityAttributeGrid::selectionDidChange);
        }

        void EntityAttributeGrid::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &EntityAttributeGrid::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &EntityAttributeGrid::documentWasLoaded);
                document->nodesDidChangeNotifier.removeObserver(this, &EntityAttributeGrid::nodesDidChange);
                document->selectionWillChangeNotifier.removeObserver(this, &EntityAttributeGrid::selectionWillChange);
                document->selectionDidChangeNotifier.removeObserver(this, &EntityAttributeGrid::selectionDidChange);
            }
        }

        void EntityAttributeGrid::documentWasNewed(MapDocument* document) {
            updateControls();
        }

        void EntityAttributeGrid::documentWasLoaded(MapDocument* document) {
            updateControls();
        }

        void EntityAttributeGrid::nodesDidChange(const Model::NodeList& nodes) {
            updateControls();
        }

        void EntityAttributeGrid::selectionWillChange() {
        }

        void EntityAttributeGrid::selectionDidChange(const Selection& selection) {
            const TemporarilySetBool ignoreSelection(m_ignoreSelection);
            updateControls();
        }

        void EntityAttributeGrid::updateControls() {
            // When you change the selected entity in the map, there's a brief intermediate state where worldspawn
            // is selected. If we call this directly, it'll cause the table to be rebuilt based on that intermediate
            // state. Everything is fine except you lose the selected row in the table, unless it's a key
            // name that exists in worldspawn. To avoid that problem, make a delayed call to update the table.
            QMetaObject::invokeMethod(m_model, "updateFromMapDocument", Qt::QueuedConnection);

            // Update buttons/checkboxes
            MapDocumentSPtr document = lock(m_document);
            const auto nodes = document->allSelectedAttributableNodes();
            m_table->setEnabled(!nodes.empty());
            m_addAttributeButton->setEnabled(!nodes.empty());
            m_removePropertiesButton->setEnabled(!nodes.empty() && canRemoveSelectedAttributes());
            //m_showDefaultPropertiesCheckBox->setChecked(m_model->showDefaultRows());

            // Update shortcuts
            updateShortcuts();
        }

        Model::AttributeName EntityAttributeGrid::selectedRowName() const {
            QModelIndex current = m_table->currentIndex();
            const AttributeRow* rowModel = m_model->dataForModelIndex(current);
            if (rowModel == nullptr) {
                return "";
            }

            return rowModel->name();
        }
    }
}
