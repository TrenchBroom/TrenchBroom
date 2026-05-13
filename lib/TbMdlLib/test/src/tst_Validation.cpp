/*
 Copyright (C) 2021 Kristian Duske
 Copyright (C) 2021 Eric Wasylishen

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

#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EmptyPropertyKeyValidator.h"
#include "mdl/EmptyPropertyValueValidator.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/GroupNode.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"
#include "mdl/WorldNodePathSeparatorValidator.h"

#include "kd/overload.h"
#include "kd/vector_utils.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
namespace
{

class AcceptAllIssues
{
public:
  bool operator()(const Issue*) const { return true; }
};

auto collectIssues(WorldNode& worldNode, const std::vector<const Validator*>& validators)
{
  auto issues = std::vector<const Issue*>{};
  worldNode.accept(kdl::overload(
    [&](auto&& thisLambda, WorldNode& w) {
      issues = kdl::vec_concat(std::move(issues), w.issues(validators));
      w.visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, LayerNode& l) {
      issues = kdl::vec_concat(std::move(issues), l.issues(validators));
      l.visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, GroupNode& g) {
      issues = kdl::vec_concat(std::move(issues), g.issues(validators));
      g.visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, EntityNode& e) {
      issues = kdl::vec_concat(std::move(issues), e.issues(validators));
      e.visitChildren(thisLambda);
    },
    [&](BrushNode& b) {
      issues = kdl::vec_concat(std::move(issues), b.issues(validators));
    },
    [&](PatchNode& p) {
      issues = kdl::vec_concat(std::move(issues), p.issues(validators));
    }));
  return issues;
}

} // namespace

TEST_CASE("Validation")
{
  using namespace Catch::Matchers;

  auto fixture = MapFixture{};
  auto& map = fixture.create({.mapFormat = MapFormat::Valve});

  map.entityDefinitionManager().setDefinitions({
    {"point_entity",
     Color{},
     "this is a point entity",
     {},
     PointEntityDefinition{vm::bbox3d{16.0}, {}, {}}},
  });

  const auto& pointEntityDefinition = map.entityDefinitionManager().definitions().front();

  SECTION("EmptyPropertyKeyValidator")
  {
    auto* entityNode = createPointEntity(map, pointEntityDefinition, vm::vec3d{0, 0, 0});

    selectNodes(map, {entityNode});
    setEntityProperty(map, "", "");
    REQUIRE(entityNode->entity().hasProperty(""));

    auto emptyPropertyKeyValidator = std::make_unique<EmptyPropertyKeyValidator>();
    auto validators = std::vector<const Validator*>{emptyPropertyKeyValidator.get()};

    const auto issues = collectIssues(map.worldNode(), validators);
    REQUIRE(issues.size() == 1);

    const auto* issue = issues.at(0);
    CHECK(issue->type() == emptyPropertyKeyValidator->type());

    auto fixes = map.worldNode().quickFixes(issue->type());
    REQUIRE(fixes.size() == 1);

    const auto* quickFix = fixes.at(0);
    quickFix->apply(map, {issue});

    // The fix should have deleted the property
    CHECK(!entityNode->entity().hasProperty(""));
  }

  SECTION("EmptyPropertyValueValidator")
  {
    auto* entityNode = createPointEntity(map, pointEntityDefinition, vm::vec3d{0, 0, 0});

    selectNodes(map, {entityNode});
    setEntityProperty(map, "", "");
    REQUIRE(entityNode->entity().hasProperty(""));

    auto emptyPropertyValueValidator = std::make_unique<EmptyPropertyValueValidator>();
    auto validators = std::vector<const Validator*>{emptyPropertyValueValidator.get()};

    const auto issues = collectIssues(map.worldNode(), validators);
    REQUIRE(issues.size() == 1);

    const auto* issue = issues.at(0);
    CHECK(issue->type() == emptyPropertyValueValidator->type());

    auto fixes = map.worldNode().quickFixes(issue->type());
    REQUIRE(fixes.size() == 1);

    const auto* quickFix = fixes.at(0);
    quickFix->apply(map, {issue});

    // The fix should have deleted the property
    CHECK(!entityNode->entity().hasProperty(""));
  }

  SECTION("WorldNodePathSeparatorValidator")
  {
    auto pathSeparatorValidator = std::make_unique<WorldNodePathSeparatorValidator>();
    auto validators = std::vector<const Validator*>{pathSeparatorValidator.get()};

    SECTION("reports backslash issues and fixes them for both wad and entity definitions")
    {
      deselectAll(map);
      selectNodes(map, {&map.worldNode()});
      setEntityProperty(map, EntityPropertyKeys::Wad, R"(id1\x.wad;mods/y.wad)");
      setEntityProperty(
        map,
        EntityPropertyKeys::TbEntityDefinitions,
        R"(external:\Applications\Quake\Quake.fgd)");
      setEntityProperty(map, EntityPropertyKeys::Targetname, R"(name\with\backslashes)");

      auto issues = collectIssues(map.worldNode(), validators);
      CHECK_THAT(
        issues | std::views::transform([](const auto& issue) { return issue->type(); }),
        RangeEquals(
          std::vector{pathSeparatorValidator->type(), pathSeparatorValidator->type()}));

      auto fixes = map.worldNode().quickFixes(pathSeparatorValidator->type());
      REQUIRE(fixes.size() == 1);

      const auto* quickFix = fixes.at(0);
      quickFix->apply(map, {issues.at(0)});
      issues = collectIssues(map.worldNode(), validators);
      REQUIRE(issues.size() == 1);
      quickFix->apply(map, {issues.at(0)});
      issues = collectIssues(map.worldNode(), validators);
      CHECK(issues.empty());

      const auto* wad = map.worldNode().entity().property(EntityPropertyKeys::Wad);
      REQUIRE(wad != nullptr);
      CHECK(*wad == "id1/x.wad;mods/y.wad");

      const auto* fgd =
        map.worldNode().entity().property(EntityPropertyKeys::TbEntityDefinitions);
      REQUIRE(fgd != nullptr);
      CHECK(*fgd == "external:/Applications/Quake/Quake.fgd");
    }

    SECTION("reports no issues when worldspawn paths use forward slashes")
    {
      deselectAll(map);
      selectNodes(map, {&map.worldNode()});
      setEntityProperty(map, EntityPropertyKeys::Wad, "id1/quake.wad;mods/tools.wad");
      setEntityProperty(
        map,
        EntityPropertyKeys::TbEntityDefinitions,
        "external:/Applications/Quake/Quake.fgd");

      CHECK(collectIssues(map.worldNode(), validators).empty());
    }

    SECTION("does not report issues for a non-worldspawn entity with backslash paths")
    {
      auto* entityNode =
        createPointEntity(map, pointEntityDefinition, vm::vec3d{0, 0, 0});
      selectNodes(map, {entityNode});
      setEntityProperty(map, EntityPropertyKeys::Wad, R"(id1\quake.wad)");

      CHECK(collectIssues(map.worldNode(), validators).empty());
    }

    SECTION("reports one issue when only wad has backslashes")
    {
      deselectAll(map);
      selectNodes(map, {&map.worldNode()});
      setEntityProperty(map, EntityPropertyKeys::Wad, R"(id1\quake.wad)");

      const auto issues = collectIssues(map.worldNode(), validators);
      REQUIRE(issues.size() == 1);
      CHECK(issues.front()->type() == pathSeparatorValidator->type());
    }

    SECTION("reports one issue when only _tb_entity_definitions has backslashes")
    {
      deselectAll(map);
      selectNodes(map, {&map.worldNode()});
      setEntityProperty(
        map, EntityPropertyKeys::TbEntityDefinitions, R"(external:\Quake\Quake.fgd)");

      const auto issues = collectIssues(map.worldNode(), validators);
      REQUIRE(issues.size() == 1);
      CHECK(issues.front()->type() == pathSeparatorValidator->type());
    }

    SECTION("reports no issues when no path properties are set")
    {
      CHECK(collectIssues(map.worldNode(), validators).empty());
    }
  }
}

} // namespace tb::mdl
