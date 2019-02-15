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
#include "Model/Entity.h"
#include "Model/Layer.h"
#include "Model/Tag.h"
#include "Model/TagMatcher.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include <chrono>
#include <thread>

namespace TrenchBroom {
    namespace View {
        TEST_F(MapDocumentTest, testInitializeBrushTags) {
            game->setSmartTags({
                Model::SmartTag("test", {}, std::make_unique<Model::EntityClassNameTagMatcher>("light"))
            });
            document->registerSmartTags();

            auto* entity = new Model::Entity();
            entity->addOrUpdateAttribute("classname", "light");
            document->addNode(entity, document->currentParent());

            auto* brush = createBrush("some_texture");
            document->addNode(brush, entity);

            brush->tags();
        }
    }
}