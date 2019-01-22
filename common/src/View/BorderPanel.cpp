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

#include "BorderPanel.h"

#include <QPainter>

#include "View/ViewConstants.h"

namespace TrenchBroom {
    namespace View {
        BorderPanel::BorderPanel() :
        QWidget(),
        m_borders(0),
        m_thickness(1) {}
        
        BorderPanel::BorderPanel(QWidget* parent, const Sides borders, const int thickness) :
        QWidget(parent) {
            Create(parent, borders, thickness);
        }
        
        BorderPanel::~BorderPanel() {}

        void BorderPanel::Create(QWidget* parent, Sides borders, int thickness) {
            m_borders = borders;
            m_thickness = thickness;
        }

        void BorderPanel::paintEvent(QPaintEvent* /*event*/) {
            QPainter painter(this);

            painter.fillRect(this->rect(), palette().color(backgroundRole()));
            painter.setPen(Colors::borderColor());

            QRect r = rect();
            if ((m_borders & LeftSide) != 0)
                painter.drawLine(r.left(), r.top(), r.left(), r.bottom());
            if ((m_borders & TopSide) != 0)
                painter.drawLine(r.left(), r.top(), r.right(), r.top());
            if ((m_borders & RightSide) != 0)
                painter.drawLine(r.right(), r.top(), r.right(), r.bottom());
            if ((m_borders & BottomSide) != 0)
                painter.drawLine(r.left(), r.bottom(), r.right(), r.bottom());
        }
    }
}
