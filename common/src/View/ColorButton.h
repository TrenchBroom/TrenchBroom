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

#ifndef TRENCHBROOM_COLORBUTTON_H
#define TRENCHBROOM_COLORBUTTON_H

#include <QPushButton>

namespace TrenchBroom::View {
    class ColorButton : public QPushButton {
        Q_OBJECT
    private:
        QColor m_color;
    public:
        explicit ColorButton(QWidget* parent = nullptr);

        void setColor(const QColor& color);
    protected:
        void paintEvent(QPaintEvent* e) override;
    signals:
        void colorChanged(const QColor& color);
    private slots:
        void clicked();
    };
}


#endif //TRENCHBROOM_COLORBUTTON_H
