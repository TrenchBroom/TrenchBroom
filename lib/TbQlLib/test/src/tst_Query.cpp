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
  // every String, Array or Map field any of the 6 node lazy maps expose, alphabetical,
  // minus "type" (a fixed discriminator literal, not free text) and "entity" (a map of
  // fields); deliberately does not include "material", which only brush faces have, and
  // faces are unreachable by a text search (see parseQuery's doc comment)
  static const auto fields = std::vector<std::string>{
    "classname",
    "groupName",
    "layerName",
    "materials",
    "name",
    "properties",
    "tags",
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
  using namespace std::string_view_literals;
  using Catch::Matchers::UnorderedRangeEquals;

  SECTION("parseQuery")
  {
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
        R"(properties.target == "door")"sv,
        "groupName == undefined"sv);
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

    SECTION("a type name compared with type doesn't need quotes")
    {
      const auto [inputText, expectedQuery] = GENERATE(table<std::string, std::string>({
        {"type is brush", R"(type is "brush")"},
        {"brush == type", R"("brush" == type)"},
        {"type != patch", R"(type != "patch")"},
        {"type in [brush, patch]", R"(type in ["brush", "patch"])"},
        {"[layer, group] contains type", R"(["layer", "group"] contains type)"},
        {"type is entity", R"(type is "entity")"},
        {"type is brush && entity.classname is 'func_door'",
         R"(type is "brush" && entity.classname is 'func_door')"},
        {"!(type is face)", R"(!(type is "face"))"},
        {"(type is brush) == false", R"((type is "brush") == false)"},
        {"{{ type is brush -> visible }}", R"({{ type is "brush" -> visible }})"},
      }));
      CAPTURE(inputText);

      CHECK(parseQuery(inputText) == toQuery(expectedQuery));
    }

    SECTION("a query with an unknown name is searched for")
    {
      const auto inputText = GENERATE(
        "light is red"sv,
        "clasname is 'light'"sv,
        "classname is light"sv,
        "classname is 'light' && foo(1)"sv,
        "classname is brush"sv,
        "type is brushes"sv);
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
        {"-90", "-90"},
        {" +1 ", "+1"},
        {"true", "true"},
        {"null", "null"},
      }));
      CAPTURE(inputText);

      CHECK(parseQuery(inputText) == toTextQuery(expectedText));
    }

    SECTION("surrounding whitespace is ignored")
    {
      const auto [inputText, expectedText] = GENERATE(table<std::string, std::string>({
        {" func_* ", "func_*"},
        {" light is red ", "light is red"},
        {" light ", "light"},
        {R"( "red light" )", "red light"},
        {" -1 ", "-1"},
        {" (light) ", "(light)"},
      }));
      CAPTURE(inputText);

      CHECK(parseQuery(inputText) == toTextQuery(expectedText));
    }

    SECTION("empty text is a query that matches nothing")
    {
      const auto inputText = GENERATE(""sv, R"("")"sv, "''"sv);
      CAPTURE(inputText);

      CHECK(parseQuery(inputText) == toQuery("false"));
    }
  }

  SECTION("evaluateQuery")
  {
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
      // layers are only searched if nothing else can match
      CHECK(
        evaluateNodeQuery(map, R"(name like "Floor*")")
        == std::vector<mdl::Node*>{floorGroup});
      CHECK(
        evaluateNodeQuery(map, R"(type == "layer" && name like "Floor*")")
        == std::vector<mdl::Node*>{floorLayer});

      // the array is a constant, but folding it into a literal must not hide the type
      // names from domain inference
      CHECK(
        evaluateNodeQuery(map, R"(type in [layer] && name like "Floor*")")
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

    SECTION("a query that can match other objects doesn't search faces")
    {
      CHECK_THAT(
        evaluateNodeQuery(map, "type in [brush, face]"),
        UnorderedRangeEquals(
          std::vector<mdl::Node*>{triggerBrush, wallBrush, floorBrush}));

      // layers aren't searched either, since they can't be searched on their own
      CHECK(evaluateNodeQuery(map, "type in [layer, face]").empty());
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

    SECTION("each part of a query only matches the object types it applies to")
    {
      // both queries are evaluated against entities and brushes, but without specializing
      // the query for each object type, the negated classname comparison would match
      // every brush, since `classname` is Undefined there
      CHECK_THAT(
        evaluateNodeQuery(
          map, R"(!(classname is "func_detail") || materials like "*trigger*")"),
        UnorderedRangeEquals(
          std::vector<mdl::Node*>{&map.worldNode(), playerStart, triggerBrush}));
      CHECK_THAT(
        evaluateNodeQuery(
          map, R"(!(classname is "func_detail" && materials like "*wall*"))"),
        UnorderedRangeEquals(std::vector<mdl::Node*>{
          &map.worldNode(), playerStart, triggerBrush, floorBrush}));
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

    SECTION("a constant that can't be evaluated only fails the objects that evaluate it")
    {
      mdl::lockNodes(map, {playerStart, wallBrush});

      // `||` doesn't evaluate the vector for locked objects
      CHECK_THAT(
        evaluateNodeQuery(map, R"(locked || center == vec("a", 0, 0))"),
        UnorderedRangeEquals(std::vector<mdl::Node*>{playerStart, wallBrush}));
    }

    SECTION("a query that doesn't evaluate to a boolean matches nothing")
    {
      // detailEntity has a message, but a string isn't true
      CHECK(evaluateNodeQuery(map, "properties.message").empty());
    }

    SECTION("fuzzy search matches an entity by classname, but not its brushes")
    {
      // executeQuery selects triggerBrush via detailEntity
      CHECK(
        evaluateNodeQuery(map, "func_detail") == std::vector<mdl::Node*>{detailEntity});
    }

    SECTION("fuzzy search matches an entity by property value")
    {
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

    SECTION("fuzzy search doesn't match a brush by the field names of its entity")
    {
      // `entity` has the fields `classname` and `properties`
      CHECK(evaluateNodeQuery(map, "prop").empty());
    }

    SECTION("fuzzy search for empty text matches nothing")
    {
      CHECK(evaluateNodeQuery(map, R"("")").empty());
    }

    SECTION("fuzzy search never spuriously matches via non-string fields")
    {
      CHECK(evaluateNodeQuery(map, "true").empty());
    }
  }

  SECTION("executeQuery")
  {
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
      // matches the entity by classname
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

    SECTION(
      "a layer is not selected via its contents if the query can match other objects")
    {
      auto* light = new mdl::EntityNode{mdl::Entity{{
        {"classname", "light"},
        {"origin", "512 0 0"},
      }}};
      mdl::addNodes(map, {{&mdl::parentForNodes(map), {light}}});

      // the box lies between the objects at the origin and the light, but the bounds of
      // the default layer contain all of them and therefore intersect the box as well
      executeQuery(
        map, parseQuery("bounds intersects bbox(vec(256, -1, -1), vec(520, 1, 1))"));

      CHECK(map.selection().nodes == std::vector<mdl::Node*>{light});
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
}

} // namespace tb::ql
