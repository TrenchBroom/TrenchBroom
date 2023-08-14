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

#include "Error.h"
#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/PathInfo.h"
#include "IO/TraversalMode.h"

#include <kdl/path_utils.h>
#include <kdl/result.h>
#include <kdl/result_fold.h>
#include <kdl/vector_utils.h>

#include <memory>
#include <string>

namespace TrenchBroom
{
namespace IO
{
DiskFileSystem::DiskFileSystem(const std::filesystem::path& root)
  : m_root{root.lexically_normal()}
{
}

const std::filesystem::path& DiskFileSystem::root() const
{
  return m_root;
}

kdl::result<std::filesystem::path, Error> DiskFileSystem::makeAbsolute(
  const std::filesystem::path& path) const
{
  const auto canonicalPath = path.lexically_normal();
  if (!canonicalPath.empty() && kdl::path_front(canonicalPath).string() == "..")
  {
    return Error{"Cannot make absolute path of '" + path.string() + "'"};
  }
  return canonicalPath.empty() ? m_root : m_root / canonicalPath;
}

PathInfo DiskFileSystem::pathInfo(const std::filesystem::path& path) const
{
  return makeAbsolute(path)
    .transform([](const auto& absPath) { return Disk::pathInfo(absPath); })
    .transform_error([](const auto&) { return PathInfo::Unknown; })
    .value();
}

kdl::result<std::vector<std::filesystem::path>, Error> DiskFileSystem::doFind(
  const std::filesystem::path& path, const TraversalMode traversalMode) const
{
  return makeAbsolute(path)
    .and_then([&](const auto& absPath) { return Disk::find(absPath, traversalMode); })
    .transform([&](const auto& paths) {
      return kdl::vec_transform(
        paths, [&](auto p) { return p.lexically_relative(m_root); });
    });
}

kdl::result<std::shared_ptr<File>, Error> DiskFileSystem::doOpenFile(
  const std::filesystem::path& path) const
{
  return makeAbsolute(path).and_then(Disk::openFile).transform([](auto cFile) {
    return std::static_pointer_cast<File>(cFile);
  });
}

WritableDiskFileSystem::WritableDiskFileSystem(const std::filesystem::path& root)
  : DiskFileSystem{root}
{
}

kdl::result<void, Error> WritableDiskFileSystem::doCreateFile(
  const std::filesystem::path& path, const std::string& contents)
{
  return makeAbsolute(path).and_then([&](const auto& absPath) {
    return Disk::withOutputStream(absPath, [&](auto& stream) { stream << contents; });
  });
}

kdl::result<bool, Error> WritableDiskFileSystem::doCreateDirectory(
  const std::filesystem::path& path)
{
  return makeAbsolute(path).and_then(Disk::createDirectory);
}

kdl::result<bool, Error> WritableDiskFileSystem::doDeleteFile(
  const std::filesystem::path& path)
{
  return makeAbsolute(path).and_then(Disk::deleteFile);
}

kdl::result<void, Error> WritableDiskFileSystem::doCopyFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return makeAbsolute(sourcePath).join(makeAbsolute(destPath)).and_then(Disk::copyFile);
}

kdl::result<void, Error> WritableDiskFileSystem::doMoveFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return makeAbsolute(sourcePath).join(makeAbsolute(destPath)).and_then(Disk::moveFile);
}
} // namespace IO
} // namespace TrenchBroom
