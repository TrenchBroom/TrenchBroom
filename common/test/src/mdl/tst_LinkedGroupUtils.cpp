/*
 Copyright (C) 2023 Kristian Duske

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

#include "TestUtils.h"
#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/ranges/adjacent_view.h"
#include "kdl/task_manager.h"

#include "vm/bbox.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"

#include <algorithm>
#include <numeric>
#include <ranges>
#include <vector>

#include "catch/Matchers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

namespace
{

void setGroupName(GroupNode& groupNode, const std::string& name)
{
  auto group = groupNode.group();
  group.setName(name);
  groupNode.setGroup(std::move(group));
}

auto* createPatchNode()
{
  // clang-format off
    return new PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on
}

template <typename K, typename V, typename S>
auto getValue(const std::unordered_map<K, V>& m, const S& key)
{
  const auto it = m.find(key);
  return it != m.end() ? std::optional{it->second} : std::nullopt;
}

std::unordered_map<const Node*, std::string> getLinkIds(const Node& node)
{
  auto result = std::unordered_map<const Node*, std::string>{};
  node.accept(kdl::overload(
    [](auto&& thisLambda, const WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, const GroupNode* groupNode) {
      result[groupNode] = groupNode->linkId();
      groupNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, const EntityNode* entityNode) {
      result[entityNode] = entityNode->linkId();
      entityNode->visitChildren(thisLambda);
    },
    [&](const BrushNode* brushNode) { result[brushNode] = brushNode->linkId(); },
    [&](const PatchNode* patchNode) { result[patchNode] = patchNode->linkId(); }));
  return result;
}

class LinkIdMatcher : public MatcherBase<const WorldNode&>
{
  std::vector<std::vector<const Node*>> m_expected;

public:
  explicit LinkIdMatcher(std::vector<std::vector<const Node*>> expected)
    : m_expected{std::move(expected)}
  {
  }

  bool match(const WorldNode& worldNode) const override
  {
    const auto linkIds = getLinkIds(worldNode);
    const auto count = std::accumulate(
      m_expected.begin(),
      m_expected.end(),
      size_t(0),
      [](const auto c, const auto& nodesWithSameLinkId) {
        return c + nodesWithSameLinkId.size();
      });

    const auto expectedLinkIds =
      m_expected | std::views::transform([&](const auto& nodesWithSameLinkId) {
        return getValue(linkIds, nodesWithSameLinkId.front());
      });

    return linkIds.size() == count
           && std::ranges::all_of(
             m_expected,
             [&](const auto& nodesWithSameLinkId) {
               if (nodesWithSameLinkId.empty())
               {
                 return false;
               }

               const auto linkId = getValue(linkIds, nodesWithSameLinkId.front());
               return linkId
                      && std::ranges::all_of(nodesWithSameLinkId, [&](auto* entity) {
                           return getValue(linkIds, entity) == linkId;
                         });
             })
           && std::ranges::none_of(
             expectedLinkIds | kdl::views::adjacent<2>, [](const auto& pair) {
               const auto& [linkId1, linkId2] = pair;
               return linkId1 && linkId2 && *linkId1 == *linkId2;
             });
  }

  std::string describe() const override
  {
    return "matches " + ::Catch::Detail::stringify(m_expected);
  }
};

auto MatchesLinkIds(std::vector<std::vector<const Node*>> expected)
{
  return LinkIdMatcher{std::move(expected)};
}

} // namespace

TEST_CASE("collectLinkedGroups")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto* groupNode1 = new GroupNode{Group{"Group 1"}};
  auto* groupNode2 = new GroupNode{Group{"Group 2"}};
  auto* groupNode3 = new GroupNode{Group{"Group 3"}};

  setLinkId(*groupNode1, "group1");
  setLinkId(*groupNode2, "group2");

  auto* linkedGroupNode1_1 =
    static_cast<GroupNode*>(groupNode1->cloneRecursively(worldBounds));

  auto* linkedGroupNode2_1 =
    static_cast<GroupNode*>(groupNode2->cloneRecursively(worldBounds));
  auto* linkedGroupNode2_2 =
    static_cast<GroupNode*>(groupNode2->cloneRecursively(worldBounds));

  worldNode.defaultLayer()->addChild(groupNode1);
  worldNode.defaultLayer()->addChild(groupNode2);
  worldNode.defaultLayer()->addChild(groupNode3);
  worldNode.defaultLayer()->addChild(linkedGroupNode1_1);
  worldNode.defaultLayer()->addChild(linkedGroupNode2_1);
  worldNode.defaultLayer()->addChild(linkedGroupNode2_2);

  auto* entityNode = new EntityNode{Entity{}};
  worldNode.defaultLayer()->addChild(entityNode);

  CHECK_THAT(
    collectGroupsWithLinkId({&worldNode}, "asdf"),
    UnorderedEquals(std::vector<GroupNode*>{}));
  CHECK_THAT(
    collectGroupsWithLinkId({&worldNode}, "group1"),
    UnorderedEquals(std::vector<GroupNode*>{groupNode1, linkedGroupNode1_1}));
  CHECK_THAT(
    collectGroupsWithLinkId({&worldNode}, "group2"),
    UnorderedEquals(
      std::vector<GroupNode*>{groupNode2, linkedGroupNode2_1, linkedGroupNode2_2}));
}

TEST_CASE("updateLinkedGroups")
{
  auto taskManager = kdl::task_manager{};

  SECTION("Group with one object")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto groupNode = GroupNode{Group{"name"}};
    auto* entityNode = new EntityNode{Entity{}};
    groupNode.addChild(entityNode);

    transformNode(groupNode, vm::translation_matrix(vm::vec3d{1, 0, 0}), worldBounds);
    REQUIRE(
      groupNode.group().transformation() == vm::translation_matrix(vm::vec3d{1, 0, 0}));
    REQUIRE(entityNode->entity().origin() == vm::vec3d{1, 0, 0});

    SECTION("Target group list is empty")
    {
      updateLinkedGroups(groupNode, {}, worldBounds, taskManager)
        | kdl::transform([&](const UpdateLinkedGroupsResult& r) { CHECK(r.empty()); })
        | kdl::transform_error([](const auto&) { FAIL(); });
    }

    SECTION("Target group list contains only source group")
    {
      updateLinkedGroups(groupNode, {&groupNode}, worldBounds, taskManager)
        | kdl::transform([&](const UpdateLinkedGroupsResult& r) { CHECK(r.empty()); })
        | kdl::transform_error([](const auto&) { FAIL(); });
    }

    SECTION("Update a single target group")
    {
      auto groupNodeClone = std::unique_ptr<GroupNode>{
        static_cast<GroupNode*>(groupNode.cloneRecursively(worldBounds))};
      REQUIRE(
        groupNodeClone->group().transformation()
        == vm::translation_matrix(vm::vec3d{1, 0, 0}));

      transformNode(
        *groupNodeClone, vm::translation_matrix(vm::vec3d{0, 2, 0}), worldBounds);
      REQUIRE(
        groupNodeClone->group().transformation()
        == vm::translation_matrix(vm::vec3d{1, 2, 0}));
      REQUIRE(
        static_cast<EntityNode*>(groupNodeClone->children().front())->entity().origin()
        == vm::vec3d{1, 2, 0});

      transformNode(*entityNode, vm::translation_matrix(vm::vec3d{0, 0, 3}), worldBounds);
      REQUIRE(entityNode->entity().origin() == vm::vec3d{1, 0, 3});

      updateLinkedGroups(groupNode, {groupNodeClone.get()}, worldBounds, taskManager)
        | kdl::transform([&](const UpdateLinkedGroupsResult& r) {
            CHECK(r.size() == 1u);

            const auto& p = r.front();
            const auto& [groupNodeToUpdate, newChildren] = p;

            CHECK(groupNodeToUpdate == groupNodeClone.get());
            CHECK(newChildren.size() == 1u);

            const auto* newEntityNode =
              dynamic_cast<EntityNode*>(newChildren.front().get());
            CHECK(newEntityNode != nullptr);

            CHECK(newEntityNode->entity().origin() == vm::vec3d{1, 2, 3});
          })
        | kdl::transform_error([](const auto&) { FAIL(); });
    }
  }

  SECTION("Nested group")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto outerGroupNode = GroupNode{Group{"outer"}};
    auto* innerGroupNode = new GroupNode{Group{"inner"}};
    outerGroupNode.addChild(innerGroupNode);

    auto* innerGroupEntityNode = new EntityNode{Entity{}};
    innerGroupNode->addChild(innerGroupEntityNode);

    auto innerGroupNodeClone = std::unique_ptr<GroupNode>{
      static_cast<GroupNode*>(innerGroupNode->cloneRecursively(worldBounds))};
    REQUIRE(innerGroupNodeClone->group().transformation() == vm::mat4x4d{});

    transformNode(
      *innerGroupNodeClone, vm::translation_matrix(vm::vec3d{0, 2, 0}), worldBounds);
    REQUIRE(
      innerGroupNodeClone->group().transformation()
      == vm::translation_matrix(vm::vec3d{0, 2, 0}));

    SECTION("Transforming the inner group node and updating the linked group")
    {
      transformNode(
        *innerGroupNode, vm::translation_matrix(vm::vec3d{1, 0, 0}), worldBounds);
      REQUIRE(outerGroupNode.group().transformation() == vm::mat4x4d{});
      REQUIRE(
        innerGroupNode->group().transformation()
        == vm::translation_matrix(vm::vec3d{1, 0, 0}));
      REQUIRE(innerGroupEntityNode->entity().origin() == vm::vec3d{1, 0, 0});
      REQUIRE(
        innerGroupNodeClone->group().transformation()
        == vm::translation_matrix(vm::vec3d{0, 2, 0}));

      updateLinkedGroups(
        *innerGroupNode, {innerGroupNodeClone.get()}, worldBounds, taskManager)
        | kdl::transform([&](const UpdateLinkedGroupsResult& r) {
            CHECK(r.size() == 1u);

            const auto& p = r.front();
            const auto& [groupNodeToUpdate, newChildren] = p;

            CHECK(groupNodeToUpdate == innerGroupNodeClone.get());
            CHECK(newChildren.size() == 1u);

            const auto* newEntityNode =
              dynamic_cast<EntityNode*>(newChildren.front().get());
            CHECK(newEntityNode != nullptr);

            CHECK(newEntityNode->entity().origin() == vm::vec3d{0, 2, 0});
          })
        | kdl::transform_error([](const auto&) { FAIL(); });
    }

    SECTION("Transforming the inner group node's entity and updating the linked group")
    {
      transformNode(
        *innerGroupEntityNode, vm::translation_matrix(vm::vec3d{1, 0, 0}), worldBounds);
      REQUIRE(outerGroupNode.group().transformation() == vm::mat4x4d{});
      REQUIRE(innerGroupNode->group().transformation() == vm::mat4x4d{});
      REQUIRE(innerGroupEntityNode->entity().origin() == vm::vec3d{1, 0, 0});
      REQUIRE(
        innerGroupNodeClone->group().transformation()
        == vm::translation_matrix(vm::vec3d{0, 2, 0}));

      updateLinkedGroups(
        *innerGroupNode, {innerGroupNodeClone.get()}, worldBounds, taskManager)
        | kdl::transform([&](const UpdateLinkedGroupsResult& r) {
            CHECK(r.size() == 1u);

            const auto& p = r.front();
            const auto& [groupNodeToUpdate, newChildren] = p;

            CHECK(groupNodeToUpdate == innerGroupNodeClone.get());
            CHECK(newChildren.size() == 1u);

            const auto* newEntityNode =
              dynamic_cast<EntityNode*>(newChildren.front().get());
            CHECK(newEntityNode != nullptr);

            CHECK(newEntityNode->entity().origin() == vm::vec3d{1, 2, 0});
          })
        | kdl::transform_error([](const auto&) { FAIL(); });
    }
  }

  SECTION("Recursively linked groups")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto outerGroupNode = GroupNode{Group{"outer"}};

    /*
    outerGroupNode
    */

    auto* innerGroupNode = new GroupNode{Group{"inner"}};
    outerGroupNode.addChild(innerGroupNode);

    /*
    outerGroupNode
    +- innerGroupNode
    */

    auto* innerGroupEntityNode = new EntityNode{Entity{}};
    innerGroupNode->addChild(innerGroupEntityNode);

    /*
    outerGroupNode
    +-innerGroupNode
       +-innerGroupEntityNode
    */

    auto outerGroupNodeClone = std::unique_ptr<GroupNode>{
      static_cast<GroupNode*>(outerGroupNode.cloneRecursively(worldBounds))};
    REQUIRE(outerGroupNodeClone->group().transformation() == vm::mat4x4d{});
    REQUIRE(outerGroupNodeClone->childCount() == 1u);

    /*
    outerGroupNode
    +-innerGroupNode
       +-innerGroupEntityNode
    outerGroupNodeClone
    +-innerGroupNodeClone
       +-innerGroupEntityNodeClone
    */

    auto* innerGroupNodeClone =
      dynamic_cast<GroupNode*>(outerGroupNodeClone->children().front());
    REQUIRE(innerGroupNodeClone != nullptr);
    REQUIRE(innerGroupNodeClone->childCount() == 1u);

    auto* innerGroupEntityNodeClone =
      dynamic_cast<EntityNode*>(innerGroupNodeClone->children().front());
    REQUIRE(innerGroupEntityNodeClone != nullptr);

    updateLinkedGroups(
      outerGroupNode, {outerGroupNodeClone.get()}, worldBounds, taskManager)
      | kdl::transform([&](const UpdateLinkedGroupsResult& r) {
          REQUIRE(r.size() == 1u);
          const auto& [groupNodeToUpdate, newChildren] = r.front();

          REQUIRE(groupNodeToUpdate == outerGroupNodeClone.get());
          REQUIRE(newChildren.size() == 1u);

          auto* newInnerGroupNodeClone =
            dynamic_cast<GroupNode*>(newChildren.front().get());
          CHECK(newInnerGroupNodeClone != nullptr);
          CHECK(newInnerGroupNodeClone->group() == innerGroupNode->group());
          CHECK(newInnerGroupNodeClone->childCount() == 1u);

          auto* newInnerGroupEntityNodeClone =
            dynamic_cast<EntityNode*>(newInnerGroupNodeClone->children().front());
          CHECK(newInnerGroupEntityNodeClone != nullptr);
          CHECK(newInnerGroupEntityNodeClone->entity() == innerGroupEntityNode->entity());
        })
      | kdl::transform_error([](const auto&) { FAIL(); });
  }

  SECTION("Linked group exceeds world bounds after update")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto groupNode = GroupNode{Group{"name"}};
    auto* entityNode = new EntityNode{Entity{}};
    groupNode.addChild(entityNode);

    auto groupNodeClone = std::unique_ptr<GroupNode>{
      static_cast<GroupNode*>(groupNode.cloneRecursively(worldBounds))};

    transformNode(
      *groupNodeClone, vm::translation_matrix(vm::vec3d{8192 - 8, 0, 0}), worldBounds);
    REQUIRE(
      groupNodeClone->children().front()->logicalBounds()
      == vm::bbox3d{{8192 - 16, -8, -8}, {8192, 8, 8}});

    transformNode(*entityNode, vm::translation_matrix(vm::vec3d{1, 0, 0}), worldBounds);
    REQUIRE(entityNode->entity().origin() == vm::vec3d{1, 0, 0});

    updateLinkedGroups(groupNode, {groupNodeClone.get()}, worldBounds, taskManager)
      | kdl::transform([](auto) { FAIL(); }) | kdl::transform_error([](auto e) {
          CHECK(e == Error{"Updating a linked node would exceed world bounds"});
        });
  }

  SECTION("Preserve nested group names")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto outerGroupNode = GroupNode{Group{"outerGroupNode"}};
    auto* innerGroupNode = new GroupNode{Group{"innerGroupNode"}};
    outerGroupNode.addChild(innerGroupNode);

    auto innerGroupNodeClone = std::unique_ptr<GroupNode>(
      static_cast<GroupNode*>(innerGroupNode->cloneRecursively(worldBounds)));
    setGroupName(*innerGroupNodeClone, "innerGroupNodeClone");

    auto outerGroupNodeClone = std::unique_ptr<GroupNode>(
      static_cast<GroupNode*>(outerGroupNode.cloneRecursively(worldBounds)));
    setGroupName(*outerGroupNodeClone, "outerGroupNodeClone");

    auto* innerGroupNodeNestedClone =
      static_cast<GroupNode*>(outerGroupNodeClone->children().front());
    setGroupName(*innerGroupNodeNestedClone, "innerGroupNodeNestedClone");

    /*
    outerGroupNode-------+
    +-innerGroupNode-----|-------+
    innerGroupNodeClone--|-------+
    outerGroupNodeClone--+       |
    +-innerGroupNodeNestedClone--+
     */

    SECTION(
      "Updating outerGroupNode retains the names of its linked group and the nested "
      "linked "
      "group")
    {
      updateLinkedGroups(
        outerGroupNode, {outerGroupNodeClone.get()}, worldBounds, taskManager)
        | kdl::transform([&](const UpdateLinkedGroupsResult& r) {
            REQUIRE(r.size() == 1u);

            const auto& [groupNodeToUpdate, newChildren] = r.front();
            REQUIRE(groupNodeToUpdate == outerGroupNodeClone.get());

            const auto* innerReplacement =
              static_cast<GroupNode*>(newChildren.front().get());
            CHECK(innerReplacement->name() == innerGroupNodeNestedClone->name());
          })
        | kdl::transform_error([](const auto&) { FAIL(); });
    }
  }

  SECTION("Preserve entity properties")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto sourceGroupNode = GroupNode{Group{"name"}};
    auto* sourceEntityNode = new EntityNode{Entity{}};
    sourceGroupNode.addChild(sourceEntityNode);

    auto targetGroupNode = std::unique_ptr<GroupNode>{
      static_cast<GroupNode*>(sourceGroupNode.cloneRecursively(worldBounds))};

    auto* targetEntityNode =
      static_cast<EntityNode*>(targetGroupNode->children().front());
    REQUIRE_THAT(
      targetEntityNode->entity().properties(),
      Equals(sourceEntityNode->entity().properties()));

    using T = std::tuple<
      std::vector<std::string>,
      std::vector<std::string>,
      std::vector<EntityProperty>,
      std::vector<EntityProperty>,
      std::vector<EntityProperty>>;

    // clang-format off
  const auto
  [srcProtProperties, trgtProtProperties, sourceProperties, 
                                          targetProperties, 
                                          expectedProperties ] = GENERATE(values<T>({
  // properties remain unchanged
  {{},                {},                 { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } } },

  {{},                { "some_key" },     { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } } },

  {{ "some_key" },    {},                 { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } } },

  {{ "some_key" },    { "some_key" },     { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } } },

  // property was added to source
  {{},                {},                 { { "some_key", "some_value" } },
                                          {},
                                          { { "some_key", "some_value" } } },

  {{},                { "some_key" },     { { "some_key", "some_value" } },
                                          {},
                                          {} },

  {{ "some_key" },    {},                 { { "some_key", "some_value" } },
                                          {},
                                          {} },

  {{ "some_key" },    { "some_key" },     { { "some_key", "some_value" } },
                                          {},
                                          {} },

  // property was changed in source
  {{},                {},                 { { "some_key", "other_value" } },
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "other_value" } } },

  {{ "some_key" },    {},                 { { "some_key", "other_value" } },
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } } },

  {{},                { "some_key" },     { { "some_key", "other_value" } },
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } } },

  {{ "some_key" },    { "some_key" },     { { "some_key", "other_value" } },
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } } },

  // property was removed in source
  {{},                {},                 {},
                                          { { "some_key", "some_value" } },
                                          {} },

  {{ "some_key" },    {},                 {},
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } } },

  {{},                { "some_key" },     {},
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } } },

  {{ "some_key" },    { "some_key" },     {},
                                          { { "some_key", "some_value" } },
                                          { { "some_key", "some_value" } } },
  }));
    // clang-format on

    CAPTURE(
      srcProtProperties,
      trgtProtProperties,
      sourceProperties,
      targetProperties,
      expectedProperties);

    {
      auto entity = sourceEntityNode->entity();
      entity.setProperties(sourceProperties);
      entity.setProtectedProperties(srcProtProperties);
      sourceEntityNode->setEntity(std::move(entity));
    }

    {
      auto entity = targetEntityNode->entity();
      entity.setProperties(targetProperties);
      entity.setProtectedProperties(trgtProtProperties);
      targetEntityNode->setEntity(std::move(entity));
    }

    // lambda can't capture structured bindings
    const auto expectedTargetProperties = expectedProperties;

    updateLinkedGroups(sourceGroupNode, {targetGroupNode.get()}, worldBounds, taskManager)
      | kdl::transform([&](const UpdateLinkedGroupsResult& r) {
          REQUIRE(r.size() == 1u);
          const auto& p = r.front();

          const auto& newChildren = p.second;
          REQUIRE(newChildren.size() == 1u);

          const auto* newEntityNode =
            dynamic_cast<EntityNode*>(newChildren.front().get());
          REQUIRE(newEntityNode != nullptr);

          CHECK_THAT(
            newEntityNode->entity().properties(),
            UnorderedEquals(expectedTargetProperties));
          CHECK_THAT(
            newEntityNode->entity().protectedProperties(),
            UnorderedEquals(targetEntityNode->entity().protectedProperties()));
        })
      | kdl::transform_error([](const auto&) { FAIL(); });
  }

  SECTION("Preserving entity properties after structural changes")
  {
    // see https://github.com/TrenchBroom/TrenchBroom/issues/4257

    const auto worldBounds = vm::bbox3d{8192.0};
    const auto brushBuilder = BrushBuilder{MapFormat::Quake3, worldBounds};

    auto sourceGroupNode = GroupNode{Group{"name"}};
    auto* sourceBrushNode =
      new BrushNode{brushBuilder.createCube(64.0, "material") | kdl::value()};
    auto* sourceEntityNode = new EntityNode{Entity{{
      {"light", "400"},
    }}};

    sourceGroupNode.addChildren({sourceBrushNode, sourceEntityNode});

    auto targetGroupNode = std::unique_ptr<GroupNode>{
      static_cast<GroupNode*>(sourceGroupNode.cloneRecursively(worldBounds))};

    auto* targetEntityNode =
      dynamic_cast<EntityNode*>(targetGroupNode->children().back());
    REQUIRE(targetEntityNode != nullptr);
    REQUIRE(targetEntityNode->entity() == sourceEntityNode->entity());

    {
      auto targetEntity = targetEntityNode->entity();
      targetEntity.setProtectedProperties({"light"});
      targetEntity.addOrUpdateProperty("light", "500");
      targetEntityNode->setEntity(std::move(targetEntity));
    }

    auto* sourceBrushEntity = new EntityNode{Entity{}};
    sourceGroupNode.removeChild(sourceBrushNode);
    sourceBrushEntity->addChildren({sourceBrushNode});
    sourceGroupNode.addChildren({sourceBrushEntity});

    updateLinkedGroups(sourceGroupNode, {targetGroupNode.get()}, worldBounds, taskManager)
      | kdl::transform([&](const UpdateLinkedGroupsResult& r) {
          REQUIRE(r.size() == 1u);
          const auto& p = r.front();

          const auto& newChildren = p.second;
          REQUIRE(newChildren.size() == 2u);

          const auto* newEntityNode =
            dynamic_cast<EntityNode*>(newChildren.front().get());
          REQUIRE(newEntityNode != nullptr);

          CHECK_THAT(
            newEntityNode->entity().properties(),
            UnorderedEquals(std::vector<EntityProperty>{
              {"light", "500"},
            }));
          CHECK_THAT(
            newEntityNode->entity().protectedProperties(),
            UnorderedEquals(std::vector<std::string>{"light"}));
        })
      | kdl::transform_error([](const auto&) { FAIL(); });
  }
}

