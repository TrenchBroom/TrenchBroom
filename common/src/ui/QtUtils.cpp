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
#include "ui/ImageUtils.h"
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

QWidget* makeDefault(QWidget* widget)
{
  widget->setFont(QFont{});
  widget->setPalette(QPalette{});
  return widget;
}

QWidget* makeEmphasized(QWidget* widget)
{
  auto font = widget->font();
  font.setBold(true);
  widget->setFont(font);
  return widget;
}

QWidget* makeUnemphasized(QWidget* widget)
{
  widget->setFont(QFont{});
  return widget;
}

QWidget* makeInfo(QWidget* widget)
{
  makeDefault(widget);

  widget = makeSmall(widget);

  const auto defaultPalette = QPalette{};
  auto palette = widget->palette();
  // Set all color groups (active, inactive, disabled) to use the disabled color, so it's
  // dimmer
  palette.setColor(
    QPalette::WindowText, defaultPalette.color(QPalette::Disabled, QPalette::WindowText));
  palette.setColor(
    QPalette::Text, defaultPalette.color(QPalette::Disabled, QPalette::Text));
  widget->setPalette(palette);
  return widget;
}

QWidget* makeSmall(QWidget* widget)
{
  widget->setAttribute(Qt::WA_MacSmallSize);
  return widget;
}

QWidget* makeHeader(QWidget* widget)
{
  makeDefault(widget);

  auto font = widget->font();
  font.setPointSize(2 * font.pointSize());
  font.setBold(true);
  widget->setFont(font);
  return widget;
}

QWidget* makeError(QWidget* widget)
{
  auto palette = widget->palette();
  palette.setColor(QPalette::Normal, QPalette::WindowText, Qt::red);
  palette.setColor(QPalette::Normal, QPalette::Text, Qt::red);
  widget->setPalette(palette);
  return widget;
}

QWidget* createDefaultPage(const QString& message, QWidget* parent)
{
  auto* container = new QWidget{parent};
  auto* layout = new QVBoxLayout{};

  auto* messageLabel = new QLabel{message};
  makeEmphasized(messageLabel);
  layout->addWidget(messageLabel, 0, Qt::AlignHCenter | Qt::AlignTop);
  container->setLayout(layout);

  return container;
}

void setWindowIconTB(QWidget* window)
{
  contract_pre(window != nullptr);

  window->setWindowIcon(QIcon{loadPixmap("AppIcon.png")});
}

void setDefaultWindowColor(QWidget* widget)
{
  widget->setAutoFillBackground(true);
  widget->setBackgroundRole(QPalette::Window);
}

void setBaseWindowColor(QWidget* widget)
{
  widget->setAutoFillBackground(true);
  widget->setBackgroundRole(QPalette::Base);
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

void insertTitleBarSeparator(QVBoxLayout* layout)
{
#ifdef _WIN32
  layout->insertWidget(0, new BorderLine{}, 1);
#endif
  unused(layout);
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

QString nativeModifierLabel(const int modifier)
{
  contract_pre(
    modifier == Qt::META || modifier == Qt::SHIFT || modifier == Qt::CTRL
    || modifier == Qt::ALT);

  const auto keySequence = QKeySequence(modifier);

  // QKeySequence doesn't totally support being given just a modifier
  // but it does seem to handle the key codes like Qt::SHIFT, which
  // it turns into native text as "Shift+" or the Shift symbol on macOS,
  // and portable text as "Shift+".

  auto nativeLabel = keySequence.toString(QKeySequence::NativeText);
  if (nativeLabel.endsWith("+"))
  {
    // On Linux we get nativeLabel as something like "Ctrl+"
    // On macOS it's just the special Command character, with no +
    nativeLabel.chop(1); // Remove last character
  }

  return nativeLabel;
}

} // namespace tb::ui
