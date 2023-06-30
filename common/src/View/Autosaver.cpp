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

#include "Autosaver.h"

#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/PathInfo.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/path_utils.h>
#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>

#include <algorithm> // for std::sort
#include <cassert>
#include <limits>
#include <memory>

namespace TrenchBroom
{
namespace View
{
IO::PathMatcher makeBackupPathMatcher(std::filesystem::path mapBasename)
{
  return [mapBasename = std::move(mapBasename)](
           const std::filesystem::path& path, const IO::GetPathInfo& getPathInfo) {
    const auto backupName = path.stem();
    const auto backupBasename = backupName.stem();
    const auto backupExtension = backupName.extension().string();
    const auto backupNum = backupExtension.empty() ? "" : backupExtension.substr(1);

    return getPathInfo(path) == IO::PathInfo::File
           && kdl::ci::str_is_equal(path.extension().string(), ".map")
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
  assert(IO::Disk::pathInfo(mapPath) == IO::PathInfo::File);

  const auto mapFilename = mapPath.filename();
  const auto mapBasename = mapPath.stem();

  try
  {
    auto fs = createBackupFileSystem(logger, mapPath);
    auto backups = collectBackups(fs, mapBasename);

    thinBackups(logger, fs, backups);
    cleanBackups(fs, backups, mapBasename);

    assert(backups.size() < m_maxBackups);
    const auto backupNo = backups.size() + 1;

    const auto backupFilePath = fs.makeAbsolute(makeBackupName(mapBasename, backupNo));

    m_lastSaveTime = Clock::now();
    m_lastModificationCount = document->modificationCount();
    document->saveDocumentTo(backupFilePath);

    logger.info() << "Created autosave backup at " << backupFilePath;
  }
  catch (const FileSystemException& e)
  {
    logger.error() << "Aborting autosave: " << e.what();
  }
}

IO::WritableDiskFileSystem Autosaver::createBackupFileSystem(
  Logger& logger, const std::filesystem::path& mapPath) const
{
  const auto basePath = mapPath.parent_path();
  const auto autosavePath = basePath / "autosave";

  try
  {
    // ensures that the directory exists or is created if it doesn't
    return IO::WritableDiskFileSystem{autosavePath, true};
  }
  catch (const FileSystemException& e)
  {
    logger.error() << "Cannot create autosave directory at " << autosavePath;
    throw e;
  }
}

namespace
{
size_t extractBackupNo(const std::filesystem::path& path)
{
  // currently this function is only used when comparing file names which have already
  // been verified as valid backup file names, so this should not go wrong, but if it
  // does, sort the invalid file names to the end to avoid modifying them
  return kdl::str_to_size(kdl::path_remove_extension(path).extension().string())
    .value_or(std::numeric_limits<size_t>::max());
}

bool compareBackupsByNo(
  const std::filesystem::path& lhs, const std::filesystem::path& rhs)
{
  return extractBackupNo(lhs) < extractBackupNo(rhs);
}
} // namespace

std::vector<std::filesystem::path> Autosaver::collectBackups(
  const IO::FileSystem& fs, const std::filesystem::path& mapBasename) const
{
  auto backups = fs.find({}, makeBackupPathMatcher(mapBasename));
  std::sort(std::begin(backups), std::end(backups), compareBackupsByNo);
  return backups;
}

void Autosaver::thinBackups(
  Logger& logger,
  IO::WritableDiskFileSystem& fs,
  std::vector<std::filesystem::path>& backups) const
{
  while (backups.size() > m_maxBackups - 1)
  {
    const auto filename = backups.front();
    try
    {
      fs.deleteFile(filename);
      logger.debug() << "Deleted autosave backup " << filename;
      backups.erase(std::begin(backups));
    }
    catch (const FileSystemException& e)
    {
      logger.error() << "Cannot delete autosave backup " << filename;
      throw e;
    }
  }
}

void Autosaver::cleanBackups(
  IO::WritableDiskFileSystem& fs,
  std::vector<std::filesystem::path>& backups,
  const std::filesystem::path& mapBasename) const
{
  for (size_t i = 0; i < backups.size(); ++i)
  {
    const auto& oldName = backups[i].filename();
    const auto newName = makeBackupName(mapBasename, i + 1);

    if (oldName != newName)
    {
      fs.moveFile(oldName, newName);
    }
  }
}

std::filesystem::path Autosaver::makeBackupName(
  const std::filesystem::path& mapBasename, const size_t index) const
{
  return kdl::path_add_extension(mapBasename, "." + kdl::str_to_string(index) + ".map");
}

} // namespace View
} // namespace TrenchBroom
