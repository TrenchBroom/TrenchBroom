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

#include "TrenchBroom.h"
#include "Exceptions.h"
#include "TestUtils.h"
#include "IO/NodeReader.h"
#include "IO/TestParserStatus.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushFaceSnapshot.h"
#include "Model/MapFormat.h"
#include "Model/NodeSnapshot.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/World.h"
#include "Model/ModelTypes.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>

#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace Model {
        TEST(BrushFaceTest, constructWithValidPoints) {
            const vm::vec3 p0(0.0,  0.0, 4.0);
            const vm::vec3 p1(1.0,  0.0, 4.0);
            const vm::vec3 p2(0.0, -1.0, 4.0);
            
            const BrushFaceAttributes attribs("");
            BrushFace face(p0, p1, p2, attribs, new ParaxialTexCoordSystem(p0, p1, p2, attribs));
            ASSERT_VEC_EQ(p0, face.points()[0]);
            ASSERT_VEC_EQ(p1, face.points()[1]);
            ASSERT_VEC_EQ(p2, face.points()[2]);
            ASSERT_VEC_EQ(vm::vec3::pos_z, face.boundary().normal);
            ASSERT_EQ(4.0, face.boundary().distance);
        }
        
        TEST(BrushFaceTest, constructWithColinearPoints) {
            const vm::vec3 p0(0.0, 0.0, 4.0);
            const vm::vec3 p1(1.0, 0.0, 4.0);
            const vm::vec3 p2(2.0, 0.0, 4.0);
            
            const BrushFaceAttributes attribs("");
            ASSERT_THROW(new BrushFace(p0, p1, p2, attribs, new ParaxialTexCoordSystem(p0, p1, p2, attribs)), GeometryException);
        }
        
        TEST(BrushFaceTest, textureUsageCount) {
            const vm::vec3 p0(0.0,  0.0, 4.0);
            const vm::vec3 p1(1.0,  0.0, 4.0);
            const vm::vec3 p2(0.0, -1.0, 4.0);
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
                clone = nullptr;
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
                                             std::vector<vm::vec3> *vertPositions,
                                             std::vector<vm::vec2f> *vertTexCoords) {
            BrushFace::VertexList::const_iterator it;
            BrushFace::VertexList verts = face->vertices();
            for (it = std::begin(verts); it != std::end(verts); ++it) {
                vertPositions->push_back(it->position());
                if (vertTexCoords != nullptr) {
                    vertTexCoords->push_back(face->textureCoords(vm::vec3(it->position())));
                }
            }
        }
        
        static void resetFaceTextureAlignment(BrushFace *face) {
            face->resetTextureAxes();
            face->setXOffset(0.0);
            face->setYOffset(0.0);
            face->setRotation(0.0);
            face->setXScale(1.0);
            face->setYScale(1.0);
        }
        
        /**
         * Assumes the UV's have been divided by the texture size.
         */
        static void checkUVListsEqual(const std::vector<vm::vec2f> &uvs,
                                      const std::vector<vm::vec2f> &transformedVertUVs,
                                      const BrushFace* face) {
            ASSERT_EQ(uvs.size(), transformedVertUVs.size());
            ASSERT_GE(uvs.size(), 3U);

            // We require a texture, so that face->textureSize() returns a correct value and not 1x1,
            // and so face->textureCoords() returns UV's that are divided by the texture size.
            // Otherwise, the UV comparisons below could spuriously pass.
            ASSERT_NE(nullptr, face->texture());
            
            EXPECT_TC_EQ(uvs[0], transformedVertUVs[0]);
            
            for (size_t i=1; i<uvs.size(); i++) {
                // note, just checking:
                //   EXPECT_TC_EQ(uvs[i], transformedVertUVs[i]);
                // would be too lenient.
                const vm::vec2f expected = uvs[i] - uvs[0];
                const vm::vec2f actual = transformedVertUVs[i] - transformedVertUVs[0];
                EXPECT_VEC_EQ(expected, actual);
            }
        }
        
        /**
         * Incomplete test for transforming a face with texture lock off.
         *
         * It only tests that texture lock off works when the face's texture
         * alignment is reset before applying the transform.
         */
        static void checkTextureLockOffWithTransform(const vm::mat4x4 &transform,
                                                     const BrushFace *origFace) {
            
            // reset alignment, transform the face (texture lock off)
            BrushFace *face = origFace->clone();
            resetFaceTextureAlignment(face);
            face->transform(transform, false);
            face->resetTexCoordSystemCache();
            
            // reset alignment, transform the face (texture lock off), then reset the alignment again
            BrushFace *resetFace = origFace->clone();
            resetFaceTextureAlignment(resetFace);
            resetFace->transform(transform, false);
            resetFaceTextureAlignment(resetFace);
            
            // UVs of the verts of `face` and `resetFace` should be the same now
            
            std::vector<vm::vec3> verts;
            getFaceVertsAndTexCoords(origFace, &verts, nullptr);
            
            // transform the verts
            std::vector<vm::vec3> transformedVerts;
            for (size_t i=0; i<verts.size(); i++) {
                transformedVerts.push_back(transform * verts[i]);
            }
            
            // get UV of each transformed vert using `face` and `resetFace`
            std::vector<vm::vec2f> face_UVs, resetFace_UVs;
            for (size_t i=0; i<verts.size(); i++) {
                face_UVs.push_back(face->textureCoords(transformedVerts[i]));
                resetFace_UVs.push_back(resetFace->textureCoords(transformedVerts[i]));
            }
            
            checkUVListsEqual(face_UVs, resetFace_UVs, face);

            delete face;
            delete resetFace;
        }
        
        /**
         * Applies the given transform to a copy of origFace.
         *
         * Checks that the UV coordinates of the verts
         * are equivelant to the UV coordinates of the non-transformed verts,
         * i.e. checks that texture lock worked.
         */
        static void checkTextureLockOnWithTransform(const vm::mat4x4 &transform,
                                                    const BrushFace *origFace) {
            std::vector<vm::vec3> verts;
            std::vector<vm::vec2f> uvs;
            getFaceVertsAndTexCoords(origFace, &verts, &uvs);
            ASSERT_GE(verts.size(), 3U);

            // transform the face
            BrushFace *face = origFace->clone();
            face->transform(transform, true);
            face->resetTexCoordSystemCache();
            
            // transform the verts
            std::vector<vm::vec3> transformedVerts;
            for (size_t i=0; i<verts.size(); i++) {
                transformedVerts.push_back(transform * verts[i]);
            }
            
            // ask the transformed face for the UVs at the transformed verts
            std::vector<vm::vec2f> transformedVertUVs;
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
            
            checkUVListsEqual(uvs, transformedVertUVs, face);

            delete face;
        }

        /**
         * Given a face and three reference verts and their UVs,
         * generates many different transformations and checks that the UVs are
         * stable after these transformations.
         */
        static void checkTextureLockWithTranslationAnd90DegreeRotations(const BrushFace *origFace) {
            for (int i=0; i<(1 << 7); i++) {
                vm::mat4x4 xform;
                
                const bool translate = (i & (1 << 0)) != 0;
                
                const bool rollMinus180  = (i & (1 << 1)) != 0;
                const bool pitchMinus180 = (i & (1 << 2)) != 0;
                const bool yawMinus180   = (i & (1 << 3)) != 0;
                
                const bool rollPlus90    = (i & (1 << 4)) != 0;
                const bool pitchPlus90   = (i & (1 << 5)) != 0;
                const bool yawPlus90     = (i & (1 << 6)) != 0;
                
                // translations
                
                if (translate) {
                    xform = vm::translationMatrix(vm::vec3(100.0, 100.0, 100.0)) * xform;
                }
                
                // -180 / -90 / 90 degree rotations
                
                if (rollMinus180) xform = vm::rotationMatrix(vm::radians(-180.0), 0.0, 0.0) * xform;
                if (pitchMinus180) xform = vm::rotationMatrix(0.0, vm::radians(-180.0), 0.0) * xform;
                if (yawMinus180) xform = vm::rotationMatrix(0.0, 0.0, vm::radians(-180.0))* xform;
                
                if (rollPlus90) xform = vm::rotationMatrix(vm::radians(90.0), 0.0, 0.0) * xform;
                if (pitchPlus90) xform = vm::rotationMatrix(0.0, vm::radians(90.0), 0.0) * xform;
                if (yawPlus90) xform = vm::rotationMatrix(0.0, 0.0, vm::radians(90.0)) * xform;

                checkTextureLockOnWithTransform(xform, origFace);
            }
        }

        /**
         * Tests texture lock by rotating by the given amount, in each axis alone,
         * as well as in all combinations of axes.
         */
        static void checkTextureLockWithMultiAxisRotations(const BrushFace *origFace,
                                                           double degrees) {
            const double rotateRadians = vm::radians(degrees);
            
            for (int i=0; i<(1 << 3); i++) {
                vm::mat4x4 xform;
                
                const bool testRoll    = (i & (1 << 0)) != 0;
                const bool testPitch   = (i & (1 << 1)) != 0;
                const bool testYaw     = (i & (1 << 2)) != 0;
                
                if (testRoll) {
                    xform = vm::rotationMatrix(rotateRadians, 0.0, 0.0) * xform;
                }
                if (testPitch) {
                    xform = vm::rotationMatrix(0.0, rotateRadians, 0.0) * xform;
                }
                if (testYaw) {
                    xform = vm::rotationMatrix(0.0, 0.0, rotateRadians) * xform;
                }

                checkTextureLockOnWithTransform(xform, origFace);
            }
        }
        
        /**
         * Tests texture lock by rotating +/- the given amount, in one axis at a time.
         */
        static void checkTextureLockWithSingleAxisRotations(const BrushFace *origFace,
                                                            double degrees) {
            const double rotateRadians = vm::radians(degrees);
            
            for (int i=0; i<6; i++) {
                vm::mat4x4 xform;
                
                switch (i) {
                    case 0: xform = vm::rotationMatrix(rotateRadians, 0.0, 0.0) * xform; break;
                    case 1: xform = vm::rotationMatrix(-rotateRadians, 0.0, 0.0) * xform; break;
                    case 2: xform = vm::rotationMatrix(0.0, rotateRadians, 0.0) * xform; break;
                    case 3: xform = vm::rotationMatrix(0.0, -rotateRadians, 0.0) * xform; break;
                    case 4: xform = vm::rotationMatrix(0.0, 0.0, rotateRadians) * xform; break;
                    case 5: xform = vm::rotationMatrix(0.0, 0.0, -rotateRadians) * xform; break;
                }
                
                checkTextureLockOnWithTransform(xform, origFace);
            }
        }
        
        static void checkTextureLockOffWithTranslation(const BrushFace *origFace) {
            vm::mat4x4 xform = vm::translationMatrix(vm::vec3(100.0, 100.0, 100.0));
            checkTextureLockOffWithTransform(xform, origFace);
        }
        
        static void checkTextureLockWithScale(const BrushFace *origFace, const vm::vec3& scaleFactors) {
            vm::mat4x4 xform = vm::scalingMatrix(scaleFactors);
            checkTextureLockOnWithTransform(xform, origFace);
        }
        
        static void checkTextureLockWithShear(const BrushFace *origFace) {
            // shear the x axis towards the y axis
            vm::mat4x4 xform = vm::shearMatrix(1.0, 0.0, 0.0, 0.0, 0.0, 0.0);
            checkTextureLockOnWithTransform(xform, origFace);
        }
        
        static void checkTextureLockForFace(const BrushFace *origFace, bool doParallelTests) {
            checkTextureLockWithTranslationAnd90DegreeRotations(origFace);
            checkTextureLockWithSingleAxisRotations(origFace, 30);
            checkTextureLockWithSingleAxisRotations(origFace, 45);
            
            // rotation on multiple axes simultaneously is only expected to work on ParallelTexCoordSystem
            if (doParallelTests) {
                checkTextureLockWithMultiAxisRotations(origFace, 30);
                checkTextureLockWithMultiAxisRotations(origFace, 45);
                
                checkTextureLockWithShear(origFace);
            }
            
            checkTextureLockOffWithTranslation(origFace);
            
            checkTextureLockWithScale(origFace, vm::vec3(2, 2, 1));
        }
        
        /**
         * For the sides of a cube, a horizontal or vertical flip should have no effect on texturing
         * when texture lock is off.
         */
        static void checkTextureLockOffWithVerticalFlip(const Brush* cube) {
            const vm::mat4x4 transform = vm::mirrorMatrix<double>(vm::axis::z);
            const BrushFace* origFace = cube->findFace(vm::vec3::pos_x);
            
            // transform the face (texture lock off)
            BrushFace* face = origFace->clone();
            face->transform(transform, false);
            face->resetTexCoordSystemCache();
            
            // UVs of the verts of `face` and `origFace` should be the same now

            // get UV of each vert using `face` and `resetFace`
            std::vector<vm::vec2f> face_UVs, origFace_UVs;
            for (const auto vert : origFace->vertices()) {
                face_UVs.push_back(face->textureCoords(vert->position()));
                origFace_UVs.push_back(origFace->textureCoords(vert->position()));
            }
            
            checkUVListsEqual(face_UVs, origFace_UVs, face);
            
            delete face;
        }
        
        static void checkTextureLockOffWithScale(const Brush* cube) {
            const vm::vec3 mins(cube->bounds().min);
            
            // translate the cube mins to the origin, scale by 2 in the X axis, then translate back
            const vm::mat4x4 transform = vm::translationMatrix(mins) * vm::scalingMatrix(vm::vec3(2.0, 1.0, 1.0)) * vm::translationMatrix(-1.0 * mins);
            const BrushFace* origFace = cube->findFace(vm::vec3::neg_y);
            
            // transform the face (texture lock off)
            BrushFace* face = origFace->clone();
            face->transform(transform, false);
            face->resetTexCoordSystemCache();
            
            // get UV at mins; should be equal
            const vm::vec2f left_origTC = origFace->textureCoords(mins);
            const vm::vec2f left_transformedTC = face->textureCoords(mins);
            EXPECT_TC_EQ(left_origTC, left_transformedTC);

            // get UVs at mins, plus the X size of the cube
            const vm::vec2f right_origTC = origFace->textureCoords(mins + vm::vec3(cube->bounds().size().x(), 0, 0));
            const vm::vec2f right_transformedTC = face->textureCoords(mins + vm::vec3(2.0 * cube->bounds().size().x(), 0, 0));
            
            // this assumes that the U axis of the texture was scaled (i.e. the texture is oriented upright)
            const vm::vec2f orig_U_width = right_origTC - left_origTC;
            const vm::vec2f transformed_U_width = right_transformedTC - left_transformedTC;
            
            EXPECT_FLOAT_EQ(orig_U_width.x() * 2.0f, transformed_U_width.x());
            EXPECT_FLOAT_EQ(orig_U_width.y(), transformed_U_width.y());
            
            delete face;
        }
        
        TEST(BrushFaceTest, testTextureLock_Paraxial) {
            const vm::bbox3 worldBounds(8192.0);
            Assets::Texture texture("testTexture", 64, 64);
            World world(MapFormat::Standard, nullptr, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            const Brush* cube = builder.createCube(128.0, "");
            const BrushFaceList& faces = cube->faces();
            
            for (size_t i = 0; i < faces.size(); ++i) {
                BrushFace *face = faces[i];
                face->setTexture(&texture);
                checkTextureLockForFace(face, false);
            }
            
            checkTextureLockOffWithVerticalFlip(cube);
            checkTextureLockOffWithScale(cube);
            
            delete cube;
        }

        TEST(BrushFaceTest, testTextureLock_Parallel) {
            const vm::bbox3 worldBounds(8192.0);
            Assets::Texture texture("testTexture", 64, 64);
            World world(MapFormat::Valve, nullptr, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            const Brush* cube = builder.createCube(128.0, "");
            const BrushFaceList& faces = cube->faces();
            
            for (size_t i = 0; i < faces.size(); ++i) {
                BrushFace *face = faces[i];
                face->setTexture(&texture);
                checkTextureLockForFace(face, true);
            }
            
            checkTextureLockOffWithVerticalFlip(cube);
            checkTextureLockOffWithScale(cube);

            delete cube;
        }
        
        TEST(BrushFaceTest, testBrushFaceSnapshot) {
            const vm::bbox3 worldBounds(8192.0);
            Assets::Texture texture("testTexture", 64, 64);
            World world(MapFormat::Valve, nullptr, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            Brush* cube = builder.createCube(128.0, "");
            
            BrushFace* topFace = cube->findFace(vm::vec3(0.0, 0.0, 1.0));
            ASSERT_NE(nullptr, topFace);
            ASSERT_EQ(0.0, topFace->rotation());
            BrushFaceSnapshot* snapshot = topFace->takeSnapshot();
            
            // Rotate texture of topFace
            topFace->rotateTexture(5.0);
            ASSERT_EQ(5.0, topFace->rotation());
            
            // Hack to get the Brush to delete and recreate its BrushFaces
            {
                NodeSnapshot* cubeSnapshot = cube->takeSnapshot();
                cubeSnapshot->restore(worldBounds);
                delete cubeSnapshot;
                
                // NOTE: topFace is a dangling pointer here
                ASSERT_NE(topFace, cube->findFace(vm::vec3(0.0, 0.0, 1.0)));
            }

            // Lookup the new copy of topFace
            topFace = cube->findFace(vm::vec3(0.0, 0.0, 1.0));
            
            // Ensure that the snapshot can be restored, despite the Brush having a new BrushFace object
            snapshot->restore();
            ASSERT_EQ(0.0, topFace->rotation());
            
            delete snapshot;
            delete cube;
        }

        // https://github.com/kduske/TrenchBroom/issues/2001
        TEST(BrushFaceTest, testValveRotation) {
            const String data("{\n"
                                      "\"classname\" \"worldspawn\"\n"
                                      "{\n"
                                      "( 24 8 48 ) ( 32 16 -16 ) ( 24 -8 48 ) tlight11 [ 0 1 0 0 ] [ 0 0 -1 56 ] -0 1 1\n"
                                      "( 8 -8 48 ) ( -0 -16 -16 ) ( 8 8 48 ) tlight11 [ 0 1 0 0 ] [ 0 0 -1 56 ] -0 1 1\n"
                                      "( 8 8 48 ) ( -0 16 -16 ) ( 24 8 48 ) tlight11 [ 1 0 0 -0 ] [ 0 0 -1 56 ] -0 1 1\n"
                                      "( 24 -8 48 ) ( 32 -16 -16 ) ( 8 -8 48 ) tlight11 [ 1 0 0 0 ] [ 0 0 -1 56 ] -0 1 1\n"
                                      "( 8 -8 48 ) ( 8 8 48 ) ( 24 -8 48 ) tlight11 [ 1 0 0 0 ] [ 0 -1 0 48 ] -0 1 1\n"
                                      "( -0 16 -16 ) ( -0 -16 -16 ) ( 32 16 -16 ) tlight11 [ -1 0 0 -0 ] [ 0 -1 0 48 ] -0 1 1\n"
                                      "}\n"
                                      "}\n");

            const vm::bbox3 worldBounds(4096.0);
            World world(MapFormat::Valve, nullptr, worldBounds);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, &world);

            NodeList nodes = reader.read(worldBounds, status);
            Brush* pyramidLight = static_cast<Brush*>(nodes.at(0)->children().at(0));
            ASSERT_NE(nullptr, pyramidLight);

            // find the faces
            BrushFace* negXFace = nullptr;
            for (BrushFace* face : pyramidLight->faces()) {
                if (firstAxis(face->boundary().normal) == vm::vec3::neg_x) {
                    ASSERT_EQ(negXFace, nullptr);
                    negXFace = face;
                }
            }
            ASSERT_NE(nullptr, negXFace);

            ASSERT_EQ(vm::vec3::pos_y, negXFace->textureXAxis());
            ASSERT_EQ(vm::vec3::neg_z, negXFace->textureYAxis());

            // This face's texture normal is in the same direction as the face normal
            const vm::vec3 textureNormal = normalize(cross(negXFace->textureXAxis(), negXFace->textureYAxis()));
            ASSERT_GT(dot(textureNormal, vm::vec3(negXFace->boundary().normal)), 0.0);

            const vm::quat3 rot45(textureNormal, vm::radians(45.0));
            const vm::vec3 newXAxis(rot45 * negXFace->textureXAxis());
            const vm::vec3 newYAxis(rot45 * negXFace->textureYAxis());

            // Rotate by 45 degrees CCW
            ASSERT_FLOAT_EQ(0.0f, negXFace->attribs().rotation());
            negXFace->rotateTexture(45.0);
            ASSERT_FLOAT_EQ(45.0f, negXFace->attribs().rotation());

            ASSERT_VEC_EQ(newXAxis, negXFace->textureXAxis());
            ASSERT_VEC_EQ(newYAxis, negXFace->textureYAxis());

            VectorUtils::clearAndDelete(nodes);
        }

        // https://github.com/kduske/TrenchBroom/issues/1995
        TEST(BrushFaceTest, testCopyTexCoordSystem) {
            const String data("{\n"
                                      "    \"classname\" \"worldspawn\"\n"
                                      "    {\n"
                                      "        ( 24 8 48 ) ( 32 16 -16 ) ( 24 -8 48 ) tlight11 [ 0 1 0 0 ] [ 0 0 -1 56 ] -0 1 1\n"
                                      "        ( 8 -8 48 ) ( -0 -16 -16 ) ( 8 8 48 ) tlight11 [ 0 1 0 0 ] [ 0 0 -1 56 ] -0 1 1\n"
                                      "        ( 8 8 48 ) ( -0 16 -16 ) ( 24 8 48 ) tlight11 [ 1 0 0 -0 ] [ 0 0 -1 56 ] -0 1 1\n"
                                      "        ( 24 -8 48 ) ( 32 -16 -16 ) ( 8 -8 48 ) tlight11 [ 1 0 0 0 ] [ 0 0 -1 56 ] -0 1 1\n"
                                      "        ( 8 -8 48 ) ( 8 8 48 ) ( 24 -8 48 ) tlight11 [ 1 0 0 0 ] [ 0 -1 0 48 ] -0 1 1\n"
                                      "        ( -0 16 -16 ) ( -0 -16 -16 ) ( 32 16 -16 ) tlight11 [ -1 0 0 -0 ] [ 0 -1 0 48 ] -0 1 1\n"
                                      "    }\n"
                                      "}\n");

            const vm::bbox3 worldBounds(4096.0);
            World world(MapFormat::Valve, nullptr, worldBounds);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, &world);

            NodeList nodes = reader.read(worldBounds, status);
            Brush* pyramidLight = static_cast<Brush*>(nodes.at(0)->children().at(0));
            ASSERT_NE(nullptr, pyramidLight);

            // find the faces
            BrushFace* negYFace = nullptr;
            BrushFace* posXFace = nullptr;
            for (BrushFace* face : pyramidLight->faces()) {
                if (firstAxis(face->boundary().normal) == vm::vec3::neg_y) {
                    ASSERT_EQ(negYFace, nullptr);
                    negYFace = face;
                } else if (firstAxis(face->boundary().normal) == vm::vec3::pos_x) {
                    ASSERT_EQ(posXFace, nullptr);
                    posXFace = face;
                }
            }
            ASSERT_NE(nullptr, negYFace);
            ASSERT_NE(nullptr, posXFace);

            ASSERT_EQ(vm::vec3::pos_x, negYFace->textureXAxis());
            ASSERT_EQ(vm::vec3::neg_z, negYFace->textureYAxis());

            TexCoordSystemSnapshot* snapshot = negYFace->takeTexCoordSystemSnapshot();

            // copy texturing from the negYFace to posXFace using the rotation method
            posXFace->copyTexCoordSystemFromFace(snapshot, negYFace->attribs(), negYFace->boundary(), WrapStyle::Rotation);
            ASSERT_VEC_EQ(vm::vec3(0.030303030303030123, 0.96969696969696961, -0.24242424242424243), posXFace->textureXAxis());
            ASSERT_VEC_EQ(vm::vec3(-0.0037296037296037088, -0.24242424242424243, -0.97016317016317011), posXFace->textureYAxis());

            // copy texturing from the negYFace to posXFace using the projection method
            posXFace->copyTexCoordSystemFromFace(snapshot, negYFace->attribs(), negYFace->boundary(), WrapStyle::Projection);
            ASSERT_VEC_EQ(vm::vec3::neg_y, posXFace->textureXAxis());
            ASSERT_VEC_EQ(vm::vec3::neg_z, posXFace->textureYAxis());

            delete snapshot;
            VectorUtils::clearAndDelete(nodes);
        }
    }
}
