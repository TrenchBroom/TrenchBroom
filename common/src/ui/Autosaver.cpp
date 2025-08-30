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

#include "io/DiskFileSystem.h"
#include "io/DiskIO.h"
#include "io/FileSystem.h"
#include "io/PathInfo.h"
#include "io/TraversalMode.h"
#include "ui/MapDocument.h"

#include "kdl/memory_utils.h"
#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/string_format.h"
#include "kdl/string_utils.h"
#include "kdl/vector_utils.h"

#include <fmt/format.h>

#include <algorithm>
#include <cassert>

namespace tb::ui
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
  return kdl::vec_transform(
           toDelete,
           [&](auto filename) {
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
  std::vector<std::filesystem::path>& backups,
  const std::filesystem::path& mapBasename)
{
  return kdl::vec_transform(
           backups,
           [&](const std::filesystem::path& backup, const size_t i) {
             const auto& oldName = backup.filename();
             const auto newName = makeBackupName(mapBasename, i + 1);

             return oldName != newName ? fs.moveFile(oldName, newName) : Result<void>{};
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
  std::weak_ptr<MapDocument> document,
  const std::chrono::milliseconds saveInterval,
  const size_t maxBackups)
  : m_document{std::move(document)}
  , m_saveInterval{saveInterval}
  , m_maxBackups{maxBackups}
  , m_lastSaveTime{Clock::now()}
  , m_lastModificationCount{kdl::mem_lock(m_document)->modificationCount()}
{
}

void Autosaver::triggerAutosave(Logger& logger)
{
  if (!kdl::mem_expired(m_document))
  {
    auto document = kdl::mem_lock(m_document);
    if (
      document->modified() && document->modificationCount() != m_lastModificationCount
      && Clock::now() - m_lastSaveTime >= m_saveInterval && document->persistent())
    {
      autosave(logger, document);
    }
  }
}

void Autosaver::autosave(Logger& logger, std::shared_ptr<MapDocument> document)
{
  const auto& mapPath = document->path();
  assert(io::Disk::pathInfo(mapPath) == io::PathInfo::File);

  const auto mapFilename = mapPath.filename();
  const auto mapBasename = mapPath.stem();

  createBackupFileSystem(mapPath) | kdl::and_then([&](auto fs) {
    return collectBackups(fs, mapBasename) | kdl::and_then([&](auto backups) {
             return thinBackups(logger, fs, backups, m_maxBackups);
           })
           | kdl::and_then([&](auto remainingBackups) {
               return cleanBackups(fs, remainingBackups, mapBasename)
                      | kdl::and_then([&]() {
                          assert(remainingBackups.size() < m_maxBackups);
                          const auto backupNo = remainingBackups.size() + 1;
                          return fs.makeAbsolute(makeBackupName(mapBasename, backupNo));
                        });
             });
  }) | kdl::transform([&](const auto& backupFilePath) {
    m_lastSaveTime = Clock::now();
    m_lastModificationCount = document->modificationCount();
    document->saveDocumentTo(backupFilePath);

    logger.info() << "Created autosave backup at " << backupFilePath;
  }) | kdl::transform_error([&](auto e) {
    logger.error() << "Aborting autosave: " << e.msg;
  });
}

} // namespace tb::ui
