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

#include "Quake3ShaderFileSystem.h"

#include "Assets/Quake3Shader.h"
#include "IO/File.h"
#include "IO/PathInfo.h"
#include "IO/Quake3ShaderParser.h"
#include "IO/SimpleParserStatus.h"
#include "IO/TraversalMode.h"
#include "Logger.h"

#include <kdl/path_utils.h>
#include <kdl/vector_utils.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace IO
{
Quake3ShaderFileSystem::Quake3ShaderFileSystem(
  const FileSystem& fs,
  std::filesystem::path shaderSearchPath,
  std::vector<std::filesystem::path> textureSearchPaths,
  Logger& logger)
  : ImageFileSystemBase{std::filesystem::path{}}
  , m_fs{fs}
  , m_shaderSearchPath{std::move(shaderSearchPath)}
  , m_textureSearchPaths{std::move(textureSearchPaths)}
  , m_logger{logger}
{
  initialize();
}

void Quake3ShaderFileSystem::doReadDirectory()
{
  auto shaders = loadShaders();
  linkShaders(shaders);
}

std::vector<Assets::Quake3Shader> Quake3ShaderFileSystem::loadShaders() const
{
  auto result = std::vector<Assets::Quake3Shader>{};

  if (m_fs.pathInfo(m_shaderSearchPath) == PathInfo::Directory)
  {
    const auto paths = m_fs.find(
      m_shaderSearchPath, TraversalMode::Flat, makeExtensionPathMatcher({".shader"}));
    for (const auto& path : paths)
    {
      const auto file = m_fs.openFile(path);
      auto bufferedReader = file->reader().buffer();

      try
      {
        auto parser = Quake3ShaderParser{bufferedReader.stringView()};
        auto status = SimpleParserStatus{m_logger, path.string()};
        result = kdl::vec_concat(std::move(result), parser.parse(status));
      }
      catch (const ParserException& e)
      {
        m_logger.warn() << "Skipping malformed shader file " << path << ": " << e.what();
      }
    }
  }

  m_logger.info() << "Loaded " << result.size() << " shaders";
  return result;
}

void Quake3ShaderFileSystem::linkShaders(std::vector<Assets::Quake3Shader>& shaders)
{
  auto allImages = std::vector<std::filesystem::path>{};
  for (const auto& textureSearchPath : m_textureSearchPaths)
  {
    if (m_fs.pathInfo(textureSearchPath) == PathInfo::Directory)
    {
      allImages = kdl::vec_concat(
        std::move(allImages),
        m_fs.find(
          textureSearchPath,
          TraversalMode::Recursive,
          makeExtensionPathMatcher({".tga", ".png", ".jpg", ".jpeg"})));
    }
  }

  m_logger.info() << "Linking shaders...";
  linkTextures(allImages, shaders);
  linkStandaloneShaders(shaders);
}

void Quake3ShaderFileSystem::linkTextures(
  const std::vector<std::filesystem::path>& textures,
  std::vector<Assets::Quake3Shader>& shaders)
{
  m_logger.debug() << "Linking textures...";
  for (const auto& texture : textures)
  {
    const auto shaderPath = kdl::path_remove_extension(texture);

    // Only link a shader if it has not been linked yet.
    if (pathInfo(shaderPath) != PathInfo::File)
    {
      const auto shaderIt =
        std::find_if(shaders.begin(), shaders.end(), [&shaderPath](const auto& shader) {
          return shaderPath == shader.shaderPath;
        });

      if (shaderIt != std::end(shaders))
      {
        // Found a matching shader.
        auto& shader = *shaderIt;

        auto shaderFile = std::make_shared<ObjectFile<Assets::Quake3Shader>>(shader);
        addFile(
          shaderPath, [shaderFile = std::move(shaderFile)]() { return shaderFile; });

        // Remove the shader so that we don't revisit it when linking standalone shaders.
        shaders.erase(shaderIt);
      }
      else
      {
        // No matching shader found, generate one.
        auto shader = Assets::Quake3Shader{shaderPath, texture};

        auto shaderFile =
          std::make_shared<ObjectFile<Assets::Quake3Shader>>(std::move(shader));
        addFile(
          shaderPath, [shaderFile = std::move(shaderFile)]() { return shaderFile; });
      }
    }
  }
}

void Quake3ShaderFileSystem::linkStandaloneShaders(
  std::vector<Assets::Quake3Shader>& shaders)
{
  m_logger.debug() << "Linking standalone shaders...";
  for (auto& shader : shaders)
  {
    const auto& shaderPath = shader.shaderPath;
    auto shaderFile = std::make_shared<ObjectFile<Assets::Quake3Shader>>(shader);
    addFile(shaderPath, [shaderFile = std::move(shaderFile)]() { return shaderFile; });
  }
}
} // namespace IO
} // namespace TrenchBroom
