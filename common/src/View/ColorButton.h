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

#include <QWidget>

class QPushButton;
class QResizeEvent;

namespace TrenchBroom {
    namespace View {
        class ColorButton : public QWidget {
            Q_OBJECT
        private:
            QWidget* m_colorIndicator;
            QPushButton* m_button;
            QColor m_color;
        public:
            explicit ColorButton(QWidget* parent = nullptr);
        signals:
            /**
             * Emitted when the color is set either programatically via setColor() or
             * as a result of the user clicking on the color button.
             */
            void colorChanged(const QColor& color);
            /**
             * Emitted only as a result of the user clicking on the color button.
             */
            void colorChangedByUser(const QColor& color);
        public slots:
            /**
             * Change the current color displayed on the button. Causes colorChanged() to be emitted.
             */
            void setColor(const QColor& color);
        };
    }
}

#endif //TRENCHBROOM_COLORBUTTON_H
