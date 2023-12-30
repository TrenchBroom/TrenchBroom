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

#include "Model/BezierPatch.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/LinkedGroupUtils.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"

#include "kdl/pair_iterator.h"
#include "kdl/vector_utils.h"
#include <kdl/map_utils.h>

#include <vecmath/bbox.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>

#include <numeric>
#include <unordered_set>
#include <vector>

#include "CatchUtils/Matchers.h"

#include "Catch2.h"

namespace TrenchBroom::Model
{

TEST_CASE("GroupNode.updateLinkedGroups")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto groupNode = GroupNode{Group{"name"}};
  auto* entityNode = new EntityNode{Entity{}};
  groupNode.addChild(entityNode);

  transformNode(groupNode, vm::translation_matrix(vm::vec3{1, 0, 0}), worldBounds);
  REQUIRE(
    groupNode.group().transformation() == vm::translation_matrix(vm::vec3{1, 0, 0}));
  REQUIRE(entityNode->entity().origin() == vm::vec3{1, 0, 0});

  SECTION("Target group list is empty")
  {
    updateLinkedGroups(groupNode, {}, worldBounds)
      .transform([&](const UpdateLinkedGroupsResult& r) { CHECK(r.empty()); })
      .transform_error([](const auto&) { FAIL(); });
  }

  SECTION("Target group list contains only source group")
  {
    updateLinkedGroups(groupNode, {&groupNode}, worldBounds)
      .transform([&](const UpdateLinkedGroupsResult& r) { CHECK(r.empty()); })
      .transform_error([](const auto&) { FAIL(); });
  }

  SECTION("Update a single target group")
  {
    auto groupNodeClone = std::unique_ptr<GroupNode>{
      static_cast<GroupNode*>(groupNode.cloneRecursively(worldBounds, SetLinkId::keep))};
    REQUIRE(
      groupNodeClone->group().transformation()
      == vm::translation_matrix(vm::vec3{1, 0, 0}));

    transformNode(
      *groupNodeClone, vm::translation_matrix(vm::vec3{0, 2, 0}), worldBounds);
    REQUIRE(
      groupNodeClone->group().transformation()
      == vm::translation_matrix(vm::vec3{1, 2, 0}));
    REQUIRE(
      static_cast<EntityNode*>(groupNodeClone->children().front())->entity().origin()
      == vm::vec3{1, 2, 0});

    transformNode(*entityNode, vm::translation_matrix(vm::vec3{0, 0, 3}), worldBounds);
    REQUIRE(entityNode->entity().origin() == vm::vec3{1, 0, 3});

    updateLinkedGroups(groupNode, {groupNodeClone.get()}, worldBounds)
      .transform([&](const UpdateLinkedGroupsResult& r) {
        CHECK(r.size() == 1u);

        const auto& p = r.front();
        const auto& [groupNodeToUpdate, newChildren] = p;

        CHECK(groupNodeToUpdate == groupNodeClone.get());
        CHECK(newChildren.size() == 1u);

        const auto* newEntityNode = dynamic_cast<EntityNode*>(newChildren.front().get());
        CHECK(newEntityNode != nullptr);

        CHECK(newEntityNode->entity().origin() == vm::vec3{1, 2, 3});
      })
      .transform_error([](const auto&) { FAIL(); });
  }
}

