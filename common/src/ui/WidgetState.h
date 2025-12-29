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

#include <QSettings>

#include "kd/contracts.h"

class QWidget;
class QString;

namespace tb::ui
{

QString widgetSettingsPath(const QWidget* widget, const QString& suffix = "");

void saveWidgetGeometry(QWidget* widget);
void restoreWidgetGeometry(QWidget* widget);

template <typename T>
void saveWidgetState(const T* widget)
{
  contract_pre(widget != nullptr);

  const auto path = widgetSettingsPath(widget, "State");
  auto settings = QSettings{};
  settings.setValue(path, widget->saveState());
}

template <typename T>
void restoreWidgetState(T* widget)
{
  contract_pre(widget != nullptr);

  const auto path = widgetSettingsPath(widget, "State");
  auto settings = QSettings{};
  widget->restoreState(settings.value(path).toByteArray());
}

} // namespace tb::ui
