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
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Material.h"
#include "mdl/MaterialManager.h"
#include "mdl/TagMatcher.h"
#include "mdl/TextureResource.h"

#include "kdl/vector_utils.h"

#include "Catch2.h"

namespace tb::mdl
{
namespace
{

class TestCallback : public TagMatcherCallback
{
private:
  size_t m_option;

public:
  explicit TestCallback(const size_t option)
    : m_option{option}
  {
  }

  size_t selectOption(const std::vector<std::string>&) override { return m_option; }
};

} // namespace

TEST_CASE("Map_Tags")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();

  const auto materialMatch = std::string{"some_material"};
  const auto materialPatternMatch = std::string{"*er_material"};
  const auto singleParamMatch = std::string{"parm2"};
  const auto multiParamsMatch =
    kdl::vector_set<std::string>{"some_parm", "parm1", "parm3"};

  auto gameConfig = MockGameConfig{};
  gameConfig.smartTags = {
    SmartTag{
      "material",
      {},
      std::make_unique<MaterialNameTagMatcher>(materialMatch),
    },
    SmartTag{
      "materialPattern",
      {},
      std::make_unique<MaterialNameTagMatcher>(materialPatternMatch),
    },
    SmartTag{
      "surfaceparm_single",
      {},
      std::make_unique<SurfaceParmTagMatcher>(singleParamMatch),
    },
    SmartTag{
      "surfaceparm_multi",
      {},
      std::make_unique<SurfaceParmTagMatcher>(multiParamsMatch),
    },
    SmartTag{
      "contentflags",
      {},
      std::make_unique<ContentFlagsTagMatcher>(1),
    },
    SmartTag{
      "surfaceflags",
      {},
      std::make_unique<SurfaceFlagsTagMatcher>(1),
    },
    SmartTag{
      "entity",
      {},
      std::make_unique<EntityClassNameTagMatcher>("brush_entity", ""),
    }};
  fixture.create({.game = MockGameFixture{std::move(gameConfig)}});

  map.entityDefinitionManager().setDefinitions({
    {"brush_entity", Color{}, "this is a brush entity", {}},
  });

  const auto* brushEntityDefinition =
    map.entityDefinitionManager().definition("brush_entity");

  auto& materialManager = map.materialManager();
  {
    auto materialA = Material{"some_material", createTextureResource(Texture{16, 16})};
    auto materialB = Material{"other_material", createTextureResource(Texture{32, 32})};
    auto materialC =
      Material{"yet_another_material", createTextureResource(Texture{64, 64})};

    const auto singleParam = std::string{"some_parm"};
    const auto multiParams = std::set<std::string>{"parm1", "parm2"};

    materialA.setSurfaceParms({singleParam});
    materialB.setSurfaceParms(multiParams);

    auto materials =
      kdl::vec_from(std::move(materialA), std::move(materialB), std::move(materialC));

    auto collections = kdl::vec_from(MaterialCollection{std::move(materials)});

    materialManager.setMaterialCollections(std::move(collections));
  }

  auto* materialA = materialManager.material("some_material");
  auto* materialB = materialManager.material("other_material");
  auto* materialC = materialManager.material("yet_another_material");

  SECTION("registerSmartTags")
  {
    CHECK(map.isRegisteredSmartTag("material"));
    CHECK(map.smartTag("material").index() == 0u);
    CHECK(map.smartTag("material").type() == 1u);

    CHECK(map.isRegisteredSmartTag("materialPattern"));
    CHECK(map.smartTag("materialPattern").index() == 1u);
    CHECK(map.smartTag("materialPattern").type() == 2u);

    CHECK(map.isRegisteredSmartTag("surfaceparm_single"));
    CHECK(map.smartTag("surfaceparm_single").index() == 2u);
    CHECK(map.smartTag("surfaceparm_single").type() == 4u);

    CHECK(map.isRegisteredSmartTag("surfaceparm_multi"));
    CHECK(map.smartTag("surfaceparm_multi").index() == 3u);
    CHECK(map.smartTag("surfaceparm_multi").type() == 8u);

    CHECK(map.isRegisteredSmartTag("contentflags"));
    CHECK(map.smartTag("contentflags").index() == 4u);
    CHECK(map.smartTag("contentflags").type() == 16u);

    CHECK(map.isRegisteredSmartTag("surfaceflags"));
    CHECK(map.smartTag("surfaceflags").index() == 5u);
    CHECK(map.smartTag("surfaceflags").type() == 32u);

    CHECK(map.isRegisteredSmartTag("entity"));
    CHECK(map.smartTag("entity").index() == 6u);
    CHECK(map.smartTag("entity").type() == 64u);

    CHECK_FALSE(map.isRegisteredSmartTag(""));
    CHECK_FALSE(map.isRegisteredSmartTag("asdf"));
  }

