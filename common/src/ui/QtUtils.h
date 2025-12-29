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

#include "Color.h"

#include <filesystem>
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

QToolButton* createBitmapButton(
  const std::filesystem::path& imagePath,
  const QString& tooltip,
  QWidget* parent = nullptr);
QToolButton* createBitmapButton(
  const QIcon& icon, const QString& tooltip, QWidget* parent = nullptr);
QToolButton* createBitmapToggleButton(
  const std::filesystem::path& imagePath,
  const QString& tooltip,
  QWidget* parent = nullptr);

QWidget* createDefaultPage(const QString& message, QWidget* parent = nullptr);

QLayout* wrapDialogButtonBox(QWidget* buttonBox);
QLayout* wrapDialogButtonBox(QLayout* buttonBox);

QWidget* makeDefault(QWidget* widget);
QWidget* makeEmphasized(QWidget* widget);
QWidget* makeUnemphasized(QWidget* widget);
QWidget* makeInfo(QWidget* widget);
QWidget* makeSmall(QWidget* widget);
QWidget* makeHeader(QWidget* widget);
QWidget* makeError(QWidget* widget);

Color fromQColor(const QColor& color);
QColor toQColor(const Color& color);
void setWindowIconTB(QWidget* window);

void setDefaultWindowColor(QWidget* widget);
void setBaseWindowColor(QWidget* widget);

QLineEdit* createSearchBox();

void checkButtonInGroup(QButtonGroup* group, const QString& objectName, bool checked);

/**
 * Insert a separating line as the first item in the given layout on platforms where
 * this is necessary.
 */
void insertTitleBarSeparator(QVBoxLayout* layout);

void deleteChildWidgetsLaterAndDeleteLayout(QWidget* widget);

void showModelessDialog(QDialog* dialog);

QString mapStringToUnicode(mdl::MapTextEncoding encoding, const std::string& string);
std::string mapStringFromUnicode(mdl::MapTextEncoding encoding, const QString& string);

/**
 * Maps one of Qt::META, Qt::SHIFT, Qt::CTRL, Qt::ALT to the
 * label for it on the current OS.
 *
 * @param modifier one of Qt::META, Qt::SHIFT, Qt::CTRL, Qt::ALT
 * @return the native label for this modifier on the current OS
 *         (e.g. "Ctrl" on Windows or the Command symbol on macOS)
 */
QString nativeModifierLabel(int modifier);

} // namespace ui
} // namespace tb
