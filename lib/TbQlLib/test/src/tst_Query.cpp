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

#include "el/ParseExpression.h"
#include "el/ParseMode.h"
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
#include "mdl/Map_Groups.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_NodeVisibility.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Node.h"
#include "mdl/Selection.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep
#include "ql/Query.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <ranges>
#include <string>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::ql
{

namespace
{

std::vector<mdl::Node*> evaluateNodeQuery(mdl::Map& map, const std::string& queryText)
{
  const auto expression = parseQuery(queryText);
  return std::get<std::vector<mdl::Node*>>(evaluateQuery(map, expression));
}

std::vector<mdl::BrushFaceHandle> evaluateFaceQuery(
  mdl::Map& map, const std::string& queryText)
{
  const auto expression = parseQuery(queryText);
  return std::get<std::vector<mdl::BrushFaceHandle>>(evaluateQuery(map, expression));
}


auto toQuery(const auto& text)
{
  return el::parseExpression(el::ParseMode::Strict, text).value();
};

auto toTextQuery(const auto& text)
{
  // every field any of the 6 node lazy maps expose, alphabetical, minus "type" (a fixed
  // discriminator literal, not free text); deliberately does not include
  // "material"/"normal", which only brush faces have, and faces are unreachable by a text
  // search (see parseQuery's doc comment)
  static const auto fields = std::vector<std::string>{
    "bounds",
    "center",
    "classname",
    "entity",
    "groupName",
    "layerName",
    "locked",
    "materials",
    "name",
    "properties",
    "tags",
    "visible",
  };

  return toQuery(fmt::format(
    "{}",
    fmt::join(
      fields | std::views::transform([&](const auto& field) {
        return fmt::format(R"({} like "{}")", field, text);
      }),
      " || ")));
};

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

  SECTION("a query matches by classname")
  {
    CHECK(
      evaluateNodeQuery(map, R"(classname == "info_player_start")")
      == std::vector<mdl::Node*>{playerStart});
  }

  SECTION("an explicit type hint narrows a query with no naturally narrowing field")
  {
    CHECK_THAT(
      evaluateNodeQuery(map, R"(name like "Floor*")"),
      UnorderedRangeEquals(std::vector<mdl::Node*>{floorLayer, floorGroup}));
    CHECK(
      evaluateNodeQuery(map, R"(type == "layer" && name like "Floor*")")
      == std::vector<mdl::Node*>{floorLayer});
  }

  SECTION("a query infers the brush/patch domain from materials")
  {
    CHECK(
      evaluateNodeQuery(map, R"(materials like "*trigger*")")
      == std::vector<mdl::Node*>{triggerBrush});
  }

  SECTION("a query infers the face domain from the singular material field")
  {
    CHECK_THAT(
      evaluateFaceQuery(map, R"(material like "*trigger*")"),
      UnorderedRangeEquals(mdl::toHandles(*triggerBrush)));
  }

  SECTION("negation is scoped to the domain the negated field infers, not everything")
  {
    // without domain narrowing, this would also match every brush/layer/group/patch,
    // since `classname` is Undefined (and hence never equal to "func_detail") there too
    // -- `classname` is bound on both World and Entity, so both are legitimately in
    // scope here (worldspawn's own classname isn't "func_detail" either)
    CHECK_THAT(
      evaluateNodeQuery(map, R"(!(classname == "func_detail"))"),
      UnorderedRangeEquals(std::vector<mdl::Node*>{playerStart, &map.worldNode()}));
  }

  SECTION("an unsatisfiable domain short-circuits to an empty result")
  {
    CHECK(evaluateNodeQuery(map, R"(classname == "x" && materials like "y")").empty());
  }

  SECTION("a boolean field on its own is a query")
  {
    mdl::lockNodes(map, {playerStart, wallBrush});

    CHECK_THAT(
      evaluateNodeQuery(map, "locked"),
      UnorderedRangeEquals(std::vector<mdl::Node*>{playerStart, wallBrush}));
  }

  SECTION("fuzzy search matches an entity by classname, and its brush as well")
  {
    // triggerBrush's owning entity is detailEntity, so the new Map/LazyMap `like`
    // case reaches `entity.classname` one level deep; wallBrush/floorBrush are owned by
    // worldspawn, whose classname isn't "func_detail", so they're excluded
    CHECK_THAT(
      evaluateNodeQuery(map, "func_detail"),
      UnorderedRangeEquals(std::vector<mdl::Node*>{detailEntity, triggerBrush}));
  }

  SECTION("fuzzy search matches an entity by property value")
  {
    // does not reach triggerBrush via entity.properties -- that's two levels deep,
    // outside the Map/LazyMap `like` case's one-level-deep scope
    CHECK(evaluateNodeQuery(map, "trap") == std::vector<mdl::Node*>{detailEntity});
  }

  SECTION("fuzzy search matches an entity by property key")
  {
    CHECK(evaluateNodeQuery(map, "message") == std::vector<mdl::Node*>{detailEntity});
  }

  SECTION("fuzzy search matches a brush by material")
  {
    CHECK(evaluateNodeQuery(map, "trigger") == std::vector<mdl::Node*>{triggerBrush});
  }

  SECTION("fuzzy search never spuriously matches via non-string fields")
  {
    CHECK(evaluateNodeQuery(map, "true").empty());
  }
}

TEST_CASE("executeQuery")
{
  using Catch::Matchers::UnorderedRangeEquals;

  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto* playerStart =
    new mdl::EntityNode{mdl::Entity{{{"classname", "info_player_start"}}}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {playerStart}}});

  auto* doorEntity = new mdl::EntityNode{mdl::Entity{{{"classname", "func_door"}}}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {doorEntity}}});

  auto* doorBrush = mdl::createBrushNode(map, "door_material");
  mdl::addNodes(map, {{doorEntity, {doorBrush}}});

  auto* combatGroup = new mdl::GroupNode{mdl::Group{"Combat"}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {combatGroup}}});

  auto* combatBrush = mdl::createBrushNode(map, "combat_material");
  mdl::addNodes(map, {{combatGroup, {combatBrush}}});

  SECTION("the current selection is replaced by the matches")
  {
    mdl::selectNodes(map, {doorBrush});

    executeQuery(map, parseQuery(R"(classname is "info_player_start")"));

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{playerStart});
  }

  SECTION("a brush entity is selected via its brushes")
  {
    // matches the entity by classname and the brush by its entity's classname
    executeQuery(map, parseQuery("func_door"));

    CHECK(map.selection().nodes == std::vector<mdl::Node*>{doorBrush});
  }

  SECTION("hidden objects are not selected")
  {
    mdl::hideNodes(map, {playerStart});

    executeQuery(map, parseQuery(R"(classname is "info_player_start")"));

    CHECK(map.selection().nodes.empty());
  }

  SECTION("locked objects are not selected")
  {
    mdl::lockNodes(map, {playerStart});

    executeQuery(map, parseQuery(R"(classname is "info_player_start")"));

    CHECK(map.selection().nodes.empty());
  }

  SECTION("objects in a closed group are not selected")
  {
    executeQuery(map, parseQuery(R"(materials like "combat_material")"));

    CHECK(map.selection().nodes.empty());
  }

  SECTION("a nested group is selected only if its containing group is opened")
  {
    auto* ambushGroup = new mdl::GroupNode{mdl::Group{"Ambush"}};
    mdl::addNodes(map, {{combatGroup, {ambushGroup}}});
    mdl::addNodes(map, {{ambushGroup, {mdl::createBrushNode(map)}}});

    SECTION("the containing group is closed")
    {
      executeQuery(map, parseQuery(R"(name is "Ambush")"));

      CHECK(map.selection().nodes.empty());
    }

    SECTION("the containing group is opened")
    {
      mdl::openGroup(map, *combatGroup);

      executeQuery(map, parseQuery(R"(name is "Ambush")"));

      CHECK(map.selection().nodes == std::vector<mdl::Node*>{ambushGroup});
    }
  }

  SECTION("the world is not selected via its contents")
  {
    executeQuery(map, parseQuery(R"(classname is "worldspawn")"));

    CHECK(map.selection().nodes.empty());
  }

  SECTION("a layer is selected via its contents")
  {
    executeQuery(map, parseQuery(R"(type is "layer")"));

    CHECK(
      map.selection().nodes
      == std::vector<mdl::Node*>{playerStart, doorBrush, combatGroup});
  }

  SECTION("matching faces are selected")
  {
    executeQuery(map, parseQuery(R"(material is "door_material")"));

    CHECK_THAT(
      map.selection().brushFaces, UnorderedRangeEquals(mdl::toHandles(*doorBrush)));
  }

  SECTION("faces of hidden brushes are not selected")
  {
    mdl::hideNodes(map, {doorBrush});

    executeQuery(map, parseQuery(R"(material is "door_material")"));

    CHECK(map.selection().brushFaces.empty());
  }
}

