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

#ifndef TRENCHBROOM_KEYBOARDSHORTCUTMODEL_H
#define TRENCHBROOM_KEYBOARDSHORTCUTMODEL_H

#include <QAbstractTableModel>

#include <vector>

namespace TrenchBroom {
    namespace View {
        struct ActionInfo;

        class KeyboardShortcutModel : public QAbstractTableModel {
            Q_OBJECT
        public:
            int rowCount(const QModelIndex& parent) const override;
            int columnCount(const QModelIndex& parent) const override;

            QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
            QVariant data(const QModelIndex& index, int role) const override;
            bool setData(const QModelIndex& index, const QVariant& value, int role) override;

            Qt::ItemFlags flags(const QModelIndex& index) const override;
        private:
            const ActionInfo& action(int index) const;
            const std::vector<ActionInfo>& actions() const;
            bool checkIndex(const QModelIndex& index) const;
        };
    }
}

#endif //TRENCHBROOM_KEYBOARDSHORTCUTMODEL_H
