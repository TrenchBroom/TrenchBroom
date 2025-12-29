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

#include "WidgetState.h"

#include <QWidget>

namespace tb::ui
{

QString widgetSettingsPath(const QWidget* window, const QString& suffix)
{
  contract_pre(window != nullptr);
  contract_pre(!window->objectName().isEmpty());

  return "Windows/" + window->objectName() + "/" + suffix;
}

void saveWidgetGeometry(QWidget* widget)
{
  contract_pre(widget != nullptr);

  const auto path = widgetSettingsPath(widget, "Geometry");
  auto settings = QSettings{};
  settings.setValue(path, widget->saveGeometry());
}

void restoreWidgetGeometry(QWidget* widget)
{
  contract_pre(widget != nullptr);

  const auto path = widgetSettingsPath(widget, "Geometry");
  auto settings = QSettings{};
  widget->restoreGeometry(settings.value(path).toByteArray());
}

} // namespace tb::ui
