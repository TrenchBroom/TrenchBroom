/*
 Copyright (C) 2010 Kristian Duske

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

class QLineEdit;
class QString;
class QVBoxLayout;
class QWidget;

namespace tb::ui
{

QWidget* setDefaultStyle(QWidget* widget);
QWidget* setEmphasizedStyle(QWidget* widget);
QWidget* setUnemphasizedSTyle(QWidget* widget);
QWidget* setInfoStyle(QWidget* widget);
QWidget* setSmallStyle(QWidget* widget);
QWidget* setHeaderStyle(QWidget* widget);
QWidget* setErrorStyle(QWidget* widget);

void setWindowIconTB(QWidget* window);

void setDefaultWindowColor(QWidget* widget);
void setBaseWindowColor(QWidget* widget);

/**
 * Insert a separating line as the first item in the given layout on platforms where
 * this is necessary.
 */
void insertTitleBarSeparator(QVBoxLayout* layout);

QString nativeModifierLabel(int modifier);

} // namespace tb::ui
