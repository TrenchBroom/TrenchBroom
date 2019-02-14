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

#ifndef TrenchBroom_TabBar
#define TrenchBroom_TabBar

#include "View/ContainerBar.h"

#include <vector>

#include <QLabel>

class QStackedLayout;
class QHBoxLayout;

namespace TrenchBroom {
    namespace View {
        class TabBook;
        class TabBookPage;

        class TabBarButton : public QLabel {
            Q_OBJECT
        private:
            bool m_pressed;
        public:
            explicit TabBarButton(QWidget* parent = nullptr, const QString& label = "");
            /**
             * Update the label color
             */
            void setPressed(bool pressed);
        protected:
            void mousePressEvent(QMouseEvent *event) override;

        signals:
            void clicked();

        private:
            void updateLabel();
        };
        
        class TabBar : public ContainerBar {
            Q_OBJECT
        private:
            using ButtonList = std::vector<TabBarButton*>;
            
            TabBook* m_tabBook;

            QStackedLayout* m_barBook;
            QHBoxLayout* m_controlSizer;
            ButtonList m_buttons;
        public:
            explicit TabBar(TabBook* tabBook);
            
            void addTab(TabBookPage* bookPage, const QString& title);
        private:
            void OnButtonClicked();
            void OnTabBookPageChanged(int newIndex);
        private:
            size_t findButtonIndex(QWidget* button) const;
            void setButtonActive(int index);
        };
    }
}

#endif /* defined(TrenchBroom_TabBar) */
