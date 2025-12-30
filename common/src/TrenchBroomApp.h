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

#include <QApplication>

namespace tb::ui
{
class AppController;

class TrenchBroomApp : public QApplication
{
  Q_OBJECT
private:
  AppController* m_appController = nullptr;

public:
  static TrenchBroomApp& instance();

  TrenchBroomApp(int& argc, char** argv);
  ~TrenchBroomApp() override;

public:
  const AppController& appController() const;
  AppController& appController();

public:
  bool notify(QObject* receiver, QEvent* event) override;

#ifdef __APPLE__
  bool event(QEvent* event) override;
#endif
};

} // namespace tb::ui
