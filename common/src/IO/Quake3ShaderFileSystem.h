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

#pragma once

#include "IO/ImageFileSystem.h"

#include <kdl/result_forward.h>

#include <filesystem>
#include <vector>

namespace TrenchBroom
{
struct Error;
class Logger;
} // namespace TrenchBroom

namespace TrenchBroom::Assets
{
class Quake3Shader;
}

namespace TrenchBroom::IO
{

/**
 * Parses Quake 3 shader scripts found in a file system and makes the shader objects
 * available as virtual files in the file system.
 *
 * Also scans for textures available at a list of search paths and generates shaders for
 * such textures which do not already have a shader by the same name.
 */
class Quake3ShaderFileSystem : public ImageFileSystemBase
{
private:
  const FileSystem& m_fs;
  std::filesystem::path m_shaderSearchPath;
  std::vector<std::filesystem::path> m_textureSearchPaths;
  Logger& m_logger;

public:
  /**
   * Creates a new instance at the given base path that uses the given file system to find
   * shaders and shader image resources. The shader search path is used to find the shader
   * scripts. The given texture search paths are recursively searched for textures, and
   * any texture found that does not have a corresponding shader will have a shader
   * generated for it.
   *
   * @param fs the filesystem to use when searching for shaders and linking image
   * resources
   * @param shaderSearchPath the path at which to search for shader scripts
   * @param textureSearchPaths the paths at which to search for texture images
   * @param logger the logger to use
   */
  Quake3ShaderFileSystem(
    const FileSystem& fs,
    std::filesystem::path shaderSearchPath,
    std::vector<std::filesystem::path> textureSearchPaths,
    Logger& logger);

private:
  kdl::result<void, Error> doReadDirectory() override;

  kdl::result<std::vector<Assets::Quake3Shader>, Error> loadShaders() const;
  kdl::result<void, Error> linkShaders(std::vector<Assets::Quake3Shader>& shaders);
  void linkTextures(
    const std::vector<std::filesystem::path>& textures,
    std::vector<Assets::Quake3Shader>& shaders);
  void linkStandaloneShaders(std::vector<Assets::Quake3Shader>& shaders);
};

} // namespace TrenchBroom::IO
