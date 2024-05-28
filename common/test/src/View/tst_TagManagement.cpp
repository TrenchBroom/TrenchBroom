/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Assets/Material.h"
#include "Assets/MaterialCollection.h"
#include "Assets/MaterialManager.h"
#include "IO/TestEnvironment.h"
#include "Logger.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EntityNode.h"
#include "Model/LayerNode.h"
#include "Model/Tag.h"
#include "Model/TagMatcher.h"
#include "Model/TestGame.h"
#include "TestUtils.h"
#include "View/MapDocumentTest.h"

#include "kdl/vector_utils.h"

#include <filesystem>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom::View
{
namespace
{

class TagManagementTest : public MapDocumentTest
{
protected:
  Assets::Material* m_textureA = nullptr;
  Assets::Material* m_textureB = nullptr;
  Assets::Material* m_textureC = nullptr;
  const Assets::MaterialCollection* m_textureCollection = nullptr;

private:
  void SetUp()
  {
    auto textureA = Assets::Material{"some_texture", 16, 16};
    auto textureB = Assets::Material{"other_texture", 32, 32};
    auto textureC = Assets::Material{"yet_another_texture", 64, 64};

    const auto singleParam = std::string{"some_parm"};
    const auto multiParams = std::set<std::string>{"parm1", "parm2"};

    textureA.setSurfaceParms({singleParam});
    textureB.setSurfaceParms(multiParams);

    auto textures =
      kdl::vec_from(std::move(textureA), std::move(textureB), std::move(textureC));

    auto collections = kdl::vec_from(Assets::MaterialCollection{std::move(textures)});

    auto& textureManager = document->textureManager();
    textureManager.setMaterialCollections(std::move(collections));
    m_textureCollection = &textureManager.collections().back();

    m_textureA = textureManager.material("some_texture");
    m_textureB = textureManager.material("other_texture");
    m_textureC = textureManager.material("yet_another_texture");

    const auto textureMatch = std::string{"some_texture"};
    const auto texturePatternMatch = std::string{"*er_texture"};
    const auto singleParamMatch = std::string{"parm2"};
    const auto multiParamsMatch =
      kdl::vector_set<std::string>{"some_parm", "parm1", "parm3"};
    game->setSmartTags(
      {Model::SmartTag{
         "texture",
         {},
         std::make_unique<Model::MaterialNameTagMatcher>(textureMatch),
       },
       Model::SmartTag{
         "texturePattern",
         {},
         std::make_unique<Model::MaterialNameTagMatcher>(texturePatternMatch),
       },
       Model::SmartTag{
         "surfaceparm_single",
         {},
         std::make_unique<Model::SurfaceParmTagMatcher>(singleParamMatch),
       },
       Model::SmartTag{
         "surfaceparm_multi",
         {},
         std::make_unique<Model::SurfaceParmTagMatcher>(multiParamsMatch),
       },
       Model::SmartTag{
         "contentflags",
         {},
         std::make_unique<Model::ContentFlagsTagMatcher>(1),
       },
       Model::SmartTag{
         "surfaceflags",
         {},
         std::make_unique<Model::SurfaceFlagsTagMatcher>(1),
       },
       Model::SmartTag{
         "entity",
         {},
         std::make_unique<Model::EntityClassNameTagMatcher>("brush_entity", ""),
       }});
    document->registerSmartTags();
  }

protected:
  TagManagementTest() { SetUp(); }
};

class TestCallback : public Model::TagMatcherCallback
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

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRegistration")
{
  CHECK(document->isRegisteredSmartTag("texture"));
  CHECK(document->isRegisteredSmartTag("texturePattern"));
  CHECK(document->isRegisteredSmartTag("surfaceparm_single"));
  CHECK(document->isRegisteredSmartTag("surfaceparm_multi"));
  CHECK(document->isRegisteredSmartTag("contentflags"));
  CHECK(document->isRegisteredSmartTag("surfaceflags"));
  CHECK(document->isRegisteredSmartTag("entity"));
  CHECK_FALSE(document->isRegisteredSmartTag(""));
  CHECK_FALSE(document->isRegisteredSmartTag("asdf"));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRegistrationAssignsIndexes")
{
  CHECK(document->smartTag("texture").index() == 0u);
  CHECK(document->smartTag("texturePattern").index() == 1u);
  CHECK(document->smartTag("surfaceparm_single").index() == 2u);
  CHECK(document->smartTag("surfaceparm_multi").index() == 3u);
  CHECK(document->smartTag("contentflags").index() == 4u);
  CHECK(document->smartTag("surfaceflags").index() == 5u);
  CHECK(document->smartTag("entity").index() == 6u);
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRegistrationAssignsTypes")
{
  CHECK(document->smartTag("texture").type() == 1u);
  CHECK(document->smartTag("texturePattern").type() == 2u);
  CHECK(document->smartTag("surfaceparm_single").type() == 4u);
  CHECK(document->smartTag("surfaceparm_multi").type() == 8u);
  CHECK(document->smartTag("contentflags").type() == 16u);
  CHECK(document->smartTag("surfaceflags").type() == 32u);
  CHECK(document->smartTag("entity").type() == 64u);
}

// https://github.com/TrenchBroom/TrenchBroom/issues/2905
TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.duplicateTag")
{
  game->setSmartTags({
    Model::SmartTag{
      "texture",
      {},
      std::make_unique<Model::MaterialNameTagMatcher>("some_texture"),
    },
    Model::SmartTag{
      "texture",
      {},
      std::make_unique<Model::SurfaceParmTagMatcher>("some_other_texture"),
    },
  });
  CHECK_THROWS_AS(document->registerSmartTags(), std::logic_error);
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchTextureNameTag")
{
  auto nodeA = std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureA->name()));
  auto nodeB = std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureB->name()));
  auto nodeC = std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureC->name()));
  const auto& tag = document->smartTag("texture");
  const auto& patternTag = document->smartTag("texturePattern");
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

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableTextureNameTag")
{
  auto* nonMatchingBrushNode = createBrushNode("asdf");
  document->addNodes({{document->parentForNodes(), {nonMatchingBrushNode}}});

  const auto& tag = document->smartTag("texture");
  CHECK(tag.canEnable());

  const auto faceHandle = Model::BrushFaceHandle{nonMatchingBrushNode, 0u};
  CHECK_FALSE(tag.matches(faceHandle.face()));

  document->selectBrushFaces({faceHandle});

  auto callback = TestCallback{0};
  tag.enable(callback, *document);

  CHECK(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableTextureNameTag")
{
  const auto& tag = document->smartTag("texture");
  CHECK_FALSE(tag.canDisable());
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchSurfaceParmTag")
{
  auto nodeA =
    std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureA->name(), [&](auto& b) {
      for (auto& face : b.faces())
      {
        face.setTexture(m_textureA);
      }
    }));
  auto nodeB =
    std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureB->name(), [&](auto& b) {
      for (auto& face : b.faces())
      {
        face.setTexture(m_textureB);
      }
    }));
  auto nodeC =
    std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureC->name(), [&](auto& b) {
      for (auto& face : b.faces())
      {
        face.setTexture(m_textureC);
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

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableSurfaceParmTag")
{
  auto* nonMatchingBrushNode = createBrushNode("asdf");
  document->addNodes({{document->parentForNodes(), {nonMatchingBrushNode}}});

  const auto& tag = document->smartTag("surfaceparm_single");
  CHECK(tag.canEnable());

  const auto faceHandle = Model::BrushFaceHandle{nonMatchingBrushNode, 0u};
  CHECK_FALSE(tag.matches(faceHandle.face()));

  document->selectBrushFaces({faceHandle});

  auto callback = TestCallback{0};
  tag.enable(callback, *document);

  CHECK(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableSurfaceParmTag")
{
  const auto& tag = document->smartTag("surfaceparm_single");
  CHECK_FALSE(tag.canDisable());
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchContentFlagsTag")
{
  auto matchingBrushNode =
    std::unique_ptr<Model::BrushNode>(createBrushNode("asdf", [](auto& b) {
      for (auto& face : b.faces())
      {
        auto attributes = face.attributes();
        attributes.setSurfaceContents(1);
        face.setAttributes(attributes);
      }
    }));
  auto nonMatchingBrushNode =
    std::unique_ptr<Model::BrushNode>(createBrushNode("asdf", [](auto& b) {
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

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableContentFlagsTag")
{
  auto* nonMatchingBrushNode = createBrushNode("asdf");
  document->addNodes({{document->parentForNodes(), {nonMatchingBrushNode}}});

  const auto& tag = document->smartTag("contentflags");
  CHECK(tag.canEnable());

  const auto faceHandle = Model::BrushFaceHandle{nonMatchingBrushNode, 0u};
  CHECK_FALSE(tag.matches(faceHandle.face()));

  document->selectBrushFaces({faceHandle});

  auto callback = TestCallback{0};
  tag.enable(callback, *document);

  CHECK(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableContentFlagsTag")
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

  const auto faceHandle = Model::BrushFaceHandle{matchingBrushNode, 0u};
  CHECK(tag.matches(faceHandle.face()));

  document->selectBrushFaces({faceHandle});

  auto callback = TestCallback{0};
  tag.disable(callback, *document);

  CHECK_FALSE(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchSurfaceFlagsTag")
{
  auto matchingBrushNode =
    std::unique_ptr<Model::BrushNode>(createBrushNode("asdf", [](auto& b) {
      for (auto& face : b.faces())
      {
        auto attributes = face.attributes();
        attributes.setSurfaceFlags(1);
        face.setAttributes(attributes);
      }
    }));
  auto nonMatchingBrushNode =
    std::unique_ptr<Model::BrushNode>(createBrushNode("asdf", [](auto& b) {
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

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableSurfaceFlagsTag")
{
  auto* nonMatchingBrushNode = createBrushNode("asdf");
  document->addNodes({{document->parentForNodes(), {nonMatchingBrushNode}}});

  const auto& tag = document->smartTag("surfaceflags");
  CHECK(tag.canEnable());

  const auto faceHandle = Model::BrushFaceHandle{nonMatchingBrushNode, 0u};
  CHECK_FALSE(tag.matches(faceHandle.face()));

  document->selectBrushFaces({faceHandle});

  auto callback = TestCallback{0};
  tag.enable(callback, *document);

  CHECK(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableSurfaceFlagsTag")
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

  const auto faceHandle = Model::BrushFaceHandle{matchingBrushNode, 0u};
  CHECK(tag.matches(faceHandle.face()));

  document->selectBrushFaces({faceHandle});

  auto callback = TestCallback{0};
  tag.disable(callback, *document);

  CHECK_FALSE(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchEntityClassnameTag")
{
  auto* matchingBrushNode = createBrushNode("asdf");
  auto* nonMatchingBrushNode = createBrushNode("asdf");

  auto matchingEntity = std::make_unique<Model::EntityNode>(
    Model::Entity{{}, {{"classname", "brush_entity"}}});
  matchingEntity->addChild(matchingBrushNode);

  auto nonMatchingEntity =
    std::make_unique<Model::EntityNode>(Model::Entity{{}, {{"classname", "something"}}});
  nonMatchingEntity->addChild(nonMatchingBrushNode);

  const auto& tag = document->smartTag("entity");
  CHECK(tag.matches(*matchingBrushNode));
  CHECK_FALSE(tag.matches(*nonMatchingBrushNode));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableEntityClassnameTag")
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

TEST_CASE_METHOD(
  TagManagementTest, "TagManagementTest.enableEntityClassnameTagRetainsAttributes")
{
  auto* brushNode = createBrushNode("asdf");

  auto* oldEntity =
    new Model::EntityNode{{}, {{"classname", "something"}, {"some_attr", "some_value"}}};

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

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableEntityClassnameTag")
{
  auto* brushNode = createBrushNode("asdf");

  auto* oldEntity = new Model::EntityNode{{}, {{"classname", "brush_entity"}}};

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

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagInitializeBrushTags")
{
  auto* entityNode = new Model::EntityNode{{}, {{"classname", "brush_entity"}}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  auto* brush = createBrushNode("some_texture");
  document->addNodes({{entityNode, {brush}}});

  const auto& tag = document->smartTag("entity");
  CHECK(brush->hasTag(tag));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRemoveBrushTags")
{
  auto* entityNode = new Model::EntityNode{{}, {{"classname", "brush_entity"}}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  auto* brush = createBrushNode("some_texture");
  document->addNodes({{entityNode, {brush}}});

  document->removeNodes({brush});

  const auto& tag = document->smartTag("entity");
  CHECK_FALSE(brush->hasTag(tag));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagUpdateBrushTags")
{
  auto* brushNode = createBrushNode("some_texture");
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  auto* entityNode = new Model::EntityNode{{}, {{"classname", "brush_entity"}}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  const auto& tag = document->smartTag("entity");
  CHECK_FALSE(brushNode->hasTag(tag));

  document->reparentNodes({{entityNode, {brushNode}}});
  CHECK(brushNode->hasTag(tag));
}

TEST_CASE_METHOD(
  TagManagementTest, "TagManagementTest.tagUpdateBrushTagsAfterReparenting")
{
  auto* lightEntityNode = new Model::EntityNode{{}, {{"classname", "brush_entity"}}};
  document->addNodes({{document->parentForNodes(), {lightEntityNode}}});

  auto* otherEntityNode = new Model::EntityNode{{}, {{"classname", "other"}}};
  document->addNodes({{document->parentForNodes(), {otherEntityNode}}});

  auto* brushNode = createBrushNode("some_texture");
  document->addNodes({{otherEntityNode, {brushNode}}});

  const auto& tag = document->smartTag("entity");
  CHECK_FALSE(brushNode->hasTag(tag));

  document->reparentNodes({{lightEntityNode, {brushNode}}});
  CHECK(brushNode->hasTag(tag));
}

TEST_CASE_METHOD(
  TagManagementTest, "TagManagementTest.tagUpdateBrushTagsAfterChangingClassname")
{
  auto* lightEntityNode = new Model::EntityNode{{}, {{"classname", "asdf"}}};
  document->addNodes({{document->parentForNodes(), {lightEntityNode}}});

  auto* brushNode = createBrushNode("some_texture");
  document->addNodes({{lightEntityNode, {brushNode}}});

  const auto& tag = document->smartTag("entity");
  CHECK_FALSE(brushNode->hasTag(tag));

  document->selectNodes({lightEntityNode});
  document->setProperty("classname", "brush_entity");
  document->deselectAll();

  CHECK(brushNode->hasTag(tag));
}

TEST_CASE_METHOD(
  TagManagementTest,
  "TagManagementTest.tagInitializeBrushFaceTags",
  "[TagManagementTest]")
{
  auto* brushNodeWithTags = createBrushNode("some_texture");
  document->addNodes({{document->parentForNodes(), {brushNodeWithTags}}});
  document->selectNodes({brushNodeWithTags});

  SECTION("No modification to brush") {}
  SECTION("Vertex manipulation")
  {
    const auto result =
      document->moveVertices({vm::vec3::fill(16.0)}, vm::vec3::fill(1.0));
    REQUIRE(result.success);
    REQUIRE(result.hasRemainingVertices);
  }

  const auto& tag = document->smartTag("texture");
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

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRemoveBrushFaceTags")
{
  auto* brushNodeWithTags = createBrushNode("some_texture");
  document->addNodes({{document->parentForNodes(), {brushNodeWithTags}}});
  document->removeNodes({brushNodeWithTags});

  const auto& tag = document->smartTag("texture");
  for (const auto& face : brushNodeWithTags->brush().faces())
  {
    CHECK_FALSE(face.hasTag(tag));
  }
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagUpdateBrushFaceTags")
{
  auto* brushNode = createBrushNode("asdf");
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const auto& tag = document->smartTag("contentflags");

  const auto faceHandle = Model::BrushFaceHandle{brushNode, 0u};
  CHECK_FALSE(faceHandle.face().hasTag(tag));

  Model::ChangeBrushFaceAttributesRequest request;
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

} // namespace TrenchBroom::View
