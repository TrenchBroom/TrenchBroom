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

#ifndef ControlListBox_h
#define ControlListBox_h

#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;

namespace TrenchBroom {
    namespace View {
        class ControlListBoxItemRenderer : public QWidget {
            Q_OBJECT
        public:
            explicit ControlListBoxItemRenderer(QWidget* parent = nullptr);
            ~ControlListBoxItemRenderer() override;

            virtual void setSelected(bool selected);
        };

        class ControlListBox : public QWidget {
            Q_OBJECT
        private:
            QListWidget* m_listWidget;
            QWidget* m_emptyTextContainer;
            QLabel* m_emptyTextLabel;
        public:
            explicit ControlListBox(const QString& emptyText, QWidget* parent = nullptr);
        protected:
            void setEmptyText(const QString& emptyText);
            void refresh();
        private:
            void clear();
            void addItemRenderer(ControlListBoxItemRenderer* renderer);

            void currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
        private:
            virtual size_t itemCount() const = 0;
            virtual ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) = 0;
        };
    }
}

#endif /* ControlListBox_h */
