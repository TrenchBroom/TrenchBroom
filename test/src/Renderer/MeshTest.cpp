/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "Renderer/Mesh.h"
#include "Renderer/VertexArray.h"

#include "TestUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST(MeshTest, CreateTriangleSet) {
            typedef String Key;
            Mesh<Key, VP3> mesh;
            
            mesh.beginTriangleSet("Texture1");
            mesh.addTriangleToSet(VP3(Vec3f(1.0f, 1.0f, 1.0f)), VP3(Vec3f(2.0f, 2.0f, 2.0f)), VP3(Vec3f(3.0f, 3.0f, 3.0f)));
            mesh.addTriangleToSet(VP3(Vec3f(4.0f, 4.0f, 4.0f)), VP3(Vec3f(5.0f, 5.0f, 5.0f)), VP3(Vec3f(6.0f, 6.0f, 6.0f)));
            mesh.addTriangleToSet(VP3(Vec3f(1.0f, 2.0f, 3.0f)), VP3(Vec3f(5.0f, 5.0f, 5.0f)), VP3(Vec3f(6.0f, 6.0f, 6.0f)));
            mesh.endTriangleSet();

            mesh.beginTriangleSet("Texture2");
            mesh.addTriangleToSet(VP3(Vec3f(2.0f, 1.0f, 1.0f)), VP3(Vec3f(3.0f, 2.0f, 2.0f)), VP3(Vec3f(4.0f, 3.0f, 3.0f)));
            mesh.addTriangleToSet(VP3(Vec3f(5.0f, 4.0f, 4.0f)), VP3(Vec3f(6.0f, 5.0f, 5.0f)), VP3(Vec3f(7.0f, 6.0f, 6.0f)));
            mesh.endTriangleSet();

            const Mesh<Key, VP3>::TriangleSetMap& sets = mesh.triangleSets();
            ASSERT_EQ(2u, sets.size());
            
            Mesh<Key, VP3>::TriangleSetMap::const_iterator it = sets.begin();
            ASSERT_EQ(String("Texture1"), it->first);
            
            const VP3::List& vertices1 = it->second;
            ASSERT_EQ(9u, vertices1.size());
            ASSERT_VEC_EQ(Vec3f(1.0f, 1.0f, 1.0f), vertices1[0].value);
            ASSERT_VEC_EQ(Vec3f(2.0f, 2.0f, 2.0f), vertices1[1].value);
            ASSERT_VEC_EQ(Vec3f(3.0f, 3.0f, 3.0f), vertices1[2].value);
            ASSERT_VEC_EQ(Vec3f(4.0f, 4.0f, 4.0f), vertices1[3].value);
            ASSERT_VEC_EQ(Vec3f(5.0f, 5.0f, 5.0f), vertices1[4].value);
            ASSERT_VEC_EQ(Vec3f(6.0f, 6.0f, 6.0f), vertices1[5].value);
            ASSERT_VEC_EQ(Vec3f(1.0f, 2.0f, 3.0f), vertices1[6].value);
            ASSERT_VEC_EQ(Vec3f(5.0f, 5.0f, 5.0f), vertices1[7].value);
            ASSERT_VEC_EQ(Vec3f(6.0f, 6.0f, 6.0f), vertices1[8].value);
            
            it = sets.find("Texture2");
            ASSERT_EQ(String("Texture2"), it->first);
            
            const VP3::List& vertices2 = it->second;
            ASSERT_EQ(6u, vertices2.size());
            ASSERT_VEC_EQ(Vec3f(2.0f, 1.0f, 1.0f), vertices2[0].value);
            ASSERT_VEC_EQ(Vec3f(3.0f, 2.0f, 2.0f), vertices2[1].value);
            ASSERT_VEC_EQ(Vec3f(4.0f, 3.0f, 3.0f), vertices2[2].value);
            ASSERT_VEC_EQ(Vec3f(5.0f, 4.0f, 4.0f), vertices2[3].value);
            ASSERT_VEC_EQ(Vec3f(6.0f, 5.0f, 5.0f), vertices2[4].value);
            ASSERT_VEC_EQ(Vec3f(7.0f, 6.0f, 6.0f), vertices2[5].value);
        }
    }
}
