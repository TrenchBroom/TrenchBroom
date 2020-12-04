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

#pragma once

#include <QWidget>

class QLabel;
class QSlider;

namespace TrenchBroom {
    namespace View {
        class SliderWithLabel : public QWidget {
            Q_OBJECT
        private:
            QSlider* m_slider;
            QLabel* m_label;
        public:
            SliderWithLabel(int minimum, int maximum, QWidget* parent = nullptr);

            int value() const;
            float ratio() const;
        public slots:
            void setValue(int value);
            void setRatio(float ratio);
        private slots:
            void valueChangedInternal(int value);
        signals:
            void valueChanged(int value);
        };
    }
}



