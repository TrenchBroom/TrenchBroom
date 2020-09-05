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

#ifndef TRENCHBROOM_ENTITYATTRIBUTETABLE_H
#define TRENCHBROOM_ENTITYATTRIBUTETABLE_H

#include <QString>
#include <QTableView>

namespace TrenchBroom {
    namespace View {
        /**
         * Hardcoded shortcuts:
         * - Ctrl+Enter emits the `addRowShortcutTriggered` signal
         * - Delete or Backspace emits the `removeRowsShortcutTriggered` signal
         */
        class EntityAttributeTable : public QTableView {
            Q_OBJECT
        public:
            static QString insertRowShortcutString();
            static QString removeRowShortcutString();
            void finishEditing(QWidget* editor);
        protected:
            bool event(QEvent *event) override;
            void keyPressEvent(QKeyEvent* event) override;
            QStyleOptionViewItem viewOptions() const override;

        signals:
            void addRowShortcutTriggered();
            void removeRowsShortcutTriggered();
        };
    }
}


#endif //TRENCHBROOM_ENTITYATTRIBUTETABLE_H
