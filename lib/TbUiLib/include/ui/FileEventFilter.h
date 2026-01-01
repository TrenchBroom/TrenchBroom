/*
 Copyright (C) 2025 Kristian Duske

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

#include <QObject>

class QEvent;

namespace tb::ui
{
class AppController;

class FileEventFilter : public QObject
{
  Q_OBJECT
private:
  AppController& m_appController;

public:
  explicit FileEventFilter(AppController& appController, QObject* parent = nullptr);

  bool eventFilter(QObject* watched, QEvent* event) override;
};

} // namespace tb::ui
