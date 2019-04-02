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

namespace TrenchBroom {
    namespace View {
        class ControlListBox : public QWidget {
            Q_OBJECT
        protected:
            class ItemRenderer : public QWidget {
            public:
                explicit ItemRenderer(QWidget* parent = nullptr);

                ~ItemRenderer() override;

                virtual void setSelectionColors(const QColor& foreground, const QColor& background);
                virtual void setDefaultColors(const QColor& foreground, const QColor& background);
            protected:
                void setColors(QWidget* window, const QColor& foreground, const QColor& background);
            };
        private:
            QListWidget* m_listWidget;
            QWidget* m_emptyTextContainer;
            QLabel* m_emptyTextLabel;
        public:
            explicit ControlListBox(const QString& emptyText, QWidget* parent = nullptr);

            void setEmptyText(const QString& emptyText);

            void refresh();
        private:
            void clear();

            void addItemRenderer(ItemRenderer* renderer);
        private:
            virtual size_t itemCount() const = 0;
            virtual ItemRenderer* createItemRenderer(QWidget* parent, size_t index) = 0;
        };
    }
}

#endif /* ControlListBox_h */