TEST_CASE("GroupNode.updateNestedLinkedGroups")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto outerGroupNode = GroupNode{Group{"outer"}};
  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  outerGroupNode.addChild(innerGroupNode);

  auto* innerGroupEntityNode = new EntityNode{Entity{}};
  innerGroupNode->addChild(innerGroupEntityNode);

  auto innerGroupNodeClone = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(
    innerGroupNode->cloneRecursively(worldBounds, SetLinkId::keep))};
  REQUIRE(innerGroupNodeClone->group().transformation() == vm::mat4x4{});

  transformNode(
    *innerGroupNodeClone, vm::translation_matrix(vm::vec3{0, 2, 0}), worldBounds);
  REQUIRE(
    innerGroupNodeClone->group().transformation()
    == vm::translation_matrix(vm::vec3{0, 2, 0}));

  SECTION("Transforming the inner group node and updating the linked group")
  {
    transformNode(
      *innerGroupNode, vm::translation_matrix(vm::vec3{1, 0, 0}), worldBounds);
    REQUIRE(outerGroupNode.group().transformation() == vm::mat4x4{});
    REQUIRE(
      innerGroupNode->group().transformation()
      == vm::translation_matrix(vm::vec3{1, 0, 0}));
    REQUIRE(innerGroupEntityNode->entity().origin() == vm::vec3{1, 0, 0});
    REQUIRE(
      innerGroupNodeClone->group().transformation()
      == vm::translation_matrix(vm::vec3{0, 2, 0}));

    updateLinkedGroups(*innerGroupNode, {innerGroupNodeClone.get()}, worldBounds)
      .transform([&](const UpdateLinkedGroupsResult& r) {
        CHECK(r.size() == 1u);

        const auto& p = r.front();
        const auto& [groupNodeToUpdate, newChildren] = p;

        CHECK(groupNodeToUpdate == innerGroupNodeClone.get());
        CHECK(newChildren.size() == 1u);

        const auto* newEntityNode = dynamic_cast<EntityNode*>(newChildren.front().get());
        CHECK(newEntityNode != nullptr);

        CHECK(newEntityNode->entity().origin() == vm::vec3{0, 2, 0});
      })
      .transform_error([](const auto&) { FAIL(); });
  }

  SECTION("Transforming the inner group node's entity and updating the linked group")
  {
    transformNode(
      *innerGroupEntityNode, vm::translation_matrix(vm::vec3{1, 0, 0}), worldBounds);
    REQUIRE(outerGroupNode.group().transformation() == vm::mat4x4{});
    REQUIRE(innerGroupNode->group().transformation() == vm::mat4x4{});
    REQUIRE(innerGroupEntityNode->entity().origin() == vm::vec3{1, 0, 0});
    REQUIRE(
      innerGroupNodeClone->group().transformation()
      == vm::translation_matrix(vm::vec3{0, 2, 0}));

    updateLinkedGroups(*innerGroupNode, {innerGroupNodeClone.get()}, worldBounds)
      .transform([&](const UpdateLinkedGroupsResult& r) {
        CHECK(r.size() == 1u);

        const auto& p = r.front();
        const auto& [groupNodeToUpdate, newChildren] = p;

        CHECK(groupNodeToUpdate == innerGroupNodeClone.get());
        CHECK(newChildren.size() == 1u);

        const auto* newEntityNode = dynamic_cast<EntityNode*>(newChildren.front().get());
        CHECK(newEntityNode != nullptr);

        CHECK(newEntityNode->entity().origin() == vm::vec3{1, 2, 0});
      })
      .transform_error([](const auto&) { FAIL(); });
  }
}

TEST_CASE("GroupNode.updateLinkedGroupsRecursively")
{
  const auto worldBounds = vm::bbox3{8192.0};

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

  auto outerGroupNodeClone = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(
    outerGroupNode.cloneRecursively(worldBounds, SetLinkId::keep))};
  REQUIRE(outerGroupNodeClone->group().transformation() == vm::mat4x4{});
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

  updateLinkedGroups(outerGroupNode, {outerGroupNodeClone.get()}, worldBounds)
    .transform([&](const UpdateLinkedGroupsResult& r) {
      REQUIRE(r.size() == 1u);
      const auto& [groupNodeToUpdate, newChildren] = r.front();

      REQUIRE(groupNodeToUpdate == outerGroupNodeClone.get());
      REQUIRE(newChildren.size() == 1u);

      auto* newInnerGroupNodeClone = dynamic_cast<GroupNode*>(newChildren.front().get());
      CHECK(newInnerGroupNodeClone != nullptr);
      CHECK(newInnerGroupNodeClone->group() == innerGroupNode->group());
      CHECK(newInnerGroupNodeClone->childCount() == 1u);

      auto* newInnerGroupEntityNodeClone =
        dynamic_cast<EntityNode*>(newInnerGroupNodeClone->children().front());
      CHECK(newInnerGroupEntityNodeClone != nullptr);
      CHECK(newInnerGroupEntityNodeClone->entity() == innerGroupEntityNode->entity());
    })
    .transform_error([](const auto&) { FAIL(); });
}

