/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "Renderer/Mesh.h"
#include "Renderer/VertexSpec.h"

#include "TestUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST(MeshTest, createTriangleSet) {
            typedef String Key;
            typedef VertexSpecs::P3::Vertex Vertex;
            Mesh<Key, VertexSpecs::P3> mesh;
            
            mesh.beginTriangleSet("Texture1");
            mesh.addTriangleToSet(Vertex(Vec3f(1.0f, 1.0f, 1.0f)), Vertex(Vec3f(2.0f, 2.0f, 2.0f)), Vertex(Vec3f(3.0f, 3.0f, 3.0f)));
            mesh.addTriangleToSet(Vertex(Vec3f(4.0f, 4.0f, 4.0f)), Vertex(Vec3f(5.0f, 5.0f, 5.0f)), Vertex(Vec3f(6.0f, 6.0f, 6.0f)));
            mesh.addTriangleToSet(Vertex(Vec3f(1.0f, 2.0f, 3.0f)), Vertex(Vec3f(5.0f, 5.0f, 5.0f)), Vertex(Vec3f(6.0f, 6.0f, 6.0f)));
            mesh.endTriangleSet();

            mesh.beginTriangleSet("Texture2");
            mesh.addTriangleToSet(Vertex(Vec3f(2.0f, 1.0f, 1.0f)), Vertex(Vec3f(3.0f, 2.0f, 2.0f)), Vertex(Vec3f(4.0f, 3.0f, 3.0f)));
            mesh.addTriangleToSet(Vertex(Vec3f(5.0f, 4.0f, 4.0f)), Vertex(Vec3f(6.0f, 5.0f, 5.0f)), Vertex(Vec3f(7.0f, 6.0f, 6.0f)));
            mesh.endTriangleSet();

            const Mesh<Key, VertexSpecs::P3>::TriangleSetMap& sets = mesh.triangleSets();
            ASSERT_EQ(2u, sets.size());
            
            Mesh<Key, VertexSpecs::P3>::TriangleSetMap::const_iterator it = sets.begin();
            ASSERT_EQ(String("Texture1"), it->first);
            
            const Vertex::List& vertices1 = it->second;
            ASSERT_EQ(9u, vertices1.size());
            ASSERT_VEC_EQ(Vec3f(1.0f, 1.0f, 1.0f), vertices1[0].v1);
            ASSERT_VEC_EQ(Vec3f(2.0f, 2.0f, 2.0f), vertices1[1].v1);
            ASSERT_VEC_EQ(Vec3f(3.0f, 3.0f, 3.0f), vertices1[2].v1);
            ASSERT_VEC_EQ(Vec3f(4.0f, 4.0f, 4.0f), vertices1[3].v1);
            ASSERT_VEC_EQ(Vec3f(5.0f, 5.0f, 5.0f), vertices1[4].v1);
            ASSERT_VEC_EQ(Vec3f(6.0f, 6.0f, 6.0f), vertices1[5].v1);
            ASSERT_VEC_EQ(Vec3f(1.0f, 2.0f, 3.0f), vertices1[6].v1);
            ASSERT_VEC_EQ(Vec3f(5.0f, 5.0f, 5.0f), vertices1[7].v1);
            ASSERT_VEC_EQ(Vec3f(6.0f, 6.0f, 6.0f), vertices1[8].v1);
            
            it = sets.find("Texture2");
            ASSERT_EQ(String("Texture2"), it->first);
            
            const Vertex::List& vertices2 = it->second;
            ASSERT_EQ(6u, vertices2.size());
            ASSERT_VEC_EQ(Vec3f(2.0f, 1.0f, 1.0f), vertices2[0].v1);
            ASSERT_VEC_EQ(Vec3f(3.0f, 2.0f, 2.0f), vertices2[1].v1);
            ASSERT_VEC_EQ(Vec3f(4.0f, 3.0f, 3.0f), vertices2[2].v1);
            ASSERT_VEC_EQ(Vec3f(5.0f, 4.0f, 4.0f), vertices2[3].v1);
            ASSERT_VEC_EQ(Vec3f(6.0f, 5.0f, 5.0f), vertices2[4].v1);
            ASSERT_VEC_EQ(Vec3f(7.0f, 6.0f, 6.0f), vertices2[5].v1);
        }
    }
}
