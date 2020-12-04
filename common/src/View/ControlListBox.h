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

#pragma once

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
            virtual void setSelected(bool selected, const QListWidget* listWidget);
        signals:
            void doubleClicked(size_t index);
        };

        /**
         * Wraps a renderer and adds a separator line at the bottom.
         */
        class ControlListBoxItemRendererWrapper : public QWidget {
            Q_OBJECT
        private:
            ControlListBoxItemRenderer* m_renderer;
        public:
            explicit ControlListBoxItemRendererWrapper(ControlListBoxItemRenderer* renderer, bool showSeparator, QWidget* parent = nullptr);

            ControlListBoxItemRenderer* renderer();
            const ControlListBoxItemRenderer* renderer() const;
        };

        class ControlListBox : public QWidget {
            Q_OBJECT
        public:
            static constexpr auto LabelColorShouldNotUpdateWhenSelected = "LabelColorShouldNotUpdateWhenSelected";
        private:
            QListWidget* m_listWidget;
            QWidget* m_emptyTextContainer;
            QLabel* m_emptyTextLabel;
            QMargins m_itemMargins;
            bool m_showSeparator;
        public:
            ControlListBox(const QString& emptyText, const QMargins& itemMargins, bool showSeparator, QWidget* parent = nullptr);
            ControlListBox(const QString& emptyText, bool showSeparator, QWidget* parent = nullptr);

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
            ControlListBoxItemRenderer* renderer(int i);
            ControlListBoxItemRendererWrapper* wrapper(int i) const;
            ControlListBoxItemRendererWrapper* wrapper(int i);
        private:
            void addItemRenderer(ControlListBoxItemRenderer* renderer);
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


