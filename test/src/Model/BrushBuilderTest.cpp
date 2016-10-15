/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/MapFormat.h"
#include "Model/World.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        TEST(BrushBuilderTest, createCube) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);

            BrushBuilder builder(&world, worldBounds);
            const Brush* cube = builder.createCube(128.0, "someName");
            ASSERT_TRUE(cube != NULL);
            ASSERT_EQ(BBox3d(-64.0, +64.0), cube->bounds());
            
            const BrushFaceList& faces = cube->faces();
            ASSERT_EQ(6u, faces.size());
            
            for (size_t i = 0; i < faces.size(); ++i)
                ASSERT_EQ(String("someName"), faces[i]->textureName());
            
            delete cube;
        }
    }
}
