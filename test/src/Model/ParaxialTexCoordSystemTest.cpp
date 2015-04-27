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

#include "TestUtils.h"

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/ParaxialTexCoordSystem.h"

namespace TrenchBroom {
    namespace Model {
        TEST(ParaxialTexCoordSystemTest, transform) {
            Assets::Texture texture("texture", 64, 64);
            const FloatType oldDistance(-583.10490580282567);
            const Vec3 oldNormal(0.62449286425754114, -0.63673782238023802, -0.45229814065711621);
            const Plane3 oldBoundary(oldDistance, oldNormal);
            ParaxialTexCoordSystem coordSystem(oldNormal);
            
            BrushFaceAttributes attribs("texture");
            attribs.setTexture(&texture);
            const Vec3 center(32.0, -48.0, 0.0);
            const Mat4x4 transform = translationMatrix(center) * rotationMatrix(Vec3::PosZ, Math::radians(15.0)) * translationMatrix(-center);
            const Vec3 invariant(-184.65096673000929, 632.60193647633696, 143.68866328257172);
            const Vec2f oldTexCoords = coordSystem.getTexCoords(invariant, attribs);
            
            coordSystem.transform(oldBoundary, transform, attribs, true, invariant);
            const Vec2f newTexCoords = coordSystem.getTexCoords(transform * invariant, attribs);
            
            ASSERT_TC_EQ(oldTexCoords, newTexCoords);
            ASSERT_VEC_EQ(Vec2f(1.20616937, 1.0), attribs.scale());
        }
        
        TEST(ParaxialTexCoordSystemTest, transformWithNewAxesForTopFace) {
            Assets::Texture texture("texture", 128, 128);
            const FloatType oldDistance(151.42562584220411);
            const Vec3 oldNormal(0.000000000000000021930331350620384,
                                 0.50000000000000144,
                                 0.86602540378443771);
            const Plane3 oldBoundary(oldDistance, oldNormal);
            ParaxialTexCoordSystem coordSystem(oldNormal);
            
            BrushFaceAttributes attribs("texture");
            attribs.setTexture(&texture);
            attribs.setOffset(Vec2f(-49.2198257f, 64.0f));
            attribs.setScale(Vec2f(0.968245804f, 0.89442718f));
            attribs.setRotation(26.565052f);

            const Vec3 center(-64.0, 64.0, 64.0);
            const Vec3 axis = Vec3::PosX;
            const FloatType angle = -0.26179938779914941;
            const Vec3 oldInvariant(-63.999999999999289, 95.999999999999091, 119.42562584220195);
            
            const Mat4x4 transform = translationMatrix(center) * rotationMatrix(axis, angle) * translationMatrix(-center);
            
            coordSystem.transform(oldBoundary, transform, attribs, true, oldInvariant);
            
        }
    }
}
