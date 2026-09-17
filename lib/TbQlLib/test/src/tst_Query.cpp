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

#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Node.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"
#include "ql/Query.h"

#include "kd/ranges/to.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <optional>
#include <ranges>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::ql
{

namespace
{

std::vector<mdl::Node*> executeNodeQuery(mdl::Map& map, const std::string& queryText)
{
  const auto expression = parseQuery(queryText).value();
  return std::get<std::vector<mdl::Node*>>(executeQuery(map, expression));
}

std::vector<mdl::BrushFaceHandle> executeFaceQuery(
  mdl::Map& map, const std::string& queryText)
{
  const auto expression = parseQuery(queryText).value();
  return std::get<std::vector<mdl::BrushFaceHandle>>(executeQuery(map, expression));
}

std::vector<mdl::Node*> executeFuzzyNodeQuery(mdl::Map& map, const std::string& inputText)
{
  return executeNodeQuery(map, queryTextFrom(inputText).value());
}

} // namespace

TEST_CASE("Query")
{
  using Catch::Matchers::UnorderedRangeEquals;

  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto* playerStart =
    new mdl::EntityNode{mdl::Entity{{{"classname", "info_player_start"}}}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {playerStart}}});

  auto* detailEntity = new mdl::EntityNode{mdl::Entity{{
    {"classname", "func_detail"},
    {"message", "Careful, trap ahead"},
  }}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {detailEntity}}});

  auto* triggerBrush = mdl::createBrushNode(map, "trigger_material");
  mdl::addNodes(map, {{detailEntity, {triggerBrush}}});

  auto* wallBrush = mdl::createBrushNode(map, "wall_material");
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {wallBrush}}});

  auto* floorLayer = new mdl::LayerNode{mdl::Layer{"Floor"}};
  mdl::addNodes(map, {{&map.worldNode(), {floorLayer}}});

  auto* floorGroup = new mdl::GroupNode{mdl::Group{"Floor"}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {floorGroup}}});

  auto* floorBrush = mdl::createBrushNode(map, "floor_material");
  mdl::addNodes(map, {{floorGroup, {floorBrush}}});

  SECTION("parseQuery reports an error for malformed input")
  {
    CHECK(parseQuery("classname ==").is_error());
  }

  SECTION("a query matches by classname")
  {
    CHECK(
      executeNodeQuery(map, R"(classname == "info_player_start")")
      == std::vector<mdl::Node*>{playerStart});
  }

  SECTION("an explicit type hint narrows a query with no naturally narrowing field")
  {
    CHECK_THAT(
      executeNodeQuery(map, R"(name like "Floor*")"),
      UnorderedRangeEquals(std::vector<mdl::Node*>{floorLayer, floorGroup}));
    CHECK(
      executeNodeQuery(map, R"(type == "layer" && name like "Floor*")")
      == std::vector<mdl::Node*>{floorLayer});
  }

  SECTION("a query infers the brush/patch domain from materials")
  {
    CHECK(
      executeNodeQuery(map, R"(materials like "*trigger*")")
      == std::vector<mdl::Node*>{triggerBrush});
  }

  SECTION("a query infers the face domain from the singular material field")
  {
    CHECK_THAT(
      executeFaceQuery(map, R"(material like "*trigger*")"),
      UnorderedRangeEquals(mdl::toHandles(*triggerBrush)));
  }

  SECTION("negation is scoped to the domain the negated field infers, not everything")
  {
    // without domain narrowing, this would also match every brush/layer/group/patch,
    // since `classname` is Undefined (and hence never equal to "func_detail") there too
    // -- `classname` is bound on both World and Entity, so both are legitimately in
    // scope here (worldspawn's own classname isn't "func_detail" either)
    CHECK_THAT(
      executeNodeQuery(map, R"(!(classname == "func_detail"))"),
      UnorderedRangeEquals(std::vector<mdl::Node*>{playerStart, &map.worldNode()}));
  }

  SECTION("an unsatisfiable domain short-circuits to an empty result")
  {
    CHECK(
      executeNodeQuery(map, R"(classname == "x" && materials like "y")")
      == std::vector<mdl::Node*>{});
  }

  SECTION(
    "fuzzy search matches an entity by classname, and its brush by owning-entity"
    " classname")
  {
    // triggerBrush's owning entity is detailEntity, so the new Map/BoundValue `like`
    // case reaches `entity.classname` one level deep; wallBrush/floorBrush are owned by
    // worldspawn, whose classname isn't "func_detail", so they're excluded
    CHECK_THAT(
      executeFuzzyNodeQuery(map, "func_detail"),
      UnorderedRangeEquals(std::vector<mdl::Node*>{detailEntity, triggerBrush}));
  }

  SECTION("fuzzy search matches an entity by property value")
  {
    // does not reach triggerBrush via entity.properties -- that's two levels deep,
    // outside the Map/BoundValue `like` case's one-level-deep scope
    CHECK(executeFuzzyNodeQuery(map, "trap") == std::vector<mdl::Node*>{detailEntity});
  }

  SECTION("fuzzy search matches an entity by property key")
  {
    CHECK(executeFuzzyNodeQuery(map, "message") == std::vector<mdl::Node*>{detailEntity});
  }

  SECTION("fuzzy search matches a brush by material")
  {
    CHECK(executeFuzzyNodeQuery(map, "trigger") == std::vector<mdl::Node*>{triggerBrush});
  }

  SECTION("fuzzy search never spuriously matches via non-string fields")
  {
    CHECK(executeFuzzyNodeQuery(map, "true") == std::vector<mdl::Node*>{});
  }
}

TEST_CASE("queryTextFrom")
{
  SECTION("empty input has no query to run")
  {
    CHECK(queryTextFrom("") == std::nullopt);
  }

  SECTION("input that looks like a query is passed through unchanged")
  {
    const auto inputText = GENERATE(
      std::string{R"(classname == "info_player_start")"},
      std::string{"visible == false"},
      std::string{"a like b"},
      std::string{R"(tags contains "Detail")"});

    CAPTURE(inputText);
    CHECK(queryTextFrom(inputText) == inputText);
  }

  SECTION("fuzzy text becomes an OR-chain across every free-text node field")
  {
    // every field any of the 6 node bindings expose, alphabetical, minus "type" (a
    // fixed discriminator literal, not free text) -- deliberately does NOT include
    // "material"/"normal" (BrushFaceBinding-only: faces are unreachable by fuzzy text,
    // see queryTextFrom's doc comment)
    const auto fields = std::vector<std::string>{
      "bounds",
      "center",
      "classname",
      "entity",
      "groupName",
      "layerName",
      "linked",
      "locked",
      "materials",
      "name",
      "properties",
      "selected",
      "tags",
      "visible",
    };

    const auto inputText = GENERATE(
      std::string{"light"},
      std::string{"func_*"},
      // "like"/"contains" only count as the keyword as a standalone word
      std::string{"likely"},
      std::string{"containskeyword"});
    CAPTURE(inputText);

    const auto expected = fmt::format(
      "{}",
      fmt::join(
        fields | std::views::transform([&](const auto& field) {
          return fmt::format(R"({} like "{}")", field, inputText);
        }),
        " || "));

    CHECK(queryTextFrom(inputText) == expected);
  }
}

} // namespace tb::ql
