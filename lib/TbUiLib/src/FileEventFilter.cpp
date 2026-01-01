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

#include "ui/FileEventFilter.h"

#include <QEvent>
#include <QFileOpenEvent>

#include "ui/AppController.h"
#include "ui/FrameManager.h"

#include <filesystem>

namespace tb::ui
{

FileEventFilter::FileEventFilter(AppController& appController, QObject* parent)
  : QObject{parent}
  , m_appController{appController}
{
}

bool FileEventFilter::eventFilter(QObject* watched, QEvent* event)
{
  if (event->type() == QEvent::FileOpen)
  {
    const auto* openEvent = static_cast<QFileOpenEvent*>(event);
    const auto path = std::filesystem::path{openEvent->file().toStdString()};
    if (m_appController.openDocument(path))
    {
      return true;
    }
    return false;
  }
  else if (event->type() == QEvent::ApplicationActivate)
  {
    if (m_appController.frameManager().allFramesClosed())
    {
      m_appController.showWelcomeWindow();
    }
  }

  return QObject::eventFilter(watched, event);
}

} // namespace tb::ui
