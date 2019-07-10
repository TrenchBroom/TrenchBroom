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
#include "Model/Object.h"
#include "View/BorderLine.h"
#include "View/EntityAttributeGridTable.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"
#include "View/wxUtils.h"

#include <QHeaderView>
#include <QTableView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QAbstractButton>
#include <QShortcut>
#include <QKeySequence>
#include <QDebug>
#include <QTextEdit>
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
            m_table->updateFromMapDocument();

            const int row = m_table->rowForAttributeName(newAttributeName);
            ensure(row != -1, "row should have been inserted");

            // Select the newly inserted attribute name
            QModelIndex mi = m_table->index(row, 0);
            m_grid->setCurrentIndex(mi);
            m_grid->setFocus();
        }

        void EntityAttributeGrid::removeSelectedAttributes() {
            qDebug("FIXME: removeSelectedAttributes");

            QItemSelectionModel *s = m_grid->selectionModel();
            if (!s->hasSelection()) {
                return;
            }
            // FIXME: support more than 1 row
            // FIXME: current vs selected
            QModelIndex current = s->currentIndex();
            if (!current.isValid()) {
                return;
            }

            const AttributeRow* temp = m_table->dataForModelIndex(current);
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
//                attributes.push_back(m_table->attributeName(row));
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




//            const int row = m_table->rowForName(key);
//            if (row == -1)
//                return;
//
//            m_grid->DeleteRows(row, 1);
//            m_grid->ClearSelection();
//
//            const int newRow = m_table->rowForName(key);
//            if (newRow != -1) {
//                m_grid->SetGridCursor(newRow, m_grid->GetGridCursorCol());
//            }
        }

        bool EntityAttributeGrid::canRemoveSelectedAttributes() const {
            return true;
            /* FIXME:
            const auto rows = selectedRowsAndCursorRow();
            if (rows.empty())
                return false;

            for (const int row : rows) {
                if (!m_table->canRemove(row))
                    return false;
            }
            return true;
             */
        }

        std::set<int> EntityAttributeGrid::selectedRowsAndCursorRow() const {
            std::set<int> result;

            // FIXME:
//            if (m_grid->GetGridCursorCol() != -1
//                && m_grid->GetGridCursorRow() != -1) {
//                result.insert(m_grid->GetGridCursorRow());
//            }
//
//            for (const int row : m_grid->GetSelectedRows()) {
//                result.insert(row);
//            }
            return result;
        }

#if 0
        /**
         * Subclass of wxGridCellTextEditor for setting up autocompletion
         */
        class EntityAttributeCellEditor : public wxGridCellTextEditor
        {
        private:
            EntityAttributeGrid* m_grid;
            EntityAttributeGridTable* m_table;
            int m_row, m_col;
            bool m_forceChange;
            String m_forceChangeAttribute;

        public:
            EntityAttributeCellEditor(EntityAttributeGrid* grid, EntityAttributeGridTable* table)
            : m_grid(grid),
            m_table(table),
            m_row(-1),
            m_col(-1),
            m_forceChange(false),
            m_forceChangeAttribute("") {}

        private:
            void OnCharHook(wxKeyEvent& event) {
                if (event.GetKeyCode() == WXK_TAB) {
                    // HACK: Consume tab key and use it for cell navigation.
                    // Otherwise, wxTextCtrl::AutoComplete uses it for cycling between completions (on Windows)

                    // First, close the cell editor
                    m_grid->gridWindow()->DisableCellEditControl();

                    // Closing the editor might reorder the cells (#2094), so m_row/m_col are no longer valid.
                    // Ask the wxGrid for the cursor row/column.
                    m_grid->tabNavigate(m_grid->gridWindow()->GetGridCursorRow(), m_grid->gridWindow()->GetGridCursorCol(), !event.ShiftDown());
                } else if (event.GetKeyCode() == WXK_RETURN && m_col == 1) {
                    // HACK: (#1976) Make the next call to EndEdit return true unconditionally
                    // so it's possible to press enter to apply a value to all entites in a selection
                    // even though the grid editor hasn't changed.

                    const TemporarilySetBool forceChange{m_forceChange};
                    const TemporarilySetAny<String> forceChangeAttribute{m_forceChangeAttribute, m_table->attributeName(m_row)};

                    m_grid->gridWindow()->SaveEditControlValue();
                    m_grid->gridWindow()->HideCellEditControl();
                } else {
                    event.Skip();
                }
            }

        public:
            void BeginEdit(int row, int col, wxGrid* grid) override {
                wxGridCellTextEditor::BeginEdit(row, col, grid);
                assert(grid == m_grid->gridWindow());

                m_row = row;
                m_col = col;

                wxTextCtrl *textCtrl = Text();
                ensure(textCtrl != nullptr, "wxGridCellTextEditor::Create should have created control");

                const QStringList completions = m_table->getCompletions(row, col);
                textCtrl->AutoComplete(completions);

                textCtrl->Bind(wxEVT_CHAR_HOOK, &EntityAttributeCellEditor::OnCharHook, this);
            }

            bool EndEdit(int row, int col, const wxGrid* grid, const QString& oldval, QString *newval) override {
                assert(grid == m_grid->gridWindow());

                wxTextCtrl *textCtrl = Text();
                ensure(textCtrl != nullptr, "wxGridCellTextEditor::Create should have created control");

                textCtrl->Unbind(wxEVT_CHAR_HOOK, &EntityAttributeCellEditor::OnCharHook, this);

                const bool superclassDidChange = wxGridCellTextEditor::EndEdit(row, col, grid, oldval, newval);

                const String changedAttribute = m_table->attributeName(row);

                if (m_forceChange
                    && col == 1
                    && m_forceChangeAttribute == changedAttribute) {
                    return true;
                } else {
                    return superclassDidChange;
                }
            }

            void ApplyEdit(int row, int col, wxGrid* grid) override {
                if (col == 0) {
                    // Hack to preserve selection when renaming a key (#2094)
                    const auto newName = GetValue().ToStdString();
                    m_grid->setLastSelectedNameAndColumn(newName, col);
                }
                wxGridCellTextEditor::ApplyEdit(row, col, grid);
            }
        };
