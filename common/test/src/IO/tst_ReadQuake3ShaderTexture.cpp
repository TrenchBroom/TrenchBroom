/*
 Copyright (C) 2023 Kristian Duske

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

#include "TestUtils.h"

#include "Assets/Texture.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/ReadQuake3ShaderTexture.h"
#include "IO/VirtualFileSystem.h"
#include "Logger.h"

#include <kdl/reflection_impl.h>
#include <kdl/result.h>
#include <kdl/result_io.h>

#include <string>

#include "Catch2.h"

namespace TrenchBroom::IO
{
namespace
{
struct TextureInfo
{
  std::string name;
  size_t width;
  size_t height;

  kdl_reflect_inline(TextureInfo, name, width, height);
};

bool operator==(const Assets::Texture& lhs, const TextureInfo& rhs)
{
  return lhs.name() == rhs.name && lhs.width() == rhs.width && lhs.height() == rhs.height;
}

bool operator==(
  const kdl::result<Assets::Texture, ReadTextureError>& lhs,
  const kdl::result<TextureInfo, ReadTextureError>& rhs)
{
  if (lhs.is_success())
  {
    return rhs.is_success() && lhs.value() == rhs.value();
  }
  return lhs.is_error() == rhs.is_error();
}
} // namespace

TEST_CASE("readQuake3ShaderTexture")
{
  auto logger = NullLogger{};

  const auto testDir =
    Disk::getCurrentWorkingDir() / Path{"fixture/test/IO/Shader/reader"};
  const auto fallbackDir =
    Disk::getCurrentWorkingDir() / Path{"fixture/test/IO/Shader/reader/fallback"};
  const auto texturePrefix = Path{"textures"};
  const auto shaderSearchPath = Path{"scripts"};
  const auto textureSearchPaths = std::vector<Path>{texturePrefix};

  auto fs = VirtualFileSystem{};
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(testDir));
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(fallbackDir));
  fs.mount(
    Path{},
    std::make_unique<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger));

  SECTION("find texture path")
  {
    CHECK(
      readQuake3ShaderTexture(
        "test/with_editor_image",
        *fs.openFile(texturePrefix / Path{"test/with_editor_image"}),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/with_editor_image", 128, 128}});

    CHECK(
      readQuake3ShaderTexture(
        "test/with_shader_path",
        *fs.openFile(texturePrefix / Path{"test/with_shader_path"}),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/with_shader_path", 64, 64}});

    CHECK(
      readQuake3ShaderTexture(
        "test/with_light_image",
        *fs.openFile(texturePrefix / Path{"test/with_light_image"}),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/with_light_image", 128, 64}});

    CHECK(
      readQuake3ShaderTexture(
        "test/with_stage_map",
        *fs.openFile(texturePrefix / Path{"test/with_stage_map"}),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/with_stage_map", 64, 128}});

    CHECK(
      readQuake3ShaderTexture(
        "test/missing_extension",
        *fs.openFile(texturePrefix / Path{"test/missing_extension"}),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/missing_extension", 128, 128}});

    CHECK(
      readQuake3ShaderTexture(
        "test/different_extension",
        *fs.openFile(texturePrefix / Path{"test/different_extension"}),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/different_extension", 128, 128}});
#
  }
}
} // namespace TrenchBroom::IO
