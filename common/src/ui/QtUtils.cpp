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

#include "ui/MapFrame.h"

#include "kd/contracts.h"

namespace tb::ui
{

void deleteChildWidgetsLaterAndDeleteLayout(QWidget* widget)
{
  const auto children = widget->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly);
  for (auto* childWidget : children)
  {
    childWidget->deleteLater();
  }

  delete widget->layout();
}

} // namespace tb::ui
