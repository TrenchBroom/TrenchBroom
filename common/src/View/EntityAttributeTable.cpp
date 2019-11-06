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

#include <QEvent>
#include <QKeyEvent>

namespace TrenchBroom {
    namespace View {
        void EntityAttributeTable::finishEditing(QWidget* editor) {
            commitData(editor);
            closeEditor(editor, QAbstractItemDelegate::EditNextItem);
        }

        bool EntityAttributeTable::event(QEvent *event) {
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

        void EntityAttributeTable::keyPressEvent(QKeyEvent* event) {
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
            return options;
        }
    }
}
