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

#include "Assets/Material.h"
#include "Assets/MaterialManager.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/LoadMaterialCollection.h"
#include "IO/ReadMipTexture.h"
#include "IO/VirtualFileSystem.h"
#include "IO/WadFileSystem.h"
#include "Logger.h"
#include "Model/GameConfig.h"
#include "TestUtils.h"

#include "kdl/reflection_impl.h"
#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include <filesystem>
#include <string>

#include "Catch2.h"

namespace TrenchBroom::IO
{

namespace
{

struct MaterialInfo
{
  std::string name;
  size_t width;
  size_t height;

  kdl_reflect_inline(MaterialInfo, name, width, height);
};

struct MaterialCollectionInfo
{
  std::filesystem::path path;
  std::vector<MaterialInfo> materials;

  kdl_reflect_inline(MaterialCollectionInfo, path, materials);
};

std::optional<MaterialCollectionInfo> makeInfo(
  const Result<Assets::MaterialCollection>& result)
{
  return result
         | kdl::transform(
           [](const auto& materialCollection) -> std::optional<MaterialCollectionInfo> {
             return MaterialCollectionInfo{
               materialCollection.path(),
               kdl::vec_transform(
                 materialCollection.materials(), [](const auto& material) {
                   return MaterialInfo{
                     material.name(),
                     material.texture()->width(),
                     material.texture()->height(),
                   };
                 })};
           })
         | kdl::value_or(std::nullopt);
}

} // namespace

TEST_CASE("loadMaterialCollection")
{
  auto fs = VirtualFileSystem{};
  fs.mount("", std::make_unique<DiskFileSystem>(std::filesystem::current_path()));

  const auto wadPath =
    std::filesystem::current_path() / "fixture/test/IO/Wad/cr8_czg.wad";
  fs.mount("textures", openFS<WadFileSystem>(wadPath));

  auto logger = NullLogger{};

  SECTION("invalid path")
  {
    const auto materialConfig = Model::MaterialConfig{
      "textures",
      {".D"},
      "fixture/test/palette.lmp",
      "wad",
      "",
      {},
    };

    CHECK(
      loadMaterialCollection("some_other_path", fs, materialConfig, logger).is_error());
  }

  SECTION("missing palette")
  {
    const auto materialConfig = Model::MaterialConfig{
      "textures",
      {".D"},
      "fixture/test/missing.lmp",
      "wad",
      "",
      {},
    };

    CHECK(
      makeInfo(loadMaterialCollection("textures", fs, materialConfig, logger))
      == MaterialCollectionInfo{
        "textures",
        {
          {"cr8_czg_1", 32, 32},       {"cr8_czg_2", 32, 32},
          {"cr8_czg_3", 32, 32},       {"cr8_czg_4", 32, 32},
          {"cr8_czg_5", 32, 32},       {"speedM_1", 32, 32},
          {"cap4can-o-jam", 32, 32},   {"can-o-jam", 32, 32},
          {"eat_me", 32, 32},          {"coffin1", 32, 32},
          {"coffin2", 32, 32},         {"czg_fronthole", 32, 32},
          {"czg_backhole", 32, 32},    {"u_get_this", 32, 32},
          {"for_sux-m-ass", 32, 32},   {"dex_5", 32, 32},
          {"polished_turd", 32, 32},   {"crackpipes", 32, 32},
          {"bongs2", 32, 32},          {"blowjob_machine", 32, 32},
          {"lasthopeofhuman", 32, 32},
        },
      });
  }

  SECTION("loading all materials")
  {
    const auto materialConfig = Model::MaterialConfig{
      "textures",
      {".D"},
      "fixture/test/palette.lmp",
      "wad",
      "",
      {},
    };

    CHECK(
      makeInfo(loadMaterialCollection("textures", fs, materialConfig, logger))
      == MaterialCollectionInfo{
        "textures",
        {
          {"cr8_czg_1", 64, 64},
          {"cr8_czg_2", 64, 64},
          {"cr8_czg_3", 64, 128},
          {"cr8_czg_4", 64, 128},
          {"cr8_czg_5", 64, 128},
          {"speedM_1", 128, 128},
          {"cap4can-o-jam", 64, 64},
          {"can-o-jam", 64, 64},
          {"eat_me", 64, 64},
          {"coffin1", 128, 128},
          {"coffin2", 128, 128},
          {"czg_fronthole", 128, 128},
          {"czg_backhole", 128, 128},
          {"u_get_this", 64, 64},
          {"for_sux-m-ass", 64, 64},
          {"dex_5", 128, 128},
          {"polished_turd", 64, 64},
          {"crackpipes", 128, 128},
          {"bongs2", 128, 128},
          {"blowjob_machine", 128, 128},
          {"lasthopeofhuman", 128, 128},
        },
      });
  }

  SECTION("loading with exclusions")
  {
    const auto materialConfig = Model::MaterialConfig{
      "textures",
      {".D"},
      "fixture/test/palette.lmp",
      "wad",
      "",
      {"*-jam", "coffin2", "czg_*"},
    };

    CHECK(
      makeInfo(loadMaterialCollection("textures", fs, materialConfig, logger))
      == MaterialCollectionInfo{
        "textures",
        {
          {"cr8_czg_1", 64, 64},
          {"cr8_czg_2", 64, 64},
          {"cr8_czg_3", 64, 128},
          {"cr8_czg_4", 64, 128},
          {"cr8_czg_5", 64, 128},
          {"speedM_1", 128, 128},
          {"eat_me", 64, 64},
          {"coffin1", 128, 128},
          {"u_get_this", 64, 64},
          {"for_sux-m-ass", 64, 64},
          {"dex_5", 128, 128},
          {"polished_turd", 64, 64},
          {"crackpipes", 128, 128},
          {"bongs2", 128, 128},
          {"blowjob_machine", 128, 128},
          {"lasthopeofhuman", 128, 128},
        },
      });
  }
}

} // namespace TrenchBroom::IO
