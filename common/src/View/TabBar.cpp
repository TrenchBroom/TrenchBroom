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

#include "TabBar.h"

#include "Macros.h"
#include "View/TabBook.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QHBoxLayout>
#include <QStackedLayout>

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        // TabBarButton

        TabBarButton::TabBarButton(const QString& label, QWidget* parent) :
        QLabel(label, parent),
        m_pressed(false) {
            makeEmphasized(this);
        }

        void TabBarButton::setPressed(const bool pressed) {
            m_pressed = pressed;
            updateLabel();
        }

        void TabBarButton::mousePressEvent(QMouseEvent *event) {
            emit clicked();
        }

        void TabBarButton::updateLabel() {
            QPalette pal;
            if (m_pressed) {
                pal.setColor(QPalette::WindowText, Colors::highlightText());
            } else {
                pal.setColor(QPalette::WindowText, Colors::defaultText());
            }
            setPalette(pal);
            repaint(); // on macOS, the label's don't repaint automatically for some reason
        }

        // TabBar

        TabBar::TabBar(TabBook* tabBook) :
        ContainerBar(BorderPanel::BottomSide, tabBook),
        m_tabBook(tabBook),
        m_barBook(new QStackedLayout()) {
            ensure(m_tabBook != nullptr, "tabBook is null");
            connect(m_tabBook, &TabBook::pageChanged, this, &TabBar::OnTabBookPageChanged);

            m_controlLayout = new QHBoxLayout();
            m_controlLayout->setContentsMargins(0, LayoutConstants::NarrowHMargin, 0, LayoutConstants::NarrowHMargin);
            m_controlLayout->addSpacing(LayoutConstants::TabBarBarLeftMargin);
            m_controlLayout->addStretch(1);
            m_controlLayout->addLayout(m_barBook, 0);
            assert(m_controlLayout->setAlignment(m_barBook, Qt::AlignVCenter));
            m_controlLayout->addSpacing(LayoutConstants::NarrowHMargin);

            setLayout(m_controlLayout);
        }

        void TabBar::addTab(TabBookPage* bookPage, const QString& title) {
            ensure(bookPage != nullptr, "bookPage is null");

            auto* button = new TabBarButton(title);
            connect(button, &TabBarButton::clicked, this, &TabBar::OnButtonClicked);
            button->setPressed(m_buttons.empty());
            m_buttons.push_back(button);

            const auto sizerIndex = static_cast<int>(2 * (m_buttons.size() - 1) + 1);
            m_controlLayout->insertWidget(sizerIndex, button, 0, Qt::AlignVCenter);
            m_controlLayout->insertSpacing(sizerIndex + 1, LayoutConstants::WideHMargin);

            QWidget* barPage = bookPage->createTabBarPage(nullptr);
            m_barBook->addWidget(barPage);
        }

        void TabBar::OnButtonClicked() {
            auto* button = dynamic_cast<QWidget*>(QObject::sender());
            const size_t index = findButtonIndex(button);
            ensure(index < m_buttons.size(), "index out of range");
            m_tabBook->switchToPage(static_cast<int>(index));
        }

        void TabBar::OnTabBookPageChanged(const int newIndex) {
            for (TabBarButton* button : m_buttons) {
                button->setPressed(false);
            }

            setButtonActive(newIndex);
            m_barBook->setCurrentIndex(newIndex);
        }

        size_t TabBar::findButtonIndex(QWidget* button) const {
            for (size_t i = 0; i < m_buttons.size(); ++i) {
                if (m_buttons[i] == button)
                    return i;
            }
            return m_buttons.size();
        }

        void TabBar::setButtonActive(const int index) {
            m_buttons.at(static_cast<size_t>(index))->setPressed(true);
        }
    }
}
