/*
 Copyright (C) 2024 Kristian Duske

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

#include "Assets/MaterialCollection.h"
#include "Assets/Resource.h"
#include "Assets/Texture.h"
#include "IO/DiskFileSystem.h"
#include "IO/LoadMaterialCollections.h"
#include "IO/VirtualFileSystem.h"
#include "IO/WadFileSystem.h"
#include "Logger.h"
#include "Model/GameConfig.h"
#include "TestUtils.h"

#include "kdl/reflection_impl.h"
#include "kdl/vector_utils.h"

#include <memory>
#include <ranges>

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
  std::vector<std::optional<MaterialInfo>> materials;

  kdl_reflect_inline(MaterialCollectionInfo, path, materials);
};

MaterialCollectionInfo makeMaterialCollectionInfo(
  const Assets::MaterialCollection& materialCollection)
{
  return MaterialCollectionInfo{
    materialCollection.path(),
    kdl::vec_transform(
      materialCollection.materials(),
      [](const auto& material) -> std::optional<MaterialInfo> {
        if (const auto* texture = material.texture())
        {
          return MaterialInfo{
            material.name(),
            material.texture()->width(),
            material.texture()->height(),
          };
        }
        return std::nullopt;
      })};
}

class MaterialCollectionsMatcher
  : public Catch::MatcherBase<Result<std::vector<Assets::MaterialCollection>>>
{
private:
  std::vector<MaterialCollectionInfo> m_expected;

public:
  explicit MaterialCollectionsMatcher(std::vector<MaterialCollectionInfo> expected)
    : m_expected{std::move(expected)}
  {
  }

  bool match(const Result<std::vector<Assets::MaterialCollection>>& result) const override
  {
    return result | kdl::transform([&](const auto& materialCollections) {
             return std::ranges::equal(
               materialCollections | std::views::transform(makeMaterialCollectionInfo),
               m_expected);
           })
           | kdl::transform_error([](auto) { return false; }) | kdl::value();
  }

  std::string describe() const override
  {
    auto str = std::stringstream{};
    str << "\nmatches\n" << kdl::make_streamable(m_expected);
    return str.str();
  }
};

auto MatchesMaterialCollections(std::vector<MaterialCollectionInfo> expected)
{
  return MaterialCollectionsMatcher{std::move(expected)};
}

auto createResource(Assets::ResourceLoader<Assets::Texture> resourceLoader)
{
  auto resource = std::make_shared<Assets::TextureResource>(std::move(resourceLoader));
  resource->loadSync();
  return resource;
}

} // namespace

TEST_CASE("loadMaterialCollections")
{
  auto fs = VirtualFileSystem{};
  auto logger = NullLogger{};

  const auto workDir = std::filesystem::current_path();

  SECTION("WAD file")
  {
    const auto wadPath = workDir / "fixture/test/IO/Wad/cr8_czg.wad";
    fs.mount("", std::make_unique<DiskFileSystem>(workDir)); // to find the palette
    fs.mount("textures", openFS<WadFileSystem>(wadPath));

    const auto materialConfig = Model::MaterialConfig{
      "textures",
      {".D"},
      "fixture/test/palette.lmp",
      "wad",
      "",
      {},
    };

    CHECK_THAT(
      loadMaterialCollections(fs, materialConfig, createResource, logger),
      MatchesMaterialCollections({
        {
          "textures/",
          {
            MaterialInfo{"blowjob_machine", 128, 128},
            MaterialInfo{"bongs2", 128, 128},
            MaterialInfo{"can-o-jam", 64, 64},
            MaterialInfo{"cap4can-o-jam", 64, 64},
            MaterialInfo{"coffin1", 128, 128},
            MaterialInfo{"coffin2", 128, 128},
            MaterialInfo{"cr8_czg_1", 64, 64},
            MaterialInfo{"cr8_czg_2", 64, 64},
            MaterialInfo{"cr8_czg_3", 64, 128},
            MaterialInfo{"cr8_czg_4", 64, 128},
            MaterialInfo{"cr8_czg_5", 64, 128},
            MaterialInfo{"crackpipes", 128, 128},
            MaterialInfo{"czg_backhole", 128, 128},
            MaterialInfo{"czg_fronthole", 128, 128},
            MaterialInfo{"dex_5", 128, 128},
            MaterialInfo{"eat_me", 64, 64},
            MaterialInfo{"for_sux-m-ass", 64, 64},
            MaterialInfo{"lasthopeofhuman", 128, 128},
            MaterialInfo{"polished_turd", 64, 64},
            MaterialInfo{"speedM_1", 128, 128},
            MaterialInfo{"u_get_this", 64, 64},
          },
        },
      }));
  }

  SECTION("Quake 3 shaders")
  {
    SECTION("Linking shaders with images")
    {
      SECTION("Shader with image")
      {
        const auto testDir = workDir / "fixture/test/IO/Shader/loader/shader_with_image";
        fs.mount("", std::make_unique<DiskFileSystem>(testDir));

        const auto materialConfig = Model::MaterialConfig{
          "textures",
          {".tga", ".png", ".jpg", ".jpeg"},
          "",
          std::nullopt,
          "scripts",
          {},
        };

        CHECK_THAT(
          loadMaterialCollections(fs, materialConfig, createResource, logger),
          MatchesMaterialCollections({
            {
              "textures/test",
              {
                MaterialInfo{"test/editor_image", 128, 128}, // generated for image file
                MaterialInfo{"test/some_shader", 128, 128},  // loaded from shader file
              },
            },
          }));
      }

      SECTION("Shader overrides image of same name")
      {
        const auto testDir =
          workDir / "fixture/test/IO/Shader/loader/shader_with_image_same_name";
        fs.mount("", std::make_unique<DiskFileSystem>(testDir));

        const auto materialConfig = Model::MaterialConfig{
          "textures",
          {".tga", ".png", ".jpg", ".jpeg"},
          "",
          std::nullopt,
          "scripts",
          {},
        };

        CHECK_THAT(
          loadMaterialCollections(fs, materialConfig, createResource, logger),
          MatchesMaterialCollections({
            {
              "textures/test",
              {
                MaterialInfo{"test/editor_image", 128, 128}, // generated for image file
                MaterialInfo{"test/image_exists_with_editor_image", 128, 128},
                MaterialInfo{"test/image_exists_without_editor_image", 64, 64},
              },
            },
          }));
      }

      SECTION("Shader with missing image file")
      {
        const auto testDir =
          workDir / "fixture/test/IO/Shader/loader/shader_with_missing_image";
        const auto fallbackDir = testDir / "fallback";

        // We need to mount the fallback dir so that we can find "__TB_empty.png" which is
        // automatically used when no texture can be found for a shader.
        fs.mount("", std::make_unique<DiskFileSystem>(fallbackDir));
        fs.mount("", std::make_unique<DiskFileSystem>(testDir));

        const auto materialConfig = Model::MaterialConfig{
          "textures",
          {".tga", ".png", ".jpg", ".jpeg"},
          "",
          std::nullopt,
          "scripts",
          {},
        };

        CHECK_THAT(
          loadMaterialCollections(fs, materialConfig, createResource, logger),
          MatchesMaterialCollections({
            {
              "textures/",
              {
                MaterialInfo{"__TB_empty", 32, 32}, // generated for fallback image
              },
            },
            {
              "textures/test",
              {
                MaterialInfo{"test/some_shader", 32, 32}, // loaded from shader file
              },
            },
          }));
      }
    }

    SECTION("Skip malformed shader files")
    {
      const auto testDir = workDir / "fixture/test/IO/Shader/loader/malformed_shader";
      fs.mount("", std::make_unique<DiskFileSystem>(testDir));

      const auto materialConfig = Model::MaterialConfig{
        "textures",
        {".tga", ".png", ".jpg", ".jpeg"},
        "",
        std::nullopt,
        "scripts",
        {},
      };

      CHECK_THAT(
        loadMaterialCollections(fs, materialConfig, createResource, logger),
        MatchesMaterialCollections({
          {
            "textures/test",
            {
              MaterialInfo{"test/editor_image", 128, 128}, // generated for image file
              MaterialInfo{"test/some_shader", 128, 128},  // loaded from shader file
            },
          },
        }));
    }

    SECTION("Find shader image")
    {
      const auto testDir = workDir / "fixture/test/IO/Shader/loader/find_shader_image";
      const auto fallbackDir = testDir / "fallback";

      // We need to mount the fallback dir so that we can find "__TB_empty.png" which is
      // automatically used when no texture can be found for a shader.
      fs.mount("", std::make_unique<DiskFileSystem>(fallbackDir));
      fs.mount("", std::make_unique<DiskFileSystem>(testDir));

      const auto materialConfig = Model::MaterialConfig{
        "textures",
        {".tga", ".png", ".jpg", ".jpeg"},
        "",
        std::nullopt,
        "scripts",
        {},
      };

      CHECK_THAT(
        loadMaterialCollections(fs, materialConfig, createResource, logger),
        MatchesMaterialCollections({
          {
            "textures/",
            {
              MaterialInfo{"__TB_empty", 32, 32}, // generated for fallback image
            },
          },
          {
            "textures/test",
            {
              MaterialInfo{"test/different_extension", 128, 128},
              MaterialInfo{"test/editor_image", 128, 128},
              MaterialInfo{"test/light_image", 128, 64},
              MaterialInfo{"test/missing_extension", 128, 128},
              MaterialInfo{"test/no_corresponding_image", 32, 32},
              MaterialInfo{"test/stage_map", 64, 128},
              MaterialInfo{"test/with_editor_image", 128, 128},
              MaterialInfo{"test/with_light_image", 128, 64},
              MaterialInfo{"test/with_shader_path", 64, 64},
              MaterialInfo{"test/with_stage_map", 64, 128},
            },
          },
        }));
    }
  }
}

} // namespace TrenchBroom::IO
