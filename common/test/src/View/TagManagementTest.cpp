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

#include <gtest/gtest.h>

#include "Logger.h"
#include "IO/Path.h"
#include "IO/TestEnvironment.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Entity.h"
#include "Model/Layer.h"
#include "Model/Tag.h"
#include "Model/TagMatcher.h"
#include "Model/TestGame.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

namespace TrenchBroom {
    namespace View {
        class TagManagementTest : public MapDocumentTest {
        protected:
            Assets::Texture* m_matchingTexture;
            Assets::Texture* m_nonMatchingTexture;
            Assets::TextureCollection* m_textureCollection;
        protected:
            void SetUp() override {
                MapDocumentTest::SetUp();

                auto matchingTexture = std::make_unique<Assets::Texture>("some_texture", 16, 16);
                auto nonMatchingTexture = std::make_unique<Assets::Texture>("other_texture", 32, 32);

                matchingTexture->setSurfaceParms({"some_parm"});

                auto textureCollection = std::make_unique<Assets::TextureCollection>(Assets::TextureList{
                    matchingTexture.get(),
                    nonMatchingTexture.get()
                });

                document->textureManager().setTextureCollections(Assets::TextureCollectionList{
                    textureCollection.get()
                });

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
        };

        class TestCallback : public Model::TagMatcherCallback {
        private:
            size_t m_option;
        public:
            explicit TestCallback(const size_t option) :
            m_option(option) {}

            size_t selectOption(const StringList& /* options */) {
                return m_option;
            }
        };

        TEST_F(TagManagementTest, tagRegistration) {
            ASSERT_TRUE(document->isRegisteredSmartTag("texture"));
            ASSERT_TRUE(document->isRegisteredSmartTag("surfaceparm"));
            ASSERT_TRUE(document->isRegisteredSmartTag("contentflags"));
            ASSERT_TRUE(document->isRegisteredSmartTag("surfaceflags"));
            ASSERT_TRUE(document->isRegisteredSmartTag("entity"));
            ASSERT_FALSE(document->isRegisteredSmartTag(""));
            ASSERT_FALSE(document->isRegisteredSmartTag("asdf"));
        }

        TEST_F(TagManagementTest, matchTextureNameTag) {
            auto matchingBrush = std::unique_ptr<Model::Brush>(createBrush("some_texture"));
            auto nonMatchingBrush = std::unique_ptr<Model::Brush>(createBrush("asdf"));

            const auto& tag = document->smartTag("texture");
            for (const auto* face : matchingBrush->faces()) {
                ASSERT_TRUE(tag.matches(*face));
            }
            for (const auto* face : nonMatchingBrush->faces()) {
                ASSERT_FALSE(tag.matches(*face));
            }
        }

        TEST_F(TagManagementTest, enableTextureNameTag) {
            auto* nonMatchingBrush = createBrush("asdf");
            document->addNode(nonMatchingBrush, document->currentParent());

            const auto& tag = document->smartTag("texture");
            ASSERT_TRUE(tag.canEnable());

            auto* face = nonMatchingBrush->faces().front();
            ASSERT_FALSE(tag.matches(*face));

            document->select(face);

            TestCallback callback(0);
            tag.enable(callback, *document);

            ASSERT_TRUE(tag.matches(*face));
        }

        TEST_F(TagManagementTest, disableTextureNameTag) {
            const auto& tag = document->smartTag("texture");
            ASSERT_FALSE(tag.canDisable());
        }

