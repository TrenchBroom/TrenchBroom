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

#include "TitleBar.h"

#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QLabel>
#include <QHBoxLayout>


namespace TrenchBroom {
    namespace View {
        TitleBar::TitleBar(const QString& title, QWidget* parent, int hMargin, int vMargin, bool boldTitle) :
        QWidget(parent),
        m_titleText(nullptr) {
            // FIXME: Should always force this color, but doesn't work in ControlListBox
            setDefaultWindowColor(this);

            m_titleText = new QLabel(title);

            if (boldTitle) {
                makeEmphasized(m_titleText);
            }

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(hMargin, vMargin, hMargin, vMargin);
            layout->addWidget(m_titleText);
            setLayout(layout);
        }
    }
}
