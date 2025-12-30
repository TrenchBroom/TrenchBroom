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

#include "TrenchBroomApp.h"

#include <QEvent>
#include <QFileOpenEvent>
#include <QMessageBox>

#include "PreferenceManager.h"
#include "ui/AppController.h"
#include "ui/CrashReporter.h"
#include "ui/FrameManager.h"

#include "kd/const_overload.h"

#include <fmt/format.h>

namespace tb::ui
{
namespace
{

auto createAppController(QObject* parent)
{
  return AppController::create(parent) | kdl::if_error([](auto e) {
           const auto msg =
             fmt::format(R"(Game configurations could not be loaded: {})", e.msg);

           QMessageBox::critical(
             nullptr, "TrenchBroom", QString::fromStdString(msg), QMessageBox::Ok);
           QCoreApplication::exit(1);
         })
         | kdl::value();
}

} // namespace

TrenchBroomApp& TrenchBroomApp::instance()
{
  return *dynamic_cast<TrenchBroomApp*>(qApp);
}

TrenchBroomApp::TrenchBroomApp(int& argc, char** argv)
  : QApplication{argc, argv}
  , m_appController{createAppController(this)}
{
}

TrenchBroomApp::~TrenchBroomApp()
{
  // destroy preference manager before app
  PreferenceManager::destroyInstance();
}

const AppController& TrenchBroomApp::appController() const
{
  return *m_appController;
}

AppController& TrenchBroomApp::appController()
{
  return KDL_CONST_OVERLOAD(appController());
}

/**
 * If we catch exceptions in main() that are otherwise uncaught, Qt prints a warning
 * to override QCoreApplication::notify() and catch exceptions there instead.
 */
bool TrenchBroomApp::notify(QObject* receiver, QEvent* event)
{
  auto result = false;
  runWithCrashReporting([&]() { result = QApplication::notify(receiver, event); });
  return result;
}

#ifdef __APPLE__
bool TrenchBroomApp::event(QEvent* event)
{
  if (event->type() == QEvent::FileOpen)
  {
    const auto* openEvent = static_cast<QFileOpenEvent*>(event);
    const auto path = std::filesystem::path{openEvent->file().toStdString()};
    if (appController().openDocument(path))
    {
      return true;
    }
    return false;
  }
  else if (event->type() == QEvent::ApplicationActivate)
  {
    if (appController().frameManager().allFramesClosed())
    {
      appController().showWelcomeWindow();
    }
  }
  return QApplication::event(event);
}
#endif

} // namespace tb::ui
