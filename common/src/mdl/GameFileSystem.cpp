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

#include "GameFileSystem.h"

#include "Logger.h"
#include "io/DiskFileSystem.h"
#include "io/DiskIO.h"
#include "io/DkPakFileSystem.h"
#include "io/IdPakFileSystem.h"
#include "io/PathInfo.h"
#include "io/SystemPaths.h"
#include "io/TraversalMode.h"
#include "io/WadFileSystem.h"
#include "io/ZipFileSystem.h"
#include "mdl/GameConfig.h"

#include "kdl/result_fold.h"
#include "kdl/string_compare.h"
#include "kdl/vector_utils.h"

#include <memory>

namespace tb::mdl
{

void GameFileSystem::initialize(
  const GameConfig& config,
  const std::filesystem::path& gamePath,
  const std::vector<std::filesystem::path>& additionalSearchPaths,
  Logger& logger)
{
  unmountAll();

  addDefaultAssetPaths(config, logger);

  if (!gamePath.empty() && io::Disk::pathInfo(gamePath) == io::PathInfo::Directory)
  {
    addGameFileSystems(config, gamePath, additionalSearchPaths, logger);
  }
}

void GameFileSystem::reloadWads(
  const std::filesystem::path& rootPath,
  const std::vector<std::filesystem::path>& wadSearchPaths,
  const std::vector<std::filesystem::path>& wadPaths,
  Logger& logger)
{
  unmountWads();
  mountWads(rootPath, wadSearchPaths, wadPaths, logger);
}

void GameFileSystem::addDefaultAssetPaths(const GameConfig& config, Logger& logger)
{
  // There are two ways of providing default assets: The 'defaults/assets' folder in
  // TrenchBroom's resources folder, and the 'assets' folder in the game configuration
  // folders. We add filesystems for both types here.

  auto defaultFolderPaths =
    io::SystemPaths::findResourceDirectories(std::filesystem::path("defaults"));
  if (!config.path.empty())
  {
    defaultFolderPaths.push_back(config.path.parent_path());
  }

  for (const auto& defaultFolderPath : defaultFolderPaths)
  {
    const auto defaultAssetsPath = defaultFolderPath / std::filesystem::path("assets");
    if (io::Disk::pathInfo(defaultAssetsPath) == io::PathInfo::Directory)
    {
      addFileSystemPath(defaultAssetsPath, logger);
    }
  }
}

void GameFileSystem::addGameFileSystems(
  const GameConfig& config,
  const std::filesystem::path& gamePath,
  const std::vector<std::filesystem::path>& additionalSearchPaths,
  Logger& logger)
{
  const auto& fileSystemConfig = config.fileSystemConfig;
  addFileSystemPath(gamePath / fileSystemConfig.searchPath, logger);
  addFileSystemPackages(config, gamePath / fileSystemConfig.searchPath, logger);

  for (const auto& searchPath : additionalSearchPaths)
  {
    addFileSystemPath(gamePath / searchPath, logger);
    addFileSystemPackages(config, gamePath / searchPath, logger);
  }
}

void GameFileSystem::addFileSystemPath(const std::filesystem::path& path, Logger& logger)
{
  logger.info() << "Adding file system path " << path;
  mount("", std::make_unique<io::DiskFileSystem>(path));
}

namespace
{
Result<std::unique_ptr<io::FileSystem>> createImageFileSystem(
  const std::string& packageFormat, const std::filesystem::path& path)
{
  const auto setMetadataAndCast = [&](auto fs) {
    fs->setMetadata(io::makeImageFileSystemMetadata(path));
    return std::unique_ptr<io::FileSystem>{std::move(fs)};
  };

  if (kdl::ci::str_is_equal(packageFormat, "idpak"))
  {
    return io::Disk::openFile(path) | kdl::and_then([&](auto file) {
             return io::createImageFileSystem<io::IdPakFileSystem>(std::move(file));
           })
           | kdl::transform(setMetadataAndCast);
  }
  else if (kdl::ci::str_is_equal(packageFormat, "dkpak"))
  {
    return io::Disk::openFile(path) | kdl::and_then([&](auto file) {
             return io::createImageFileSystem<io::DkPakFileSystem>(std::move(file));
           })
           | kdl::transform(setMetadataAndCast);
  }
  else if (kdl::ci::str_is_equal(packageFormat, "zip"))
  {
    return io::Disk::openFile(path) | kdl::and_then([&](auto file) {
             return io::createImageFileSystem<io::ZipFileSystem>(std::move(file));
           })
           | kdl::transform(setMetadataAndCast);
  }
  return Error{"Unknown package format: " + packageFormat};
}
} // namespace

void GameFileSystem::addFileSystemPackages(
  const GameConfig& config, const std::filesystem::path& searchPath, Logger& logger)
{
  const auto& fileSystemConfig = config.fileSystemConfig;
  const auto& packageFormatConfig = fileSystemConfig.packageFormat;

  const auto& packageExtensions = packageFormatConfig.extensions;
  const auto& packageFormat = packageFormatConfig.format;

  if (io::Disk::pathInfo(searchPath) == io::PathInfo::Directory)
  {
    const auto diskFS = io::DiskFileSystem{searchPath};
    diskFS.find(
      std::filesystem::path{},
      io::TraversalMode::Flat,
      io::makeExtensionPathMatcher(packageExtensions))
      | kdl::and_then([&](auto packagePaths) {
          return kdl::vec_transform(
                   std::move(packagePaths),
                   [&](auto packagePath) {
                     return diskFS.makeAbsolute(packagePath)
                            | kdl::and_then([&](const auto& absPackagePath) {
                                return createImageFileSystem(
                                  packageFormat, absPackagePath);
                              })
                            | kdl::transform([&](auto fs) {
                                logger.info()
                                  << "Adding file system package " << packagePath;
                                mount("", std::move(fs));
                              });
                   })
                 | kdl::fold;
        })
      | kdl::transform_error([&](auto e) {
          logger.error() << "Could not add file system packages: " << e.msg;
        });
  }
}

void GameFileSystem::mountWads(
  const std::filesystem::path& rootPath,
  const std::vector<std::filesystem::path>& wadSearchPaths,
  const std::vector<std::filesystem::path>& wadPaths,
  Logger& logger)
{
  for (const auto& wadPath : wadPaths)
  {
    const auto resolvedWadPath = io::Disk::resolvePath(wadSearchPaths, wadPath);
    io::Disk::openFile(resolvedWadPath) | kdl::and_then([](auto file) {
      return io::createImageFileSystem<io::WadFileSystem>(std::move(file));
    }) | kdl::transform([&](auto fs) {
      fs->setMetadata(io::makeImageFileSystemMetadata(resolvedWadPath));
      m_wadMountPoints.push_back(mount(rootPath, std::move(fs)));
    }) | kdl::transform_error([&](auto e) {
      logger.error() << "Could not load wad file at '" << wadPath << "': " << e.msg;
    });
  }
}

void GameFileSystem::unmountWads()
{
  for (const auto& id : m_wadMountPoints)
  {
    unmount(id);
  }
  m_wadMountPoints.clear();
}

} // namespace tb::mdl
