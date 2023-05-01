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

#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/LoadTextureCollection.h"
#include "IO/Path.h"
#include "IO/ReadMipTexture.h"
#include "IO/VirtualFileSystem.h"
#include "IO/WadFileSystem.h"
#include "Logger.h"
#include "Model/GameConfig.h"

#include "kdl/vector_utils.h"
#include <kdl/reflection_impl.h>
#include <kdl/result.h>

#include <string>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
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

struct TextureCollectionInfo
{
  Path path;
  std::vector<TextureInfo> textures;

  kdl_reflect_inline(TextureCollectionInfo, path, textures);
};

std::optional<TextureCollectionInfo> makeInfo(
  const kdl::result<Assets::TextureCollection, LoadTextureCollectionError>& result)
{
  return result
    .transform([](const auto& textureCollection) -> std::optional<TextureCollectionInfo> {
      return TextureCollectionInfo{
        textureCollection.path(),
        kdl::vec_transform(textureCollection.textures(), [](const auto& texture) {
          return TextureInfo{
            texture.name(),
            texture.width(),
            texture.height(),
          };
        })};
    })
    .value_or(std::nullopt);
}

} // namespace

TEST_CASE("loadTextureCollection")
{
  auto fs = VirtualFileSystem{};
  fs.mount(Path{}, std::make_unique<DiskFileSystem>(Disk::getCurrentWorkingDir()));

  const auto wadPath =
    Disk::getCurrentWorkingDir() / Path{"fixture/test/IO/Wad/cr8_czg.wad"};
  fs.mount(Path{"textures"} / wadPath.back(), std::make_unique<WadFileSystem>(wadPath));

  auto logger = NullLogger{};

  SECTION("invalid path")
  {
    const auto textureConfig = Model::TextureConfig{
      Path{"textures"},
      {".D"},
      Path{"fixture/test/palette.lmp"},
      "wad",
      Path{},
      {},
    };

    CHECK(loadTextureCollection(Path{"textures/missing.wad"}, fs, textureConfig, logger)
            .is_error());
  }

  SECTION("missing palette")
  {
    const auto textureConfig = Model::TextureConfig{
      Path{"textures"},
      {".D"},
      Path{"fixture/test/missing.lmp"},
      "wad",
      Path{},
      {},
    };

    CHECK(
      makeInfo(
        loadTextureCollection(Path{"textures/cr8_czg.wad"}, fs, textureConfig, logger))
      == TextureCollectionInfo{
        Path{"textures/cr8_czg.wad"},
        {
          {"blowjob_machine", 32, 32}, {"bongs2", 32, 32},
          {"can-o-jam", 32, 32},       {"cap4can-o-jam", 32, 32},
          {"coffin1", 32, 32},         {"coffin2", 32, 32},
          {"cr8_czg_1", 32, 32},       {"cr8_czg_2", 32, 32},
          {"cr8_czg_3", 32, 32},       {"cr8_czg_4", 32, 32},
          {"cr8_czg_5", 32, 32},       {"crackpipes", 32, 32},
          {"czg_backhole", 32, 32},    {"czg_fronthole", 32, 32},
          {"dex_5", 32, 32},           {"eat_me", 32, 32},
          {"for_sux-m-ass", 32, 32},   {"lasthopeofhuman", 32, 32},
          {"polished_turd", 32, 32},   {"speedM_1", 32, 32},
          {"u_get_this", 32, 32},
        },
      });
  }

  SECTION("loading all textures")
  {
    const auto textureConfig = Model::TextureConfig{
      Path{"textures"},
      {".D"},
      Path{"fixture/test/palette.lmp"},
      "wad",
      Path{},
      {},
    };

    CHECK(
      makeInfo(
        loadTextureCollection(Path{"textures/cr8_czg.wad"}, fs, textureConfig, logger))
      == TextureCollectionInfo{
        Path{"textures/cr8_czg.wad"},
        {
          {"blowjob_machine", 128, 128}, {"bongs2", 128, 128},
          {"can-o-jam", 64, 64},         {"cap4can-o-jam", 64, 64},
          {"coffin1", 128, 128},         {"coffin2", 128, 128},
          {"cr8_czg_1", 64, 64},         {"cr8_czg_2", 64, 64},
          {"cr8_czg_3", 64, 128},        {"cr8_czg_4", 64, 128},
          {"cr8_czg_5", 64, 128},        {"crackpipes", 128, 128},
          {"czg_backhole", 128, 128},    {"czg_fronthole", 128, 128},
          {"dex_5", 128, 128},           {"eat_me", 64, 64},
          {"for_sux-m-ass", 64, 64},     {"lasthopeofhuman", 128, 128},
          {"polished_turd", 64, 64},     {"speedM_1", 128, 128},
          {"u_get_this", 64, 64},
        },
      });
  }

  SECTION("loading with texture exclusions")
  {
    const auto textureConfig = Model::TextureConfig{
      Path{"textures"},
      {".D"},
      Path{"fixture/test/palette.lmp"},
      "wad",
      Path{},
      {"*-jam", "coffin2", "czg_*"},
    };

    CHECK(
      makeInfo(
        loadTextureCollection(Path{"textures/cr8_czg.wad"}, fs, textureConfig, logger))
      == TextureCollectionInfo{
        Path{"textures/cr8_czg.wad"},
        {
          {"blowjob_machine", 128, 128},
          {"bongs2", 128, 128},
          {"coffin1", 128, 128},
          {"cr8_czg_1", 64, 64},
          {"cr8_czg_2", 64, 64},
          {"cr8_czg_3", 64, 128},
          {"cr8_czg_4", 64, 128},
          {"cr8_czg_5", 64, 128},
          {"crackpipes", 128, 128},
          {"dex_5", 128, 128},
          {"eat_me", 64, 64},
          {"for_sux-m-ass", 64, 64},
          {"lasthopeofhuman", 128, 128},
          {"polished_turd", 64, 64},
          {"speedM_1", 128, 128},
          {"u_get_this", 64, 64},
        },
      });
  }
}
} // namespace IO
} // namespace TrenchBroom
