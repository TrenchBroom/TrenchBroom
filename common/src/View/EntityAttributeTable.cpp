/*
 Copyright (C) 2010-2019 Kristian Duske

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

#include "EntityAttributeTable.h"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>

namespace TrenchBroom {
    namespace View {
        void EntityAttributeTable::finishEditing(QWidget* editor) {
            qDebug() << "finish editing";
            commitData(editor);
            closeEditor(editor, QAbstractItemDelegate::EditNextItem);
        }

        /**
         * Just for generating tooltips, keep in sync with isInsertRowShortcut
         */
        QString EntityAttributeTable::insertRowShortcutString() {
            return QKeySequence(Qt::Key_Return | Qt::CTRL).toString(QKeySequence::NativeText);
        }

        /**
         * Just for generating tooltips, keep in sync with isRemoveRowsShortcut
         */
        QString EntityAttributeTable::removeRowShortcutString() {
            return QObject::tr("%1 or %2")
                .arg(QKeySequence(Qt::Key_Delete).toString(QKeySequence::NativeText))
                .arg(QKeySequence(Qt::Key_Backspace).toString(QKeySequence::NativeText));
        }

        static bool isInsertRowShortcut(QKeyEvent* event) {
            return event->key() == Qt::Key_Return && event->modifiers() == Qt::CTRL;
        }

        static bool isRemoveRowsShortcut(QKeyEvent* event) {
            return (event->key() == Qt::Key_Delete && event->modifiers() == 0)
                || (event->key() == Qt::Key_Backspace && event->modifiers() == 0);
        }

        bool EntityAttributeTable::event(QEvent *event) {
            if (event->type() == QEvent::ShortcutOverride) {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

                // Accepting a QEvent::ShortcutOverride suppresses QShortcut/QAction from being triggered
                // and causes a normal key press to be delivered to the focused widget.

                // This is necessary so e.g. pressing U (UV lock menu item) types a U character into the
                // current row, rather than activating the UV lock menu shortcut.
                if (keyEvent->key() < Qt::Key_Escape &&
                    (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::KeypadModifier)) {
                    event->setAccepted(true);
                    return true;
                }

                // These insert/remove row shortcut are handled here so they take precedence
                // over the Delete menu action for deleting brushes.
                if (isInsertRowShortcut(keyEvent) || isRemoveRowsShortcut(keyEvent)) {
                    event->setAccepted(true);
                    return true;
                }

                qDebug("not overriding shortcut key %d\n", keyEvent->key());
            }
            return QTableView::event(event);
        }

        void EntityAttributeTable::keyPressEvent(QKeyEvent* event) {
            if (isInsertRowShortcut(event)) {
                emit addRowShortcutTriggered();
                return;
            }
            if (isRemoveRowsShortcut(event)) {
                emit removeRowsShortcutTriggered();
                return;
            }

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
        QStyleOptionViewItem EntityAttributeTable::viewOptions() const {
            QStyleOptionViewItem options = QTableView::viewOptions();
            options.decorationPosition = QStyleOptionViewItem::Right;
            // Qt high-dpi bug: if we don't specify the size explicitly Qt, sees the larger
            // pixmap in the QIcon and tries to draw the icon larger than its actual 12x12 size.
            options.decorationSize = QSize(12,12);
            return options;
        }
    }
}
