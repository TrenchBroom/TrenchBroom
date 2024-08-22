/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "IO/PathMatcher.h"

#include <chrono>
#include <filesystem>
#include <memory>

namespace TrenchBroom
{
class Logger;
} // namespace TrenchBroom

namespace TrenchBroom::View
{
class Command;
class MapDocument;

IO::PathMatcher makeBackupPathMatcher(std::filesystem::path mapBasename);

class Autosaver
{
private:
  using Clock = std::chrono::system_clock;

  std::weak_ptr<MapDocument> m_document;

  /**
   * The time after which a new autosave is attempted, in seconds.
   */
  std::chrono::milliseconds m_saveInterval;

  /**
   * The maximum number of backups to create. When this number is exceeded, old backups
   * are deleted until the number of backups is equal to the number of backups again.
   */
  size_t m_maxBackups;

  /**
   * The time at which the last autosave has succeeded.
   */
  std::chrono::time_point<Clock> m_lastSaveTime;

  /**
   * The modification count that was last recorded.
   */
  size_t m_lastModificationCount;

public:
  explicit Autosaver(
    std::weak_ptr<MapDocument> document,
    std::chrono::milliseconds saveInterval = std::chrono::milliseconds(10 * 60 * 1000),
    size_t maxBackups = 50);

  void triggerAutosave(Logger& logger);

private:
  void autosave(Logger& logger, std::shared_ptr<View::MapDocument> document);
};
} // namespace TrenchBroom::View
