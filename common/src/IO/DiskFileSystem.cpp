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

#include <memory>
#include <string>

namespace TrenchBroom
{
namespace IO
{
DiskFileSystem::DiskFileSystem(const Path& root, const bool ensureExists)
  : DiskFileSystem{nullptr, root, ensureExists}
{
}

DiskFileSystem::DiskFileSystem(
  std::shared_ptr<FileSystem> next, const Path& root, const bool ensureExists)
  : FileSystem{std::move(next)}
  , m_root{root.makeCanonical()}
{
  if (ensureExists && !Disk::directoryExists(m_root))
  {
    throw FileSystemException{"Directory not found: '" + m_root.asString() + "'"};
  }
}

const Path& DiskFileSystem::root() const
{
  return m_root;
}

bool DiskFileSystem::doCanMakeAbsolute(const Path& path) const
{
  const auto resolvedPath = path.makeCanonical();
  return doFileExists(resolvedPath) || doDirectoryExists(resolvedPath);
}

Path DiskFileSystem::doMakeAbsolute(const Path& path) const
{
  return m_root + path.makeCanonical();
}

bool DiskFileSystem::doDirectoryExists(const Path& path) const
{
  return Disk::directoryExists(doMakeAbsolute(path));
}

bool DiskFileSystem::doFileExists(const Path& path) const
{
  return Disk::fileExists(doMakeAbsolute(path));
}

std::vector<Path> DiskFileSystem::doGetDirectoryContents(const Path& path) const
{
  return Disk::getDirectoryContents(doMakeAbsolute(path));
}

std::shared_ptr<File> DiskFileSystem::doOpenFile(const Path& path) const
{
  auto file = Disk::openFile(doMakeAbsolute(path));
  return std::make_shared<FileView>(path, file, 0u, file->size());
}

WritableDiskFileSystem::WritableDiskFileSystem(const Path& root, const bool create)
  : WritableDiskFileSystem(nullptr, root, create)
{
}

WritableDiskFileSystem::WritableDiskFileSystem(
  std::shared_ptr<FileSystem> next, const Path& root, const bool create)
  : DiskFileSystem{std::move(next), root, !create}
{
  if (create && !Disk::directoryExists(m_root))
  {
    Disk::createDirectory(m_root);
  }
}

void WritableDiskFileSystem::doCreateFile(const Path& path, const std::string& contents)
{
  Disk::createFile(doMakeAbsolute(path), contents);
}

void WritableDiskFileSystem::doCreateDirectory(const Path& path)
{
  Disk::createDirectory(doMakeAbsolute(path));
}

void WritableDiskFileSystem::doDeleteFile(const Path& path)
{
  Disk::deleteFile(doMakeAbsolute(path));
}

void WritableDiskFileSystem::doCopyFile(
  const Path& sourcePath, const Path& destPath, const bool overwrite)
{
  Disk::copyFile(doMakeAbsolute(sourcePath), doMakeAbsolute(destPath), overwrite);
}

void WritableDiskFileSystem::doMoveFile(
  const Path& sourcePath, const Path& destPath, const bool overwrite)
{
  Disk::moveFile(doMakeAbsolute(sourcePath), doMakeAbsolute(destPath), overwrite);
}
} // namespace IO
} // namespace TrenchBroom
