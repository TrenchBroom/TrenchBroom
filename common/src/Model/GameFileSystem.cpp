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
#include "IO/FileMatcher.h"
#include "IO/IdPakFileSystem.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/SystemPaths.h"
#include "IO/ZipFileSystem.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <memory>

namespace TrenchBroom
{
namespace Model
{
GameFileSystem::GameFileSystem()
  : FileSystem()
  , m_shaderFS(nullptr)
{
}

void GameFileSystem::initialize(
  const GameConfig& config,
  const IO::Path& gamePath,
  const std::vector<IO::Path>& additionalSearchPaths,
  Logger& logger)
{
  // delete the existing file system
  releaseNext();
  m_shaderFS = nullptr;

  addDefaultAssetPaths(config, logger);

  if (!gamePath.isEmpty() && IO::Disk::directoryExists(gamePath))
  {
    addGameFileSystems(config, gamePath, additionalSearchPaths, logger);
    addShaderFileSystem(config, logger);
  }
}

void GameFileSystem::reloadShaders()
{
  if (m_shaderFS != nullptr)
  {
    m_shaderFS->reload();
  }
}

void GameFileSystem::addDefaultAssetPaths(const GameConfig& config, Logger& logger)
{
  // There are two ways of providing default assets: The 'defaults/assets' folder in
  // TrenchBroom's resources folder, and the 'assets' folder in the game configuration
  // folders. We add filesystems for both types here.

  std::vector<IO::Path> defaultFolderPaths =
    IO::SystemPaths::findResourceDirectories(IO::Path("defaults"));
  const auto& configPath = config.path;
  if (!configPath.isEmpty())
  {
    defaultFolderPaths.push_back(configPath.deleteLastComponent());
  }

  for (const auto& defaultFolderPath : defaultFolderPaths)
  {
    const auto defaultAssetsPath = defaultFolderPath + IO::Path("assets");
    auto exists = [](const IO::Path& path) {
      try
      {
        return IO::Disk::directoryExists(path);
      }
      catch (const FileSystemException&)
      {
        return false;
      }
    };
    if (exists(defaultAssetsPath))
    {
      addFileSystemPath(defaultAssetsPath, logger);
    }
  }
}

void GameFileSystem::addGameFileSystems(
  const GameConfig& config,
  const IO::Path& gamePath,
  const std::vector<IO::Path>& additionalSearchPaths,
  Logger& logger)
{
  const auto& fileSystemConfig = config.fileSystemConfig;
  addFileSystemPath(gamePath + fileSystemConfig.searchPath, logger);
  addFileSystemPackages(config, gamePath + fileSystemConfig.searchPath, logger);

  for (const auto& searchPath : additionalSearchPaths)
  {
    addFileSystemPath(gamePath + searchPath, logger);
    addFileSystemPackages(config, gamePath + searchPath, logger);
  }
}

void GameFileSystem::addFileSystemPath(const IO::Path& path, Logger& logger)
{
  try
  {
    logger.info() << "Adding file system path " << path;
    m_next = std::make_shared<IO::DiskFileSystem>(m_next, path);
  }
  catch (const FileSystemException& e)
  {
    logger.error() << "Could not add file system search path '" << path
                   << "': " << e.what();
  }
}

void GameFileSystem::addFileSystemPackages(
  const GameConfig& config, const IO::Path& searchPath, Logger& logger)
{
  const auto& fileSystemConfig = config.fileSystemConfig;
  const auto& packageFormatConfig = fileSystemConfig.packageFormat;

  const auto& packageExtensions = packageFormatConfig.extensions;
  const auto& packageFormat = packageFormatConfig.format;

  if (IO::Disk::directoryExists(searchPath))
  {
    const IO::DiskFileSystem diskFS(searchPath);
    auto packages =
      diskFS.findItems(IO::Path(""), IO::FileExtensionMatcher(packageExtensions));
    packages = kdl::vec_sort(std::move(packages), IO::Path::Less<kdl::ci::string_less>());

    for (const auto& packagePath : packages)
    {
      try
      {
        if (kdl::ci::str_is_equal(packageFormat, "idpak"))
        {
          logger.info() << "Adding file system package " << packagePath;
          m_next = std::make_shared<IO::IdPakFileSystem>(
            m_next, diskFS.makeAbsolute(packagePath));
        }
        else if (kdl::ci::str_is_equal(packageFormat, "dkpak"))
        {
          logger.info() << "Adding file system package " << packagePath;
          m_next = std::make_shared<IO::DkPakFileSystem>(
            m_next, diskFS.makeAbsolute(packagePath));
        }
        else if (kdl::ci::str_is_equal(packageFormat, "zip"))
        {
          logger.info() << "Adding file system package " << packagePath;
          m_next =
            std::make_shared<IO::ZipFileSystem>(m_next, diskFS.makeAbsolute(packagePath));
        }
      }
      catch (const std::exception& e)
      {
        logger.error() << e.what();
      }
    }
  }
}

void GameFileSystem::addShaderFileSystem(const GameConfig& config, Logger& logger)
{
  // To support Quake 3 shaders, we add a shader file system that loads the shaders
  // and makes them available as virtual files.
  const auto& textureConfig = config.textureConfig;
  const auto& textureFormat = textureConfig.format.format;
  if (kdl::ci::str_is_equal(textureFormat, "q3shader"))
  {
    logger.info() << "Adding shader file system";
    auto shaderSearchPath = textureConfig.shaderSearchPath;
    auto textureSearchPaths =
      std::vector<IO::Path>{getRootDirectory(textureConfig.package), IO::Path("models")};
    auto shaderFS = std::make_shared<IO::Quake3ShaderFileSystem>(
      m_next, std::move(shaderSearchPath), std::move(textureSearchPaths), logger);
    m_shaderFS = shaderFS.get();
    m_next = std::move(shaderFS);
  }
}

bool GameFileSystem::doDirectoryExists(const IO::Path& /* path */) const
{
  return false;
}

bool GameFileSystem::doFileExists(const IO::Path& /* path */) const
{
  return false;
}

std::vector<IO::Path> GameFileSystem::doGetDirectoryContents(
  const IO::Path& /* path */) const
{
  return std::vector<IO::Path>();
}

std::shared_ptr<IO::File> GameFileSystem::doOpenFile(const IO::Path& path) const
{
  throw FileSystemException("File not found: '" + path.asString() + "'");
}
} // namespace Model
} // namespace TrenchBroom
