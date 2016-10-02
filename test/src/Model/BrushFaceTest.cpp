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

#include "TrenchBroom.h"
#include "Exceptions.h"
#include "VecMath.h"
#include "TestUtils.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/MapFormat.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/World.h"
#include "Model/ModelTypes.h"

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
            EXPECT_EQ(1, texture.usageCount());
            
            {
                // test constructor
                BrushFace face(p0, p1, p2, attribs, new ParaxialTexCoordSystem(p0, p1, p2, attribs));
                EXPECT_EQ(2, texture.usageCount());
                
                // test clone()
                BrushFace *clone = face.clone();
                EXPECT_EQ(3, texture.usageCount());

                // test destructor
                delete clone;
                clone = NULL;
                EXPECT_EQ(2, texture.usageCount());
                
                // test setTexture
                face.setTexture(&texture2);
                EXPECT_EQ(1, texture.usageCount());
                EXPECT_EQ(1, texture2.usageCount());
                
                // test setTexture with the same texture
                face.setTexture(&texture2);
                EXPECT_EQ(1, texture2.usageCount());
                
                // test setFaceAttributes
                EXPECT_EQ(&texture, attribs.texture());
                face.setAttribs(attribs);
                EXPECT_EQ(2, texture.usageCount());
                EXPECT_EQ(0, texture2.usageCount());
                
                // test setFaceAttributes with the same attributes
                face.setAttribs(attribs);
                EXPECT_EQ(2, texture.usageCount());
                EXPECT_EQ(0, texture2.usageCount());
            }
            
            EXPECT_EQ(1, texture.usageCount());
            EXPECT_EQ(0, texture2.usageCount());
        }
        
        static void getFaceVertsAndTexCoords(const BrushFace *face,
                                             std::vector<Vec3> *vertPositions,
                                             std::vector<Vec2> *vertTexCoords) {
            BrushFace::VertexList::const_iterator it;
            BrushFace::VertexList verts = face->vertices();
            for (it = verts.begin(); it != verts.end(); ++it) {
                vertPositions->push_back(it->position());
                vertTexCoords->push_back(face->textureCoords(it->position()));
            }
        }
        
        /**
         * Applies the given transform to a copy of origFace, and also
         * the three reference verts.
         *
         * Checks that the UV coordinates of the 3 transformed points
         * are equivelant to the UV coordinates of the non-transformed points,
         * i.e. checks that texture lock worked.
         */
        static void checkTextureLockWithTransform(const Mat4x4 &transform,
                                                  const BrushFace *origFace) {
            std::vector<Vec3> verts;
            std::vector<Vec2> uvs;
            getFaceVertsAndTexCoords(origFace, &verts, &uvs);
            ASSERT_GE(verts.size(), 3);

            // transform the face
            BrushFace *face = origFace->clone();
            face->transform(transform, true);
            
            // transform the verts
            std::vector<Vec3> transformedVerts;
            for (size_t i=0; i<verts.size(); i++) {
                transformedVerts.push_back(transform * verts[i]);
            }
            
            // ask the transformed face for the UVs at the transformed verts
            std::vector<Vec2> transformedVertUVs;
            for (size_t i=0; i<verts.size(); i++) {
                transformedVertUVs.push_back(face->textureCoords(transformedVerts[i]));
            }
            
#if 0
            printf("transformed face attribs: scale %f %f, rotation %f, offset %f %f\n",
                   face->attribs().scale().x(),
                   face->attribs().scale().y(),
                   face->attribs().rotation(),
                   face->attribs().offset().x(),
                   face->attribs().offset().y());
#endif
            
            EXPECT_TC_EQ(uvs[0], transformedVertUVs[0]);
            
            for (size_t i=1; i<verts.size(); i++) {
                // note, just checking:
                //   EXPECT_TC_EQ(uvs[i], transformedVertUVs[i]);
                // would be too lenient.
                EXPECT_VEC_EQ(uvs[i] - uvs[0], transformedVertUVs[i] - transformedVertUVs[0]);
            }
            
            delete face;
        }

        /**
         * Given a face and three reference verts and their UVs,
         * generates many different transformations and checks that the UVs are
         * stable after these transformations.
         */
        static void checkTextureLockWithTranslationAnd90DegreeRotations(const BrushFace *origFace) {
            for (int i=0; i<(1 << 12); i++) {
                Mat4x4 xform;
                
                const bool xMinus50 = (i & (1 << 0)) != 0;
                const bool yMinus50 = (i & (1 << 1)) != 0;
                const bool zMinus50 = (i & (1 << 2)) != 0;
                
                const bool xPlus100 = (i & (1 << 3)) != 0;
                const bool yPlus100 = (i & (1 << 4)) != 0;
                const bool zPlus100 = (i & (1 << 5)) != 0;
                
                const bool rollMinus180  = (i & (1 << 6)) != 0;
                const bool pitchMinus180 = (i & (1 << 7)) != 0;
                const bool yawMinus180   = (i & (1 << 8)) != 0;
                
                const bool rollPlus90    = (i & (1 << 9)) != 0;
                const bool pitchPlus90   = (i & (1 << 10)) != 0;
                const bool yawPlus90     = (i & (1 << 11)) != 0;
                
                // translations
                
                if (xMinus50) xform = translationMatrix(Vec3(-50.0, 0.0,   0.0)) * xform;
                if (yMinus50) xform = translationMatrix(Vec3(0.0,   -50.0, 0.0)) * xform;
                if (zMinus50) xform = translationMatrix(Vec3(0.0,   0.0,   -50.0)) * xform;
                
                if (xPlus100) xform = translationMatrix(Vec3(100.0, 0.0,   0.0)) * xform;
                if (yPlus100) xform = translationMatrix(Vec3(0.0,   100.0, 0.0)) * xform;
                if (zPlus100) xform = translationMatrix(Vec3(0.0,   0.0,   100.0)) * xform;
                
                // -180 / -90 / 90 degree rotations
                
                if (rollMinus180) xform = rotationMatrix(Math::radians(-180.0), 0.0, 0.0) * xform;
                if (pitchMinus180) xform = rotationMatrix(0.0, Math::radians(-180.0), 0.0) * xform;
                if (yawMinus180) xform = rotationMatrix(0.0, 0.0, Math::radians(-180.0))* xform;
                
                if (rollPlus90) xform = rotationMatrix(Math::radians(90.0), 0.0, 0.0) * xform;
                if (pitchPlus90) xform = rotationMatrix(0.0, Math::radians(90.0), 0.0) * xform;
                if (yawPlus90) xform = rotationMatrix(0.0, 0.0, Math::radians(90.0)) * xform;

                checkTextureLockWithTransform(xform, origFace);
            }
        }

        /**
         * Tests texture lock by rotating by the given amount, in each axis alone,
         * as well as in all combinations of axes.
         */
        static void checkTextureLockWithMultiAxisRotations(const BrushFace *origFace,
                                                           double degrees) {
            const double rotateRadians = Math::radians(degrees);
            
            for (int i=0; i<(1 << 3); i++) {
                Mat4x4 xform;
                
                const bool testRoll    = (i & (1 << 0)) != 0;
                const bool testPitch   = (i & (1 << 1)) != 0;
                const bool testYaw     = (i & (1 << 2)) != 0;
                
                if (testRoll) {
                    xform = rotationMatrix(rotateRadians, 0.0, 0.0) * xform;
                }
                if (testPitch) {
                    xform = rotationMatrix(0.0, rotateRadians, 0.0) * xform;
                }
                if (testYaw) {
                    xform = rotationMatrix(0.0, 0.0, rotateRadians) * xform;
                }

                checkTextureLockWithTransform(xform, origFace);
            }
        }
        
        /**
         * Tests texture lock by rotating +/- the given amount, in one axis at a time.
         */
        static void checkTextureLockWithSingleAxisRotations(const BrushFace *origFace,
                                                            double degrees) {
            const double rotateRadians = Math::radians(degrees);
            
            for (int i=0; i<6; i++) {
                Mat4x4 xform;
                
                switch (i) {
                    case 0: xform = rotationMatrix(rotateRadians, 0.0, 0.0) * xform; break;
                    case 1: xform = rotationMatrix(-rotateRadians, 0.0, 0.0) * xform; break;
                    case 2: xform = rotationMatrix(0.0, rotateRadians, 0.0) * xform; break;
                    case 3: xform = rotationMatrix(0.0, -rotateRadians, 0.0) * xform; break;
                    case 4: xform = rotationMatrix(0.0, 0.0, rotateRadians) * xform; break;
                    case 5: xform = rotationMatrix(0.0, 0.0, -rotateRadians) * xform; break;
                }
                
                checkTextureLockWithTransform(xform, origFace);
            }
        }
        
        static void checkTextureLockForFace(const BrushFace *origFace, bool doParallelTests) {
            checkTextureLockWithTranslationAnd90DegreeRotations(origFace);
            checkTextureLockWithSingleAxisRotations(origFace, 30);
            checkTextureLockWithSingleAxisRotations(origFace, 45);
            if (doParallelTests) {
                checkTextureLockWithMultiAxisRotations(origFace, 30);
                checkTextureLockWithMultiAxisRotations(origFace, 45);
            }
        }
        
        TEST(BrushFaceTest, testTextureLock_Paraxial) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            const Brush* cube = builder.createCube(128.0, "someName");
            const BrushFaceList& faces = cube->faces();
            
            for (size_t i = 0; i < faces.size(); ++i) {
                const BrushFace *face = faces[i];
                checkTextureLockForFace(face, false);
            }
        }

        TEST(BrushFaceTest, testTextureLock_Parallel) {
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Valve, NULL, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            const Brush* cube = builder.createCube(128.0, "someName");
            const BrushFaceList& faces = cube->faces();
            
            for (size_t i = 0; i < faces.size(); ++i) {
                const BrushFace *face = faces[i];
                checkTextureLockForFace(face, true);
            }
        }
    }
}