#endif

        class MyTable : public QTableView {
        protected:
            bool event(QEvent *event) override {
                if (event->type() == QEvent::ShortcutOverride) {
                    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

                    if (keyEvent->key() < Qt::Key_Escape &&
                        (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::KeypadModifier)) {
                        qDebug("overriding shortcut key %d\n", keyEvent->key());
                        event->setAccepted(true);
                        return true;
                    } else {
                        qDebug("not overriding shortcut key %d\n", keyEvent->key());
                    }

                }
                return QTableView::event(event);
            }

            void keyPressEvent(QKeyEvent* event) override {
                // Set up Qt::Key_Return to open the editor. Doing this binding via a QShortcut makes it so you can't close
                // an open editor, so do it this way.
                if (event->key() == Qt::Key_Return
                    && (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::KeypadModifier)
                    && state() != QAbstractItemView::EditingState) {

                    // open the editor
                    qDebug("opening editor...");
                    edit(currentIndex());
                } else {
                    QTableView::keyPressEvent(event);
                }
            }

            /**
             * The decorations (padlock icon for locked cells) goes on the right of the text
             */
            QStyleOptionViewItem viewOptions() const override {
                QStyleOptionViewItem options = QTableView::viewOptions();
                options.decorationPosition = QStyleOptionViewItem::Right;
                return options;
            }
        };

        void EntityAttributeGrid::createGui(MapDocumentWPtr document) {
            m_table = new EntityAttributeGridTable(document, this);

            m_grid = new MyTable();
            m_grid->setStyleSheet("QTableView { border: none; }");
            m_grid->setModel(m_table);
            m_grid->verticalHeader()->setVisible(false);
            m_grid->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            m_grid->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            m_grid->horizontalHeader()->setSectionsClickable(false);
            m_grid->setSelectionBehavior(QAbstractItemView::SelectItems);



//            m_grid->Bind(wxEVT_GRID_SELECT_CELL, &EntityAttributeGrid::OnAttributeGridSelectCell, this);

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
                //m_table->setShowDefaultRows(state == Qt::Checked);
            });

            connect(m_grid->selectionModel(), &QItemSelectionModel::currentChanged, this, [=](const QModelIndex& current, const QModelIndex& previous){
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
            layout->addWidget(m_grid, 1);
            layout->addWidget(new BorderLine(BorderLine::Direction_Horizontal), 0);
            layout->addLayout(toolBar, 0);
            setLayout(layout);

            printf("et: %d\n", m_grid->editTriggers());

            //m_grid->setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::AnyKeyPressed);
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

//            m_openCellEditorShortcut = new QShortcut(QKeySequence(Qt::Key_Return), m_grid);// "Enter"), this);
//            //m_openCellEditorShortcut->setContext(Qt::WidgetWithChildrenShortcut);
//            connect(m_openCellEditorShortcut, &QShortcut::activated, this, [=](){
//                bool open = m_grid->isPersistentEditorOpen(m_grid->currentIndex());
//
//
//                qDebug("enter activated unambiguously, open? %d", (int)open);
//                if (!open) {
//                    m_grid->edit(m_grid->currentIndex());
//                }
//            });

//            connect(m_openCellEditorShortcut, &QShortcut::activatedAmbiguously, this, [=](){
//                qDebug("enter activated ambiguously");
//                m_grid->edit(m_grid->currentIndex());
//            });
       }

       void EntityAttributeGrid::updateShortcuts() {
           m_insertRowShortcut->setEnabled(true);
           m_removeRowShortcut->setEnabled(canRemoveSelectedAttributes());
           m_removeRowAlternateShortcut->setEnabled(canRemoveSelectedAttributes());
           // FIXME:
           //m_openCellEditorShortcut->setEnabled(m_grid->CanEnableCellControl() && !m_grid->IsCellEditControlShown());
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
            // FIXME: Needed?
//            m_grid->SaveEditControlValue();
//            m_grid->HideCellEditControl();
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
            QMetaObject::invokeMethod(m_table, "updateFromMapDocument", Qt::QueuedConnection);

            // Update buttons/checkboxes
            MapDocumentSPtr document = lock(m_document);
            m_grid->setEnabled(!document->allSelectedAttributableNodes().empty());
            m_addAttributeButton->setEnabled(!document->allSelectedAttributableNodes().empty());
            m_removePropertiesButton->setEnabled(canRemoveSelectedAttributes());
            //m_showDefaultPropertiesCheckBox->setChecked(m_table->showDefaultRows());

            // Update shortcuts
            updateShortcuts();
        }

        Model::AttributeName EntityAttributeGrid::selectedRowName() const {
            QModelIndex current = m_grid->currentIndex();
            const AttributeRow* rowModel = m_table->dataForModelIndex(current);
            if (rowModel == nullptr) {
                return "";
            }

            return rowModel->name();
        }
    }
}
