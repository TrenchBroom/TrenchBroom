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

#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "IO/Path.h"
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
#include "View/MapDocumentTest.h"

#include <vector>

#include "TestUtils.h"

#include "Catch2.h"

namespace TrenchBroom {
namespace View {
class TagManagementTest : public MapDocumentTest {
protected:
  Assets::Texture* m_textureA;
  Assets::Texture* m_textureB;
  Assets::Texture* m_textureC;
  const Assets::TextureCollection* m_textureCollection;

private:
  void SetUp() {
    auto textureA = Assets::Texture("some_texture", 16, 16);
    auto textureB = Assets::Texture("other_texture", 32, 32);
    auto textureC = Assets::Texture("yet_another_texture", 64, 64);

    const std::string singleParam("some_parm");
    const std::set<std::string> multiParams({"parm1", "parm2"});

    textureA.setSurfaceParms({singleParam});
    textureB.setSurfaceParms(multiParams);

    std::vector<Assets::Texture> textures;
    textures.push_back(std::move(textureA));
    textures.push_back(std::move(textureB));
    textures.push_back(std::move(textureC));

    std::vector<Assets::TextureCollection> collections;
    collections.emplace_back(std::move(textures));

    auto& textureManager = document->textureManager();
    textureManager.setTextureCollections(std::move(collections));
    m_textureCollection = &textureManager.collections().back();

    m_textureA = textureManager.texture("some_texture");
    m_textureB = textureManager.texture("other_texture");
    m_textureC = textureManager.texture("yet_another_texture");

    const std::string textureMatch("some_texture");
    const std::string texturePatternMatch("*er_texture");
    const std::string singleParamMatch("parm2");
    const kdl::vector_set<std::string> multiParamsMatch{"some_parm", "parm1", "parm3"};
    game->setSmartTags(
      {Model::SmartTag("texture", {}, std::make_unique<Model::TextureNameTagMatcher>(textureMatch)),
       Model::SmartTag(
         "texturePattern", {}, std::make_unique<Model::TextureNameTagMatcher>(texturePatternMatch)),
       Model::SmartTag(
         "surfaceparm_single", {},
         std::make_unique<Model::SurfaceParmTagMatcher>(singleParamMatch)),
       Model::SmartTag(
         "surfaceparm_multi", {}, std::make_unique<Model::SurfaceParmTagMatcher>(multiParamsMatch)),
       Model::SmartTag("contentflags", {}, std::make_unique<Model::ContentFlagsTagMatcher>(1)),
       Model::SmartTag("surfaceflags", {}, std::make_unique<Model::SurfaceFlagsTagMatcher>(1)),
       Model::SmartTag(
         "entity", {}, std::make_unique<Model::EntityClassNameTagMatcher>("brush_entity", ""))});
    document->registerSmartTags();
  }

protected:
  TagManagementTest()
    : MapDocumentTest() {
    SetUp();
  }
};

class TestCallback : public Model::TagMatcherCallback {
private:
  size_t m_option;

public:
  explicit TestCallback(const size_t option)
    : m_option(option) {}