  SECTION("registerSmartTags checks duplicate tags")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/2905

    gameConfig = MockGameConfig{};
    gameConfig.smartTags = {
      SmartTag{
        "material",
        {},
        std::make_unique<MaterialNameTagMatcher>("some_material"),
      },
      SmartTag{
        "material",
        {},
        std::make_unique<SurfaceParmTagMatcher>("some_other_material"),
      },
    };
    CHECK_THROWS_AS(
      fixture.create({.game = MockGameFixture{std::move(gameConfig)}}), std::logic_error);
  }

  SECTION("addNodes initializes brush tags")
  {
    auto* entityNode = new EntityNode{Entity{{
      {"classname", "brush_entity"},
    }}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});
    REQUIRE(entityNode->entity().definition() == brushEntityDefinition);

    auto* brush = createBrushNode(map, "some_material");
    addNodes(map, {{entityNode, {brush}}});

    const auto& tag = map.smartTag("entity");
    CHECK(brush->hasTag(tag));
  }

  SECTION("removeNodes removes tags")
  {
    SECTION("Brush tags")
    {
      auto* entityNode = new EntityNode{Entity{{
        {"classname", "brush_entity"},
      }}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      REQUIRE(entityNode->entity().definition() == brushEntityDefinition);

      auto* brush = createBrushNode(map, "some_material");
      addNodes(map, {{entityNode, {brush}}});

      removeNodes(map, {brush});

      const auto& tag = map.smartTag("entity");
      CHECK_FALSE(brush->hasTag(tag));
    }

    SECTION("Brush face tags")
    {
      auto* brushNodeWithTags = createBrushNode(map, "some_material");
      addNodes(map, {{parentForNodes(map), {brushNodeWithTags}}});
      removeNodes(map, {brushNodeWithTags});

      const auto& tag = map.smartTag("material");
      for (const auto& face : brushNodeWithTags->brush().faces())
      {
        CHECK_FALSE(face.hasTag(tag));
      }
    }
  }

  SECTION("reparentNodes updates brush tags")
  {
    SECTION("Reparent from world to entity")
    {
      auto* brushNode = createBrushNode(map, "some_material");
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      auto* entityNode = new EntityNode{Entity{{
        {"classname", "brush_entity"},
      }}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      REQUIRE(entityNode->entity().definition() == brushEntityDefinition);

      const auto& tag = map.smartTag("entity");
      CHECK_FALSE(brushNode->hasTag(tag));

      reparentNodes(map, {{entityNode, {brushNode}}});
      CHECK(brushNode->hasTag(tag));
    }

    SECTION("Reparent between entities")
    {
      auto* lightEntityNode = new EntityNode{Entity{{
        {"classname", "brush_entity"},
      }}};
      auto* otherEntityNode = new EntityNode{Entity{{
        {"classname", "other"},
      }}};
      addNodes(map, {{parentForNodes(map), {lightEntityNode, otherEntityNode}}});
      REQUIRE(lightEntityNode->entity().definition() == brushEntityDefinition);

      auto* brushNode = createBrushNode(map, "some_material");
      addNodes(map, {{otherEntityNode, {brushNode}}});

      const auto& tag = map.smartTag("entity");
      CHECK_FALSE(brushNode->hasTag(tag));

      reparentNodes(map, {{lightEntityNode, {brushNode}}});
      CHECK(brushNode->hasTag(tag));
    }

    SECTION("setEntityProperty updates tags")
    {
      auto* lightEntityNode = new EntityNode{Entity{{
        {"classname", "asdf"},
      }}};
      addNodes(map, {{parentForNodes(map), {lightEntityNode}}});

      auto* brushNode = createBrushNode(map, "some_material");
      addNodes(map, {{lightEntityNode, {brushNode}}});

      const auto& tag = map.smartTag("entity");
      CHECK_FALSE(brushNode->hasTag(tag));

      map.selectNodes({lightEntityNode});
      setEntityProperty(map, "classname", "brush_entity");
      map.deselectAll();

      CHECK(brushNode->hasTag(tag));
    }

    SECTION("setBrushFaceAttributes updates tags")
    {
      auto* brushNode = createBrushNode(map, "asdf");
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const auto& tag = map.smartTag("contentflags");

      const auto faceHandle = BrushFaceHandle{brushNode, 0u};
      CHECK_FALSE(faceHandle.face().hasTag(tag));

      auto request = ChangeBrushFaceAttributesRequest{};
      request.setContentFlags(1);

      map.selectBrushFaces({faceHandle});
      setBrushFaceAttributes(map, request);
      map.deselectAll();

      const auto& faces = brushNode->brush().faces();
      CHECK(faces[0].hasTag(tag));
      for (size_t i = 1u; i < faces.size(); ++i)
      {
        CHECK(!faces[i].hasTag(tag));
      }
    }
  }

  SECTION("Material name tag")
  {
    SECTION("matches")
    {
      auto nodeA = std::unique_ptr<BrushNode>{createBrushNode(map, materialA->name())};
      auto nodeB = std::unique_ptr<BrushNode>{createBrushNode(map, materialB->name())};
      auto nodeC = std::unique_ptr<BrushNode>{createBrushNode(map, materialC->name())};
      const auto& tag = map.smartTag("material");
      const auto& patternTag = map.smartTag("materialPattern");
      for (const auto& face : nodeA->brush().faces())
      {
        CHECK(tag.matches(face));
        CHECK_FALSE(patternTag.matches(face));
      }
      for (const auto& face : nodeB->brush().faces())
      {
        CHECK_FALSE(tag.matches(face));
        CHECK(patternTag.matches(face));
      }
      for (const auto& face : nodeC->brush().faces())
      {
        CHECK_FALSE(tag.matches(face));
        CHECK(patternTag.matches(face));
      }
    }

    SECTION("enable")
    {
      auto* nonMatchingBrushNode = createBrushNode(map, "asdf");
      addNodes(map, {{parentForNodes(map), {nonMatchingBrushNode}}});

      const auto& tag = map.smartTag("material");
      CHECK(tag.canEnable());

      const auto faceHandle = BrushFaceHandle{nonMatchingBrushNode, 0u};
      CHECK_FALSE(tag.matches(faceHandle.face()));

      map.selectBrushFaces({faceHandle});

      auto callback = TestCallback{0};
      tag.enable(callback, map);

      CHECK(tag.matches(faceHandle.face()));
    }

    SECTION("disable")
    {
      const auto& tag = map.smartTag("material");
      CHECK_FALSE(tag.canDisable());
    }
  }

  SECTION("Surface parameter tag")
  {
    SECTION("matches")
    {
      auto nodeA =
        std::unique_ptr<BrushNode>{createBrushNode(map, materialA->name(), [&](auto& b) {
          for (auto& face : b.faces())
          {
            face.setMaterial(materialA);
          }
        })};
      auto nodeB =
        std::unique_ptr<BrushNode>{createBrushNode(map, materialB->name(), [&](auto& b) {
          for (auto& face : b.faces())
          {
            face.setMaterial(materialB);
          }
        })};
      auto nodeC =
        std::unique_ptr<BrushNode>{createBrushNode(map, materialC->name(), [&](auto& b) {
          for (auto& face : b.faces())
          {
            face.setMaterial(materialC);
          }
        })};
      const auto& singleTag = map.smartTag("surfaceparm_single");
      const auto& multiTag = map.smartTag("surfaceparm_multi");
      for (const auto& face : nodeA->brush().faces())
      {
        CHECK_FALSE(singleTag.matches(face));
        CHECK(multiTag.matches(face));
      }
      for (const auto& face : nodeB->brush().faces())
      {
        CHECK(singleTag.matches(face));
        CHECK(multiTag.matches(face));
      }
      for (const auto& face : nodeC->brush().faces())
      {
        CHECK_FALSE(singleTag.matches(face));
        CHECK_FALSE(multiTag.matches(face));
      }
    }

    SECTION("enable")
    {
      auto* nonMatchingBrushNode = createBrushNode(map, "asdf");
      addNodes(map, {{parentForNodes(map), {nonMatchingBrushNode}}});

      const auto& tag = map.smartTag("surfaceparm_single");
      CHECK(tag.canEnable());

      const auto faceHandle = BrushFaceHandle{nonMatchingBrushNode, 0u};
      CHECK_FALSE(tag.matches(faceHandle.face()));

      map.selectBrushFaces({faceHandle});

      auto callback = TestCallback{0};
      tag.enable(callback, map);

      CHECK(tag.matches(faceHandle.face()));
    }

    SECTION("disable")
    {
      const auto& tag = map.smartTag("surfaceparm_single");
      CHECK_FALSE(tag.canDisable());
    }
  }

  SECTION("Content flags tag")
  {
    SECTION("matches")
    {
      auto matchingBrushNode =
        std::unique_ptr<BrushNode>{createBrushNode(map, "asdf", [](auto& b) {
          for (auto& face : b.faces())
          {
            auto attributes = face.attributes();
            attributes.setSurfaceContents(1);
            face.setAttributes(attributes);
          }
        })};
      auto nonMatchingBrushNode =
        std::unique_ptr<BrushNode>{createBrushNode(map, "asdf", [](auto& b) {
          for (auto& face : b.faces())
          {
            auto attributes = face.attributes();
            attributes.setSurfaceContents(2);
            face.setAttributes(attributes);
          }
        })};

      const auto& tag = map.smartTag("contentflags");
      for (const auto& face : matchingBrushNode->brush().faces())
      {
        CHECK(tag.matches(face));
      }
      for (const auto& face : nonMatchingBrushNode->brush().faces())
      {
        CHECK_FALSE(tag.matches(face));
      }
    }

    SECTION("enable")
    {
      auto* nonMatchingBrushNode = createBrushNode(map, "asdf");
      addNodes(map, {{parentForNodes(map), {nonMatchingBrushNode}}});

      const auto& tag = map.smartTag("contentflags");
      CHECK(tag.canEnable());

      const auto faceHandle = BrushFaceHandle{nonMatchingBrushNode, 0u};
      CHECK_FALSE(tag.matches(faceHandle.face()));

      map.selectBrushFaces({faceHandle});

      auto callback = TestCallback{0};
      tag.enable(callback, map);

      CHECK(tag.matches(faceHandle.face()));
    }

    SECTION("disable")
    {
      auto* matchingBrushNode = createBrushNode(map, "asdf", [](auto& b) {
        for (auto& face : b.faces())
        {
          auto attributes = face.attributes();
          attributes.setSurfaceContents(1);
          face.setAttributes(attributes);
        }
      });

      addNodes(map, {{parentForNodes(map), {matchingBrushNode}}});

      const auto& tag = map.smartTag("contentflags");
      CHECK(tag.canDisable());

      const auto faceHandle = BrushFaceHandle{matchingBrushNode, 0u};
      CHECK(tag.matches(faceHandle.face()));

      map.selectBrushFaces({faceHandle});

      auto callback = TestCallback{0};
      tag.disable(callback, map);

      CHECK_FALSE(tag.matches(faceHandle.face()));
    }
  }

  SECTION("Surface flags tag")
  {
    SECTION("matches")
    {
      auto matchingBrushNode =
        std::unique_ptr<BrushNode>{createBrushNode(map, "asdf", [](auto& b) {
          for (auto& face : b.faces())
          {
            auto attributes = face.attributes();
            attributes.setSurfaceFlags(1);
            face.setAttributes(attributes);
          }
        })};
      auto nonMatchingBrushNode =
        std::unique_ptr<BrushNode>{createBrushNode(map, "asdf", [](auto& b) {
          for (auto& face : b.faces())
          {
            auto attributes = face.attributes();
            attributes.setSurfaceFlags(2);
            face.setAttributes(attributes);
          }
        })};

      const auto& tag = map.smartTag("surfaceflags");
      for (const auto& face : matchingBrushNode->brush().faces())
      {
        CHECK(tag.matches(face));
      }
      for (const auto& face : nonMatchingBrushNode->brush().faces())
      {
        CHECK_FALSE(tag.matches(face));
      }
    }

    SECTION("enable")
    {
      auto* nonMatchingBrushNode = createBrushNode(map, "asdf");
      addNodes(map, {{parentForNodes(map), {nonMatchingBrushNode}}});

      const auto& tag = map.smartTag("surfaceflags");
      CHECK(tag.canEnable());

      const auto faceHandle = BrushFaceHandle{nonMatchingBrushNode, 0u};
      CHECK_FALSE(tag.matches(faceHandle.face()));

      map.selectBrushFaces({faceHandle});

      auto callback = TestCallback{0};
      tag.enable(callback, map);

      CHECK(tag.matches(faceHandle.face()));
    }

    SECTION("disable")
    {
      auto* matchingBrushNode = createBrushNode(map, "asdf", [](auto& b) {
        for (auto& face : b.faces())
        {
          auto attributes = face.attributes();
          attributes.setSurfaceFlags(1);
          face.setAttributes(attributes);
        }
      });

      addNodes(map, {{parentForNodes(map), {matchingBrushNode}}});

      const auto& tag = map.smartTag("surfaceflags");
      CHECK(tag.canDisable());

      const auto faceHandle = BrushFaceHandle{matchingBrushNode, 0u};
      CHECK(tag.matches(faceHandle.face()));

      map.selectBrushFaces({faceHandle});

      auto callback = TestCallback{0};
      tag.disable(callback, map);

      CHECK_FALSE(tag.matches(faceHandle.face()));
    }
  }

  SECTION("Entity classname tag")
  {
    SECTION("matches")
    {
      auto* matchingBrushNode = createBrushNode(map, "asdf");
      auto* nonMatchingBrushNode = createBrushNode(map, "asdf");

      auto matchingEntity =
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}});
      matchingEntity->addChild(matchingBrushNode);

      auto nonMatchingEntity =
        std::make_unique<EntityNode>(Entity{{{"classname", "something"}}});
      nonMatchingEntity->addChild(nonMatchingBrushNode);

      const auto& tag = map.smartTag("entity");
      CHECK(tag.matches(*matchingBrushNode));
      CHECK_FALSE(tag.matches(*nonMatchingBrushNode));
    }

    SECTION("enable")
    {
      auto* brushNode = createBrushNode(map, "asdf");
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const auto& tag = map.smartTag("entity");
      CHECK_FALSE(tag.matches(*brushNode));

      CHECK(tag.canEnable());

      map.selectNodes({brushNode});

      auto callback = TestCallback{0};
      tag.enable(callback, map);
      CHECK(tag.matches(*brushNode));
    }

    SECTION("enable retains entity properties")
    {
      auto* brushNode = createBrushNode(map, "asdf");

      auto* oldEntity = new EntityNode{Entity{{
        {"classname", "something"},
        {"some_attr", "some_value"},
      }}};

      addNodes(map, {{parentForNodes(map), {oldEntity}}});
      addNodes(map, {{oldEntity, {brushNode}}});

      const auto& tag = map.smartTag("entity");
      map.selectNodes({brushNode});

      auto callback = TestCallback{0};
      tag.enable(callback, map);
      CHECK(tag.matches(*brushNode));

      auto* newEntityNode = brushNode->entity();
      CHECK(newEntityNode != oldEntity);

      CHECK(newEntityNode != nullptr);
      CHECK(newEntityNode->entity().hasProperty("some_attr"));
      CHECK(*newEntityNode->entity().property("some_attr") == "some_value");
    }

    SECTION("disable")
    {
      auto* brushNode = createBrushNode(map, "asdf");

      auto* oldEntityNode = new EntityNode{Entity{{
        {"classname", "brush_entity"},
      }}};

      addNodes(map, {{parentForNodes(map), {oldEntityNode}}});
      addNodes(map, {{oldEntityNode, {brushNode}}});
      REQUIRE(oldEntityNode->entity().definition() == brushEntityDefinition);

      const auto& tag = map.smartTag("entity");
      CHECK(tag.matches(*brushNode));

      CHECK(tag.canDisable());

      map.selectNodes({brushNode});

      auto callback = TestCallback{0};
      tag.disable(callback, map);
      CHECK_FALSE(tag.matches(*brushNode));
    }
  }
}

} // namespace tb::mdl
