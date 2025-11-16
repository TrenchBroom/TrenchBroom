/*
 Copyright (C) 2025 Kristian Duske

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

#include "MapFixture.h"
#include "TestUtils.h"
#include "io/TestEnvironment.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Assets.h"
#include "mdl/Map_Entities.h"
#include "mdl/MaterialManager.h"
#include "mdl/Observer.h"
#include "mdl/WorldNode.h"

#include <algorithm>
#include <optional>
#include <ranges>
#include <string>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("Map_Assets")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();

  SECTION("entityDefinitionFile")
  {
    using T = std::tuple<std::optional<std::string>, EntityDefinitionFileSpec>;

    const auto [entityProperty, expectedEntityDefinitionFileSpec] = GENERATE(values<T>({
      {std::nullopt, EntityDefinitionFileSpec::makeBuiltin("Quake.def")},
      {"", EntityDefinitionFileSpec::makeBuiltin("Quake.def")},
      {"asdf", EntityDefinitionFileSpec::makeBuiltin("Quake.def")},
      {"builtin:ad.fgd", EntityDefinitionFileSpec::makeBuiltin("ad.fgd")},
      {"builtin:Quake.def", EntityDefinitionFileSpec::makeBuiltin("Quake.def")},
      {"external:/Applications/Quake/Quake.fgd",
       EntityDefinitionFileSpec::makeExternal("/Applications/Quake/Quake.fgd")},
    }));

    CAPTURE(entityProperty);

    auto mockGameConfig = MockGameConfig{};
    mockGameConfig.entityConfig.defFilePaths = std::vector<std::filesystem::path>{
      "Quake.def",
      "ad.fgd",
      "Quoth.fgd",
    };

    fixture.create({.game = MockGameFixture{std::move(mockGameConfig)}});

    if (entityProperty)
    {
      setEntityProperty(map, EntityPropertyKeys::EntityDefinitions, *entityProperty);
    }

    CHECK(entityDefinitionFile(map) == expectedEntityDefinitionFileSpec);
  }

  SECTION("setEntityDefinitionFile")
  {
    auto entityDefinitionsWillChange =
      Observer<void>{map.entityDefinitionsWillChangeNotifier};
    auto entityDefinitionsDidChange =
      Observer<void>{map.entityDefinitionsDidChangeNotifier};

    using T = std::tuple<EntityDefinitionFileSpec, std::string>;

    const auto [entityDefinitionFileSpec, expectedPropertyValue] = GENERATE(values<T>({
      {EntityDefinitionFileSpec::makeBuiltin("Quake.def"), "builtin:Quake.def"},
      {EntityDefinitionFileSpec::makeBuiltin("ad.fgd"), "builtin:ad.fgd"},
      {
        EntityDefinitionFileSpec::makeExternal("/Applications/Quake/Quake.fgd"),
        "external:/Applications/Quake/Quake.fgd",
      },
    }));

    CAPTURE(entityDefinitionFileSpec);

    auto mockGameConfig = MockGameConfig{};
    mockGameConfig.entityConfig.defFilePaths = std::vector<std::filesystem::path>{
      "Quake.def",
      "ad.fgd",
      "Quoth.fgd",
    };

    fixture.create({.game = MockGameFixture{std::move(mockGameConfig)}});

    setEntityDefinitionFile(map, entityDefinitionFileSpec);

    CHECK(entityDefinitionsWillChange.called);
    CHECK(entityDefinitionsDidChange.called);

    const auto* worldNode = map.world();
    const auto& entity = worldNode->entity();
    const auto* propertyValue = entity.property(EntityPropertyKeys::EntityDefinitions);

    REQUIRE(propertyValue);
    CHECK(*propertyValue == expectedPropertyValue);
  }

  SECTION("enabledMaterialCollections")
  {
    fixture.create({.mapFormat = MapFormat::Quake2, .game = LoadGameFixture{"Quake2"}});

    REQUIRE(map.materialManager().collections().size() == 3);

    const auto* worldNode = map.world();
    REQUIRE(worldNode);

    SECTION("When no material collections are explicitly enabled")
    {
      REQUIRE(
        worldNode->entity().property(EntityPropertyKeys::EnabledMaterialCollections)
        == nullptr);

      CHECK(
        enabledMaterialCollections(map)
        == std::vector<std::filesystem::path>{
          "textures",
          "textures/e1m1",
          "textures/e1m1/f1",
        });
    }

    SECTION("When a material collection is explicitly enabled")
    {
      setEntityProperty(
        map,
        EntityPropertyKeys::EnabledMaterialCollections,
        "textures/e1m1;textures/e1m1/f1");

      CHECK(
        enabledMaterialCollections(map)
        == std::vector<std::filesystem::path>{
          "textures/e1m1",
          "textures/e1m1/f1",
        });
    }

    SECTION("Enabled material collections are sorted and unique")
    {
      setEntityProperty(
        map,
        EntityPropertyKeys::EnabledMaterialCollections,
        "textures/e1m1/f1;textures/e1m1;textures/e1m1");

      CHECK(
        enabledMaterialCollections(map)
        == std::vector<std::filesystem::path>{
          "textures/e1m1",
          "textures/e1m1/f1",
        });
    }

    SECTION("Unknown material collections are returned")
    {
      setEntityProperty(
        map,
        EntityPropertyKeys::EnabledMaterialCollections,
        "textures/unknown;textures/e1m1");

      CHECK(
        enabledMaterialCollections(map)
        == std::vector<std::filesystem::path>{
          "textures/e1m1",
          "textures/unknown",
        });
    }
  }

  SECTION("disabledMaterialCollections")
  {
    fixture.create({.mapFormat = MapFormat::Quake2, .game = LoadGameFixture{"Quake2"}});

    REQUIRE(map.materialManager().collections().size() == 3);

    const auto* worldNode = map.world();
    REQUIRE(worldNode);

    SECTION("When no material collections are explicitly enabled")
    {
      REQUIRE(
        worldNode->entity().property(EntityPropertyKeys::EnabledMaterialCollections)
        == nullptr);

      CHECK(disabledMaterialCollections(map) == std::vector<std::filesystem::path>{});
    }

    SECTION("When a material collection is explicitly enabled")
    {
      setEntityProperty(
        map, EntityPropertyKeys::EnabledMaterialCollections, "textures/e1m1");

      CHECK(
        disabledMaterialCollections(map)
        == std::vector<std::filesystem::path>{"textures", "textures/e1m1/f1"});
    }
  }

  SECTION("setEnabledMaterialCollections")
  {
    fixture.create({.mapFormat = MapFormat::Quake2, .game = LoadGameFixture{"Quake2"}});


    const auto collectionPaths =
      map.materialManager().collections()
      | std::views::transform([](const auto& collection) { return collection.path(); })
      | kdl::ranges::to<std::vector>();

    REQUIRE(collectionPaths.size() == 3);

    const auto* worldNode = map.world();
    REQUIRE(worldNode);

    const auto getEnabledMaterialCollections = [&] {
      return worldNode->entity().property(EntityPropertyKeys::EnabledMaterialCollections);
    };

    REQUIRE(!getEnabledMaterialCollections());

    SECTION("Enabling all collections resets the property")
    {
      setEnabledMaterialCollections(map, collectionPaths);
      REQUIRE(getEnabledMaterialCollections() == nullptr);
    }

    SECTION("Disabling all collections sets the property to an empty string")
    {
      setEnabledMaterialCollections(map, {});
      REQUIRE(getEnabledMaterialCollections() != nullptr);
      CHECK(*getEnabledMaterialCollections() == "");
    }

    SECTION("Setting a non-empty vector sets the property")
    {
      setEnabledMaterialCollections(map, {collectionPaths.front()});
      REQUIRE(getEnabledMaterialCollections());
      CHECK(*getEnabledMaterialCollections() == collectionPaths.front());
    }

    SECTION("Enabled material collections are sorted and unique")
    {
      setEnabledMaterialCollections(
        map, {"textures/e1m1/f1", "textures/e1m1", "textures/e1m1"});
      REQUIRE(getEnabledMaterialCollections());
      CHECK(*getEnabledMaterialCollections() == "textures/e1m1;textures/e1m1/f1");
    }
  }

  SECTION("reloadMaterialCollections")
  {
    auto materialCollectionsWillChange =
      Observer<void>{map.materialCollectionsWillChangeNotifier};
    auto materialCollectionsDidChange =
      Observer<void>{map.materialCollectionsDidChangeNotifier};

    fixture.load(
      "fixture/test/mdl/Map/reloadMaterialCollectionsQ2.map",
      {.mapFormat = MapFormat::Quake2, .game = LoadGameFixture{"Quake2"}});

    const auto faces = map.world()->defaultLayer()->children()
                       | std::views::transform([&](const auto* node) {
                           const auto* brushNode =
                             dynamic_cast<const mdl::BrushNode*>(node);
                           REQUIRE(brushNode);
                           return &brushNode->brush().faces().front();
                         })
                       | kdl::ranges::to<std::vector>();

    REQUIRE(faces.size() == 4);
    REQUIRE_THAT(
      faces | std::views::transform([](const auto* face) {
        return face->attributes().materialName();
      }),
      RangeEquals(std::vector<std::string>{
        "b_pv_v1a1", "e1m1/b_pv_v1a2", "e1m1/f1/b_rc_v4", "lavatest"}));

    REQUIRE(std::ranges::none_of(
      faces, [](const auto* face) { return face->material() == nullptr; }));

    reloadMaterialCollections(map);
    CHECK(materialCollectionsWillChange.called);
    CHECK(materialCollectionsDidChange.called);

    CHECK(std::ranges::none_of(
      faces, [](const auto* face) { return face->material() == nullptr; }));
  }

  SECTION("reloadEntityDefinitions")
  {
    auto entityDefinitionsWillChange =
      Observer<void>{map.entityDefinitionsWillChangeNotifier};
    auto entityDefinitionsDidChange =
      Observer<void>{map.entityDefinitionsDidChangeNotifier};

    const auto fgdFilename = "Test.fgd";

    auto env = io::TestEnvironment{};
    env.createFile(fgdFilename, R"x(
@SolidClass = worldspawn : "World entity"
[
  message(string) : "Text on entering the world"
]
    )x");

    fixture.create();

    setEntityDefinitionFile(
      map, EntityDefinitionFileSpec::makeExternal(env.dir() / fgdFilename));

    REQUIRE(
      entityDefinitionFile(map)
      == EntityDefinitionFileSpec::makeExternal(env.dir() / fgdFilename));

    reloadEntityDefinitions(map);

    CHECK(entityDefinitionsWillChange.called);
    CHECK(entityDefinitionsDidChange.called);
  }
}

} // namespace tb::mdl
