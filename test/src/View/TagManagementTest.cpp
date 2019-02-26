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
            void SetUp() override {
                MapDocumentTest::SetUp();

                game->setSmartTags({
                    Model::SmartTag("texture", {}, std::make_unique<Model::TextureNameTagMatcher>("some_texture")),
                    Model::SmartTag("surfaceparm", {}, std::make_unique<Model::SurfaceParmTagMatcher>("some_parm")),
                    Model::SmartTag("contentflags", {}, std::make_unique<Model::ContentFlagsTagMatcher>(1)),
                    Model::SmartTag("surfaceflags", {}, std::make_unique<Model::SurfaceFlagsTagMatcher>(1)),
                    Model::SmartTag("entity", {}, std::make_unique<Model::EntityClassNameTagMatcher>("light"))
                });
                document->registerSmartTags();
            }
        };

        TEST_F(TagManagementTest, testTagRegistration) {
            ASSERT_TRUE(document->isRegisteredSmartTag("texture"));
            ASSERT_TRUE(document->isRegisteredSmartTag("surfaceparm"));
            ASSERT_TRUE(document->isRegisteredSmartTag("contentflags"));
            ASSERT_TRUE(document->isRegisteredSmartTag("surfaceflags"));
            ASSERT_TRUE(document->isRegisteredSmartTag("entity"));
            ASSERT_FALSE(document->isRegisteredSmartTag(""));
            ASSERT_FALSE(document->isRegisteredSmartTag("asdf"));
        }

        TEST_F(TagManagementTest, testTagInitializeBrushTags) {
            auto* entity = new Model::Entity();
            entity->addOrUpdateAttribute("classname", "light");
            document->addNode(entity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, entity);

            const auto& tag = document->smartTag("entity");
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_F(TagManagementTest, testTagRemoveBrushTags) {
            auto* entity = new Model::Entity();
            entity->addOrUpdateAttribute("classname", "light");
            document->addNode(entity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, entity);

            document->removeNode(brush);

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(brush->hasTag(tag));
        }

        TEST_F(TagManagementTest, testTagUpdateBrushTags) {
            auto* brush = createBrush("some_texture");
            document->addNode(brush, document->currentParent());

            auto* entity = new Model::Entity();
            entity->addOrUpdateAttribute("classname", "light");
            document->addNode(entity, document->currentParent());

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(brush->hasTag(tag));

            document->reparentNodes(entity, Model::NodeList{brush});
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_F(TagManagementTest, testTagUpdateBrushTagsAfterReparenting) {
            auto* lightEntity = new Model::Entity();
            lightEntity->addOrUpdateAttribute("classname", "light");
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

        TEST_F(TagManagementTest, testTagUpdateBrushTagsAfterChangingClassname) {
            auto* lightEntity = new Model::Entity();
            lightEntity->addOrUpdateAttribute("classname", "asdf");
            document->addNode(lightEntity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, lightEntity);

            const auto& tag = document->smartTag("entity");
            ASSERT_FALSE(brush->hasTag(tag));

            document->select(lightEntity);
            document->setAttribute("classname", "light");
            document->deselectAll();

            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_F(TagManagementTest, testTagInitializeBrushFaceTags) {
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

        TEST_F(TagManagementTest, testTagRemoveBrushFaceTags) {
            auto* brushWithTags = createBrush("some_texture");
            document->addNode(brushWithTags, document->currentParent());
            document->removeNode(brushWithTags);

            const auto& tag = document->smartTag("texture");
            for (const auto* face : brushWithTags->faces()) {
                ASSERT_FALSE(face->hasTag(tag));
            }
        }

        TEST_F(TagManagementTest, testTagUpdateBrushFaceTags) {
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
