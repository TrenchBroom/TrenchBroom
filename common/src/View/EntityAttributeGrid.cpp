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
#include "View/EntityAttributeModel.h"
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

#if 0
        /**
         * Subclass of wxGridCellTextEditor for setting up autocompletion
         */
        class EntityAttributeCellEditor : public wxGridCellTextEditor
        {
        private:
            EntityAttributeGrid* m_table;
            EntityAttributeModel* m_model;
            int m_row, m_col;
            bool m_forceChange;
            String m_forceChangeAttribute;

        public:
            EntityAttributeCellEditor(EntityAttributeGrid* grid, EntityAttributeModel* table)
            : m_table(grid),
            m_model(table),
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
                    m_table->gridWindow()->DisableCellEditControl();

                    // Closing the editor might reorder the cells (#2094), so m_row/m_col are no longer valid.
                    // Ask the wxGrid for the cursor row/column.
                    m_table->tabNavigate(m_table->gridWindow()->GetGridCursorRow(), m_table->gridWindow()->GetGridCursorCol(), !event.ShiftDown());
                } else if (event.GetKeyCode() == WXK_RETURN && m_col == 1) {
                    // HACK: (#1976) Make the next call to EndEdit return true unconditionally
                    // so it's possible to press enter to apply a value to all entites in a selection
                    // even though the grid editor hasn't changed.

                    const TemporarilySetBool forceChange{m_forceChange};
                    const TemporarilySetAny<String> forceChangeAttribute{m_forceChangeAttribute, m_model->attributeName(m_row)};

                    m_table->gridWindow()->SaveEditControlValue();
                    m_table->gridWindow()->HideCellEditControl();
                } else {
                    event.Skip();
                }
            }

        public:
            void BeginEdit(int row, int col, wxGrid* grid) override {
                wxGridCellTextEditor::BeginEdit(row, col, grid);
                assert(grid == m_table->gridWindow());

                m_row = row;
                m_col = col;

                wxTextCtrl *textCtrl = Text();
                ensure(textCtrl != nullptr, "wxGridCellTextEditor::Create should have created control");

                const QStringList completions = m_model->getCompletions(row, col);
                textCtrl->AutoComplete(completions);

                textCtrl->Bind(wxEVT_CHAR_HOOK, &EntityAttributeCellEditor::OnCharHook, this);
            }

            bool EndEdit(int row, int col, const wxGrid* grid, const QString& oldval, QString *newval) override {
                assert(grid == m_table->gridWindow());

                wxTextCtrl *textCtrl = Text();
                ensure(textCtrl != nullptr, "wxGridCellTextEditor::Create should have created control");

                textCtrl->Unbind(wxEVT_CHAR_HOOK, &EntityAttributeCellEditor::OnCharHook, this);

                const bool superclassDidChange = wxGridCellTextEditor::EndEdit(row, col, grid, oldval, newval);

                const String changedAttribute = m_model->attributeName(row);

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
                    m_table->setLastSelectedNameAndColumn(newName, col);
                }
                wxGridCellTextEditor::ApplyEdit(row, col, grid);
            }
        };
#endif

        class EntityAttributeTable : public QTableView {
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
            m_table = new EntityAttributeTable();
            m_model = new EntityAttributeModel(document, this);
            m_model->setParent(m_table); // ensure the table takes ownership of the model in setModel
            m_table->setModel(m_model);

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
                //m_model->setShowDefaultRows(state == Qt::Checked);
            });

            connect(m_table->selectionModel(), &QItemSelectionModel::currentChanged, this, [=](const QModelIndex& current, const QModelIndex& previous){
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

//            m_openCellEditorShortcut = new QShortcut(QKeySequence(Qt::Key_Return), m_table);// "Enter"), this);
//            //m_openCellEditorShortcut->setContext(Qt::WidgetWithChildrenShortcut);
//            connect(m_openCellEditorShortcut, &QShortcut::activated, this, [=](){
//                bool open = m_table->isPersistentEditorOpen(m_table->currentIndex());
//
//
//                qDebug("enter activated unambiguously, open? %d", (int)open);
//                if (!open) {
//                    m_table->edit(m_table->currentIndex());
//                }
//            });

//            connect(m_openCellEditorShortcut, &QShortcut::activatedAmbiguously, this, [=](){
//                qDebug("enter activated ambiguously");
//                m_table->edit(m_table->currentIndex());
//            });
       }

       void EntityAttributeGrid::updateShortcuts() {
           m_insertRowShortcut->setEnabled(true);
           m_removeRowShortcut->setEnabled(canRemoveSelectedAttributes());
           m_removeRowAlternateShortcut->setEnabled(canRemoveSelectedAttributes());
           // FIXME:
           //m_openCellEditorShortcut->setEnabled(m_table->CanEnableCellControl() && !m_table->IsCellEditControlShown());
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
//            m_table->SaveEditControlValue();
//            m_table->HideCellEditControl();
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
            m_table->setEnabled(!document->allSelectedAttributableNodes().empty());
            m_addAttributeButton->setEnabled(!document->allSelectedAttributableNodes().empty());
            m_removePropertiesButton->setEnabled(canRemoveSelectedAttributes());
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
