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

#include "TitledPanel.h"

#include "View/BorderLine.h"
#include "View/TitleBar.h"
#include "View/ViewConstants.h"

#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        TitledPanel::TitledPanel(const QString& title, QWidget* parent, bool showDivider, bool boldTitle) :
        QWidget(parent),
        m_titleBar(nullptr),
        m_panel(nullptr) {
            m_titleBar = new TitleBar(title, LayoutConstants::NarrowHMargin, LayoutConstants::NarrowVMargin, boldTitle);
            m_panel = new QWidget();

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(m_titleBar);
            if (showDivider) {
                layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
            }
            layout->addWidget(m_panel, 1);
            setLayout(layout);
        }

        TitledPanel::TitledPanel(const QString& title, const bool showDivider, const bool boldTitle) :
        TitledPanel(title, nullptr, showDivider, boldTitle) {}

        TitleBar* TitledPanel::getTitleBar() const {
            return m_titleBar;
        }

        QWidget* TitledPanel::getPanel() const {
            return m_panel;
        }
    }
}
