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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SpinControl.h"

#include <QDebug>
#include <QGuiApplication>

namespace TrenchBroom {
    namespace View {
        void SpinControl::stepBy(const int steps) {
            int sign = (steps > 0) ? 1 : -1;

            int newsteps = steps;
            if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) {
                newsteps = sign * 2;
            } else if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier) {
                newsteps = sign * 4;
            } else {
                newsteps = sign * 1;
            }

            qDebug("requested %d, doing %d", steps, newsteps);

            QDoubleSpinBox::stepBy(newsteps);
        }
    }
}
