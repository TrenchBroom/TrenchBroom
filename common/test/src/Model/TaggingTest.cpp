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

#include "Exceptions.h"
#include "Model/Tag.h"
#include "Model/TagManager.h"
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("TaggingTest.testTagBrush", "[TaggingTest]") {
            const vm::bbox3 worldBounds{4096.0};
            WorldNode world{Model::Entity{}, MapFormat::Standard};

            BrushBuilder builder{&world, worldBounds};
            BrushNode* brush = world.createBrush(builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom").value());

            world.defaultLayer()->addChild(brush);

            Tag tag1{"tag1", {}};
            Tag tag2{"tag2", {}};

            tag1.setIndex(0);
            tag2.setIndex(1);

            ASSERT_FALSE(brush->hasTag(tag1));
            ASSERT_FALSE(brush->hasTag(tag2));

            ASSERT_TRUE(brush->addTag(tag1));
            ASSERT_FALSE(brush->addTag(tag1));

            ASSERT_TRUE(brush->hasTag(tag1));
            ASSERT_FALSE(brush->hasTag(tag2));

            ASSERT_TRUE(brush->removeTag(tag1));
            ASSERT_FALSE(brush->removeTag(tag1));

            ASSERT_FALSE(brush->hasTag(tag1));
            ASSERT_FALSE(brush->hasTag(tag2));
        }
    }
}
