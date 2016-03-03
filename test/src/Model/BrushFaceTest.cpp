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
#include "Exceptions.h"
#include "VecMath.h"
#include "TestUtils.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace Model {
        TEST(BrushFaceTest, constructWithValidPoints) {
            const Vec3 p0(0.0,  0.0, 4.0);
            const Vec3 p1(1.f,  0.0, 4.0);
            const Vec3 p2(0.0, -1.0, 4.0);
            
            const BrushFaceAttributes attribs("");
            BrushFace face(p0, p1, p2, attribs, new ParaxialTexCoordSystem(p0, p1, p2, attribs));
            ASSERT_VEC_EQ(p0, face.points()[0]);
            ASSERT_VEC_EQ(p1, face.points()[1]);
            ASSERT_VEC_EQ(p2, face.points()[2]);
            ASSERT_VEC_EQ(Vec3::PosZ, face.boundary().normal);
            ASSERT_EQ(4.0, face.boundary().distance);
        }
        
        TEST(BrushFaceTest, constructWithColinearPoints) {
            const Vec3 p0(0.0, 0.0, 4.0);
            const Vec3 p1(1.f, 0.0, 4.0);
            const Vec3 p2(2.0, 0.0, 4.0);
            
            const BrushFaceAttributes attribs("");
            ASSERT_THROW(new BrushFace(p0, p1, p2, attribs, new ParaxialTexCoordSystem(p0, p1, p2, attribs)), GeometryException);
        }
        
        TEST(BrushFaceTest, textureUsageCount) {
            const Vec3 p0(0.0,  0.0, 4.0);
            const Vec3 p1(1.f,  0.0, 4.0);
            const Vec3 p2(0.0, -1.0, 4.0);
            Assets::Texture texture("testTexture", 64, 64);
            Assets::Texture texture2("testTexture2", 64, 64);
            
            EXPECT_EQ(0, texture.usageCount());
            EXPECT_EQ(0, texture2.usageCount());
            
            // BrushFaceAttributes doesn't increase usage count
            BrushFaceAttributes attribs("");
            attribs.setTexture(&texture);
            EXPECT_EQ(0, texture.usageCount());
            
            {
                // test constructor
                BrushFace face(p0, p1, p2, attribs, new ParaxialTexCoordSystem(p0, p1, p2, attribs));
                EXPECT_EQ(1, texture.usageCount());
                
                // test clone()
                BrushFace *clone = face.clone();
                EXPECT_EQ(2, texture.usageCount());

                // test destructor
                delete clone;
                clone = NULL;
                EXPECT_EQ(1, texture.usageCount());
                
                // test setTexture
                face.setTexture(&texture2);
                EXPECT_EQ(0, texture.usageCount());
                EXPECT_EQ(1, texture2.usageCount());
                
                // test setTexture with the same texture
                face.setTexture(&texture2);
                EXPECT_EQ(1, texture2.usageCount());
                
                // test setFaceAttributes
                EXPECT_EQ(&texture, attribs.texture());
                face.setAttribs(attribs);
                EXPECT_EQ(1, texture.usageCount());
                EXPECT_EQ(0, texture2.usageCount());
                
                // test setFaceAttributes with the same attributes
                face.setAttribs(attribs);
                EXPECT_EQ(1, texture.usageCount());
                EXPECT_EQ(0, texture2.usageCount());
            }
            
            EXPECT_EQ(0, texture.usageCount());
            EXPECT_EQ(0, texture2.usageCount());
        }
    }
}
