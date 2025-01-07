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

#include "DiskFileSystem.h"

#include "io/DiskIO.h"
#include "io/File.h"
#include "io/PathInfo.h"
#include "io/TraversalMode.h"

#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <memory>
#include <string>

namespace tb::io
{

DiskFileSystem::DiskFileSystem(const std::filesystem::path& root)
  : m_root{root.lexically_normal()}
{
}

const std::filesystem::path& DiskFileSystem::root() const
{
  return m_root;
}

Result<std::filesystem::path> DiskFileSystem::makeAbsolute(
  const std::filesystem::path& path) const
{
  const auto canonicalPath = path.lexically_normal();
  if (!canonicalPath.empty() && kdl::path_front(canonicalPath).string() == "..")
  {
    return Error{fmt::format("Failed to make absolute path of {}", fmt::streamed(path))};
  }
  return canonicalPath.empty() ? m_root : m_root / canonicalPath;
}

PathInfo DiskFileSystem::pathInfo(const std::filesystem::path& path) const
{
  return makeAbsolute(path)
         | kdl::transform([](const auto& absPath) { return Disk::pathInfo(absPath); })
         | kdl::transform_error([](const auto&) { return PathInfo::Unknown; })
         | kdl::value();
}

const FileSystemMetadata* DiskFileSystem::metadata(
  const std::filesystem::path&, const std::string&) const
{
  return nullptr;
}

Result<std::vector<std::filesystem::path>> DiskFileSystem::doFind(
  const std::filesystem::path& path, const TraversalMode& traversalMode) const
{
  return makeAbsolute(path) | kdl::and_then([&](const auto& absPath) {
           return Disk::find(absPath, traversalMode);
         })
         | kdl::transform([&](const auto& paths) {
             return kdl::vec_transform(
               paths, [&](auto p) { return p.lexically_relative(m_root); });
           });
}

Result<std::shared_ptr<File>> DiskFileSystem::doOpenFile(
  const std::filesystem::path& path) const
{
  return makeAbsolute(path) | kdl::and_then(Disk::openFile)
         | kdl::transform(
           [](auto cFile) { return std::static_pointer_cast<File>(cFile); });
}

WritableDiskFileSystem::WritableDiskFileSystem(const std::filesystem::path& root)
  : DiskFileSystem{root}
{
}

Result<void> WritableDiskFileSystem::doCreateFile(
  const std::filesystem::path& path, const std::string& contents)
{
  return makeAbsolute(path) | kdl::and_then([&](const auto& absPath) {
           return Disk::withOutputStream(
             absPath, [&](auto& stream) { stream << contents; });
         });
}

Result<bool> WritableDiskFileSystem::doCreateDirectory(const std::filesystem::path& path)
{
  return makeAbsolute(path) | kdl::and_then(Disk::createDirectory);
}

Result<bool> WritableDiskFileSystem::doDeleteFile(const std::filesystem::path& path)
{
  return makeAbsolute(path) | kdl::and_then(Disk::deleteFile);
}

Result<void> WritableDiskFileSystem::doCopyFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return makeAbsolute(sourcePath).join(makeAbsolute(destPath))
         | kdl::and_then(Disk::copyFile);
}

Result<void> WritableDiskFileSystem::doMoveFile(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return makeAbsolute(sourcePath).join(makeAbsolute(destPath))
         | kdl::and_then(Disk::moveFile);
}

Result<void> WritableDiskFileSystem::doRenameDirectory(
  const std::filesystem::path& sourcePath, const std::filesystem::path& destPath)
{
  return makeAbsolute(sourcePath).join(makeAbsolute(destPath))
         | kdl::and_then(Disk::renameDirectory);
}

} // namespace tb::io