        TEST_F(TagManagementTest, matchSurfaceParmTag) {
            auto texture = std::make_unique<Assets::Texture>("texturename", 16, 16);
            texture->setSurfaceParms({"some_parm"});

            auto matchingBrush = std::unique_ptr<Model::Brush>(createBrush("some_texture"));
            auto nonMatchingBrush = std::unique_ptr<Model::Brush>(createBrush("asdf"));

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

        TEST_F(TagManagementTest, enableSurfaceParmTag) {
            const auto& tag = document->smartTag("surfaceparm");
            ASSERT_FALSE(tag.canEnable());
        }

        TEST_F(TagManagementTest, disableSurfaceParmTag) {
            const auto& tag = document->smartTag("surfaceparm");
            ASSERT_FALSE(tag.canDisable());
        }

        TEST_F(TagManagementTest, matchContentFlagsTag) {
            auto matchingBrush = std::unique_ptr<Model::Brush>(createBrush("asdf"));
            auto nonMatchingBrush = std::unique_ptr<Model::Brush>(createBrush("asdf"));

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

        TEST_F(TagManagementTest, enableContentFlagsTag) {
            auto* nonMatchingBrush = createBrush("asdf");
            document->addNode(nonMatchingBrush, document->currentParent());

            const auto& tag = document->smartTag("contentflags");
            ASSERT_TRUE(tag.canEnable());

            auto* face = nonMatchingBrush->faces().front();
            ASSERT_FALSE(tag.matches(*face));

            document->select(face);

            TestCallback callback(0);
            tag.enable(callback, *document);

            ASSERT_TRUE(tag.matches(*face));
        }

        TEST_F(TagManagementTest, disableContentFlagsTag) {
            auto* matchingBrush = createBrush("asdf");
            for (auto* face : matchingBrush->faces()) {
                face->setSurfaceContents(1);
            }

            document->addNode(matchingBrush, document->currentParent());

            const auto& tag = document->smartTag("contentflags");
            ASSERT_TRUE(tag.canDisable());

            auto* face = matchingBrush->faces().front();
            ASSERT_TRUE(tag.matches(*face));

            document->select(face);

            TestCallback callback(0);
            tag.disable(callback, *document);

            ASSERT_FALSE(tag.matches(*face));
        }

        TEST_F(TagManagementTest, matchSurfaceFlagsTag) {
            auto matchingBrush = std::unique_ptr<Model::Brush>(createBrush("asdf"));
            auto nonMatchingBrush = std::unique_ptr<Model::Brush>(createBrush("asdf"));

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
        TEST_F(TagManagementTest, enableSurfaceFlagsTag) {
            auto* nonMatchingBrush = createBrush("asdf");
            document->addNode(nonMatchingBrush, document->currentParent());

            const auto& tag = document->smartTag("surfaceflags");
            ASSERT_TRUE(tag.canEnable());

            auto* face = nonMatchingBrush->faces().front();
            ASSERT_FALSE(tag.matches(*face));

            document->select(face);

            TestCallback callback(0);
            tag.enable(callback, *document);

            ASSERT_TRUE(tag.matches(*face));
        }

        TEST_F(TagManagementTest, disableSurfaceFlagsTag) {
            auto* matchingBrush = createBrush("asdf");
            for (auto* face : matchingBrush->faces()) {
                face->setSurfaceFlags(1);
            }

            document->addNode(matchingBrush, document->currentParent());

            const auto& tag = document->smartTag("surfaceflags");
            ASSERT_TRUE(tag.canDisable());

            auto* face = matchingBrush->faces().front();
            ASSERT_TRUE(tag.matches(*face));

            document->select(face);

            TestCallback callback(0);
            tag.disable(callback, *document);

            ASSERT_FALSE(tag.matches(*face));
        }

        TEST_F(TagManagementTest, matchEntityClassnameTag) {
            auto* matchingBrush = createBrush("asdf");
            auto* nonMatchingBrush = createBrush("asdf");

            auto matchingEntity = std::make_unique<Model::Entity>();
            matchingEntity->addOrUpdateAttribute("classname", "brush_entity");
            matchingEntity->addChild(matchingBrush);

            auto nonMatchingEntity = std::make_unique<Model::Entity>();
            nonMatchingEntity->addOrUpdateAttribute("classname", "something");
            nonMatchingEntity->addChild(nonMatchingBrush);

            const auto& tag = document->smartTag("entity");
            ASSERT_TRUE(tag.matches(*matchingBrush));
            ASSERT_FALSE(tag.matches(*nonMatchingBrush));
        }

        TEST_F(TagManagementTest, enableEntityClassnameTag) {
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

        TEST_F(TagManagementTest, enableEntityClassnameTagRetainsAttributes) {
            auto* brush = createBrush("asdf");

            auto* oldEntity = new Model::Entity();
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

        TEST_F(TagManagementTest, disableEntityClassnameTag) {
            auto* brush = createBrush("asdf");

            auto* oldEntity = new Model::Entity();
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

        TEST_F(TagManagementTest, tagInitializeBrushTags) {
            auto* entity = new Model::Entity();
            entity->addOrUpdateAttribute("classname", "brush_entity");
            document->addNode(entity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, entity);

            const auto& tag = document->smartTag("entity");
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_F(TagManagementTest, tagRemoveBrushTags) {
            auto* entity = new Model::Entity();
            entity->addOrUpdateAttribute("classname", "brush_entity");
            document->addNode(entity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, entity);

            document->removeNode(brush);

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(brush->hasTag(tag));
        }

        TEST_F(TagManagementTest, tagUpdateBrushTags) {
            auto* brush = createBrush("some_texture");
            document->addNode(brush, document->currentParent());

            auto* entity = new Model::Entity();
            entity->addOrUpdateAttribute("classname", "brush_entity");
            document->addNode(entity, document->currentParent());

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(brush->hasTag(tag));

            document->reparentNodes(entity, Model::NodeList{brush});
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_F(TagManagementTest, tagUpdateBrushTagsAfterReparenting) {
            auto* lightEntity = new Model::Entity();
            lightEntity->addOrUpdateAttribute("classname", "brush_entity");
            document->addNode(lightEntity, document->currentParent());

            auto* otherEntity = new Model::Entity();
            otherEntity->addOrUpdateAttribute("classname", "other");
            document->addNode(otherEntity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, otherEntity);

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(brush->hasTag(tag));

            document->reparentNodes(lightEntity, Model::NodeList{brush});
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_F(TagManagementTest, tagUpdateBrushTagsAfterChangingClassname) {
            auto* lightEntity = new Model::Entity();
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

        TEST_F(TagManagementTest, tagInitializeBrushFaceTags) {
            auto* brushWithTags = createBrush("some_texture");
            document->addNode(brushWithTags, document->currentParent());

            const auto& tag = document->smartTag("texture");
            for (const auto* face : brushWithTags->faces()) {
                ASSERT_TRUE(face->hasTag(tag));
            }

            auto* brushWithoutTags = createBrush("asdf");
            document->addNode(brushWithoutTags, document->currentParent());

            for (const auto* face : brushWithoutTags->faces()) {
                ASSERT_FALSE(face->hasTag(tag));
            }
        }

        TEST_F(TagManagementTest, tagRemoveBrushFaceTags) {
            auto* brushWithTags = createBrush("some_texture");
            document->addNode(brushWithTags, document->currentParent());
            document->removeNode(brushWithTags);

            const auto& tag = document->smartTag("texture");
            for (const auto* face : brushWithTags->faces()) {
                ASSERT_FALSE(face->hasTag(tag));
            }
        }

        TEST_F(TagManagementTest, tagUpdateBrushFaceTags) {
            auto* brush = createBrush("asdf");
            document->addNode(brush, document->currentParent());

            const auto& tag = document->smartTag("contentflags");

            auto* face = brush->faces().front();
            ASSERT_FALSE(face->hasTag(tag));

            Model::ChangeBrushFaceAttributesRequest request;
            request.setContentFlag(0);

            document->select(face);
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
