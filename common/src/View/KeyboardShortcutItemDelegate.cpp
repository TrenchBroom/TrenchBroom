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

#include "KeyboardShortcutItemDelegate.h"

#include "View/KeyboardShortcutModel.h"
#include "View/KeySequenceEdit.h"

#include <QDebug>
#include <QEvent>
#include <QItemEditorFactory>

namespace TrenchBroom {
    namespace View {
        KeyboardShortcutItemDelegate::KeyboardShortcutItemDelegate() {
            auto* itemEditorFactory = new QItemEditorFactory();
            itemEditorFactory->registerEditor(QVariant::KeySequence, new QStandardItemEditorCreator<KeySequenceEdit>());
            setItemEditorFactory(itemEditorFactory);
        }

        bool KeyboardShortcutItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                                       const QStyleOptionViewItem& option, const QModelIndex& index) {
            qDebug() << event->type();
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        }

        void KeyboardShortcutItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
            qDebug() << "set data for " << index;
            QStyledItemDelegate::setEditorData(editor, index);
            editor->setFocus();
        }
    }
}
