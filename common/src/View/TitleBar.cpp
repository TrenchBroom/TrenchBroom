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

#include <QLabel>
#include <QHBoxLayout>

#include "View/ViewConstants.h"

namespace TrenchBroom {
    namespace View {
        TitleBar::TitleBar(QWidget* parent, const QString& title, const int hMargin, const int vMargin, const bool boldTitle) :
        QWidget(parent),
        m_titleText(new QLabel(title)) {
            if (boldTitle) {
                QFont boldFont = m_titleText->font();
                boldFont.setBold(true);

                m_titleText->setFont(boldFont);
            }
            
            auto* sizer = new QHBoxLayout();
            sizer->addSpacing(hMargin);
            sizer->addWidget(m_titleText, 0);
            sizer->addStretch(1);
            sizer->addSpacing(hMargin);

            setLayout(sizer);
        }
    }
}
