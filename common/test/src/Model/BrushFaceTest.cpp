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
#include "FloatType.h"
#include "Assets/Texture.h"
#include "IO/NodeReader.h"
#include "IO/TestParserStatus.h"
#include "Model/BrushError.h"
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/Entity.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/NodeSnapshot.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/Polyhedron.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>
#include <kdl/vector_utils.h>

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>

#include <memory>
#include <vector>

#include "Catch2.h"
#include "GTestCompat.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("BrushFaceTest.constructWithValidPoints", "[BrushFaceTest]") {
            const vm::vec3 p0(0.0,  0.0, 4.0);
            const vm::vec3 p1(1.0,  0.0, 4.0);
            const vm::vec3 p2(0.0, -1.0, 4.0);

            const BrushFaceAttributes attribs("");
            BrushFace face = BrushFace::create(p0, p1, p2, attribs, std::make_unique<ParaxialTexCoordSystem>(p0, p1, p2, attribs)).value();
            ASSERT_VEC_EQ(p0, face.points()[0]);
            ASSERT_VEC_EQ(p1, face.points()[1]);
            ASSERT_VEC_EQ(p2, face.points()[2]);
            ASSERT_VEC_EQ(vm::vec3::pos_z(), face.boundary().normal);
            ASSERT_EQ(4.0, face.boundary().distance);
        }

        TEST_CASE("BrushFaceTest.constructWithColinearPoints", "[BrushFaceTest]") {
            const vm::vec3 p0(0.0, 0.0, 4.0);
            const vm::vec3 p1(1.0, 0.0, 4.0);
            const vm::vec3 p2(2.0, 0.0, 4.0);

            const BrushFaceAttributes attribs("");
            CHECK_FALSE(BrushFace::create(p0, p1, p2, attribs, std::make_unique<ParaxialTexCoordSystem>(p0, p1, p2, attribs)).is_success());
        }

        TEST_CASE("BrushFaceTest.textureUsageCount", "[BrushFaceTest]") {
            const vm::vec3 p0(0.0,  0.0, 4.0);
            const vm::vec3 p1(1.0,  0.0, 4.0);
            const vm::vec3 p2(0.0, -1.0, 4.0);
            Assets::Texture texture("testTexture", 64, 64);
            Assets::Texture texture2("testTexture2", 64, 64);

            EXPECT_EQ(0u, texture.usageCount());
            EXPECT_EQ(0u, texture2.usageCount());

            BrushFaceAttributes attribs("");
            {
                // test constructor
                BrushFace face = BrushFace::create(p0, p1, p2, attribs, std::make_unique<ParaxialTexCoordSystem>(p0, p1, p2, attribs)).value();
                EXPECT_EQ(0u, texture.usageCount());

                // test setTexture
                face.setTexture(&texture);
                EXPECT_EQ(1u, texture.usageCount());
                EXPECT_EQ(0u, texture2.usageCount());

                {
                    // test copy constructor
                    BrushFace clone = face;
                    EXPECT_EQ(2u, texture.usageCount());
                }

                // test destructor
                EXPECT_EQ(1u, texture.usageCount());

                // test setTexture with different texture
                face.setTexture(&texture2);
                EXPECT_EQ(0u, texture.usageCount());
                EXPECT_EQ(1u, texture2.usageCount());

                // test setTexture with the same texture
                face.setTexture(&texture2);
                EXPECT_EQ(1u, texture2.usageCount());
            }

            EXPECT_EQ(0u, texture.usageCount());
            EXPECT_EQ(0u, texture2.usageCount());
        }

        static void getFaceVertsAndTexCoords(const BrushFace& face,
                                             std::vector<vm::vec3> *vertPositions,
                                             std::vector<vm::vec2f> *vertTexCoords) {
            for (const auto* vertex : face.vertices()) {
                vertPositions->push_back(vertex->position());
                if (vertTexCoords != nullptr) {
                    vertTexCoords->push_back(face.textureCoords(vm::vec3(vertex->position())));
                }
            }
        }

        static void resetFaceTextureAlignment(BrushFace& face) {
            BrushFaceAttributes attributes = face.attributes();
            attributes.setXOffset(0.0);
            attributes.setYOffset(0.0);
            attributes.setRotation(0.0);
            attributes.setXScale(1.0);
            attributes.setYScale(1.0);
            
            face.setAttributes(attributes);
            face.resetTextureAxes();
        }

        /**
         * Assumes the UV's have been divided by the texture size.
         */
        static void checkUVListsEqual(const std::vector<vm::vec2f> &uvs,
                                      const std::vector<vm::vec2f> &transformedVertUVs,
                                      const BrushFace& face) {
            // We require a texture, so that face.textureSize() returns a correct value and not 1x1,
            // and so face.textureCoords() returns UV's that are divided by the texture size.
            // Otherwise, the UV comparisons below could spuriously pass.
            ASSERT_NE(nullptr, face.texture());

            CHECK(UVListsEqual(uvs, transformedVertUVs));
        }

        /**
         * Incomplete test for transforming a face with texture lock off.
         *
         * It only tests that texture lock off works when the face's texture
         * alignment is reset before applying the transform.
         */
        static void checkTextureLockOffWithTransform(const vm::mat4x4 &transform,
                                                     const BrushFace& origFace) {

            // reset alignment, transform the face (texture lock off)
            BrushFace face = origFace;
            resetFaceTextureAlignment(face);
            REQUIRE(face.transform(transform, false));
            face.resetTexCoordSystemCache();

            // reset alignment, transform the face (texture lock off), then reset the alignment again
            BrushFace resetFace = origFace;
            resetFaceTextureAlignment(resetFace);
            REQUIRE(resetFace.transform(transform, false));
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
                face_UVs.push_back(face.textureCoords(transformedVerts[i]));
                resetFace_UVs.push_back(resetFace.textureCoords(transformedVerts[i]));
            }

            checkUVListsEqual(face_UVs, resetFace_UVs, face);
        }

        static void checkFaceUVsEqual(const BrushFace& face, const BrushFace& other) {
            std::vector<vm::vec3> verts;
            std::vector<vm::vec2f> faceUVs;
            std::vector<vm::vec2f> otherFaceUVs;

            for (const auto* vertex : face.vertices()) {
                verts.push_back(vertex->position());

                const vm::vec3 position(vertex->position());
                faceUVs.push_back(face.textureCoords(position));
                otherFaceUVs.push_back(other.textureCoords(position));
            }

            checkUVListsEqual(faceUVs, otherFaceUVs, face);
        }

        static void checkBrushUVsEqual(const Brush& brush, const Brush& other) {
            assert(brush.faceCount() == other.faceCount());

            for (size_t i = 0; i < brush.faceCount(); ++i) {
                checkFaceUVsEqual(brush.face(i), other.face(i));
            }
        }

        /**
         * Applies the given transform to a copy of origFace.
         *
         * Checks that the UV coordinates of the verts
         * are equivelant to the UV coordinates of the non-transformed verts,
         * i.e. checks that texture lock worked.
         */
        static void checkTextureLockOnWithTransform(const vm::mat4x4 &transform,
                                                    const BrushFace& origFace) {
            std::vector<vm::vec3> verts;
            std::vector<vm::vec2f> uvs;
            getFaceVertsAndTexCoords(origFace, &verts, &uvs);
            ASSERT_GE(verts.size(), 3U);

            // transform the face
            BrushFace face = origFace;
            REQUIRE(face.transform(transform, true));
            face.resetTexCoordSystemCache();

            // transform the verts
            std::vector<vm::vec3> transformedVerts;
            for (size_t i=0; i<verts.size(); i++) {
                transformedVerts.push_back(transform * verts[i]);
            }

            // ask the transformed face for the UVs at the transformed verts
            std::vector<vm::vec2f> transformedVertUVs;
            for (size_t i=0; i<verts.size(); i++) {
                transformedVertUVs.push_back(face.textureCoords(transformedVerts[i]));
            }

#if 0
            printf("transformed face attribs: scale %f %f, rotation %f, offset %f %f\n",
                   face.attributes().scale().x(),
                   face.attributes().scale().y(),
                   face.attributes().rotation(),
                   face.attributes().offset().x(),
                   face.attributes().offset().y());
#endif

            checkUVListsEqual(uvs, transformedVertUVs, face);
        }

        /**
         * Given a face and three reference verts and their UVs,
         * generates many different transformations and checks that the UVs are
         * stable after these transformations.
         */
        template <class L>
        static void doWithTranslationAnd90DegreeRotations(L&& lambda) {
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
                    xform = vm::translation_matrix(vm::vec3(100.0, 100.0, 100.0)) * xform;
                }

                // -180 / -90 / 90 degree rotations

                if (rollMinus180) xform = vm::rotation_matrix(vm::to_radians(-180.0), 0.0, 0.0) * xform;
                if (pitchMinus180) xform = vm::rotation_matrix(0.0, vm::to_radians(-180.0), 0.0) * xform;
                if (yawMinus180) xform = vm::rotation_matrix(0.0, 0.0, vm::to_radians(-180.0))* xform;

                if (rollPlus90) xform = vm::rotation_matrix(vm::to_radians(90.0), 0.0, 0.0) * xform;
                if (pitchPlus90) xform = vm::rotation_matrix(0.0, vm::to_radians(90.0), 0.0) * xform;
                if (yawPlus90) xform = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0)) * xform;

                lambda(xform);
            }
        }

        /**
         * Generates transforms for testing texture lock, etc., by rotating by the given amount, in each axis alone,
         * as well as in all combinations of axes.
         */
        template <class L>
        static void doMultiAxisRotations(const double degrees, L&& lambda) {
            const double rotateRadians = vm::to_radians(degrees);

            for (int i=0; i<(1 << 3); i++) {
                vm::mat4x4 xform;

                const bool testRoll    = (i & (1 << 0)) != 0;
                const bool testPitch   = (i & (1 << 1)) != 0;
                const bool testYaw     = (i & (1 << 2)) != 0;

                if (testRoll) {
                    xform = vm::rotation_matrix(rotateRadians, 0.0, 0.0) * xform;
                }
                if (testPitch) {
                    xform = vm::rotation_matrix(0.0, rotateRadians, 0.0) * xform;
                }
                if (testYaw) {
                    xform = vm::rotation_matrix(0.0, 0.0, rotateRadians) * xform;
                }

                lambda(xform);
            }
        }

        /**
         * Runs the given lambda of type `const vm::mat4x4& -> void` with
         * rotations of the given angle in degrees in +/- pitch, yaw, and roll.
         */
        template <class L>
        static void doWithSingleAxisRotations(const double degrees, L&& lambda) {
            const double rotateRadians = vm::to_radians(degrees);

            for (int i=0; i<6; i++) {
                vm::mat4x4 xform;

                switch (i) {
                    case 0: xform = vm::rotation_matrix(rotateRadians, 0.0, 0.0) * xform; break;
                    case 1: xform = vm::rotation_matrix(-rotateRadians, 0.0, 0.0) * xform; break;
                    case 2: xform = vm::rotation_matrix(0.0, rotateRadians, 0.0) * xform; break;
                    case 3: xform = vm::rotation_matrix(0.0, -rotateRadians, 0.0) * xform; break;
                    case 4: xform = vm::rotation_matrix(0.0, 0.0, rotateRadians) * xform; break;
                    case 5: xform = vm::rotation_matrix(0.0, 0.0, -rotateRadians) * xform; break;
                }

                lambda(xform);
            }
        }

        static void checkTextureLockOffWithTranslation(const BrushFace& origFace) {
            vm::mat4x4 xform = vm::translation_matrix(vm::vec3(100.0, 100.0, 100.0));
            checkTextureLockOffWithTransform(xform, origFace);
        }

        template <class L>
        static void doWithScale(const vm::vec3& scaleFactors, L&& lambda) {
            vm::mat4x4 xform = vm::scaling_matrix(scaleFactors);
            lambda(xform);
        }

        template <class L>
        static void doWithShear(L&& lambda) {
            // shear the x axis towards the y axis
            vm::mat4x4 xform = vm::shear_matrix(1.0, 0.0, 0.0, 0.0, 0.0, 0.0);
            lambda(xform);
        }

        template <class L>
        static void doWithTextureLockTestTransforms(const bool doParallelTests, L&& lambda) {
            doWithTranslationAnd90DegreeRotations(lambda);
            doWithSingleAxisRotations(30, lambda);
            doWithSingleAxisRotations(45, lambda);

            // rotation on multiple axes simultaneously is only expected to work on ParallelTexCoordSystem
            if (doParallelTests) {
                doMultiAxisRotations(30.0, lambda);
                doMultiAxisRotations(45.0, lambda);

                doWithShear(lambda);
            }

            doWithScale(vm::vec3(2, 2, 1), lambda);
            doWithScale(vm::vec3(2, 2, -1), lambda);
        }

        static void checkTextureLockForFace(const BrushFace& origFace, const bool doParallelTests) {
            doWithTextureLockTestTransforms(doParallelTests, [&](const vm::mat4x4& xform){
                checkTextureLockOnWithTransform(xform, origFace);
            });

            checkTextureLockOffWithTranslation(origFace);
        }

        /**
         * For the sides of a cube, a horizontal or vertical flip should have no effect on texturing
         * when texture lock is off.
         */
        static void checkTextureLockOffWithVerticalFlip(const Brush& cube) {
            const vm::mat4x4 transform = vm::mirror_matrix<double>(vm::axis::z);
            const auto origFaceIndex = cube.findFace(vm::vec3::pos_x());
            REQUIRE(origFaceIndex);
            const BrushFace& origFace = cube.face(*origFaceIndex);

            // transform the face (texture lock off)
            BrushFace face = origFace;
            REQUIRE(face.transform(transform, false));
            face.resetTexCoordSystemCache();

            // UVs of the verts of `face` and `origFace` should be the same now

            // get UV of each vert using `face` and `resetFace`
            std::vector<vm::vec2f> face_UVs, origFace_UVs;
            for (const auto vert : origFace.vertices()) {
                face_UVs.push_back(face.textureCoords(vert->position()));
                origFace_UVs.push_back(origFace.textureCoords(vert->position()));
            }

            checkUVListsEqual(face_UVs, origFace_UVs, face);
        }

        static void checkTextureLockOffWithScale(const Brush& cube) {
            const vm::vec3 mins(cube.bounds().min);

            // translate the cube mins to the origin, scale by 2 in the X axis, then translate back
            const vm::mat4x4 transform = vm::translation_matrix(mins) * vm::scaling_matrix(vm::vec3(2.0, 1.0, 1.0)) * vm::translation_matrix(-1.0 * mins);
            const auto origFaceIndex = cube.findFace(vm::vec3::neg_y());
            REQUIRE(origFaceIndex);
            const BrushFace& origFace = cube.face(*origFaceIndex);

            // transform the face (texture lock off)
            BrushFace face = origFace;
            REQUIRE(face.transform(transform, false));
            face.resetTexCoordSystemCache();

            // get UV at mins; should be equal
            const vm::vec2f left_origTC = origFace.textureCoords(mins);
            const vm::vec2f left_transformedTC = face.textureCoords(mins);
            EXPECT_TC_EQ(left_origTC, left_transformedTC);

            // get UVs at mins, plus the X size of the cube
            const vm::vec2f right_origTC = origFace.textureCoords(mins + vm::vec3(cube.bounds().size().x(), 0, 0));
            const vm::vec2f right_transformedTC = face.textureCoords(mins + vm::vec3(2.0 * cube.bounds().size().x(), 0, 0));

            // this assumes that the U axis of the texture was scaled (i.e. the texture is oriented upright)
            const vm::vec2f orig_U_width = right_origTC - left_origTC;
            const vm::vec2f transformed_U_width = right_transformedTC - left_transformedTC;

            EXPECT_FLOAT_EQ(orig_U_width.x() * 2.0f, transformed_U_width.x());
            EXPECT_FLOAT_EQ(orig_U_width.y(), transformed_U_width.y());
        }
        
        TEST_CASE("BrushFaceTest.testSetRotation_Paraxial", "[BrushFaceTest]") {
            const vm::bbox3 worldBounds(8192.0);
            Assets::Texture texture("testTexture", 64, 64);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush cube = builder.createCube(128.0, "").value();
            BrushFace& face = cube.faces().front();

            // This face's texture normal is in the same direction as the face normal
            const vm::vec3 textureNormal = normalize(cross(face.textureXAxis(), face.textureYAxis()));

            const vm::quat3 rot45(textureNormal, vm::to_radians(45.0));
            const vm::vec3 newXAxis(rot45 * face.textureXAxis());
            const vm::vec3 newYAxis(rot45 * face.textureYAxis());
            
            BrushFaceAttributes attributes = face.attributes();
            attributes.setRotation(-45.0f);
            face.setAttributes(attributes);
            
            ASSERT_VEC_EQ(newXAxis, face.textureXAxis());
            ASSERT_VEC_EQ(newYAxis, face.textureYAxis());
        }

        TEST_CASE("BrushFaceTest.testTextureLock_Paraxial", "[BrushFaceTest]") {
            const vm::bbox3 worldBounds(8192.0);
            Assets::Texture texture("testTexture", 64, 64);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush cube = builder.createCube(128.0, "").value();
            auto& faces = cube.faces();

            for (size_t i = 0; i < faces.size(); ++i) {
                BrushFace& face = faces[i];
                face.setTexture(&texture);
                checkTextureLockForFace(face, false);
            }

            checkTextureLockOffWithVerticalFlip(cube);
            checkTextureLockOffWithScale(cube);
        }

        TEST_CASE("BrushFaceTest.testTextureLock_Parallel", "[BrushFaceTest]") {
            const vm::bbox3 worldBounds(8192.0);
            Assets::Texture texture("testTexture", 64, 64);
            WorldNode world(Entity(), MapFormat::Valve);

            BrushBuilder builder(&world, worldBounds);
            Brush cube = builder.createCube(128.0, "").value();
            auto& faces = cube.faces();

            for (size_t i = 0; i < faces.size(); ++i) {
                BrushFace& face = faces[i];
                face.setTexture(&texture);
                checkTextureLockForFace(face, true);
            }

            checkTextureLockOffWithVerticalFlip(cube);
            checkTextureLockOffWithScale(cube);
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/2001
        TEST_CASE("BrushFaceTest.testValveRotation", "[BrushFaceTest]") {
            const std::string data("{\n"
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
            WorldNode world(Entity(), MapFormat::Valve);

            IO::TestParserStatus status;
            std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            BrushNode* pyramidLight = static_cast<BrushNode*>(nodes.at(0)->children().at(0));
            REQUIRE(pyramidLight != nullptr);
            
            Brush brush = pyramidLight->brush();

            // find the faces
            BrushFace* negXFace = nullptr;
            for (BrushFace& face : brush.faces()) {
                if (vm::get_abs_max_component_axis(face.boundary().normal) == vm::vec3::neg_x()) {
                    REQUIRE(negXFace == nullptr);
                    negXFace = &face;
                }
            }
            REQUIRE(negXFace != nullptr);

            ASSERT_EQ(vm::vec3::pos_y(), negXFace->textureXAxis());
            ASSERT_EQ(vm::vec3::neg_z(), negXFace->textureYAxis());

            // This face's texture normal is in the same direction as the face normal
            const vm::vec3 textureNormal = normalize(cross(negXFace->textureXAxis(), negXFace->textureYAxis()));
            ASSERT_GT(dot(textureNormal, vm::vec3(negXFace->boundary().normal)), 0.0);

            const vm::quat3 rot45(textureNormal, vm::to_radians(45.0));
            const vm::vec3 newXAxis(rot45 * negXFace->textureXAxis());
            const vm::vec3 newYAxis(rot45 * negXFace->textureYAxis());

            // Rotate by 45 degrees CCW
            ASSERT_FLOAT_EQ(0.0f, negXFace->attributes().rotation());
            negXFace->rotateTexture(45.0);
            ASSERT_FLOAT_EQ(45.0f, negXFace->attributes().rotation());

            ASSERT_VEC_EQ(newXAxis, negXFace->textureXAxis());
            ASSERT_VEC_EQ(newYAxis, negXFace->textureYAxis());

            kdl::vec_clear_and_delete(nodes);
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/1995
        TEST_CASE("BrushFaceTest.testCopyTexCoordSystem", "[BrushFaceTest]") {
            const std::string data("{\n"
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
            WorldNode world(Entity(), MapFormat::Valve);

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            BrushNode* pyramidLight = static_cast<BrushNode*>(nodes.at(0)->children().at(0));
            REQUIRE(pyramidLight != nullptr);
            
            Brush brush = pyramidLight->brush();

            // find the faces
            BrushFace* negYFace = nullptr;
            BrushFace* posXFace = nullptr;
            for (BrushFace& face : brush.faces()) {
                if (vm::get_abs_max_component_axis(face.boundary().normal) == vm::vec3::neg_y()) {
                    REQUIRE(negYFace == nullptr);
                    negYFace = &face;
                } else if (vm::get_abs_max_component_axis(face.boundary().normal) == vm::vec3::pos_x()) {
                    REQUIRE(posXFace == nullptr);
                    posXFace = &face;
                }
            }
            REQUIRE(negYFace != nullptr);
            REQUIRE(posXFace != nullptr);

            ASSERT_EQ(vm::vec3::pos_x(), negYFace->textureXAxis());
            ASSERT_EQ(vm::vec3::neg_z(), negYFace->textureYAxis());

            auto snapshot = negYFace->takeTexCoordSystemSnapshot();

            // copy texturing from the negYFace to posXFace using the rotation method
            posXFace->copyTexCoordSystemFromFace(*snapshot, negYFace->attributes(), negYFace->boundary(), WrapStyle::Rotation);
            ASSERT_VEC_EQ(vm::vec3(0.030303030303030123, 0.96969696969696961, -0.24242424242424243), posXFace->textureXAxis());
            ASSERT_VEC_EQ(vm::vec3(-0.0037296037296037088, -0.24242424242424243, -0.97016317016317011), posXFace->textureYAxis());

            // copy texturing from the negYFace to posXFace using the projection method
            posXFace->copyTexCoordSystemFromFace(*snapshot, negYFace->attributes(), negYFace->boundary(), WrapStyle::Projection);
            ASSERT_VEC_EQ(vm::vec3::neg_y(), posXFace->textureXAxis());
            ASSERT_VEC_EQ(vm::vec3::neg_z(), posXFace->textureYAxis());

            kdl::vec_clear_and_delete(nodes);
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/2315
        TEST_CASE("BrushFaceTest.move45DegreeFace", "[BrushFaceTest]") {
            const std::string data(R"(
// entity 0
{
"classname" "worldspawn"
// brush 0
{
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) __TB_empty [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) __TB_empty [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) __TB_empty [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) __TB_empty [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 32 -64 16 ) ( 48 -48 16 ) ( 48 -48 144 ) __TB_empty [ -0.707107 -0.707107 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Valve);

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            BrushNode* brushNode = static_cast<BrushNode*>(nodes.at(0)->children().at(0));
            EXPECT_NE(nullptr, brushNode);
            
            Brush brush = brushNode->brush();

            // find the face
            const auto angledFaceIndex = brush.findFace(vm::vec3(-0.70710678118654746, 0.70710678118654746, 0));
            REQUIRE(angledFaceIndex);
            
            CHECK(brush.moveBoundary(worldBounds, *angledFaceIndex, vm::vec3(-7.9999999999999973, 7.9999999999999973, 0), true).is_success());

            kdl::vec_clear_and_delete(nodes);
        }

        TEST_CASE("BrushFaceTest.formatConversion", "[BrushFaceTest]") {
            const vm::bbox3 worldBounds(4096.0);

            WorldNode standardWorld(Model::Entity(), MapFormat::Standard);
            WorldNode valveWorld(Model::Entity(), MapFormat::Valve);

            BrushBuilder standardBuilder(&standardWorld, worldBounds);
            BrushBuilder valveBuilder(&valveWorld, worldBounds);

            Assets::Texture texture("testTexture", 64, 64);

            const Brush startingCube = standardBuilder.createCube(128.0, "")
                    .and_then([&](Brush&& brush){
                        for (size_t i = 0; i < brush.faceCount(); ++i) {
                            BrushFace& face = brush.face(i);
                            face.setTexture(&texture);
                        }
                        return kdl::result<Brush>::success(brush);
                    }).value();

            auto testTransform = [&](const vm::mat4x4& transform){
                const Brush standardCube = startingCube.transform(worldBounds, transform, true).value();
                CHECK(dynamic_cast<const ParaxialTexCoordSystem*>(&standardCube.face(0).texCoordSystem()));

                const Brush valveCube = standardCube.convertToParallel();
                CHECK(dynamic_cast<const ParallelTexCoordSystem*>(&valveCube.face(0).texCoordSystem()));
                checkBrushUVsEqual(standardCube, valveCube);

                const Brush standardCubeRoundTrip = valveCube.convertToParaxial();
                CHECK(dynamic_cast<const ParaxialTexCoordSystem*>(&standardCubeRoundTrip.face(0).texCoordSystem()));
                checkBrushUVsEqual(standardCube, standardCubeRoundTrip);
            };

            // NOTE: intentionally include the shear/multi-axis rotations which won't work properly on Standard.
            // We're not testing texture lock, just generating interesting brushes to test
            // Standard -> Valve -> Standard round trip, so it doesn't matter if texture lock works.
            doWithTextureLockTestTransforms(true, testTransform);
        }

        TEST_CASE("BrushFaceTest.nodeReaderConversion", "[BrushFaceTest]") {
            const std::string data(R"(
// entity 0
{
"classname" "worldspawn"
"mapversion" "220"
// brush 0
{
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) __TB_empty [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) __TB_empty [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) __TB_empty [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) __TB_empty [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) __TB_empty [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) __TB_empty [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            auto* brushNode = dynamic_cast<BrushNode*>(nodes.at(0)->children().at(0));
            REQUIRE(brushNode != nullptr);

            Brush brush = brushNode->brush();
            CHECK(dynamic_cast<const ParaxialTexCoordSystem*>(&brush.face(0).texCoordSystem()) != nullptr);
        }

        TEST_CASE("BrushFaceTest.nodeReaderGroupConversion", "[BrushFaceTest]") {
            // Data comes from copying a Group in 2020.2
            const std::string data(R"(// entity 0
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Unnamed"
"_tb_id" "3"
// brush 0
{
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) __TB_empty [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) __TB_empty [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) __TB_empty [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) __TB_empty [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) __TB_empty [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) __TB_empty [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);

            auto* groupNode = dynamic_cast<GroupNode*>(nodes.at(0));
            REQUIRE(groupNode != nullptr);

            auto* brushNode = dynamic_cast<BrushNode*>(groupNode->children().at(0));
            REQUIRE(brushNode != nullptr);

            const Brush brush = brushNode->brush();
            CHECK(dynamic_cast<const ParaxialTexCoordSystem*>(&brush.face(0).texCoordSystem()) != nullptr);
        }

        TEST_CASE("BrushFaceTest.parseFaceAsNode", "[BrushFaceTest]") {
            const std::string data(R"(
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) __TB_empty [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
)");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Valve);

            IO::TestParserStatus status;

            CHECK(IO::NodeReader::read(data, world, worldBounds, status).empty());
        }
    }
}
