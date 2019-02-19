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
        TEST_F(MapDocumentTest, testTagRegistration) {
            game->setSmartTags({
                Model::SmartTag("test", {}, std::make_unique<Model::EntityClassNameTagMatcher>("light"))
            });
            document->registerSmartTags();

            ASSERT_TRUE(document->isRegisteredSmartTag("test"));
        }

        TEST_F(MapDocumentTest, testTagInitializeBrushTags) {
            game->setSmartTags({
                Model::SmartTag("test", {}, std::make_unique<Model::EntityClassNameTagMatcher>("light"))
            });
            document->registerSmartTags();

            auto* entity = new Model::Entity();
            entity->addOrUpdateAttribute("classname", "light");
            document->addNode(entity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, entity);

            const auto& tag = document->smartTag("test");
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_F(MapDocumentTest, testTagRemoveBrushTags) {
            game->setSmartTags({
                Model::SmartTag("test", {}, std::make_unique<Model::EntityClassNameTagMatcher>("light"))
            });
            document->registerSmartTags();

            auto* entity = new Model::Entity();
            entity->addOrUpdateAttribute("classname", "light");
            document->addNode(entity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, entity);

            document->removeNode(brush);

            const auto& tag = document->smartTag("test");
            ASSERT_FALSE(brush->hasTag(tag));
        }

        TEST_F(MapDocumentTest, testTagUpdateBrushTags) {
            game->setSmartTags({
                Model::SmartTag("test", {}, std::make_unique<Model::EntityClassNameTagMatcher>("light"))
            });
            document->registerSmartTags();

            auto* brush = createBrush("some_texture");
            document->addNode(brush, document->currentParent());

            auto* entity = new Model::Entity();
            entity->addOrUpdateAttribute("classname", "light");
            document->addNode(entity, document->currentParent());

            const auto& tag = document->smartTag("test");
            ASSERT_FALSE(brush->hasTag(tag));

            document->reparentNodes(entity, Model::NodeList{brush});
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_F(MapDocumentTest, testTagUpdateBrushTagsAfterReparenting) {
            game->setSmartTags({
                Model::SmartTag("test", {}, std::make_unique<Model::EntityClassNameTagMatcher>("light"))
            });
            document->registerSmartTags();

            auto* lightEntity = new Model::Entity();
            lightEntity->addOrUpdateAttribute("classname", "light");
            document->addNode(lightEntity, document->currentParent());

            auto* otherEntity = new Model::Entity();
            otherEntity->addOrUpdateAttribute("classname", "other");
            document->addNode(otherEntity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, otherEntity);

            const auto& tag = document->smartTag("test");
            ASSERT_FALSE(brush->hasTag(tag));

            document->reparentNodes(lightEntity, Model::NodeList{brush});
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_F(MapDocumentTest, testTagUpdateBrushTagsAfterChangingClassname) {
            game->setSmartTags({
                Model::SmartTag("test", {}, std::make_unique<Model::EntityClassNameTagMatcher>("light"))
            });
            document->registerSmartTags();

            auto* lightEntity = new Model::Entity();
            lightEntity->addOrUpdateAttribute("classname", "asdf");
            document->addNode(lightEntity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, lightEntity);

            const auto& tag = document->smartTag("test");
            ASSERT_FALSE(brush->hasTag(tag));

            document->select(lightEntity);
            document->setAttribute("classname", "light");
            document->deselectAll();
            
            ASSERT_TRUE(brush->hasTag(tag));
        }

        TEST_F(MapDocumentTest, testTagInitializeBrushFaceTags) {
            game->setSmartTags({
                Model::SmartTag("test", {}, std::make_unique<Model::TextureNameTagMatcher>("some_texture"))
            });
            document->registerSmartTags();

            auto* brushWithTags = createBrush("some_texture");
            document->addNode(brushWithTags, document->currentParent());

            const auto& tag = document->smartTag("test");
            for (const auto* face : brushWithTags->faces()) {
                ASSERT_TRUE(face->hasTag(tag));
            }

            auto* brushWithoutTags = createBrush("asdf");
            document->addNode(brushWithoutTags, document->currentParent());

            for (const auto* face : brushWithoutTags->faces()) {
                ASSERT_FALSE(face->hasTag(tag));
            }
        }

        TEST_F(MapDocumentTest, testTagRemoveBrushFaceTags) {
            game->setSmartTags({
                Model::SmartTag("test", {}, std::make_unique<Model::TextureNameTagMatcher>("some_texture"))
            });
            document->registerSmartTags();

            auto* brushWithTags = createBrush("some_texture");
            document->addNode(brushWithTags, document->currentParent());
            document->removeNode(brushWithTags);

            const auto& tag = document->smartTag("test");
            for (const auto* face : brushWithTags->faces()) {
                ASSERT_FALSE(face->hasTag(tag));
            }
        }

        TEST_F(MapDocumentTest, testTagUpdateBrushFaceTags) {
            game->setSmartTags({
                Model::SmartTag("test", {}, std::make_unique<Model::ContentFlagsTagMatcher>(1))
            });
            document->registerSmartTags();

            auto* brush = createBrush("asdf");
            document->addNode(brush, document->currentParent());

            const auto& tag = document->smartTag("test");

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
