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

#include <QItemEditorFactory>
#include <QKeySequenceEdit>

namespace TrenchBroom {
    namespace View {
        KeyboardShortcutItemDelegate::KeyboardShortcutItemDelegate() {
            // FIXME: there is currently no way to delete a shortcut; we need to add a clear button to the editor
            // which means writing our own editor class that uses QKeySequenceEdit internally
            auto* itemEditorFactory = new QItemEditorFactory();
            itemEditorFactory->registerEditor(QVariant::KeySequence, new QStandardItemEditorCreator<QKeySequenceEdit>());
            setItemEditorFactory(itemEditorFactory);
        }

        void KeyboardShortcutItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
            const auto* model = dynamic_cast<const KeyboardShortcutModel*>(index.model());
            if (model != nullptr && model->hasConflicts(index)) {
                QStyleOptionViewItem itemOption(option);
                itemOption.palette.setColor(QPalette::Normal, QPalette::Text, Qt::red);
                QStyledItemDelegate::paint(painter, itemOption, index);
            } else {
                QStyledItemDelegate::paint(painter, option, index);
            }
        }
    }
}
