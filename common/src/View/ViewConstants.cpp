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

#include "ViewConstants.h"

#include <QColor>
#include <QPalette>
#include <QFont>

namespace TrenchBroom {
    namespace View {
        namespace Fonts {
            QFont fixedWidthFont() {
                QFont result("Courier", 10);
                result.setStyleHint(QFont::TypeWriter);
                return result;
            }
        }

        namespace Colors {
            QColor defaultText() {
                QPalette pal;
                QColor result = pal.color(QPalette::Normal, QPalette::WindowText);
                return result;
            }

            QColor highlightText() {
                // Used for selected tabs of TabBar control.
                QPalette pal;
                QColor result = pal.color(QPalette::Normal, QPalette::Highlight);
                return result;
            }

            QColor disabledText() {
                QPalette pal;
                QColor result = pal.color(QPalette::Disabled, QPalette::WindowText);
                return result;
            }

            QColor borderColor() {
                static const QColor col =
#if defined __APPLE__
                QColor(67, 67, 67);
#else
                QColor(Qt::black);
#endif
                return col;
            }

            QColor separatorColor() {
                static const QColor col =
#if defined __APPLE__
                QColor(183, 183, 183);
#else
                QColor(Qt::lightGray);
#endif
                return col;
            }
        }
    }
}
