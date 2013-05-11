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

#include "Model/Brush.h"
#include "Model/Face.h"

namespace TrenchBroom {
    namespace Model {
        TEST(BrushTest, ConstructFromFaces) {
            FaceList faces;
            faces.push_back(new Face(Vec3(0.0, 0.0, 0.0),
                                     Vec3(1.0, 0.0, 0.0),
                                     Vec3(0.0, 1.0, 0.0)));
            faces.push_back(new Face(Vec3(0.0, 0.0, 0.0),
                                     Vec3(1.0, 0.0, 0.0),
                                     Vec3(0.0, 1.0, 0.0)));
            faces.push_back(new Face(Vec3(0.0, 0.0, 0.0),
                                     Vec3(1.0, 0.0, 0.0),
                                     Vec3(0.0, 1.0, 0.0)));
            Brush brush(faces);
            const FaceList& brushFaces = brush.faces();
            ASSERT_EQ(faces.size(), brushFaces.size());
            for (size_t i = 0; i < faces.size(); i++)
                ASSERT_EQ(faces[i], brushFaces[i]);
        }
    }
}
