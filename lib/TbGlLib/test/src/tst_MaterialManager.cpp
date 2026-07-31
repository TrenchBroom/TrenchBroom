/*
 Copyright (C) 2026 Kristian Duske

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

#include "base/Logger.h"
#include "gl/Material.h"
#include "gl/MaterialCollection.h"
#include "gl/MaterialManager.h"
#include "gl/Texture.h"
#include "gl/TextureResource.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::gl
{
namespace
{

std::vector<Material> makeMaterials(const std::vector<std::string>& names)
{
  auto result = std::vector<Material>{};
  for (const auto& name : names)
  {
    result.emplace_back(name, createTextureResource(Texture{4, 4}));
  }
  return result;
}

} // namespace

TEST_CASE("MaterialManager")
{
  auto logger = NullLogger{};
  auto manager = MaterialManager{logger};

  SECTION("a fresh manager is empty")
  {
    CHECK(manager.materials().empty());
    CHECK(manager.collections().empty());
    CHECK(manager.material("a") == nullptr);
  }

  SECTION("setMaterialCollections")
  {
    SECTION("adds the collections and indexes their materials")
    {
      auto collections = std::vector<MaterialCollection>{};
      collections.emplace_back(makeMaterials({"a", "b"}));
      collections.emplace_back(makeMaterials({"c"}));
      manager.setMaterialCollections(std::move(collections));

      CHECK(manager.collections().size() == 2u);
      CHECK(manager.materials().size() == 3u);

      REQUIRE(manager.material("a") != nullptr);
      CHECK(manager.material("a")->name() == "a");
      REQUIRE(manager.material("c") != nullptr);
      CHECK(manager.material("c")->name() == "c");
      CHECK(manager.material("nonexistent") == nullptr);

      CHECK(std::as_const(manager).material("b") != nullptr);
    }

    SECTION("material lookup is case-insensitive")
    {
      auto collections = std::vector<MaterialCollection>{};
      collections.emplace_back(makeMaterials({"SomeMaterial"}));
      manager.setMaterialCollections(std::move(collections));

      REQUIRE(manager.material("somematerial") != nullptr);
      CHECK(manager.material("somematerial")->name() == "SomeMaterial");
    }

    SECTION("a material with the same name as an earlier one replaces it in the index")
    {
      auto collections = std::vector<MaterialCollection>{};
      collections.emplace_back(makeMaterials({"a"}));
      collections.emplace_back(makeMaterials({"a"}));
      manager.setMaterialCollections(std::move(collections));

      // the later collection's material wins and shadows the earlier one entirely
      CHECK(manager.materials().size() == 1u);
      CHECK(manager.material("a") == manager.collections()[1].materialByIndex(0));
    }
  }

  SECTION("clear removes all collections and materials")
  {
    auto collections = std::vector<MaterialCollection>{};
    collections.emplace_back(makeMaterials({"a"}));
    manager.setMaterialCollections(std::move(collections));
    REQUIRE_FALSE(manager.materials().empty());

    manager.clear();
    CHECK(manager.materials().empty());
    CHECK(manager.collections().empty());
    CHECK(manager.material("a") == nullptr);
  }

  SECTION("findMaterialsByTextureResourceId")
  {
    SECTION("finds materials whose texture resource id is in the given list")
    {
      auto collections = std::vector<MaterialCollection>{};
      collections.emplace_back(makeMaterials({"a", "b", "c"}));
      manager.setMaterialCollections(std::move(collections));

      const auto* materialA = manager.material("a");
      const auto* materialB = manager.material("b");
      REQUIRE(materialA != nullptr);
      REQUIRE(materialB != nullptr);

      const auto ids = std::vector<ResourceId>{
        materialA->textureResource().id(), materialB->textureResource().id()};
      const auto found = manager.findMaterialsByTextureResourceId(ids);

      CHECK(found.size() == 2u);
      CHECK(std::ranges::find(found, materialA) != found.end());
      CHECK(std::ranges::find(found, materialB) != found.end());
    }

    SECTION("returns nothing for an unknown id")
    {
      auto collections = std::vector<MaterialCollection>{};
      collections.emplace_back(makeMaterials({"a"}));
      manager.setMaterialCollections(std::move(collections));

      const auto otherResource = createTextureResource(Texture{4, 4});
      const auto found = manager.findMaterialsByTextureResourceId({otherResource->id()});

      CHECK(found.empty());
    }
  }
}

} // namespace tb::gl
