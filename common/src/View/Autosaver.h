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

#include "IO/FileSystem.h"

#include <kdl/result_forward.h>

#include <chrono>
#include <filesystem>
#include <memory>

namespace TrenchBroom
{
class Logger;

namespace IO
{
class FileSystem;
struct FileSystemError;
class WritableDiskFileSystem;
} // namespace IO

namespace View
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
  kdl::result<IO::WritableDiskFileSystem, IO::FileSystemError> createBackupFileSystem(
    const std::filesystem::path& mapPath) const;
  std::vector<std::filesystem::path> collectBackups(
    const IO::FileSystem& fs, const std::filesystem::path& mapBasename) const;
  kdl::result<std::vector<std::filesystem::path>, IO::FileSystemError> thinBackups(
    Logger& logger,
    IO::WritableDiskFileSystem& fs,
    const std::vector<std::filesystem::path>& backups) const;
  kdl::result<void, IO::FileSystemError> cleanBackups(
    IO::WritableDiskFileSystem& fs,
    std::vector<std::filesystem::path>& backups,
    const std::filesystem::path& mapBasename) const;
  std::filesystem::path makeBackupName(
    const std::filesystem::path& mapBasename, size_t index) const;
};
} // namespace View
} // namespace TrenchBroom
