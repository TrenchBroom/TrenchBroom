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

#include "mdl/GameFileSystem.h"

#include "Logger.h"
#include "fs/DiskFileSystem.h"
#include "fs/DiskIO.h"
#include "fs/DkPakFileSystem.h"
#include "fs/IdPakFileSystem.h"
#include "fs/PathInfo.h"
#include "fs/SiNPakFileSystem.h"
#include "fs/TraversalMode.h"
#include "fs/WadFileSystem.h"
#include "fs/ZipFileSystem.h"
#include "mdl/EnvironmentConfig.h"
#include "mdl/GameConfig.h"

#include "kd/ranges/as_rvalue_view.h"
#include "kd/result_fold.h"
#include "kd/string_compare.h"

#include <memory>
#include <ranges>

namespace tb::mdl
{

void GameFileSystem::initialize(
  const EnvironmentConfig& environmentConfig,
  const GameConfig& gameConfig,
  const std::filesystem::path& gamePath,
  const std::vector<std::filesystem::path>& additionalSearchPaths,
  Logger& logger)
{
  unmountAll();

  addDefaultAssetPaths(environmentConfig, gameConfig, logger);

  if (!gamePath.empty() && fs::Disk::pathInfo(gamePath) == fs::PathInfo::Directory)
  {
    addGameFileSystems(gameConfig, gamePath, additionalSearchPaths, logger);
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

void GameFileSystem::addDefaultAssetPaths(
  const EnvironmentConfig& environmentConfig, const GameConfig& config, Logger& logger)
{
  // There are two ways of providing default assets: The 'defaults/assets' folder in
  // TrenchBroom's resources folder, and the 'assets' folder in the game configuration
  // folders. We add filesystems for both types here.

  auto defaultFolderPaths = environmentConfig.defaultAssetFolderPaths;
  if (!config.path.empty())
  {
    defaultFolderPaths.push_back(config.path.parent_path());
  }

  for (const auto& defaultFolderPath : defaultFolderPaths)
  {
    const auto defaultAssetsPath = defaultFolderPath / std::filesystem::path("assets");
    if (fs::Disk::pathInfo(defaultAssetsPath) == fs::PathInfo::Directory)
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
  addSearchPath(config, gamePath, fileSystemConfig.searchPath, logger);

  for (const auto& searchPath : additionalSearchPaths)
  {
    addSearchPath(config, gamePath, searchPath, logger);
  }
}

void GameFileSystem::addSearchPath(
  const GameConfig& config,
  const std::filesystem::path& gamePath,
  const std::filesystem::path& searchPath,
  Logger& logger)
{
  const auto fixedPath = fs::Disk::fixPath(gamePath / searchPath);
  addFileSystemPath(fixedPath, logger);
  addFileSystemPackages(config, fixedPath, logger);
}

void GameFileSystem::addFileSystemPath(const std::filesystem::path& path, Logger& logger)
{
  logger.info() << "Adding file system path " << path;
  mount("", std::make_unique<fs::DiskFileSystem>(path));
}

namespace
{
Result<std::unique_ptr<fs::FileSystem>> createImageFileSystem(
  const std::string& packageFormat, const std::filesystem::path& path)
{
  const auto setMetadataAndCast = [&](auto fs) {
    fs->setMetadata(fs::makeImageFileSystemMetadata(path));
    return std::unique_ptr<fs::FileSystem>{std::move(fs)};
  };

  if (kdl::ci::str_is_equal(packageFormat, "idpak"))
  {
    return fs::Disk::openFile(path) | kdl::and_then([&](auto file) {
             return fs::createImageFileSystem<fs::IdPakFileSystem>(std::move(file));
           })
           | kdl::transform(setMetadataAndCast);
  }
  else if (kdl::ci::str_is_equal(packageFormat, "dkpak"))
  {
    return fs::Disk::openFile(path) | kdl::and_then([&](auto file) {
             return fs::createImageFileSystem<fs::DkPakFileSystem>(std::move(file));
           })
           | kdl::transform(setMetadataAndCast);
  }
  else if (kdl::ci::str_is_equal(packageFormat, "sinpak"))
  {
    return fs::Disk::openFile(path) | kdl::and_then([&](auto file) {
             return fs::createImageFileSystem<fs::SiNPakFileSystem>(std::move(file));
           })
           | kdl::transform(setMetadataAndCast);
  }
  else if (kdl::ci::str_is_equal(packageFormat, "zip"))
  {
    return fs::Disk::openFile(path) | kdl::and_then([&](auto file) {
             return fs::createImageFileSystem<fs::ZipFileSystem>(std::move(file));
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

  if (fs::Disk::pathInfo(searchPath) == fs::PathInfo::Directory)
  {
    const auto diskFS = fs::DiskFileSystem{searchPath};
    diskFS.find(
      std::filesystem::path{},
      fs::TraversalMode::Flat,
      fs::makeExtensionPathMatcher(packageExtensions))
      | kdl::and_then([&](auto packagePaths) {
          std::ranges::sort(packagePaths);
          return packagePaths | kdl::views::as_rvalue
                 | std::views::transform([&](auto packagePath) {
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
    const auto resolvedWadPath = fs::Disk::resolvePath(wadSearchPaths, wadPath);
    fs::Disk::openFile(resolvedWadPath) | kdl::and_then([](auto file) {
      return fs::createImageFileSystem<fs::WadFileSystem>(std::move(file));
    }) | kdl::transform([&](auto fs) {
      fs->setMetadata(fs::makeImageFileSystemMetadata(resolvedWadPath));
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
