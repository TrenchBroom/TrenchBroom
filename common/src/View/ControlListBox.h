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

#include "View/ViewConstants.h"

#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;

namespace TrenchBroom {
    namespace View {
        class ControlListBoxItemRenderer : public QWidget {
            Q_OBJECT
        protected:
            size_t m_index;
        public:
            explicit ControlListBoxItemRenderer(QWidget* parent = nullptr);
            ~ControlListBoxItemRenderer() override;

            void setIndex(size_t index);
        protected:
            void mouseDoubleClickEvent(QMouseEvent* event) override;
        public:
            virtual void updateItem();
            virtual void setSelected(bool selected);
        signals:
            void doubleClicked(size_t index);
        };

        class ControlListBox : public QWidget {
            Q_OBJECT
        private:
            QListWidget* m_listWidget;
            QWidget* m_emptyTextContainer;
            QLabel* m_emptyTextLabel;
            QMargins m_itemMargins;
        public:
            ControlListBox(const QString& emptyText, const QMargins& itemMargins, QWidget* parent = nullptr);
            explicit ControlListBox(const QString& emptyText, QWidget* parent = nullptr);

            void setEmptyText(const QString& emptyText);
            void setItemMargins(const QMargins& itemMargins);

            int count() const;
            int currentRow() const;
            void setCurrentRow(int currentRow);
        protected:
            /**
             * Deletes all item renderers, re-fetches the item count from `itemCount()`, and
             * rebuilds the item renderers using `createItemRenderer()`.
             */
            void reload();

            /**
             * Calls updateItem() on each ControlListBoxItemRenderer in the list box.
             * 
             * You should call this when you know that the order and number of items hasn't changed, but
             * you want to update the details displayed in the item renderers (e.g. if the labels changed.)
             */
            void updateItems();

            const ControlListBoxItemRenderer* renderer(int i) const;
        private:
            void addItemRenderer(ControlListBoxItemRenderer* renderer);
            void setItemRenderer(QListWidgetItem* widgetItem, ControlListBoxItemRenderer* renderer);
        private:
            virtual size_t itemCount() const = 0;
            virtual ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) = 0;
            virtual void selectedRowChanged(int index);
            virtual void doubleClicked(size_t index);
        private slots:
            void listItemSelectionChanged();
        signals:
            void itemSelectionChanged();
        };
    }
}

#endif /* ControlListBox_h */
