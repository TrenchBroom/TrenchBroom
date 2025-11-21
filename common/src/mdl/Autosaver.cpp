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

#include "Autosaver.h"

#include "Logger.h"
#include "io/DiskFileSystem.h"
#include "io/DiskIO.h"
#include "io/FileSystem.h"
#include "io/PathInfo.h"
#include "io/TraversalMode.h"
#include "mdl/Map.h"

#include "kd/contracts.h"
#include "kd/path_utils.h"
#include "kd/ranges/enumerate_view.h"
#include "kd/result.h"
#include "kd/result_fold.h"
#include "kd/string_format.h"
#include "kd/string_utils.h"
#include "kd/vector_utils.h"

#include <fmt/format.h>

#include <algorithm>
#include <ranges>

namespace tb::mdl
{
namespace
{

Result<io::WritableDiskFileSystem> createBackupFileSystem(
  const std::filesystem::path& mapPath)
{
  const auto basePath = mapPath.parent_path();
  const auto autosavePath = basePath / "autosave";

  return io::Disk::createDirectory(autosavePath)
         | kdl::transform([&](auto) { return io::WritableDiskFileSystem{autosavePath}; });
}

Result<std::vector<std::filesystem::path>> collectBackups(
  const io::FileSystem& fs, const std::filesystem::path& mapBasename)
{
  return fs.find({}, io::TraversalMode::Flat, makeBackupPathMatcher(mapBasename))
         | kdl::transform(
           [](auto backupPaths) { return kdl::vec_sort(std::move(backupPaths)); });
}

Result<std::vector<std::filesystem::path>> thinBackups(
  Logger& logger,
  io::WritableDiskFileSystem& fs,
  const std::vector<std::filesystem::path>& backups,
  const size_t maxBackups)
{
  if (backups.size() < maxBackups)
  {
    return backups;
  }

  const auto toDelete = kdl::vec_slice_prefix(backups, 1);
  return toDelete | std::views::transform([&](auto filename) {
           return fs.deleteFile(filename) | kdl::transform([&](const auto deleted) {
                    if (deleted)
                    {
                      logger.debug() << "Deleted autosave backup " << filename;
                    }
                  });
         })
         | kdl::fold | kdl::transform([&]() {
             return kdl::vec_slice_suffix(backups, backups.size() - 1);
           });
}

std::filesystem::path makeBackupName(
  const std::filesystem::path& mapBasename, const size_t index)
{
  return kdl::path_add_extension(mapBasename, fmt::format(".{}.map", index));
}

Result<void> cleanBackups(
  io::WritableDiskFileSystem& fs,
  const std::vector<std::filesystem::path>& backups,
  const std::filesystem::path& mapBasename)
{
  const auto cleanBackup = [&](const auto i, const auto& backup) {
    const auto& oldName = backup.filename();
    const auto newName = makeBackupName(mapBasename, size_t(i) + 1);

    return oldName != newName ? fs.moveFile(oldName, newName) : Result<void>{};
  };

  return backups | kdl::views::enumerate | std::views::transform([&](const auto& pair) {
           return std::apply(cleanBackup, pair);
         })
         | kdl::fold;
}

} // namespace

io::PathMatcher makeBackupPathMatcher(std::filesystem::path mapBasename_)
{
  return
    [mapBasename = std::move(mapBasename_)](const auto& path, const auto& getPathInfo) {
      const auto backupName = path.stem();
      const auto backupBasename = backupName.stem();
      const auto backupExtension = backupName.extension().string();
      const auto backupNum = backupExtension.empty() ? "" : backupExtension.substr(1);

      return getPathInfo(path) == io::PathInfo::File
             && kdl::path_to_lower(path.extension()) == ".map"
             && backupBasename == mapBasename && kdl::str_is_numeric(backupNum)
             && kdl::str_to_size(backupNum).value_or(0u) > 0u;
    };
}

Autosaver::Autosaver(
  Map& map, const std::chrono::milliseconds saveInterval, const size_t maxBackups)
  : m_map{map}
  , m_saveInterval{saveInterval}
  , m_maxBackups{maxBackups}
  , m_lastSaveTime{Clock::now()}
  , m_lastModificationCount{m_map.modificationCount()}
{
}

void Autosaver::triggerAutosave()
{
  if (
    m_map.modified() && m_map.modificationCount() != m_lastModificationCount
    && Clock::now() - m_lastSaveTime >= m_saveInterval && m_map.persistent())
  {
    autosave();
  }
}

void Autosaver::autosave()
{
  const auto& mapPath = m_map.path();
  contract_assert(io::Disk::pathInfo(mapPath) == io::PathInfo::File);

  const auto mapFilename = mapPath.filename();
  const auto mapBasename = mapPath.stem();

  createBackupFileSystem(mapPath) | kdl::and_then([&](auto fs) {
    return collectBackups(fs, mapBasename) | kdl::and_then([&](auto backups) {
             return thinBackups(m_map.logger(), fs, backups, m_maxBackups);
           })
           | kdl::and_then([&](auto remainingBackups) {
               return cleanBackups(fs, remainingBackups, mapBasename)
                      | kdl::and_then([&]() {
                          contract_assert(remainingBackups.size() < m_maxBackups);

                          const auto backupNo = remainingBackups.size() + 1;
                          return fs.makeAbsolute(makeBackupName(mapBasename, backupNo));
                        });
             });
  }) | kdl::and_then([&](const auto& backupFilePath) {
    m_lastSaveTime = Clock::now();
    m_lastModificationCount = m_map.modificationCount();
    return m_map.saveTo(backupFilePath) | kdl::transform([&]() {
             m_map.logger().info() << "Created autosave backup at " << backupFilePath;
           });
  }) | kdl::transform_error([&](auto e) {
    m_map.logger().error() << "Aborting autosave: " << e.msg;
  });
}

} // namespace tb::mdl
