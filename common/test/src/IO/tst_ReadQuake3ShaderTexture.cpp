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

#include "Assets/Texture.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Quake3ShaderFileSystem.h"
#include "IO/ReadQuake3ShaderTexture.h"
#include "IO/VirtualFileSystem.h"
#include "Logger.h"
#include "TestUtils.h"

#include <kdl/reflection_impl.h>
#include <kdl/result.h>
#include <kdl/result_io.h>

#include <filesystem>
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

  const auto testDir = std::filesystem::current_path() / "fixture/test/IO/Shader/reader";
  const auto fallbackDir =
    std::filesystem::current_path() / "fixture/test/IO/Shader/reader/fallback";
  const auto texturePrefix = std::filesystem::path{"textures"};
  const auto shaderSearchPath = std::filesystem::path{"scripts"};
  const auto textureSearchPaths = std::vector<std::filesystem::path>{texturePrefix};

  auto fs = VirtualFileSystem{};
  fs.mount("", std::make_unique<DiskFileSystem>(testDir));
  fs.mount("", std::make_unique<DiskFileSystem>(fallbackDir));
  fs.mount(
    "",
    createImageFileSystem<Quake3ShaderFileSystem>(
      fs, shaderSearchPath, textureSearchPaths, logger)
      .value());

  SECTION("find texture path")
  {
    CHECK(
      readQuake3ShaderTexture(
        "test/with_editor_image",
        *fs.openFile(texturePrefix / "test/with_editor_image").value(),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/with_editor_image", 128, 128}});

    CHECK(
      readQuake3ShaderTexture(
        "test/with_shader_path",
        *fs.openFile(texturePrefix / "test/with_shader_path").value(),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/with_shader_path", 64, 64}});

    CHECK(
      readQuake3ShaderTexture(
        "test/with_light_image",
        *fs.openFile(texturePrefix / "test/with_light_image").value(),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/with_light_image", 128, 64}});

    CHECK(
      readQuake3ShaderTexture(
        "test/with_stage_map",
        *fs.openFile(texturePrefix / "test/with_stage_map").value(),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/with_stage_map", 64, 128}});

    CHECK(
      readQuake3ShaderTexture(
        "test/missing_extension",
        *fs.openFile(texturePrefix / "test/missing_extension").value(),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/missing_extension", 128, 128}});

    CHECK(
      readQuake3ShaderTexture(
        "test/different_extension",
        *fs.openFile(texturePrefix / "test/different_extension").value(),
        fs)
      == kdl::result<TextureInfo, ReadTextureError>{
        TextureInfo{"test/different_extension", 128, 128}});
#
  }
}
} // namespace TrenchBroom::IO
