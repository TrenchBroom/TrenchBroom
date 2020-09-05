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

#ifndef TRENCHBROOM_ENTITYATTRIBUTEITEMDELEGATE_H
#define TRENCHBROOM_ENTITYATTRIBUTEITEMDELEGATE_H

#include <QStyledItemDelegate>

class QLineEdit;
class QSortFilterProxyModel;

namespace TrenchBroom {
    namespace View {
        class EntityAttributeModel;
        class EntityAttributeTable;

        class EntityAttributeItemDelegate : public QStyledItemDelegate {
            Q_OBJECT
        private:
            EntityAttributeTable* m_table;
            const EntityAttributeModel* m_model;
            const QSortFilterProxyModel* m_proxyModel;
        public:
            EntityAttributeItemDelegate(EntityAttributeTable* table, const EntityAttributeModel* model, const QSortFilterProxyModel* proxyModel, QWidget* parent = nullptr);

            QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
            void setEditorData(QWidget* editor, const QModelIndex& index) const override;
        private:
            void setupCompletions(QLineEdit* lineEdit, const QModelIndex& index) const;
            QStringList getCompletions(const QModelIndex& index) const;
        };
    }
}

#endif //TRENCHBROOM_ENTITYATTRIBUTEITEMDELEGATE_H
