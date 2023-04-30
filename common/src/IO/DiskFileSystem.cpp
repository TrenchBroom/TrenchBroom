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

#include <kdl/vector_utils.h>

#include <memory>
#include <string>

namespace TrenchBroom
{
namespace IO
{
DiskFileSystem::DiskFileSystem(const Path& root, const bool ensureExists)
  : m_root{root.makeCanonical()}
{
  if (ensureExists && Disk::pathInfo(m_root) != PathInfo::Directory)
  {
    throw FileSystemException{"Directory not found: '" + m_root.asString() + "'"};
  }
}

const Path& DiskFileSystem::root() const
{
  return m_root;
}

Path DiskFileSystem::doMakeAbsolute(const Path& path) const
{
  const auto canonicalPath = path.makeCanonical();
  return canonicalPath.isEmpty() ? m_root : m_root / canonicalPath;
}

PathInfo DiskFileSystem::doGetPathInfo(const Path& path) const
{
  return Disk::pathInfo(makeAbsolute(path));
}

std::vector<Path> DiskFileSystem::doGetDirectoryContents(const Path& path) const
{
  return Disk::directoryContents(makeAbsolute(path));
}

std::shared_ptr<File> DiskFileSystem::doOpenFile(const Path& path) const
{
  auto file = Disk::openFile(makeAbsolute(path));
  return std::make_shared<FileView>(path, file, 0u, file->size());
}

WritableDiskFileSystem::WritableDiskFileSystem(const Path& root, const bool create)
  : DiskFileSystem{root, !create}
{
  if (create && Disk::pathInfo(m_root) != PathInfo::Directory)
  {
    Disk::createDirectory(m_root);
  }
}

void WritableDiskFileSystem::doCreateFile(const Path& path, const std::string& contents)
{
  Disk::createFile(makeAbsolute(path), contents);
}

void WritableDiskFileSystem::doCreateDirectory(const Path& path)
{
  Disk::createDirectory(makeAbsolute(path));
}

void WritableDiskFileSystem::doDeleteFile(const Path& path)
{
  Disk::deleteFile(makeAbsolute(path));
}

void WritableDiskFileSystem::doCopyFile(
  const Path& sourcePath, const Path& destPath, const bool overwrite)
{
  Disk::copyFile(makeAbsolute(sourcePath), makeAbsolute(destPath), overwrite);
}

void WritableDiskFileSystem::doMoveFile(
  const Path& sourcePath, const Path& destPath, const bool overwrite)
{
  Disk::moveFile(makeAbsolute(sourcePath), makeAbsolute(destPath), overwrite);
}
} // namespace IO
} // namespace TrenchBroom
