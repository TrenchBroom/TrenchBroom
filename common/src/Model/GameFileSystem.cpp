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

#include "GameFileSystem.h"

#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/DkPakFileSystem.h"
#include "IO/File.h"
#include "IO/IdPakFileSystem.h"
#include "IO/PathInfo.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/SystemPaths.h"
#include "IO/TraversalMode.h"
#include "IO/WadFileSystem.h"
#include "IO/ZipFileSystem.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include "kdl/result_fold.h"
#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <memory>

namespace TrenchBroom
{
namespace Model
{

void GameFileSystem::initialize(
  const GameConfig& config,
  const std::filesystem::path& gamePath,
  const std::vector<std::filesystem::path>& additionalSearchPaths,
  Logger& logger)
{
  unmountAll();
  m_shaderFS = nullptr;

  addDefaultAssetPaths(config, logger);

  if (!gamePath.empty() && IO::Disk::pathInfo(gamePath) == IO::PathInfo::Directory)
  {
    addGameFileSystems(config, gamePath, additionalSearchPaths, logger);
    addShaderFileSystem(config, logger);
  }
}

kdl::result<void, Error> GameFileSystem::reloadShaders()
{
  return m_shaderFS ? m_shaderFS->reload() : kdl::result<void, Error>{};
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
    IO::SystemPaths::findResourceDirectories(std::filesystem::path("defaults"));
  if (!config.path.empty())
  {
    defaultFolderPaths.push_back(config.path.parent_path());
  }

  for (const auto& defaultFolderPath : defaultFolderPaths)
  {
    const auto defaultAssetsPath = defaultFolderPath / std::filesystem::path("assets");
    if (IO::Disk::pathInfo(defaultAssetsPath) == IO::PathInfo::Directory)
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
  mount("", std::make_unique<IO::DiskFileSystem>(path));
}

namespace
{
kdl::result<std::unique_ptr<IO::FileSystem>, Error> createImageFileSystem(
  const std::string& packageFormat, std::filesystem::path path)
{
  if (kdl::ci::str_is_equal(packageFormat, "idpak"))
  {
    return IO::Disk::openFile(path)
      .and_then([](auto file) {
        return IO::createImageFileSystem<IO::IdPakFileSystem>(std::move(file));
      })
      .transform([](auto fs) { return std::unique_ptr<IO::FileSystem>{std::move(fs)}; });
  }
  else if (kdl::ci::str_is_equal(packageFormat, "dkpak"))
  {
    return IO::Disk::openFile(path)
      .and_then([](auto file) {
        return IO::createImageFileSystem<IO::DkPakFileSystem>(std::move(file));
      })
      .transform([](auto fs) { return std::unique_ptr<IO::FileSystem>{std::move(fs)}; });
  }
  else if (kdl::ci::str_is_equal(packageFormat, "zip"))
  {
    return IO::Disk::openFile(path)
      .and_then([](auto file) {
        return IO::createImageFileSystem<IO::ZipFileSystem>(std::move(file));
      })
      .transform([](auto fs) { return std::unique_ptr<IO::FileSystem>{std::move(fs)}; });
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

  if (IO::Disk::pathInfo(searchPath) == IO::PathInfo::Directory)
  {
    const auto diskFS = IO::DiskFileSystem{searchPath};
    diskFS
      .find(
        std::filesystem::path{},
        IO::TraversalMode::Flat,
        IO::makeExtensionPathMatcher(packageExtensions))
      .and_then([&](auto packagePaths) {
        return kdl::fold_results(
          kdl::vec_transform(std::move(packagePaths), [&](auto packagePath) {
            return diskFS.makeAbsolute(packagePath)
              .and_then([&](const auto& absPackagePath) {
                return createImageFileSystem(packageFormat, absPackagePath);
              })
              .transform([&](auto fs) {
                logger.info() << "Adding file system package " << packagePath;
                mount("", std::move(fs));
              });
          }));
      })
      .transform_error([&](auto e) {
        logger.error() << "Could not add file system packages: " << e.msg;
      });
  }
}

void GameFileSystem::addShaderFileSystem(const GameConfig& config, Logger& logger)
{
  // To support Quake 3 shaders, we add a shader file system that loads the shaders
  // and makes them available as virtual files.
  const auto& textureConfig = config.textureConfig;
  if (!textureConfig.shaderSearchPath.empty())
  {
    logger.info() << "Adding shader file system";
    auto shaderSearchPath = textureConfig.shaderSearchPath;
    auto textureSearchPaths =
      std::vector<std::filesystem::path>{textureConfig.root, "models"};

    auto shaderFs =
      IO::createImageFileSystem<IO::Quake3ShaderFileSystem>(
        *this, std::move(shaderSearchPath), std::move(textureSearchPaths), logger)
        .value();
    m_shaderFS = shaderFs.get();
    mount(std::filesystem::path{}, std::move(shaderFs));
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
    const auto mountPath = rootPath / wadPath.filename();
    const auto resolvedWadPath = IO::Disk::resolvePath(wadSearchPaths, wadPath);
    IO::Disk::openFile(resolvedWadPath)
      .and_then([](auto file) {
        return IO::createImageFileSystem<IO::WadFileSystem>(std::move(file));
      })
      .transform(
        [&](auto fs) { m_wadMountPoints.push_back(mount(mountPath, std::move(fs))); })
      .transform_error([&](auto e) {
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

} // namespace Model
} // namespace TrenchBroom
