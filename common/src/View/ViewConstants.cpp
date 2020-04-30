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
#include <QFont>
#include <QFontDatabase>
#include <QWidget>

#include <QDebug>

namespace TrenchBroom {
    namespace View {
        namespace Fonts {
            QFont fixedWidthFont() {
#if defined __APPLE__
                QFont font("Monaco");
                font.setStyleHint(QFont::TypeWriter);
                return font;
#else
                return QFontDatabase::systemFont(QFontDatabase::FixedFont);
#endif
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

            /**
             * Table cell/text edit widget disabled text. Intended for use against a QPalette::Base background.
             */
            QColor disabledCellText() {
                QPalette pal;
                QColor result = pal.color(QPalette::Disabled, QPalette::Text);
                return result;
            }

            QColor disabledText(const QWidget* widget) {
                const QPalette& pal = widget->palette();
                QColor result = pal.color(QPalette::Disabled, QPalette::WindowText);
                return result;
            }

            QColor borderColor() {
                static const QColor col =
#if defined __APPLE__
                QColor(90, 90, 90);
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
