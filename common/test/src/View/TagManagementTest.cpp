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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Logger.h"
#include "IO/Path.h"
#include "IO/TestEnvironment.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EntityNode.h"
#include "Model/LayerNode.h"
#include "Model/Tag.h"
#include "Model/TagMatcher.h"
#include "Model/TestGame.h"
#include "View/MapDocumentTest.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class TagManagementTest : public MapDocumentTest {
        protected:
            Assets::Texture* m_matchingTexture;
            Assets::Texture* m_nonMatchingTexture;
            Assets::TextureCollection* m_textureCollection;
        private:
            void SetUp() {
                auto matchingTexture = std::make_unique<Assets::Texture>("some_texture", 16, 16);
                auto nonMatchingTexture = std::make_unique<Assets::Texture>("other_texture", 32, 32);

                matchingTexture->setSurfaceParms({"some_parm"});

                auto textureCollection = std::make_unique<Assets::TextureCollection>(std::vector<Assets::Texture*>({
                    matchingTexture.get(),
                    nonMatchingTexture.get()
                }));

                document->textureManager().setTextureCollections(std::vector<Assets::TextureCollection*>({
                    textureCollection.get()
                }));

                m_matchingTexture = matchingTexture.release();
                m_nonMatchingTexture = nonMatchingTexture.release();
                m_textureCollection = textureCollection.release();

                game->setSmartTags({
                    Model::SmartTag("texture", {}, std::make_unique<Model::TextureNameTagMatcher>("some_texture")),
                    Model::SmartTag("surfaceparm", {}, std::make_unique<Model::SurfaceParmTagMatcher>("some_parm")),
                    Model::SmartTag("contentflags", {}, std::make_unique<Model::ContentFlagsTagMatcher>(1)),
                    Model::SmartTag("surfaceflags", {}, std::make_unique<Model::SurfaceFlagsTagMatcher>(1)),
                    Model::SmartTag("entity", {}, std::make_unique<Model::EntityClassNameTagMatcher>("brush_entity", ""))
                });
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
            explicit TestCallback(const size_t option) :
            m_option(option) {}

            size_t selectOption(const std::vector<std::string>& /* options */) {
                return m_option;
            }
        };

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRegistration") {
            ASSERT_TRUE(document->isRegisteredSmartTag("texture"));
            ASSERT_TRUE(document->isRegisteredSmartTag("surfaceparm"));
            ASSERT_TRUE(document->isRegisteredSmartTag("contentflags"));
            ASSERT_TRUE(document->isRegisteredSmartTag("surfaceflags"));
            ASSERT_TRUE(document->isRegisteredSmartTag("entity"));
            ASSERT_FALSE(document->isRegisteredSmartTag(""));
            ASSERT_FALSE(document->isRegisteredSmartTag("asdf"));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRegistrationAssignsIndexes") {
            CHECK(0u == document->smartTag("texture").index());
            CHECK(1u == document->smartTag("surfaceparm").index());
            CHECK(2u == document->smartTag("contentflags").index());
            CHECK(3u == document->smartTag("surfaceflags").index());
            CHECK(4u == document->smartTag("entity").index());
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRegistrationAssignsTypes") {
            CHECK(1u == document->smartTag("texture").type());
            CHECK(2u == document->smartTag("surfaceparm").type());
            CHECK(4u == document->smartTag("contentflags").type());
            CHECK(8u == document->smartTag("surfaceflags").type());
            CHECK(16u == document->smartTag("entity").type());
        }
    

        // https://github.com/kduske/TrenchBroom/issues/2905
        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.duplicateTag") {
            game->setSmartTags({
                Model::SmartTag("texture", {}, std::make_unique<Model::TextureNameTagMatcher>("some_texture")),
                Model::SmartTag("texture", {}, std::make_unique<Model::SurfaceParmTagMatcher>("some_other_texture")),
            });
            ASSERT_THROW(document->registerSmartTags(), std::logic_error);
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchTextureNameTag") {
            auto matchingBrush = std::unique_ptr<Model::BrushNode>(createBrush("some_texture"));
            auto nonMatchingBrush = std::unique_ptr<Model::BrushNode>(createBrush("asdf"));

            const auto& tag = document->smartTag("texture");
            for (const auto* face : matchingBrush->faces()) {
                ASSERT_TRUE(tag.matches(*face));
            }
            for (const auto* face : nonMatchingBrush->faces()) {
                ASSERT_FALSE(tag.matches(*face));
            }
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableTextureNameTag") {
            auto* nonMatchingBrush = createBrush("asdf");
            document->addNode(nonMatchingBrush, document->currentParent());

            const auto& tag = document->smartTag("texture");
            ASSERT_TRUE(tag.canEnable());

            auto* face = nonMatchingBrush->faces().front();
            ASSERT_FALSE(tag.matches(*face));

            document->select({ nonMatchingBrush, face });

            TestCallback callback(0);
            tag.enable(callback, *document);

            ASSERT_TRUE(tag.matches(*face));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableTextureNameTag") {
            const auto& tag = document->smartTag("texture");
            ASSERT_FALSE(tag.canDisable());
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchSurfaceParmTag") {
            auto texture = std::make_unique<Assets::Texture>("texturename", 16, 16);
            texture->setSurfaceParms({"some_parm"});

            auto matchingBrush = std::unique_ptr<Model::BrushNode>(createBrush("some_texture"));
            auto nonMatchingBrush = std::unique_ptr<Model::BrushNode>(createBrush("asdf"));

            for (auto* face : matchingBrush->faces()) {
                face->setTexture(texture.get());
            }

            const auto& tag = document->smartTag("surfaceparm");
            for (const auto* face : matchingBrush->faces()) {
                ASSERT_TRUE(tag.matches(*face));
            }
            for (const auto* face : nonMatchingBrush->faces()) {
                ASSERT_FALSE(tag.matches(*face));
            }
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableSurfaceParmTag") {
            const auto& tag = document->smartTag("surfaceparm");
            ASSERT_FALSE(tag.canEnable());
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableSurfaceParmTag") {
            const auto& tag = document->smartTag("surfaceparm");
            ASSERT_FALSE(tag.canDisable());
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchContentFlagsTag") {
            auto matchingBrush = std::unique_ptr<Model::BrushNode>(createBrush("asdf"));
            auto nonMatchingBrush = std::unique_ptr<Model::BrushNode>(createBrush("asdf"));

            for (auto* face : matchingBrush->faces()) {
                face->setSurfaceContents(1);
            }
            for (auto* face : nonMatchingBrush->faces()) {
                face->setSurfaceContents(2);
            }

            const auto& tag = document->smartTag("contentflags");
            for (const auto* face : matchingBrush->faces()) {
                ASSERT_TRUE(tag.matches(*face));
            }
            for (const auto* face : nonMatchingBrush->faces()) {
                ASSERT_FALSE(tag.matches(*face));
            }
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableContentFlagsTag") {
            auto* nonMatchingBrush = createBrush("asdf");
            document->addNode(nonMatchingBrush, document->currentParent());

            const auto& tag = document->smartTag("contentflags");
            ASSERT_TRUE(tag.canEnable());

            auto* face = nonMatchingBrush->faces().front();
            ASSERT_FALSE(tag.matches(*face));

            document->select({ nonMatchingBrush, face });

            TestCallback callback(0);
            tag.enable(callback, *document);

            ASSERT_TRUE(tag.matches(*face));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableContentFlagsTag") {
            auto* matchingBrush = createBrush("asdf");
            for (auto* face : matchingBrush->faces()) {
                face->setSurfaceContents(1);
            }

            document->addNode(matchingBrush, document->currentParent());

            const auto& tag = document->smartTag("contentflags");
            ASSERT_TRUE(tag.canDisable());

            auto* face = matchingBrush->faces().front();
            ASSERT_TRUE(tag.matches(*face));

            document->select({ matchingBrush, face });

            TestCallback callback(0);
            tag.disable(callback, *document);

            ASSERT_FALSE(tag.matches(*face));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchSurfaceFlagsTag") {
            auto matchingBrush = std::unique_ptr<Model::BrushNode>(createBrush("asdf"));
            auto nonMatchingBrush = std::unique_ptr<Model::BrushNode>(createBrush("asdf"));

            for (auto* face : matchingBrush->faces()) {
                face->setSurfaceFlags(1);
            }
            for (auto* face : nonMatchingBrush->faces()) {
                face->setSurfaceFlags(2);
            }

            const auto& tag = document->smartTag("surfaceflags");
            for (const auto* face : matchingBrush->faces()) {
                ASSERT_TRUE(tag.matches(*face));
            }
            for (const auto* face : nonMatchingBrush->faces()) {
                ASSERT_FALSE(tag.matches(*face));
            }
        }
        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableSurfaceFlagsTag") {
            auto* nonMatchingBrush = createBrush("asdf");
            document->addNode(nonMatchingBrush, document->currentParent());

            const auto& tag = document->smartTag("surfaceflags");
            ASSERT_TRUE(tag.canEnable());

            auto* face = nonMatchingBrush->faces().front();
            ASSERT_FALSE(tag.matches(*face));

            document->select({ nonMatchingBrush, face });

            TestCallback callback(0);
            tag.enable(callback, *document);

            ASSERT_TRUE(tag.matches(*face));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableSurfaceFlagsTag") {
            auto* matchingBrush = createBrush("asdf");
            for (auto* face : matchingBrush->faces()) {
                face->setSurfaceFlags(1);
            }

            document->addNode(matchingBrush, document->currentParent());

            const auto& tag = document->smartTag("surfaceflags");
            ASSERT_TRUE(tag.canDisable());

            auto* face = matchingBrush->faces().front();
            ASSERT_TRUE(tag.matches(*face));

            document->select({ matchingBrush, face });

            TestCallback callback(0);
            tag.disable(callback, *document);

            ASSERT_FALSE(tag.matches(*face));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.matchEntityClassnameTag") {
            auto* matchingBrush = createBrush("asdf");
            auto* nonMatchingBrush = createBrush("asdf");

            auto matchingEntity = std::make_unique<Model::EntityNode>();
            matchingEntity->addOrUpdateAttribute("classname", "brush_entity");
            matchingEntity->addChild(matchingBrush);

            auto nonMatchingEntity = std::make_unique<Model::EntityNode>();
            nonMatchingEntity->addOrUpdateAttribute("classname", "something");
            nonMatchingEntity->addChild(nonMatchingBrush);

            const auto& tag = document->smartTag("entity");
            ASSERT_TRUE(tag.matches(*matchingBrush));
            ASSERT_FALSE(tag.matches(*nonMatchingBrush));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableEntityClassnameTag") {
            auto* brush = createBrush("asdf");
            document->addNode(brush, document->currentParent());

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(tag.matches(*brush));

            ASSERT_TRUE(tag.canEnable());

            document->select(brush);

            TestCallback callback(0);
            tag.enable(callback, *document);
            ASSERT_TRUE(tag.matches(*brush));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.enableEntityClassnameTagRetainsAttributes") {
            auto* brush = createBrush("asdf");

            auto* oldEntity = new Model::EntityNode();
            oldEntity->addOrUpdateAttribute("classname", "something");
            oldEntity->addOrUpdateAttribute("some_attr", "some_value");

            document->addNode(oldEntity, document->currentParent());
            document->addNode(brush, oldEntity);

            const auto& tag = document->smartTag("entity");
            document->select(brush);

            TestCallback callback(0);
            tag.enable(callback, *document);
            ASSERT_TRUE(tag.matches(*brush));

            auto* newEntity = brush->entity();
            ASSERT_NE(oldEntity, newEntity);

            ASSERT_NE(nullptr, newEntity);
            ASSERT_TRUE(newEntity->hasAttribute("some_attr"));
            ASSERT_EQ("some_value", newEntity->attribute("some_attr", ""));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.disableEntityClassnameTag") {
            auto* brush = createBrush("asdf");

            auto* oldEntity = new Model::EntityNode();
            oldEntity->addOrUpdateAttribute("classname", "brush_entity");

            document->addNode(oldEntity, document->currentParent());
            document->addNode(brush, oldEntity);

            const auto& tag = document->smartTag("entity");
            ASSERT_TRUE(tag.matches(*brush));

            ASSERT_TRUE(tag.canDisable());

            document->select(brush);

            TestCallback callback(0);
            tag.disable(callback, *document);
            ASSERT_FALSE(tag.matches(*brush));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagInitializeBrushTags") {
            auto* entity = new Model::EntityNode();
            entity->addOrUpdateAttribute("classname", "brush_entity");
            document->addNode(entity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, entity);

            const auto& tag = document->smartTag("entity");
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRemoveBrushTags") {
            auto* entity = new Model::EntityNode();
            entity->addOrUpdateAttribute("classname", "brush_entity");
            document->addNode(entity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, entity);

            document->removeNode(brush);

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(brush->hasTag(tag));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagUpdateBrushTags") {
            auto* brush = createBrush("some_texture");
            document->addNode(brush, document->currentParent());

            auto* entity = new Model::EntityNode();
            entity->addOrUpdateAttribute("classname", "brush_entity");
            document->addNode(entity, document->currentParent());

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(brush->hasTag(tag));

            document->reparentNodes(entity, { brush });
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagUpdateBrushTagsAfterReparenting") {
            auto* lightEntity = new Model::EntityNode();
            lightEntity->addOrUpdateAttribute("classname", "brush_entity");
            document->addNode(lightEntity, document->currentParent());

            auto* otherEntity = new Model::EntityNode();
            otherEntity->addOrUpdateAttribute("classname", "other");
            document->addNode(otherEntity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, otherEntity);

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(brush->hasTag(tag));

            document->reparentNodes(lightEntity, { brush });
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagUpdateBrushTagsAfterChangingClassname") {
            auto* lightEntity = new Model::EntityNode();
            lightEntity->addOrUpdateAttribute("classname", "asdf");
            document->addNode(lightEntity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, lightEntity);

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(brush->hasTag(tag));

            document->select(lightEntity);
            document->setAttribute("classname", "brush_entity");
            document->deselectAll();

            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagInitializeBrushFaceTags", "[TagManagementTest]") {
            auto* brushWithTags = createBrush("some_texture");
            document->addNode(brushWithTags, document->currentParent());
            document->select(brushWithTags);

            SECTION("No modification to brush") {
            }
            SECTION("Vertex manipulation") {
                const auto verticesToMove = std::map<vm::vec3, std::vector<Model::BrushNode*>>{ { vm::vec3::fill(16.0), { brushWithTags } } };
                const auto result = document->moveVertices(verticesToMove, vm::vec3::fill(1.0));
                REQUIRE(result.success);
                REQUIRE(result.hasRemainingVertices);
            }

            const auto& tag = document->smartTag("texture");
            for (const auto* face : brushWithTags->faces()) {
                CHECK(face->hasTag(tag));
            }

            auto* brushWithoutTags = createBrush("asdf");
            document->addNode(brushWithoutTags, document->currentParent());

            for (const auto* face : brushWithoutTags->faces()) {
                CHECK(!face->hasTag(tag));
            }
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagRemoveBrushFaceTags") {
            auto* brushWithTags = createBrush("some_texture");
            document->addNode(brushWithTags, document->currentParent());
            document->removeNode(brushWithTags);

            const auto& tag = document->smartTag("texture");
            for (const auto* face : brushWithTags->faces()) {
                ASSERT_FALSE(face->hasTag(tag));
            }
        }

        TEST_CASE_METHOD(TagManagementTest, "TagManagementTest.tagUpdateBrushFaceTags") {
            auto* brush = createBrush("asdf");
            document->addNode(brush, document->currentParent());

            const auto& tag = document->smartTag("contentflags");

            auto* face = brush->faces().front();
            ASSERT_FALSE(face->hasTag(tag));

            Model::ChangeBrushFaceAttributesRequest request;
            request.setContentFlag(0);

            document->select({ brush, face });
            document->setFaceAttributes(request);
            document->deselectAll();

            for (const auto* f : brush->faces()) {
                if (f == face) {
                    ASSERT_TRUE(f->hasTag(tag));
                } else {
                    ASSERT_FALSE(f->hasTag(tag));
                }
            }
        }
    }
}