  size_t selectOption(const std::vector<std::string>& /* options */) { return m_option; }
};

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRegistration") {
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

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRegistrationAssignsIndexes") {
  CHECK(document->smartTag("texture").index() == 0u);
  CHECK(document->smartTag("texturePattern").index() == 1u);
  CHECK(document->smartTag("surfaceparm_single").index() == 2u);
  CHECK(document->smartTag("surfaceparm_multi").index() == 3u);
  CHECK(document->smartTag("contentflags").index() == 4u);
  CHECK(document->smartTag("surfaceflags").index() == 5u);
  CHECK(document->smartTag("entity").index() == 6u);
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRegistrationAssignsTypes") {
  CHECK(document->smartTag("texture").type() == 1u);
  CHECK(document->smartTag("texturePattern").type() == 2u);
  CHECK(document->smartTag("surfaceparm_single").type() == 4u);
  CHECK(document->smartTag("surfaceparm_multi").type() == 8u);
  CHECK(document->smartTag("contentflags").type() == 16u);
  CHECK(document->smartTag("surfaceflags").type() == 32u);
  CHECK(document->smartTag("entity").type() == 64u);
}

// https://github.com/TrenchBroom/TrenchBroom/issues/2905
TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.duplicateTag") {
  game->setSmartTags({
    Model::SmartTag("texture", {}, std::make_unique<Model::TextureNameTagMatcher>("some_texture")),
    Model::SmartTag(
      "texture", {}, std::make_unique<Model::SurfaceParmTagMatcher>("some_other_texture")),
  });
  CHECK_THROWS_AS(document->registerSmartTags(), std::logic_error);
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchTextureNameTag") {
  auto nodeA = std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureA->name()));
  auto nodeB = std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureB->name()));
  auto nodeC = std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureC->name()));
  const auto& tag = document->smartTag("texture");
  const auto& patternTag = document->smartTag("texturePattern");
  for (const auto& face : nodeA->brush().faces()) {
    CHECK(tag.matches(face));
    CHECK_FALSE(patternTag.matches(face));
  }
  for (const auto& face : nodeB->brush().faces()) {
    CHECK_FALSE(tag.matches(face));
    CHECK(patternTag.matches(face));
  }
  for (const auto& face : nodeC->brush().faces()) {
    CHECK_FALSE(tag.matches(face));
    CHECK(patternTag.matches(face));
  }
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableTextureNameTag") {
  auto* nonMatchingBrushNode = createBrushNode("asdf");
  addNode(*document, document->parentForNodes(), nonMatchingBrushNode);

  const auto& tag = document->smartTag("texture");
  CHECK(tag.canEnable());

  const auto faceHandle = Model::BrushFaceHandle(nonMatchingBrushNode, 0u);
  CHECK_FALSE(tag.matches(faceHandle.face()));

  document->select(faceHandle);

  TestCallback callback(0);
  tag.enable(callback, *document);

  CHECK(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableTextureNameTag") {
  const auto& tag = document->smartTag("texture");
  CHECK_FALSE(tag.canDisable());
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchSurfaceParmTag") {
  auto nodeA = std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureA->name(), [&](auto& b) {
    for (auto& face : b.faces()) {
      face.setTexture(m_textureA);
    }
  }));
  auto nodeB = std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureB->name(), [&](auto& b) {
    for (auto& face : b.faces()) {
      face.setTexture(m_textureB);
    }
  }));
  auto nodeC = std::unique_ptr<Model::BrushNode>(createBrushNode(m_textureC->name(), [&](auto& b) {
    for (auto& face : b.faces()) {
      face.setTexture(m_textureC);
    }
  }));
  const auto& singleTag = document->smartTag("surfaceparm_single");
  const auto& multiTag = document->smartTag("surfaceparm_multi");
  for (const auto& face : nodeA->brush().faces()) {
    CHECK_FALSE(singleTag.matches(face));
    CHECK(multiTag.matches(face));
  }
  for (const auto& face : nodeB->brush().faces()) {
    CHECK(singleTag.matches(face));
    CHECK(multiTag.matches(face));
  }
  for (const auto& face : nodeC->brush().faces()) {
    CHECK_FALSE(singleTag.matches(face));
    CHECK_FALSE(multiTag.matches(face));
  }
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableSurfaceParmTag") {
  auto* nonMatchingBrushNode = createBrushNode("asdf");
  addNode(*document, document->parentForNodes(), nonMatchingBrushNode);

  const auto& tag = document->smartTag("surfaceparm_single");
  CHECK(tag.canEnable());

  const auto faceHandle = Model::BrushFaceHandle(nonMatchingBrushNode, 0u);
  CHECK_FALSE(tag.matches(faceHandle.face()));

  document->select(faceHandle);

  TestCallback callback(0);
  tag.enable(callback, *document);

  CHECK(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableSurfaceParmTag") {
  const auto& tag = document->smartTag("surfaceparm_single");
  CHECK_FALSE(tag.canDisable());
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchContentFlagsTag") {
  auto matchingBrushNode = std::unique_ptr<Model::BrushNode>(createBrushNode("asdf", [](auto& b) {
    for (auto& face : b.faces()) {
      auto attributes = face.attributes();
      attributes.setSurfaceContents(1);
      face.setAttributes(attributes);
    }
  }));
  auto nonMatchingBrushNode =
    std::unique_ptr<Model::BrushNode>(createBrushNode("asdf", [](auto& b) {
      for (auto& face : b.faces()) {
        auto attributes = face.attributes();
        attributes.setSurfaceContents(2);
        face.setAttributes(attributes);
      }
    }));

  const auto& tag = document->smartTag("contentflags");
  for (const auto& face : matchingBrushNode->brush().faces()) {
    CHECK(tag.matches(face));
  }
  for (const auto& face : nonMatchingBrushNode->brush().faces()) {
    CHECK_FALSE(tag.matches(face));
  }
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableContentFlagsTag") {
  auto* nonMatchingBrushNode = createBrushNode("asdf");
  addNode(*document, document->parentForNodes(), nonMatchingBrushNode);

  const auto& tag = document->smartTag("contentflags");
  CHECK(tag.canEnable());

  const auto faceHandle = Model::BrushFaceHandle(nonMatchingBrushNode, 0u);
  CHECK_FALSE(tag.matches(faceHandle.face()));

  document->select(faceHandle);

  TestCallback callback(0);
  tag.enable(callback, *document);

  CHECK(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableContentFlagsTag") {
  auto* matchingBrushNode = createBrushNode("asdf", [](auto& b) {
    for (auto& face : b.faces()) {
      auto attributes = face.attributes();
      attributes.setSurfaceContents(1);
      face.setAttributes(attributes);
    }
  });

  addNode(*document, document->parentForNodes(), matchingBrushNode);

  const auto& tag = document->smartTag("contentflags");
  CHECK(tag.canDisable());

  const auto faceHandle = Model::BrushFaceHandle(matchingBrushNode, 0u);
  CHECK(tag.matches(faceHandle.face()));

  document->select(faceHandle);

  TestCallback callback(0);
  tag.disable(callback, *document);

  CHECK_FALSE(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchSurfaceFlagsTag") {
  auto matchingBrushNode = std::unique_ptr<Model::BrushNode>(createBrushNode("asdf", [](auto& b) {
    for (auto& face : b.faces()) {
      auto attributes = face.attributes();
      attributes.setSurfaceFlags(1);
      face.setAttributes(attributes);
    }
  }));
  auto nonMatchingBrushNode =
    std::unique_ptr<Model::BrushNode>(createBrushNode("asdf", [](auto& b) {
      for (auto& face : b.faces()) {
        auto attributes = face.attributes();
        attributes.setSurfaceFlags(2);
        face.setAttributes(attributes);
      }
    }));

  const auto& tag = document->smartTag("surfaceflags");
  for (const auto& face : matchingBrushNode->brush().faces()) {
    CHECK(tag.matches(face));
  }
  for (const auto& face : nonMatchingBrushNode->brush().faces()) {
    CHECK_FALSE(tag.matches(face));
  }
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableSurfaceFlagsTag") {
  auto* nonMatchingBrushNode = createBrushNode("asdf");
  addNode(*document, document->parentForNodes(), nonMatchingBrushNode);

  const auto& tag = document->smartTag("surfaceflags");
  CHECK(tag.canEnable());

  const auto faceHandle = Model::BrushFaceHandle(nonMatchingBrushNode, 0u);
  CHECK_FALSE(tag.matches(faceHandle.face()));

  document->select(faceHandle);

  TestCallback callback(0);
  tag.enable(callback, *document);

  CHECK(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableSurfaceFlagsTag") {
  auto* matchingBrushNode = createBrushNode("asdf", [](auto& b) {
    for (auto& face : b.faces()) {
      auto attributes = face.attributes();
      attributes.setSurfaceFlags(1);
      face.setAttributes(attributes);
    }
  });

  addNode(*document, document->parentForNodes(), matchingBrushNode);

  const auto& tag = document->smartTag("surfaceflags");
  CHECK(tag.canDisable());

  const auto faceHandle = Model::BrushFaceHandle(matchingBrushNode, 0u);
  CHECK(tag.matches(faceHandle.face()));

  document->select(faceHandle);

  TestCallback callback(0);
  tag.disable(callback, *document);

  CHECK_FALSE(tag.matches(faceHandle.face()));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchEntityClassnameTag") {
  auto* matchingBrushNode = createBrushNode("asdf");
  auto* nonMatchingBrushNode = createBrushNode("asdf");

  auto matchingEntity =
    std::make_unique<Model::EntityNode>(Model::Entity{{}, {{"classname", "brush_entity"}}});
  matchingEntity->addChild(matchingBrushNode);

  auto nonMatchingEntity =
    std::make_unique<Model::EntityNode>(Model::Entity{{}, {{"classname", "something"}}});
  nonMatchingEntity->addChild(nonMatchingBrushNode);

  const auto& tag = document->smartTag("entity");
  CHECK(tag.matches(*matchingBrushNode));
  CHECK_FALSE(tag.matches(*nonMatchingBrushNode));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableEntityClassnameTag") {
  auto* brushNode = createBrushNode("asdf");
  addNode(*document, document->parentForNodes(), brushNode);

  const auto& tag = document->smartTag("entity");
  CHECK_FALSE(tag.matches(*brushNode));

  CHECK(tag.canEnable());

  document->select(brushNode);

  TestCallback callback(0);
  tag.enable(callback, *document);
  CHECK(tag.matches(*brushNode));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableEntityClassnameTagRetainsAttributes") {
  auto* brushNode = createBrushNode("asdf");

  auto* oldEntity =
    new Model::EntityNode{{}, {{"classname", "something"}, {"some_attr", "some_value"}}};

  addNode(*document, document->parentForNodes(), oldEntity);
  addNode(*document, oldEntity, brushNode);

  const auto& tag = document->smartTag("entity");
  document->select(brushNode);

  TestCallback callback(0);
  tag.enable(callback, *document);
  CHECK(tag.matches(*brushNode));

  auto* newEntityNode = brushNode->entity();
  CHECK(newEntityNode != oldEntity);

  CHECK(newEntityNode != nullptr);
  CHECK(newEntityNode->entity().hasProperty("some_attr"));
  CHECK(*newEntityNode->entity().property("some_attr") == "some_value");
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableEntityClassnameTag") {
  auto* brushNode = createBrushNode("asdf");

  auto* oldEntity = new Model::EntityNode{{}, {{"classname", "brush_entity"}}};

  addNode(*document, document->parentForNodes(), oldEntity);
  addNode(*document, oldEntity, brushNode);

  const auto& tag = document->smartTag("entity");
  CHECK(tag.matches(*brushNode));

  CHECK(tag.canDisable());

  document->select(brushNode);

  TestCallback callback(0);
  tag.disable(callback, *document);
  CHECK_FALSE(tag.matches(*brushNode));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagInitializeBrushTags") {
  auto* entityNode = new Model::EntityNode{{}, {{"classname", "brush_entity"}}};
  addNode(*document, document->parentForNodes(), entityNode);

  auto* brush = createBrushNode("some_texture");
  addNode(*document, entityNode, brush);

  const auto& tag = document->smartTag("entity");
  CHECK(brush->hasTag(tag));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRemoveBrushTags") {
  auto* entityNode = new Model::EntityNode{{}, {{"classname", "brush_entity"}}};
  addNode(*document, document->parentForNodes(), entityNode);

  auto* brush = createBrushNode("some_texture");
  addNode(*document, entityNode, brush);

  removeNode(*document, brush);

  const auto& tag = document->smartTag("entity");
  CHECK_FALSE(brush->hasTag(tag));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagUpdateBrushTags") {
  auto* brushNode = createBrushNode("some_texture");
  addNode(*document, document->parentForNodes(), brushNode);

  auto* entityNode = new Model::EntityNode{{}, {{"classname", "brush_entity"}}};
  addNode(*document, document->parentForNodes(), entityNode);

  const auto& tag = document->smartTag("entity");
  CHECK_FALSE(brushNode->hasTag(tag));

  reparentNodes(*document, entityNode, {brushNode});
  CHECK(brushNode->hasTag(tag));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagUpdateBrushTagsAfterReparenting") {
  auto* lightEntityNode = new Model::EntityNode{{}, {{"classname", "brush_entity"}}};
  addNode(*document, document->parentForNodes(), lightEntityNode);

  auto* otherEntityNode = new Model::EntityNode{{}, {{"classname", "other"}}};
  addNode(*document, document->parentForNodes(), otherEntityNode);

  auto* brushNode = createBrushNode("some_texture");
  addNode(*document, otherEntityNode, brushNode);

  const auto& tag = document->smartTag("entity");
  CHECK_FALSE(brushNode->hasTag(tag));

  reparentNodes(*document, lightEntityNode, {brushNode});
  CHECK(brushNode->hasTag(tag));
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagUpdateBrushTagsAfterChangingClassname") {
  auto* lightEntityNode = new Model::EntityNode{{}, {{"classname", "asdf"}}};
  addNode(*document, document->parentForNodes(), lightEntityNode);

  auto* brushNode = createBrushNode("some_texture");
  addNode(*document, lightEntityNode, brushNode);

  const auto& tag = document->smartTag("entity");
  CHECK_FALSE(brushNode->hasTag(tag));

  document->select(lightEntityNode);
  document->setProperty("classname", "brush_entity");
  document->deselectAll();

  CHECK(brushNode->hasTag(tag));
}

TEST_CASE_METHOD(
  TagManagementTest, "TagManagementTest.tagInitializeBrushFaceTags", "[TagManagementTest]") {
  auto* brushNodeWithTags = createBrushNode("some_texture");
  addNode(*document, document->parentForNodes(), brushNodeWithTags);
  document->select(brushNodeWithTags);

  SECTION("No modification to brush") {}
  SECTION("Vertex manipulation") {
    const auto result = document->moveVertices({vm::vec3::fill(16.0)}, vm::vec3::fill(1.0));
    REQUIRE(result.success);
    REQUIRE(result.hasRemainingVertices);
  }

  const auto& tag = document->smartTag("texture");
  for (const auto& face : brushNodeWithTags->brush().faces()) {
    CHECK(face.hasTag(tag));
  }

  auto* brushNodeWithoutTags = createBrushNode("asdf");
  addNode(*document, document->parentForNodes(), brushNodeWithoutTags);

  for (const auto& face : brushNodeWithoutTags->brush().faces()) {
    CHECK(!face.hasTag(tag));
  }
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRemoveBrushFaceTags") {
  auto* brushNodeWithTags = createBrushNode("some_texture");
  addNode(*document, document->parentForNodes(), brushNodeWithTags);
  removeNode(*document, brushNodeWithTags);

  const auto& tag = document->smartTag("texture");
  for (const auto& face : brushNodeWithTags->brush().faces()) {
    CHECK_FALSE(face.hasTag(tag));
  }
}

TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagUpdateBrushFaceTags") {
  auto* brushNode = createBrushNode("asdf");
  addNode(*document, document->parentForNodes(), brushNode);

  const auto& tag = document->smartTag("contentflags");

  const auto faceHandle = Model::BrushFaceHandle(brushNode, 0u);
  CHECK_FALSE(faceHandle.face().hasTag(tag));

  Model::ChangeBrushFaceAttributesRequest request;
  request.setContentFlags(1);

  document->select(faceHandle);
  document->setFaceAttributes(request);
  document->deselectAll();

  const auto& faces = brushNode->brush().faces();
  CHECK(faces[0].hasTag(tag));
  for (size_t i = 1u; i < faces.size(); ++i) {
    CHECK(!faces[i].hasTag(tag));
  }
}
} // namespace View
} // namespace TrenchBroom
