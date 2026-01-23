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

#include "ui/MapWindowManager.h"

#include <QApplication>

#include "mdl/GameInfo.h"
#include "ui/AppController.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"

#include "kd/contracts.h"

#include <algorithm>
#include <memory>

namespace tb::ui
{

MapWindowManager::MapWindowManager(
  AppController& appController, const bool singleMapWindow)
  : QObject{&appController}
  , m_appController{appController}
  , m_singleMapWindow{singleMapWindow}
{
  connect(qApp, &QApplication::focusChanged, this, &MapWindowManager::onFocusChange);
}

MapWindowManager::~MapWindowManager() = default;

std::vector<MapWindow*> MapWindowManager::mapWindows() const
{
  return m_mapWindows;
}

MapWindow* MapWindowManager::topMapWindow() const
{
  return m_mapWindows.empty() ? nullptr : m_mapWindows.front();
}

Result<void> MapWindowManager::createDocument(
  const mdl::EnvironmentConfig& environmentConfig,
  const mdl::GameInfo& gameInfo,
  mdl::MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  kdl::task_manager& taskManager)
{
  if (shouldCreateWindowForDocument())
  {
    return MapDocument::createDocument(
             environmentConfig, gameInfo, mapFormat, worldBounds, taskManager)
           | kdl::transform([&](auto document) { createMapWindow(std::move(document)); });
  }

  auto* mapWindow = topMapWindow();
  contract_assert(mapWindow != nullptr);

  return mapWindow->document().create(
    environmentConfig, gameInfo, mapFormat, worldBounds);
}

Result<void> MapWindowManager::loadDocument(
  const mdl::EnvironmentConfig& environmentConfig,
  const mdl::GameInfo& gameInfo,
  mdl::MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  std::filesystem::path path,
  kdl::task_manager& taskManager)
{
  if (shouldCreateWindowForDocument())
  {
    return MapDocument::loadDocument(
             environmentConfig,
             gameInfo,
             mapFormat,
             worldBounds,
             std::move(path),
             taskManager)
           | kdl::transform([&](auto document) { createMapWindow(std::move(document)); });
  }

  auto* mapWindow = topMapWindow();
  contract_assert(mapWindow != nullptr);

  return mapWindow->document().load(
    environmentConfig, gameInfo, mapFormat, worldBounds, std::move(path));
}

bool MapWindowManager::allMapWindowsClosed() const
{
  return m_mapWindows.empty();
}

void MapWindowManager::onFocusChange(QWidget* /* old */, QWidget* now)
{
  if (now)
  {
    // The QApplication::focusChanged signal also notifies us of focus changes between
    // child widgets, so get the top-level widget with QWidget::window()
    if (auto* mapWindow = dynamic_cast<MapWindow*>(now->window()))
    {
      if (auto it = std::ranges::find(m_mapWindows, mapWindow);
          it != m_mapWindows.end() && it != m_mapWindows.begin())
      {
        std::rotate(m_mapWindows.begin(), it, std::next(it));
      }
    }
  }
}

bool MapWindowManager::shouldCreateWindowForDocument() const
{
  return !m_singleMapWindow || m_mapWindows.empty();
}

MapWindow* MapWindowManager::createMapWindow(std::unique_ptr<MapDocument> document)
{
  contract_pre(document != nullptr);

  auto* mapWindow = new MapWindow{m_appController, std::move(document)};
  mapWindow->positionOnScreen(topMapWindow());
  m_mapWindows.insert(m_mapWindows.begin(), mapWindow);

  mapWindow->show();
  mapWindow->activateWindow();
  return mapWindow;
}

void MapWindowManager::removeMapWindow(MapWindow* mapWindow)
{
  // This is called from MapWindow::closeEvent
  if (auto it = std::ranges::find(m_mapWindows, mapWindow); it != m_mapWindows.end())
  {
    m_mapWindows.erase(it);
    // MapWindow uses Qt::WA_DeleteOnClose so we don't need to delete it here
  }
}

} // namespace tb::ui
