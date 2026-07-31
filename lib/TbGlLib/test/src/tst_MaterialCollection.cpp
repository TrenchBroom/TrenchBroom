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

#include "gl/Material.h"
#include "gl/MaterialCollection.h"
#include "gl/Texture.h"
#include "gl/TextureResource.h"

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

TEST_CASE("MaterialCollection")
{
  SECTION("default constructor")
  {
    const auto collection = MaterialCollection{};
    CHECK(collection.path().empty());
    CHECK(collection.materialCount() == 0u);
    CHECK(collection.materials().empty());
  }

  SECTION("constructor with materials")
  {
    auto collection = MaterialCollection{makeMaterials({"a", "b"})};
    CHECK(collection.path().empty());
    CHECK(collection.materialCount() == 2u);
    CHECK(collection.materials().size() == 2u);
  }

  SECTION("constructor with a path")
  {
    const auto collection = MaterialCollection{"some/path"};
    CHECK(collection.path() == "some/path");
    CHECK(collection.materialCount() == 0u);
  }

  SECTION("constructor with a path and materials")
  {
    auto collection = MaterialCollection{"some/path", makeMaterials({"a"})};
    CHECK(collection.path() == "some/path");
    CHECK(collection.materialCount() == 1u);
  }

  SECTION("materials is mutable")
  {
    auto collection = MaterialCollection{makeMaterials({"a"})};
    collection.materials().emplace_back("b", createTextureResource(Texture{4, 4}));

    CHECK(collection.materialCount() == 2u);
  }

  SECTION("materialByIndex")
  {
    auto collection = MaterialCollection{makeMaterials({"a", "b"})};

    REQUIRE(collection.materialByIndex(0) != nullptr);
    CHECK(collection.materialByIndex(0)->name() == "a");
    REQUIRE(collection.materialByIndex(1) != nullptr);
    CHECK(collection.materialByIndex(1)->name() == "b");
    CHECK(collection.materialByIndex(2) == nullptr);

    CHECK(std::as_const(collection).materialByIndex(0)->name() == "a");
  }

  SECTION("materialByName")
  {
    auto collection = MaterialCollection{makeMaterials({"a", "b"})};

    REQUIRE(collection.materialByName("b") != nullptr);
    CHECK(collection.materialByName("b")->name() == "b");
    CHECK(collection.materialByName("nonexistent") == nullptr);

    CHECK(std::as_const(collection).materialByName("a")->name() == "a");
  }
}

} // namespace tb::gl