TEST_CASE("GroupNode.updateLinkedGroupsExceedsWorldBounds")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto groupNode = GroupNode{Group{"name"}};
  auto* entityNode = new EntityNode{Entity{}};
  groupNode.addChild(entityNode);

  auto groupNodeClone = std::unique_ptr<GroupNode>{
    static_cast<GroupNode*>(groupNode.cloneRecursively(worldBounds, SetLinkId::keep))};

  transformNode(
    *groupNodeClone, vm::translation_matrix(vm::vec3{8192 - 8, 0, 0}), worldBounds);
  REQUIRE(
    groupNodeClone->children().front()->logicalBounds()
    == vm::bbox3{{8192 - 16, -8, -8}, {8192, 8, 8}});

  transformNode(*entityNode, vm::translation_matrix(vm::vec3{1, 0, 0}), worldBounds);
  REQUIRE(entityNode->entity().origin() == vm::vec3{1, 0, 0});

  updateLinkedGroups(groupNode, {groupNodeClone.get()}, worldBounds)
    .transform([](auto) { FAIL(); })
    .transform_error([](auto e) {
      CHECK(e == Error{"Updating a linked node would exceed world bounds"});
    });
}

static void setGroupName(GroupNode& groupNode, const std::string& name)
{
  auto group = groupNode.group();
  group.setName(name);
  groupNode.setGroup(std::move(group));
}

TEST_CASE("GroupNode.updateLinkedGroupsAndPreserveNestedGroupNames")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto outerGroupNode = GroupNode{Group{"outerGroupNode"}};
  auto* innerGroupNode = new GroupNode{Group{"innerGroupNode"}};
  outerGroupNode.addChild(innerGroupNode);

  auto innerGroupNodeClone = std::unique_ptr<GroupNode>(static_cast<GroupNode*>(
    innerGroupNode->cloneRecursively(worldBounds, SetLinkId::keep)));
  setGroupName(*innerGroupNodeClone, "innerGroupNodeClone");

  auto outerGroupNodeClone = std::unique_ptr<GroupNode>(static_cast<GroupNode*>(
    outerGroupNode.cloneRecursively(worldBounds, SetLinkId::keep)));
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
    "Updating outerGroupNode retains the names of its linked group and the nested linked "
    "group")
  {
    updateLinkedGroups(outerGroupNode, {outerGroupNodeClone.get()}, worldBounds)
      .transform([&](const UpdateLinkedGroupsResult& r) {
        REQUIRE(r.size() == 1u);

        const auto& [groupNodeToUpdate, newChildren] = r.front();
        REQUIRE(groupNodeToUpdate == outerGroupNodeClone.get());

        const auto* innerReplacement = static_cast<GroupNode*>(newChildren.front().get());
        CHECK(innerReplacement->name() == innerGroupNodeNestedClone->name());
      })
      .transform_error([](const auto&) { FAIL(); });
  }
}

