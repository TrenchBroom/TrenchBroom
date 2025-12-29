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

#include "QtUtils.h"

#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QColor>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFont>
#include <QGuiApplication>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QResizeEvent>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringBuilder>
#include <QStringDecoder>
#include <QStringEncoder>
#include <QTableView>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWindow>
#include <QtGlobal>

#include "Macros.h"
#include "mdl/MapTextEncoding.h"
#include "ui/MapFrame.h"

#include "kd/contracts.h"

namespace tb::ui
{

namespace
{

QStringConverter::Encoding codecForEncoding(const mdl::MapTextEncoding encoding)
{
  switch (encoding)
  {
  case mdl::MapTextEncoding::Quake:
    // Quake uses the full 1-255 range for its bitmap font.
    // So using a "just assume UTF-8" approach would not work here.
    // See: https://github.com/TrenchBroom/TrenchBroom/issues/3122
    return QStringConverter::System;
  case mdl::MapTextEncoding::Utf8:
    return QStringConverter::Utf8;
    switchDefault();
  }
}

} // namespace

bool widgetOrChildHasFocus(const QWidget* widget)
{
  contract_pre(widget != nullptr);

  const auto* focusWidget = QApplication::focusWidget();
  return widget == focusWidget || widget->isAncestorOf(focusWidget);
}

void checkButtonInGroup(QButtonGroup* group, const QString& objectName, bool checked)
{
  for (auto* button : group->buttons())
  {
    if (button->objectName() == objectName)
    {
      button->setChecked(checked);
      return;
    }
  }
}

void deleteChildWidgetsLaterAndDeleteLayout(QWidget* widget)
{
  const auto children = widget->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly);
  for (auto* childWidget : children)
  {
    childWidget->deleteLater();
  }

  delete widget->layout();
}

void showModelessDialog(QDialog* dialog)
{
  // https://doc.qt.io/qt-5/qdialog.html#code-examples
  dialog->show();
  dialog->raise();
  dialog->activateWindow();
}

QString mapStringToUnicode(const mdl::MapTextEncoding encoding, const std::string& string)
{
  const auto codec = codecForEncoding(encoding);
  auto decode = QStringDecoder{codec};
  return decode(QByteArray::fromStdString(string));
}

std::string mapStringFromUnicode(
  const mdl::MapTextEncoding encoding, const QString& string)
{
  const auto codec = codecForEncoding(encoding);
  auto encode = QStringEncoder{codec};

  return QByteArray{encode(string)}.toStdString();
}

} // namespace tb::ui
