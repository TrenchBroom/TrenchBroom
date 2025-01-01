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

#include "KeyboardShortcutItemDelegate.h"

#include <QItemEditorFactory>

#include "ui/KeySequenceEdit.h"

namespace tb::ui
{

KeyboardShortcutItemDelegate::KeyboardShortcutItemDelegate()
{
  auto* itemEditorFactory = new QItemEditorFactory{};
  itemEditorFactory->registerEditor(
    QMetaType::Type::QKeySequence, new QStandardItemEditorCreator<KeySequenceEdit>{});
  setItemEditorFactory(itemEditorFactory);
}

QWidget* KeyboardShortcutItemDelegate::createEditor(
  QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  auto* widget = QStyledItemDelegate::createEditor(parent, option, index);
  if (auto* editor = dynamic_cast<KeySequenceEdit*>(widget))
  {
    connect(
      editor,
      &KeySequenceEdit::editingFinished,
      this,
      &KeyboardShortcutItemDelegate::commitAndCloseEditor);
  }
  return widget;
}

void KeyboardShortcutItemDelegate::commitAndCloseEditor()
{
  if (auto* editor = dynamic_cast<KeySequenceEdit*>(sender()))
  {
    emit commitData(editor);
    emit closeEditor(editor);
  }
}

} // namespace tb::ui