TEST_CASE("initializeLinkIds")
{
  auto brushBuilder = BrushBuilder{MapFormat::Quake3, vm::bbox3d{8192.0}};

  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto& layerNode = *worldNode.defaultLayer();

  auto* unlinkedGroupNode = new GroupNode{Group{"unlinked"}};
  auto* unlinkedEntityNode = new EntityNode{Entity{}};

  unlinkedGroupNode->addChildren({unlinkedEntityNode});
  layerNode.addChildren({unlinkedGroupNode});

  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* outerEntityNode = new EntityNode{Entity{}};
  auto* outerBrushNode =
    new BrushNode{brushBuilder.createCube(64.0, "material") | kdl::value()};

  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* innerPatchNode = createPatchNode();
  auto* innerEntityNode = new EntityNode{Entity{}};

  innerGroupNode->addChildren({innerPatchNode, innerEntityNode});
  outerGroupNode->addChildren({outerEntityNode, outerBrushNode, innerGroupNode});

  auto* linkedOuterGroupNode = new GroupNode{Group{"outer"}};
  auto* linkedOuterEntityNode = new EntityNode{Entity{}};
  auto* linkedOuterBrushNode =
    new BrushNode{brushBuilder.createCube(64.0, "material") | kdl::value()};

  auto* linkedInnerGroupNode = new GroupNode{Group{"inner"}};
  auto* linkedInnerPatchNode = createPatchNode();
  auto* linkedInnerEntityNode = new EntityNode{Entity{}};

  setLinkId(*outerGroupNode, "outerGroupLinkId");
  setLinkId(*linkedOuterGroupNode, "outerGroupLinkId");
  setLinkId(*innerGroupNode, "innerGroupLinkId");
  setLinkId(*linkedInnerGroupNode, "innerGroupLinkId");

  layerNode.addChildren({outerGroupNode, linkedOuterGroupNode});

  SECTION("If both groups have the same structure")
  {
    linkedInnerGroupNode->addChildren({linkedInnerPatchNode, linkedInnerEntityNode});
    linkedOuterGroupNode->addChildren(
      {linkedOuterEntityNode, linkedOuterBrushNode, linkedInnerGroupNode});

    REQUIRE_THAT(
      worldNode,
      MatchesLinkIds({
        {unlinkedGroupNode},
        {unlinkedEntityNode},
        {outerGroupNode, linkedOuterGroupNode},
        {outerEntityNode},
        {outerBrushNode},
        {innerGroupNode, linkedInnerGroupNode},
        {innerEntityNode},
        {innerPatchNode},
        {linkedOuterEntityNode},
        {linkedOuterBrushNode},
        {linkedInnerEntityNode},
        {linkedInnerPatchNode},
      }));

    SECTION("With two groups")
    {
      CHECK(initializeLinkIds({&worldNode}).empty());

      CHECK_THAT(
        worldNode,
        MatchesLinkIds({
          {unlinkedGroupNode},
          {unlinkedEntityNode},
          {outerGroupNode, linkedOuterGroupNode},
          {outerEntityNode, linkedOuterEntityNode},
          {outerBrushNode, linkedOuterBrushNode},
          {innerGroupNode, linkedInnerGroupNode},
          {innerEntityNode, linkedInnerEntityNode},
          {innerPatchNode, linkedInnerPatchNode},
        }));
    }

    SECTION("With three groups")
    {
      auto* linkedOuterGroupNode2 = new GroupNode{Group{"outer"}};
      auto* linkedOuterEntityNode2 = new EntityNode{Entity{}};
      auto* linkedOuterBrushNode2 =
        new BrushNode{brushBuilder.createCube(64.0, "material") | kdl::value()};

      auto* linkedInnerGroupNode2 = new GroupNode{Group{"inner"}};
      auto* linkedInnerPatchNode2 = createPatchNode();
      auto* linkedInnerEntityNode2 = new EntityNode{Entity{}};

      linkedInnerGroupNode2->addChildren({linkedInnerPatchNode2, linkedInnerEntityNode2});
      linkedOuterGroupNode2->addChildren(
        {linkedOuterEntityNode2, linkedOuterBrushNode2, linkedInnerGroupNode2});
      layerNode.addChildren({linkedOuterGroupNode2});

      setLinkId(*linkedOuterGroupNode2, "outerGroupLinkId");
      setLinkId(*linkedInnerGroupNode2, "innerGroupLinkId");

      CHECK(initializeLinkIds({&worldNode}).empty());

      CHECK_THAT(
        worldNode,
        MatchesLinkIds({
          {unlinkedGroupNode},
          {unlinkedEntityNode},
          {outerGroupNode, linkedOuterGroupNode, linkedOuterGroupNode2},
          {outerEntityNode, linkedOuterEntityNode, linkedOuterEntityNode2},
          {outerBrushNode, linkedOuterBrushNode, linkedOuterBrushNode2},
          {innerGroupNode, linkedInnerGroupNode, linkedInnerGroupNode2},
          {innerEntityNode, linkedInnerEntityNode, linkedInnerEntityNode2},
          {innerPatchNode, linkedInnerPatchNode, linkedInnerPatchNode2},
        }));
    }

    SECTION("If inner groups have different link IDs")
    {
      setLinkId(*linkedInnerGroupNode, "someOtherId");

      CHECK(initializeLinkIds({&worldNode}).empty());

      CHECK_THAT(
        worldNode,
        MatchesLinkIds({
          {unlinkedGroupNode},
          {unlinkedEntityNode},
          {outerGroupNode, linkedOuterGroupNode},
          {outerEntityNode, linkedOuterEntityNode},
          {outerBrushNode, linkedOuterBrushNode},
          {innerGroupNode, linkedInnerGroupNode},
          {innerEntityNode, linkedInnerEntityNode},
          {innerPatchNode, linkedInnerPatchNode},
        }));
    }

    SECTION("If a nested group is linked to a top level duplicate")
    {
      auto* topLevelLinkedInnerGroupNode = new GroupNode{Group{"inner"}};
      auto* topLevelLinkedInnerPatchNode = createPatchNode();
      auto* topLevelLinkedInnerEntityNode = new EntityNode{Entity{}};

      topLevelLinkedInnerGroupNode->addChildren(
        {topLevelLinkedInnerPatchNode, topLevelLinkedInnerEntityNode});

      setLinkId(*topLevelLinkedInnerGroupNode, "innerGroupLinkId");
      layerNode.addChildren({topLevelLinkedInnerGroupNode});

      REQUIRE_THAT(
        worldNode,
        MatchesLinkIds({
          {unlinkedGroupNode},
          {unlinkedEntityNode},
          {outerGroupNode, linkedOuterGroupNode},
          {outerEntityNode},
          {outerBrushNode},
          {innerGroupNode, linkedInnerGroupNode, topLevelLinkedInnerGroupNode},
          {innerEntityNode},
          {innerPatchNode},
          {linkedOuterEntityNode},
          {linkedOuterBrushNode},
          {linkedInnerEntityNode},
          {linkedInnerPatchNode},
          {topLevelLinkedInnerEntityNode},
          {topLevelLinkedInnerPatchNode},
        }));

      CHECK(initializeLinkIds({&worldNode}).empty());

      CHECK_THAT(
        worldNode,
        MatchesLinkIds({
          {unlinkedGroupNode},
          {unlinkedEntityNode},
          {outerGroupNode, linkedOuterGroupNode},
          {outerEntityNode, linkedOuterEntityNode},
          {outerBrushNode, linkedOuterBrushNode},
          {innerGroupNode, linkedInnerGroupNode, topLevelLinkedInnerGroupNode},
          {innerEntityNode, linkedInnerEntityNode, topLevelLinkedInnerEntityNode},
          {innerPatchNode, linkedInnerPatchNode, topLevelLinkedInnerPatchNode},
        }));
    }
  }

  SECTION("If the groups have a structural mismatch")
  {
    SECTION("One outer group node has no children")
    {
      CHECK(
        initializeLinkIds({&worldNode})
        == std::vector{Error{"Inconsistent linked group structure"}});

      CHECK_THAT(
        worldNode,
        MatchesLinkIds({
          {unlinkedGroupNode},
          {unlinkedEntityNode},
          {outerGroupNode},
          {outerEntityNode},
          {outerBrushNode},
          {innerGroupNode},
          {innerEntityNode},
          {innerPatchNode},
          {linkedOuterGroupNode},
        }));
    }

    SECTION("One outer group node has fewer children")
    {
      linkedOuterGroupNode->addChildren({linkedOuterEntityNode, linkedOuterBrushNode});

      CHECK(
        initializeLinkIds({&worldNode})
        == std::vector{Error{"Inconsistent linked group structure"}});

      CHECK_THAT(
        worldNode,
        MatchesLinkIds({
          {unlinkedGroupNode},
          {unlinkedEntityNode},
          {outerGroupNode},
          {outerEntityNode},
          {outerBrushNode},
          {innerGroupNode},
          {innerEntityNode},
          {innerPatchNode},
          {linkedOuterGroupNode},
          {linkedOuterEntityNode},
          {linkedOuterBrushNode},
        }));
    }

    SECTION("One inner group node has fewer children")
    {
      linkedOuterGroupNode->addChildren(
        {linkedOuterEntityNode, linkedOuterBrushNode, linkedInnerGroupNode});
      linkedInnerGroupNode->addChildren({linkedInnerPatchNode});

      CHECK_THAT(
        initializeLinkIds({&worldNode}),
        UnorderedEquals(std::vector{
          Error{"Inconsistent linked group structure"},
          Error{"Inconsistent linked group structure"}}));

      CHECK_THAT(
        worldNode,
        MatchesLinkIds({
          {unlinkedGroupNode},
          {unlinkedEntityNode},
          {outerGroupNode},
          {outerEntityNode},
          {outerBrushNode},
          {innerGroupNode},
          {innerEntityNode},
          {innerPatchNode},
          {linkedOuterGroupNode},
          {linkedOuterEntityNode},
          {linkedOuterBrushNode},
          {linkedInnerGroupNode},
          {linkedInnerPatchNode},
        }));
    }

    SECTION("One outer group node has children in different order")
    {
      linkedInnerGroupNode->addChildren({linkedInnerPatchNode, linkedInnerEntityNode});
      linkedOuterGroupNode->addChildren(
        {linkedOuterEntityNode, linkedInnerGroupNode, linkedOuterBrushNode});

      CHECK(
        initializeLinkIds({&worldNode})
        == std::vector{Error{"Inconsistent linked group structure"}});

      CHECK_THAT(
        worldNode,
        MatchesLinkIds({
          {unlinkedGroupNode},
          {unlinkedEntityNode},
          {outerGroupNode},
          {outerEntityNode},
          {outerBrushNode},
          {innerGroupNode, linkedInnerGroupNode},
          {innerEntityNode, linkedInnerEntityNode},
          {innerPatchNode, linkedInnerPatchNode},
          {linkedOuterGroupNode},
          {linkedOuterEntityNode},
          {linkedOuterBrushNode},
        }));
    }

    SECTION("One inner group node has children in different order")
    {
      linkedInnerGroupNode->addChildren({linkedInnerEntityNode, linkedInnerPatchNode});
      linkedOuterGroupNode->addChildren(
        {linkedOuterEntityNode, linkedOuterBrushNode, linkedInnerGroupNode});

      CHECK_THAT(
        initializeLinkIds({&worldNode}),
        UnorderedEquals(std::vector{
          Error{"Inconsistent linked group structure"},
          Error{"Inconsistent linked group structure"}}));

      CHECK_THAT(
        worldNode,
        MatchesLinkIds({
          {unlinkedGroupNode},
          {unlinkedEntityNode},
          {outerGroupNode},
          {outerEntityNode},
          {outerBrushNode},
          {innerGroupNode},
          {innerEntityNode},
          {innerPatchNode},
          {linkedOuterGroupNode},
          {linkedOuterEntityNode},
          {linkedOuterBrushNode},
          {linkedInnerGroupNode},
          {linkedInnerEntityNode},
          {linkedInnerPatchNode},
        }));
    }
  }
}

