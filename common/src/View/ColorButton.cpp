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

#include "ColorButton.h"

#include <QColorDialog>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStylePainter>

namespace TrenchBroom::View {
    ColorButton::ColorButton(QWidget* parent) :
    QPushButton(parent) {
        connect(this, &QPushButton::clicked, this, &ColorButton::clicked);
    }

    void ColorButton::setColor(const QColor& color) {
        if (color != m_color) {
            m_color = color;
            update();
            emit colorChanged(m_color);
        }
    }

    void ColorButton::paintEvent(QPaintEvent* e) {
        QStylePainter painter(this);
        QStyleOptionButton option;
        option.initFrom(this);

        painter.drawControl(QStyle::CE_PushButtonBevel, option);
    }

    void ColorButton::clicked() {
        const auto color = QColorDialog::getColor(m_color, this);
        if (color.isValid()) {
            setColor(color);
        }
    }
}