TEST_CASE("parseQuery")
{
  using namespace std::string_view_literals;

  SECTION("a valid query is returned as parsed")
  {
    const auto inputText = GENERATE(
      R"(classname == "info_player_start")"sv,
      "visible == false"sv,
      "visible"sv,
      "locked"sv,
      R"(name like "b")"sv,
      R"(tags contains "Detail")"sv,
      "visible is true"sv,
      "bounds intersects bbox(vec(0, 0, 0), vec(1, 1, 1))"sv,
      "center distanceTo vec(0, 0, 0)"sv,
      R"(properties.target == "door")"sv);
    CAPTURE(inputText);

    CHECK(parseQuery(inputText) == toQuery(inputText));
  }

  SECTION("text that isn't a valid query is searched for")
  {
    const auto inputText = GENERATE(
      "func_*"sv,
      "red light"sv,
      "door frame trim"sv,
      "is locked"sv,
      "door in"sv,
      "classname =="sv);
    CAPTURE(inputText);

    CHECK(parseQuery(inputText) == toTextQuery(inputText));
  }

  SECTION("a query with an unknown name is searched for")
  {
    const auto inputText = GENERATE(
      "light is red"sv,
      "clasname is 'light'"sv,
      "classname is light"sv,
      "classname is 'light' && foo(1)"sv);
    CAPTURE(inputText);

    CHECK(parseQuery(inputText) == toTextQuery(inputText));
  }

  SECTION("a single variable or literal is searched for")
  {
    const auto [inputText, expectedText] = GENERATE(table<std::string, std::string>({
      {"light", "light"},
      {" light ", "light"},
      {"classname", "classname"},
      {R"("visible")", "visible"},
      {R"("red light")", "red light"},
      {"300", "300"},
      {"true", "true"},
      {"null", "null"},
    }));
    CAPTURE(inputText);

    CHECK(parseQuery(inputText) == toTextQuery(expectedText));
  }
}

} // namespace tb::ql
