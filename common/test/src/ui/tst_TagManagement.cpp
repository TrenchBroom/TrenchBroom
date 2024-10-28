/*
 Copyright (C) 2010 Kristian Duske

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
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "mdl/EntityNode.h"
#include "mdl/Material.h"
#include "mdl/MaterialCollection.h"
#include "mdl/MaterialManager.h"
#include "mdl/Tag.h"
#include "mdl/TagMatcher.h"
#include "mdl/TestGame.h" // IWYU pragma: keep
#include "mdl/Texture.h"
#include "ui/MapDocumentTest.h"

#include "kdl/vector_utils.h"

#include <vector>

#include "Catch2.h"

namespace tb::ui
{
namespace
{

class TestCallback : public mdl::TagMatcherCallback
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

TEST_CASE_METHOD(MapDocumentTest, "TagManagementTest")
{
  auto& materialManager = document->materialManager();

  [&]() {
    auto materialA =
      mdl::Material{"some_material", createTextureResource(mdl::Texture{16, 16})};
    auto materialB =
      mdl::Material{"other_material", createTextureResource(mdl::Texture{32, 32})};
    auto materialC =
      mdl::Material{"yet_another_material", createTextureResource(mdl::Texture{64, 64})};

    const auto singleParam = std::string{"some_parm"};
    const auto multiParams = std::set<std::string>{"parm1", "parm2"};

    materialA.setSurfaceParms({singleParam});
    materialB.setSurfaceParms(multiParams);

    auto materials =
      kdl::vec_from(std::move(materialA), std::move(materialB), std::move(materialC));

    auto collections = kdl::vec_from(mdl::MaterialCollection{std::move(materials)});

    materialManager.setMaterialCollections(std::move(collections));
  }();

  auto* materialA = materialManager.material("some_material");
  auto* materialB = materialManager.material("other_material");
  auto* materialC = materialManager.material("yet_another_material");

  const auto materialMatch = std::string{"some_material"};
  const auto materialPatternMatch = std::string{"*er_material"};
  const auto singleParamMatch = std::string{"parm2"};
  const auto multiParamsMatch =
    kdl::vector_set<std::string>{"some_parm", "parm1", "parm3"};
  game->setSmartTags(
    {mdl::SmartTag{
       "material",
       {},
       std::make_unique<mdl::MaterialNameTagMatcher>(materialMatch),
     },
     mdl::SmartTag{
       "materialPattern",
       {},
       std::make_unique<mdl::MaterialNameTagMatcher>(materialPatternMatch),
     },
     mdl::SmartTag{
       "surfaceparm_single",
       {},
       std::make_unique<mdl::SurfaceParmTagMatcher>(singleParamMatch),
     },
     mdl::SmartTag{
       "surfaceparm_multi",
       {},
       std::make_unique<mdl::SurfaceParmTagMatcher>(multiParamsMatch),
     },
     mdl::SmartTag{
       "contentflags",
       {},
       std::make_unique<mdl::ContentFlagsTagMatcher>(1),
     },
     mdl::SmartTag{
       "surfaceflags",
       {},
       std::make_unique<mdl::SurfaceFlagsTagMatcher>(1),
     },
     mdl::SmartTag{
       "entity",
       {},
       std::make_unique<mdl::EntityClassNameTagMatcher>("brush_entity", ""),
     }});
  document->registerSmartTags();

  SECTION("tagRegistration")
  {
    CHECK(document->isRegisteredSmartTag("material"));
    CHECK(document->isRegisteredSmartTag("materialPattern"));
    CHECK(document->isRegisteredSmartTag("surfaceparm_single"));
    CHECK(document->isRegisteredSmartTag("surfaceparm_multi"));
    CHECK(document->isRegisteredSmartTag("contentflags"));
    CHECK(document->isRegisteredSmartTag("surfaceflags"));
    CHECK(document->isRegisteredSmartTag("entity"));
    CHECK_FALSE(document->isRegisteredSmartTag(""));
    CHECK_FALSE(document->isRegisteredSmartTag("asdf"));
  }

  SECTION("tagRegistrationAssignsIndexes")
  {
    CHECK(document->smartTag("material").index() == 0u);
    CHECK(document->smartTag("materialPattern").index() == 1u);
    CHECK(document->smartTag("surfaceparm_single").index() == 2u);
    CHECK(document->smartTag("surfaceparm_multi").index() == 3u);
    CHECK(document->smartTag("contentflags").index() == 4u);
    CHECK(document->smartTag("surfaceflags").index() == 5u);
    CHECK(document->smartTag("entity").index() == 6u);
  }

  SECTION("tagRegistrationAssignsTypes")
  {
    CHECK(document->smartTag("material").type() == 1u);
    CHECK(document->smartTag("materialPattern").type() == 2u);
    CHECK(document->smartTag("surfaceparm_single").type() == 4u);
    CHECK(document->smartTag("surfaceparm_multi").type() == 8u);
    CHECK(document->smartTag("contentflags").type() == 16u);
    CHECK(document->smartTag("surfaceflags").type() == 32u);
    CHECK(document->smartTag("entity").type() == 64u);
  }

  // https://github.com/TrenchBroom/TrenchBroom/issues/2905
  SECTION("duplicateTag")
  {
    game->setSmartTags({
      mdl::SmartTag{
        "material",
        {},
        std::make_unique<mdl::MaterialNameTagMatcher>("some_material"),
      },
      mdl::SmartTag{
        "material",
        {},
        std::make_unique<mdl::SurfaceParmTagMatcher>("some_other_material"),
      },
    });
    CHECK_THROWS_AS(document->registerSmartTags(), std::logic_error);
  }

  SECTION("matchMaterialNameTag")
  {
    auto nodeA = std::unique_ptr<mdl::BrushNode>(createBrushNode(materialA->name()));
    auto nodeB = std::unique_ptr<mdl::BrushNode>(createBrushNode(materialB->name()));
    auto nodeC = std::unique_ptr<mdl::BrushNode>(createBrushNode(materialC->name()));
    const auto& tag = document->smartTag("material");
    const auto& patternTag = document->smartTag("materialPattern");
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

  SECTION("enableMaterialNameTag")
  {
    auto* nonMatchingBrushNode = createBrushNode("asdf");
    document->addNodes({{document->parentForNodes(), {nonMatchingBrushNode}}});

    const auto& tag = document->smartTag("material");
    CHECK(tag.canEnable());

    const auto faceHandle = mdl::BrushFaceHandle{nonMatchingBrushNode, 0u};
    CHECK_FALSE(tag.matches(faceHandle.face()));

    document->selectBrushFaces({faceHandle});

    auto callback = TestCallback{0};
    tag.enable(callback, *document);

    CHECK(tag.matches(faceHandle.face()));
  }

  SECTION("disableMaterialNameTag")
  {
    const auto& tag = document->smartTag("material");
    CHECK_FALSE(tag.canDisable());
  }

  SECTION("matchSurfaceParmTag")
  {
    auto nodeA =
      std::unique_ptr<mdl::BrushNode>(createBrushNode(materialA->name(), [&](auto& b) {
        for (auto& face : b.faces())
        {
          face.setMaterial(materialA);
        }
      }));
    auto nodeB =
      std::unique_ptr<mdl::BrushNode>(createBrushNode(materialB->name(), [&](auto& b) {
        for (auto& face : b.faces())
        {
          face.setMaterial(materialB);
        }
      }));
    auto nodeC =
      std::unique_ptr<mdl::BrushNode>(createBrushNode(materialC->name(), [&](auto& b) {
        for (auto& face : b.faces())
        {
          face.setMaterial(materialC);
        }
      }));
    const auto& singleTag = document->smartTag("surfaceparm_single");
    const auto& multiTag = document->smartTag("surfaceparm_multi");
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

  SECTION("enableSurfaceParmTag")
  {
    auto* nonMatchingBrushNode = createBrushNode("asdf");
    document->addNodes({{document->parentForNodes(), {nonMatchingBrushNode}}});

    const auto& tag = document->smartTag("surfaceparm_single");
    CHECK(tag.canEnable());

    const auto faceHandle = mdl::BrushFaceHandle{nonMatchingBrushNode, 0u};
    CHECK_FALSE(tag.matches(faceHandle.face()));

    document->selectBrushFaces({faceHandle});

    auto callback = TestCallback{0};
    tag.enable(callback, *document);

    CHECK(tag.matches(faceHandle.face()));
  }

  SECTION("disableSurfaceParmTag")
  {
    const auto& tag = document->smartTag("surfaceparm_single");
    CHECK_FALSE(tag.canDisable());
  }

  SECTION("matchContentFlagsTag")
  {
    auto matchingBrushNode =
      std::unique_ptr<mdl::BrushNode>(createBrushNode("asdf", [](auto& b) {
        for (auto& face : b.faces())
        {
          auto attributes = face.attributes();
          attributes.setSurfaceContents(1);
          face.setAttributes(attributes);
        }
      }));
    auto nonMatchingBrushNode =
      std::unique_ptr<mdl::BrushNode>(createBrushNode("asdf", [](auto& b) {
        for (auto& face : b.faces())
        {
          auto attributes = face.attributes();
          attributes.setSurfaceContents(2);
          face.setAttributes(attributes);
        }
      }));

    const auto& tag = document->smartTag("contentflags");
    for (const auto& face : matchingBrushNode->brush().faces())
    {
      CHECK(tag.matches(face));
    }
    for (const auto& face : nonMatchingBrushNode->brush().faces())
    {
      CHECK_FALSE(tag.matches(face));
    }
  }

  SECTION("enableContentFlagsTag")
  {
    auto* nonMatchingBrushNode = createBrushNode("asdf");
    document->addNodes({{document->parentForNodes(), {nonMatchingBrushNode}}});

    const auto& tag = document->smartTag("contentflags");
    CHECK(tag.canEnable());

    const auto faceHandle = mdl::BrushFaceHandle{nonMatchingBrushNode, 0u};
    CHECK_FALSE(tag.matches(faceHandle.face()));

    document->selectBrushFaces({faceHandle});

    auto callback = TestCallback{0};
    tag.enable(callback, *document);

    CHECK(tag.matches(faceHandle.face()));
  }

  SECTION("disableContentFlagsTag")
  {
    auto* matchingBrushNode = createBrushNode("asdf", [](auto& b) {
      for (auto& face : b.faces())
      {
        auto attributes = face.attributes();
        attributes.setSurfaceContents(1);
        face.setAttributes(attributes);
      }
    });

    document->addNodes({{document->parentForNodes(), {matchingBrushNode}}});

    const auto& tag = document->smartTag("contentflags");
    CHECK(tag.canDisable());

    const auto faceHandle = mdl::BrushFaceHandle{matchingBrushNode, 0u};
    CHECK(tag.matches(faceHandle.face()));

    document->selectBrushFaces({faceHandle});

    auto callback = TestCallback{0};
    tag.disable(callback, *document);

    CHECK_FALSE(tag.matches(faceHandle.face()));
  }

  SECTION("matchSurfaceFlagsTag")
  {
    auto matchingBrushNode =
      std::unique_ptr<mdl::BrushNode>(createBrushNode("asdf", [](auto& b) {
        for (auto& face : b.faces())
        {
          auto attributes = face.attributes();
          attributes.setSurfaceFlags(1);
          face.setAttributes(attributes);
        }
      }));
    auto nonMatchingBrushNode =
      std::unique_ptr<mdl::BrushNode>(createBrushNode("asdf", [](auto& b) {
        for (auto& face : b.faces())
        {
          auto attributes = face.attributes();
          attributes.setSurfaceFlags(2);
          face.setAttributes(attributes);
        }
      }));

    const auto& tag = document->smartTag("surfaceflags");
    for (const auto& face : matchingBrushNode->brush().faces())
    {
      CHECK(tag.matches(face));
    }
    for (const auto& face : nonMatchingBrushNode->brush().faces())
    {
      CHECK_FALSE(tag.matches(face));
    }
  }

  SECTION("enableSurfaceFlagsTag")
  {
    auto* nonMatchingBrushNode = createBrushNode("asdf");
    document->addNodes({{document->parentForNodes(), {nonMatchingBrushNode}}});

    const auto& tag = document->smartTag("surfaceflags");
    CHECK(tag.canEnable());

    const auto faceHandle = mdl::BrushFaceHandle{nonMatchingBrushNode, 0u};
    CHECK_FALSE(tag.matches(faceHandle.face()));

    document->selectBrushFaces({faceHandle});

    auto callback = TestCallback{0};
    tag.enable(callback, *document);

    CHECK(tag.matches(faceHandle.face()));
  }

  SECTION("disableSurfaceFlagsTag")
  {
    auto* matchingBrushNode = createBrushNode("asdf", [](auto& b) {
      for (auto& face : b.faces())
      {
        auto attributes = face.attributes();
        attributes.setSurfaceFlags(1);
        face.setAttributes(attributes);
      }
    });

    document->addNodes({{document->parentForNodes(), {matchingBrushNode}}});

    const auto& tag = document->smartTag("surfaceflags");
    CHECK(tag.canDisable());

    const auto faceHandle = mdl::BrushFaceHandle{matchingBrushNode, 0u};
    CHECK(tag.matches(faceHandle.face()));

    document->selectBrushFaces({faceHandle});

    auto callback = TestCallback{0};
    tag.disable(callback, *document);

    CHECK_FALSE(tag.matches(faceHandle.face()));
  }

  SECTION("matchEntityClassnameTag")
  {
    auto* matchingBrushNode = createBrushNode("asdf");
    auto* nonMatchingBrushNode = createBrushNode("asdf");

    auto matchingEntity =
      std::make_unique<mdl::EntityNode>(mdl::Entity{{{"classname", "brush_entity"}}});
    matchingEntity->addChild(matchingBrushNode);

    auto nonMatchingEntity =
      std::make_unique<mdl::EntityNode>(mdl::Entity{{{"classname", "something"}}});
    nonMatchingEntity->addChild(nonMatchingBrushNode);

    const auto& tag = document->smartTag("entity");
    CHECK(tag.matches(*matchingBrushNode));
    CHECK_FALSE(tag.matches(*nonMatchingBrushNode));
  }

  SECTION("enableEntityClassnameTag")
  {
    auto* brushNode = createBrushNode("asdf");
    document->addNodes({{document->parentForNodes(), {brushNode}}});

    const auto& tag = document->smartTag("entity");
    CHECK_FALSE(tag.matches(*brushNode));

    CHECK(tag.canEnable());

    document->selectNodes({brushNode});

    auto callback = TestCallback{0};
    tag.enable(callback, *document);
    CHECK(tag.matches(*brushNode));
  }

  SECTION("enableEntityClassnameTagRetainsAttributes")
  {
    auto* brushNode = createBrushNode("asdf");

    auto* oldEntity = new mdl::EntityNode{mdl::Entity{{
      {"classname", "something"},
      {"some_attr", "some_value"},
    }}};

    document->addNodes({{document->parentForNodes(), {oldEntity}}});
    document->addNodes({{oldEntity, {brushNode}}});

    const auto& tag = document->smartTag("entity");
    document->selectNodes({brushNode});

    auto callback = TestCallback{0};
    tag.enable(callback, *document);
    CHECK(tag.matches(*brushNode));

    auto* newEntityNode = brushNode->entity();
    CHECK(newEntityNode != oldEntity);

    CHECK(newEntityNode != nullptr);
    CHECK(newEntityNode->entity().hasProperty("some_attr"));
    CHECK(*newEntityNode->entity().property("some_attr") == "some_value");
  }

  SECTION("disableEntityClassnameTag")
  {
    auto* brushNode = createBrushNode("asdf");

    auto* oldEntity = new mdl::EntityNode{mdl::Entity{{
      {"classname", "brush_entity"},
    }}};

    document->addNodes({{document->parentForNodes(), {oldEntity}}});
    document->addNodes({{oldEntity, {brushNode}}});

    const auto& tag = document->smartTag("entity");
    CHECK(tag.matches(*brushNode));

    CHECK(tag.canDisable());

    document->selectNodes({brushNode});

    auto callback = TestCallback{0};
    tag.disable(callback, *document);
    CHECK_FALSE(tag.matches(*brushNode));
  }

  SECTION("tagInitializeBrushTags")
  {
    auto* entityNode = new mdl::EntityNode{mdl::Entity{{
      {"classname", "brush_entity"},
    }}};
    document->addNodes({{document->parentForNodes(), {entityNode}}});

    auto* brush = createBrushNode("some_material");
    document->addNodes({{entityNode, {brush}}});

    const auto& tag = document->smartTag("entity");
    CHECK(brush->hasTag(tag));
  }

  SECTION("tagRemoveBrushTags")
  {
    auto* entityNode = new mdl::EntityNode{mdl::Entity{{
      {"classname", "brush_entity"},
    }}};
    document->addNodes({{document->parentForNodes(), {entityNode}}});

    auto* brush = createBrushNode("some_material");
    document->addNodes({{entityNode, {brush}}});

    document->removeNodes({brush});

    const auto& tag = document->smartTag("entity");
    CHECK_FALSE(brush->hasTag(tag));
  }

  SECTION("tagUpdateBrushTags")
  {
    auto* brushNode = createBrushNode("some_material");
    document->addNodes({{document->parentForNodes(), {brushNode}}});

    auto* entityNode = new mdl::EntityNode{mdl::Entity{{
      {"classname", "brush_entity"},
    }}};
    document->addNodes({{document->parentForNodes(), {entityNode}}});

    const auto& tag = document->smartTag("entity");
    CHECK_FALSE(brushNode->hasTag(tag));

    document->reparentNodes({{entityNode, {brushNode}}});
    CHECK(brushNode->hasTag(tag));
  }

  SECTION("tagUpdateBrushTagsAfterReparenting")
  {
    auto* lightEntityNode = new mdl::EntityNode{mdl::Entity{{
      {"classname", "brush_entity"},
    }}};
    document->addNodes({{document->parentForNodes(), {lightEntityNode}}});

    auto* otherEntityNode = new mdl::EntityNode{mdl::Entity{{
      {"classname", "other"},
    }}};
    document->addNodes({{document->parentForNodes(), {otherEntityNode}}});

    auto* brushNode = createBrushNode("some_material");
    document->addNodes({{otherEntityNode, {brushNode}}});

    const auto& tag = document->smartTag("entity");
    CHECK_FALSE(brushNode->hasTag(tag));

    document->reparentNodes({{lightEntityNode, {brushNode}}});
    CHECK(brushNode->hasTag(tag));
  }

  SECTION("tagUpdateBrushTagsAfterChangingClassname")
  {
    auto* lightEntityNode = new mdl::EntityNode{mdl::Entity{{
      {"classname", "asdf"},
    }}};
    document->addNodes({{document->parentForNodes(), {lightEntityNode}}});

    auto* brushNode = createBrushNode("some_material");
    document->addNodes({{lightEntityNode, {brushNode}}});

    const auto& tag = document->smartTag("entity");
    CHECK_FALSE(brushNode->hasTag(tag));

    document->selectNodes({lightEntityNode});
    document->setProperty("classname", "brush_entity");
    document->deselectAll();

    CHECK(brushNode->hasTag(tag));
  }

  SECTION("tagInitializeBrushFaceTags")
  {
    auto* brushNodeWithTags = createBrushNode("some_material");
    document->addNodes({{document->parentForNodes(), {brushNodeWithTags}}});
    document->selectNodes({brushNodeWithTags});

    SECTION("No modification to brush") {}
    SECTION("Vertex manipulation")
    {
      const auto result =
        document->moveVertices({vm::vec3d::fill(16.0)}, vm::vec3d::fill(1.0));
      REQUIRE(result.success);
      REQUIRE(result.hasRemainingVertices);
    }

    const auto& tag = document->smartTag("material");
    for (const auto& face : brushNodeWithTags->brush().faces())
    {
      CHECK(face.hasTag(tag));
    }

    auto* brushNodeWithoutTags = createBrushNode("asdf");
    document->addNodes({{document->parentForNodes(), {brushNodeWithoutTags}}});

    for (const auto& face : brushNodeWithoutTags->brush().faces())
    {
      CHECK(!face.hasTag(tag));
    }
  }

  SECTION("tagRemoveBrushFaceTags")
  {
    auto* brushNodeWithTags = createBrushNode("some_material");
    document->addNodes({{document->parentForNodes(), {brushNodeWithTags}}});
    document->removeNodes({brushNodeWithTags});

    const auto& tag = document->smartTag("material");
    for (const auto& face : brushNodeWithTags->brush().faces())
    {
      CHECK_FALSE(face.hasTag(tag));
    }
  }

  SECTION("tagUpdateBrushFaceTags")
  {
    auto* brushNode = createBrushNode("asdf");
    document->addNodes({{document->parentForNodes(), {brushNode}}});

    const auto& tag = document->smartTag("contentflags");

    const auto faceHandle = mdl::BrushFaceHandle{brushNode, 0u};
    CHECK_FALSE(faceHandle.face().hasTag(tag));

    mdl::ChangeBrushFaceAttributesRequest request;
    request.setContentFlags(1);

    document->selectBrushFaces({faceHandle});
    document->setFaceAttributes(request);
    document->deselectAll();

    const auto& faces = brushNode->brush().faces();
    CHECK(faces[0].hasTag(tag));
    for (size_t i = 1u; i < faces.size(); ++i)
    {
      CHECK(!faces[i].hasTag(tag));
    }
  }
}

} // namespace tb::ui
