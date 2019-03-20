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

#include "BorderLine.h"

#include "View/ViewConstants.h"

#include <QPainter>

namespace TrenchBroom {
    namespace View {
        BorderLine::BorderLine(QWidget* parent, Direction direction, const int thickness) :
        QWidget(parent),
        m_direction(direction),
        m_thickness(thickness) {
            if (direction == Direction_Horizontal) {
                setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
            } else {
                setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored));
            }
        }

        QSize BorderLine::sizeHint() const {
            return QSize(m_thickness, m_thickness);
        }

        void BorderLine::paintEvent(QPaintEvent* event) {
            QPainter painter(this);
            painter.fillRect(this->rect(), Colors::borderColor());
        }
    }
}
