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

#undef CursorShape

#include <QBoxLayout>
#include <QLocale>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <string>

class QButtonGroup;
class QColor;
class QCompleter;
class QDialog;
class QDialogButtonBox;
class QEvent;
class QFont;
class QLayout;
class QLineEdit;
class QMainWindow;
class QPalette;
class QSlider;
class QSplitter;
class QString;
class QTableView;
class QToolButton;
class QVBoxLayout;
class QWidget;

namespace tb
{
namespace mdl
{
enum class MapTextEncoding;
}

namespace ui
{

/**
 * Return true if the given widget or any of its children currently has focus.
 */
bool widgetOrChildHasFocus(const QWidget* widget);

void checkButtonInGroup(QButtonGroup* group, const QString& objectName, bool checked);

void deleteChildWidgetsLaterAndDeleteLayout(QWidget* widget);

void showModelessDialog(QDialog* dialog);

QString mapStringToUnicode(mdl::MapTextEncoding encoding, const std::string& string);
std::string mapStringFromUnicode(mdl::MapTextEncoding encoding, const QString& string);

} // namespace ui
} // namespace tb
