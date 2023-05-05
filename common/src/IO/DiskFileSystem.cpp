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

#include "DiskFileSystem.h"

#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/PathInfo.h"

#include <kdl/path_utils.h>
#include <kdl/vector_utils.h>

#include <memory>
#include <string>

namespace TrenchBroom
{
namespace IO
{
DiskFileSystem::DiskFileSystem(const std::filesystem::path& root, const bool ensureExists)
  : m_root{root.lexically_normal()}
{
  if (ensureExists && Disk::pathInfo(m_root) != PathInfo::Directory)
  {
    throw FileSystemException{"Directory not found: '" + m_root.string() + "'"};
  }
}

const std::filesystem::path& DiskFileSystem::root() const
{
  return m_root;
}

std::filesystem::path DiskFileSystem::doMakeAbsolute(
  const std::filesystem::path& path) const
{
  const auto canonicalPath = path.lexically_normal();
  if (!canonicalPath.empty() && kdl::path_front(canonicalPath).string() == "..")
  {
    throw FileSystemException{"Cannot make absolute path of '" + path.string() + "'"};
  }
  return canonicalPath.empty() ? m_root : m_root / canonicalPath;
}

PathInfo DiskFileSystem::doGetPathInfo(const std::filesystem::path& path) const
{
  return Disk::pathInfo(makeAbsolute(path));
}

std::vector<std::filesystem::path> DiskFileSystem::doGetDirectoryContents(
  const std::filesystem::path& path) const
{
  return Disk::directoryContents(makeAbsolute(path));
}

std::shared_ptr<File> DiskFileSystem::doOpenFile(const std::filesystem::path& path) const
{
  auto file = Disk::openFile(makeAbsolute(path));
  return std::make_shared<FileView>(path, file, 0u, file->size());
}

WritableDiskFileSystem::WritableDiskFileSystem(
  const std::filesystem::path& root, const bool create)
  : DiskFileSystem{root, !create}
{
  if (create && Disk::pathInfo(m_root) != PathInfo::Directory)
  {
    Disk::createDirectory(m_root);
  }
}

void WritableDiskFileSystem::doCreateFile(
  const std::filesystem::path& path, const std::string& contents)
{
  Disk::createFile(makeAbsolute(path), contents);
}

void WritableDiskFileSystem::doCreateDirectory(const std::filesystem::path& path)
{
  Disk::createDirectory(makeAbsolute(path));
}

void WritableDiskFileSystem::doDeleteFile(const std::filesystem::path& path)
{
  Disk::deleteFile(makeAbsolute(path));
}

void WritableDiskFileSystem::doCopyFile(
  const std::filesystem::path& sourcePath,
  const std::filesystem::path& destPath,
  const bool overwrite)
{
  Disk::copyFile(makeAbsolute(sourcePath), makeAbsolute(destPath), overwrite);
}

void WritableDiskFileSystem::doMoveFile(
  const std::filesystem::path& sourcePath,
  const std::filesystem::path& destPath,
  const bool overwrite)
{
  Disk::moveFile(makeAbsolute(sourcePath), makeAbsolute(destPath), overwrite);
}
} // namespace IO
} // namespace TrenchBroom