TEST_CASE("GroupNode.updateLinkedGroupsAndPreserveEntityProperties")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto sourceGroupNode = GroupNode{Group{"name"}};
  auto* sourceEntityNode = new EntityNode{Entity{}};
  sourceGroupNode.addChild(sourceEntityNode);

  auto targetGroupNode = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(
    sourceGroupNode.cloneRecursively(worldBounds, SetLinkId::keep))};

  auto* targetEntityNode = static_cast<EntityNode*>(targetGroupNode->children().front());
  REQUIRE_THAT(
    targetEntityNode->entity().properties(),
    Catch::Equals(sourceEntityNode->entity().properties()));

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
    entity.setProperties({}, sourceProperties);
    entity.setProtectedProperties(srcProtProperties);
    sourceEntityNode->setEntity(std::move(entity));
  }

  {
    auto entity = targetEntityNode->entity();
    entity.setProperties({}, targetProperties);
    entity.setProtectedProperties(trgtProtProperties);
    targetEntityNode->setEntity(std::move(entity));
  }

  // lambda can't capture structured bindings
  const auto expectedTargetProperties = expectedProperties;

  updateLinkedGroups(sourceGroupNode, {targetGroupNode.get()}, worldBounds)
    .transform([&](const UpdateLinkedGroupsResult& r) {
      REQUIRE(r.size() == 1u);
      const auto& p = r.front();

      const auto& newChildren = p.second;
      REQUIRE(newChildren.size() == 1u);

      const auto* newEntityNode = dynamic_cast<EntityNode*>(newChildren.front().get());
      REQUIRE(newEntityNode != nullptr);

      CHECK_THAT(
        newEntityNode->entity().properties(),
        Catch::UnorderedEquals(expectedTargetProperties));
      CHECK_THAT(
        newEntityNode->entity().protectedProperties(),
        Catch::UnorderedEquals(targetEntityNode->entity().protectedProperties()));
    })
    .transform_error([](const auto&) { FAIL(); });
}

namespace
{
auto* createPatchNode()
{
  // clang-format off
    return new PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
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
      result[groupNode] = groupNode->group().linkId();
      groupNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, const EntityNode* entityNode) {
      result[entityNode] = entityNode->entity().linkId();
      entityNode->visitChildren(thisLambda);
    },
    [&](const BrushNode* brushNode) { result[brushNode] = brushNode->brush().linkId(); },
    [&](const PatchNode* patchNode) {
      result[patchNode] = patchNode->patch().linkId();
    }));
  return result;
}

class LinkIdMatcher : public Catch::MatcherBase<const WorldNode&>
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
      kdl::vec_transform(m_expected, [&](const auto& nodesWithSameLinkId) {
        return getValue(linkIds, nodesWithSameLinkId.front());
      });

    return linkIds.size() == count
           && kdl::all_of(
             m_expected,
             [&](const auto& nodesWithSameLinkId) {
               if (nodesWithSameLinkId.empty())
               {
                 return false;
               }

               const auto linkId = getValue(linkIds, nodesWithSameLinkId.front());
               return linkId && kdl::all_of(nodesWithSameLinkId, [&](auto* entity) {
                        return getValue(linkIds, entity) == linkId;
                      });
             })
           && kdl::none_of(
             kdl::make_pair_range(expectedLinkIds), [](const auto& linkIdPair) {
               const auto& [linkId1, linkId2] = linkIdPair;
               return linkId1 == linkId2;
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

TEST_CASE("initializeLinkIds")
{
  auto brushBuilder = BrushBuilder{MapFormat::Quake3, vm::bbox3{8192.0}};

  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
  auto& layerNode = *worldNode.defaultLayer();

  auto* unlinkedGroupNode = new GroupNode{Group{"unlinked"}};
  auto* unlinkedEntityNode = new EntityNode{Entity{}};

  unlinkedGroupNode->addChildren({unlinkedEntityNode});
  layerNode.addChildren({unlinkedGroupNode});

  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* outerEntityNode = new EntityNode{Entity{}};
  auto* outerBrushNode = new BrushNode{brushBuilder.createCube(64.0, "texture").value()};

  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* innerPatchNode = createPatchNode();
  auto* innerEntityNode = new EntityNode{Entity{}};

  innerGroupNode->addChildren({innerPatchNode, innerEntityNode});
  outerGroupNode->addChildren({outerEntityNode, outerBrushNode, innerGroupNode});

  auto* linkedOuterGroupNode = new GroupNode{Group{"outer"}};
  auto* linkedOuterEntityNode = new EntityNode{Entity{}};
  auto* linkedOuterBrushNode =
    new BrushNode{brushBuilder.createCube(64.0, "texture").value()};

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
        new BrushNode{brushBuilder.createCube(64.0, "texture").value()};

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
        Catch::UnorderedEquals(std::vector{
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
        Catch::UnorderedEquals(std::vector{
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

} // namespace TrenchBroom::Model
