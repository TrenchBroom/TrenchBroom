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

#include "EntityAttributeItemDelegate.h"

#include "View/EntityAttributeModel.h"
#include "View/EntityAttributeTable.h"

#include <string>
#include <vector>

#include <QCompleter>
#include <QDebug>
#include <QEvent>
#include <QLineEdit>
#include <QSortFilterProxyModel>

namespace TrenchBroom {
    namespace View {
        EntityAttributeItemDelegate::EntityAttributeItemDelegate(EntityAttributeTable* table, const EntityAttributeModel* model, const QSortFilterProxyModel* proxyModel, QWidget* parent) :
        QStyledItemDelegate(parent),
        m_table(table),
        m_model(model),
        m_proxyModel(proxyModel) {}

        QWidget* EntityAttributeItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
            auto* editor = QStyledItemDelegate::createEditor(parent, option, index);
            auto* lineEdit = dynamic_cast<QLineEdit*>(editor);
            if (lineEdit != nullptr) {
                setupCompletions(lineEdit, index);
            }
            return editor;
        }

        void EntityAttributeItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
            QStyledItemDelegate::setEditorData(editor, index);

            // show the completions immediately when the editor is opened if the editor's text is empty
            auto* lineEdit = dynamic_cast<QLineEdit*>(editor);
            if (lineEdit != nullptr) {
                const auto text = lineEdit->text();
                if (text.isEmpty()) {
                    auto* completer = lineEdit->completer();
                    if (completer != nullptr) {
                        completer->setCompletionPrefix(text);
                        completer->complete();
                    }
                }
            }
        }

        void EntityAttributeItemDelegate::setupCompletions(QLineEdit* lineEdit, const QModelIndex& index) const {
            auto* completer = new QCompleter(getCompletions(index), lineEdit);
            completer->setCaseSensitivity(Qt::CaseInsensitive);
            completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
            lineEdit->setCompleter(completer);

            connect(completer, QOverload<const QString&>::of(&QCompleter::activated), this, [this, lineEdit](const QString& /* value */) {
                m_table->finishEditing(lineEdit);
            });

            connect(lineEdit, &QLineEdit::returnPressed, this, [this, lineEdit, completer]() {
                if (completer->popup()->isVisible()) {
                    m_table->finishEditing(lineEdit);
                }
            });
        }

        QStringList EntityAttributeItemDelegate::getCompletions(const QModelIndex& index) const {
            auto completions = m_model->getCompletions(m_proxyModel->mapToSource(index));
            completions.sort(Qt::CaseInsensitive);
            return completions;
        }
    }
}
