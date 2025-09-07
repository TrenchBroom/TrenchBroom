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

#include "MapFixture.h"
#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/EmptyPropertyKeyValidator.h"
#include "mdl/EmptyPropertyValueValidator.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/overload.h"
#include "kdl/vector_utils.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{

class AcceptAllIssues
{
public:
  bool operator()(const Issue*) const { return true; }
};

} // namespace

TEST_CASE("Validation")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create({.mapFormat = MapFormat::Valve});

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

    auto issues = std::vector<const Issue*>{};
    map.world()->accept(kdl::overload(
      [&](auto&& thisLambda, WorldNode* worldNode) {
        issues = kdl::vec_concat(std::move(issues), worldNode->issues(validators));
        worldNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, LayerNode* layerNode) {
        issues = kdl::vec_concat(std::move(issues), layerNode->issues(validators));
        layerNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, GroupNode* groupNode) {
        issues = kdl::vec_concat(std::move(issues), groupNode->issues(validators));
        groupNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, EntityNode* entityNode_) {
        issues = kdl::vec_concat(std::move(issues), entityNode_->issues(validators));
        entityNode_->visitChildren(thisLambda);
      },
      [&](BrushNode* brushNode) {
        issues = kdl::vec_concat(std::move(issues), brushNode->issues(validators));
      },
      [&](PatchNode* patchNode) {
        issues = kdl::vec_concat(std::move(issues), patchNode->issues(validators));
      }));

    REQUIRE(issues.size() == 1);

    const auto* issue = issues.at(0);
    CHECK(issue->type() == emptyPropertyKeyValidator->type());

    auto fixes = map.world()->quickFixes(issue->type());
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

    auto issues = std::vector<const Issue*>{};
    map.world()->accept(kdl::overload(
      [&](auto&& thisLambda, WorldNode* worldNode) {
        issues = kdl::vec_concat(std::move(issues), worldNode->issues(validators));
        worldNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, LayerNode* layerNode) {
        issues = kdl::vec_concat(std::move(issues), layerNode->issues(validators));
        layerNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, GroupNode* groupNode) {
        issues = kdl::vec_concat(std::move(issues), groupNode->issues(validators));
        groupNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, EntityNode* entityNode_) {
        issues = kdl::vec_concat(std::move(issues), entityNode_->issues(validators));
        entityNode_->visitChildren(thisLambda);
      },
      [&](BrushNode* brushNode) {
        issues = kdl::vec_concat(std::move(issues), brushNode->issues(validators));
      },
      [&](PatchNode* patchNode) {
        issues = kdl::vec_concat(std::move(issues), patchNode->issues(validators));
      }));

    REQUIRE(issues.size() == 1);

    const auto* issue = issues.at(0);
    CHECK(issue->type() == emptyPropertyValueValidator->type());

    auto fixes = map.world()->quickFixes(issue->type());
    REQUIRE(fixes.size() == 1);

    const auto* quickFix = fixes.at(0);
    quickFix->apply(map, {issue});

    // The fix should have deleted the property
    CHECK(!entityNode->entity().hasProperty(""));
  }
}

} // namespace tb::mdl