TEST_CASE("resetLinkIds")
{
  const auto worldBounds = vm::bbox3d{8192.0};
  auto brushBuilder = BrushBuilder{MapFormat::Quake3, worldBounds};

  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* outerEntityNode = new EntityNode{Entity{}};
  auto* outerBrushNode =
    new BrushNode{brushBuilder.createCube(64.0, "material") | kdl::value()};

  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* innerPatchNode = createPatchNode();
  auto* innerEntityNode = new EntityNode{Entity{}};

  innerGroupNode->addChildren({innerPatchNode, innerEntityNode});
  outerGroupNode->addChildren({outerEntityNode, outerBrushNode, innerGroupNode});

  auto* linkedOuterGroupNode =
    dynamic_cast<GroupNode*>(outerGroupNode->cloneRecursively(worldBounds));
  REQUIRE(linkedOuterGroupNode != nullptr);

  auto* linkedOuterEntityNode =
    dynamic_cast<EntityNode*>(linkedOuterGroupNode->children().at(0));
  REQUIRE(linkedOuterEntityNode != nullptr);

  auto* linkedOuterBrushNode =
    dynamic_cast<BrushNode*>(linkedOuterGroupNode->children().at(1));
  REQUIRE(linkedOuterBrushNode != nullptr);

  auto* linkedInnerGroupNode =
    dynamic_cast<GroupNode*>(linkedOuterGroupNode->children().at(2));
  REQUIRE(linkedInnerGroupNode != nullptr);

  auto* linkedInnerPatchNode =
    dynamic_cast<PatchNode*>(linkedInnerGroupNode->children().at(0));
  REQUIRE(linkedInnerPatchNode != nullptr);

  auto* linkedInnerEntityNode =
    dynamic_cast<EntityNode*>(linkedInnerGroupNode->children().at(1));
  REQUIRE(linkedInnerGroupNode != nullptr);

  REQUIRE(outerGroupNode->linkId() == linkedOuterGroupNode->linkId());
  REQUIRE(outerEntityNode->linkId() == linkedOuterEntityNode->linkId());
  REQUIRE(outerBrushNode->linkId() == linkedOuterBrushNode->linkId());
  REQUIRE(innerGroupNode->linkId() == linkedInnerGroupNode->linkId());
  REQUIRE(innerPatchNode->linkId() == linkedInnerPatchNode->linkId());
  REQUIRE(innerEntityNode->linkId() == linkedInnerEntityNode->linkId());

  SECTION("Reset link IDs of only outer group")
  {
    resetLinkIds({linkedOuterGroupNode});

    CHECK(outerGroupNode->linkId() != linkedOuterGroupNode->linkId());
    CHECK(outerEntityNode->linkId() != linkedOuterEntityNode->linkId());
    CHECK(outerBrushNode->linkId() != linkedOuterBrushNode->linkId());
    CHECK(innerGroupNode->linkId() == linkedInnerGroupNode->linkId());
    CHECK(innerPatchNode->linkId() == linkedInnerPatchNode->linkId());
    CHECK(innerEntityNode->linkId() == linkedInnerEntityNode->linkId());
  }

  SECTION("Reset link IDs of only inner group")
  {
    resetLinkIds({linkedInnerGroupNode});

    CHECK(outerGroupNode->linkId() == linkedOuterGroupNode->linkId());
    CHECK(outerEntityNode->linkId() == linkedOuterEntityNode->linkId());
    CHECK(outerBrushNode->linkId() == linkedOuterBrushNode->linkId());
    CHECK(innerGroupNode->linkId() != linkedInnerGroupNode->linkId());
    CHECK(innerPatchNode->linkId() != linkedInnerPatchNode->linkId());
    CHECK(innerEntityNode->linkId() != linkedInnerEntityNode->linkId());
  }

  SECTION("Reset link IDs of outer and inner groups")
  {
    resetLinkIds({linkedOuterGroupNode, linkedInnerGroupNode});

    CHECK(outerGroupNode->linkId() != linkedOuterGroupNode->linkId());
    CHECK(outerEntityNode->linkId() != linkedOuterEntityNode->linkId());
    CHECK(outerBrushNode->linkId() != linkedOuterBrushNode->linkId());
    CHECK(innerGroupNode->linkId() != linkedInnerGroupNode->linkId());
    CHECK(innerPatchNode->linkId() != linkedInnerPatchNode->linkId());
    CHECK(innerEntityNode->linkId() != linkedInnerEntityNode->linkId());
  }
}

} // namespace tb::mdl
