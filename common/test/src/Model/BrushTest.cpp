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
#include "IO/IOUtils.h"
#include "IO/DiskIO.h"
#include "IO/NodeReader.h"
#include "IO/TestParserStatus.h"
#include "Model/Brush.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushBuilder.h"
#include "Model/Entity.h"
#include "Model/Polyhedron.h"
#include "Model/WorldNode.h"

#include <kdl/intrusive_circular_list.h>
#include <kdl/result.h>
#include <kdl/vector_utils.h>

#include <vecmath/polygon.h>
#include <vecmath/ray.h>
#include <vecmath/segment.h>
#include <vecmath/vec.h>
#include <vecmath/vec_ext.h>

#include <fstream>
#include <string>
#include <vector>

#include "Catch2.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace Model {
        static bool canMoveBoundary(const Brush& brush, const vm::bbox3& worldBounds, const size_t faceIndex, const vm::vec3& delta) {
            return brush.moveBoundary(worldBounds, faceIndex, delta, false)
                .visit(kdl::overload(
                    [&](const Brush& b) {
                        return worldBounds.contains(b.bounds());
                    },
                    [](const BrushError) {
                        return false;
                    }
                ));
        }

        TEST_CASE("BrushTest.constructBrushWithFaces", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);

            // build a cube with length 16 at the origin
            const Brush brush = Brush::create(worldBounds, {
                // left
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0),
                    vm::vec3(0.0, 0.0, 1.0)),
                // right
                createParaxial(
                    vm::vec3(16.0, 0.0, 0.0),
                    vm::vec3(16.0, 0.0, 1.0),
                    vm::vec3(16.0, 1.0, 0.0)),
                // front
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(0.0, 0.0, 1.0),
                    vm::vec3(1.0, 0.0, 0.0)),
                // back
                createParaxial(
                    vm::vec3(0.0, 16.0, 0.0),
                    vm::vec3(1.0, 16.0, 0.0),
                    vm::vec3(0.0, 16.0, 1.0)),
                // top
                createParaxial(
                    vm::vec3(0.0, 0.0, 16.0),
                    vm::vec3(0.0, 1.0, 16.0),
                    vm::vec3(1.0, 0.0, 16.0)),
                // bottom
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(1.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0)),
            }).value();

            REQUIRE(brush.fullySpecified());
            REQUIRE(brush.faceCount() == 6u);
            CHECK(brush.findFace(vm::vec3::pos_x()));
            CHECK(brush.findFace(vm::vec3::neg_x()));
            CHECK(brush.findFace(vm::vec3::pos_y()));
            CHECK(brush.findFace(vm::vec3::neg_y()));
            CHECK(brush.findFace(vm::vec3::pos_z()));
            CHECK(brush.findFace(vm::vec3::neg_z()));
        }

        TEST_CASE("BrushTest.constructBrushWithRedundantFaces", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);

            CHECK(Brush::create(worldBounds, {
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(1.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0)),
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(1.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0)),
                createParaxial(
                    vm::vec3(0.0, 0.0, 0.0),
                    vm::vec3(1.0, 0.0, 0.0),
                    vm::vec3(0.0, 1.0, 0.0)),
            }).is_error());
        }


        /*
         Regex to turn a face definition into a c++ statement to add a face to a vector of faces:
         Find: \(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*\(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*\(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*[^\n]+
         Replace: faces.push_back(createParaxial(vm::vec3($1, $2, $3), vm::vec3($4, $5, $6), vm::vec3($7, $8, $9)));
         */

        TEST_CASE("BrushTest.constructWithFailingFaces", "[BrushTest]") {
            /* from rtz_q1
             {
             ( -192 704 128 ) ( -156 650 128 ) ( -156 650 160 ) mt_sr_v16 32 0 -180 1 -1
             ( -202 604 160 ) ( -164 664 128 ) ( -216 613 128 ) mt_sr_v16 0 0 -180 1 -1
             ( -156 650 128 ) ( -202 604 128 ) ( -202 604 160 ) mt_sr_v16 32 0 -180 1 -1
             ( -192 704 160 ) ( -256 640 160 ) ( -256 640 128 ) mt_sr_v16 32 0 -180 1 -1
             ( -256 640 160 ) ( -202 604 160 ) ( -202 604 128 ) mt_sr_v16 0 0 -180 1 -1
             ( -217 672 160 ) ( -161 672 160 ) ( -161 603 160 ) mt_sr_v16 0 0 -180 1 -1
             ( -161 603 128 ) ( -161 672 128 ) ( -217 672 128 ) mt_sr_v13 32 0 0 1 1
             }
             */

            const vm::bbox3 worldBounds(4096.0);

            const Brush brush = Brush::create(worldBounds, {
                createParaxial(vm::vec3(-192.0, 704.0, 128.0), vm::vec3(-156.0, 650.0, 128.0), vm::vec3(-156.0, 650.0, 160.0)),
                createParaxial(vm::vec3(-202.0, 604.0, 160.0), vm::vec3(-164.0, 664.0, 128.0), vm::vec3(-216.0, 613.0, 128.0)),
                createParaxial(vm::vec3(-156.0, 650.0, 128.0), vm::vec3(-202.0, 604.0, 128.0), vm::vec3(-202.0, 604.0, 160.0)),
                createParaxial(vm::vec3(-192.0, 704.0, 160.0), vm::vec3(-256.0, 640.0, 160.0), vm::vec3(-256.0, 640.0, 128.0)),
                createParaxial(vm::vec3(-256.0, 640.0, 160.0), vm::vec3(-202.0, 604.0, 160.0), vm::vec3(-202.0, 604.0, 128.0)),
                createParaxial(vm::vec3(-217.0, 672.0, 160.0), vm::vec3(-161.0, 672.0, 160.0), vm::vec3(-161.0, 603.0, 160.0)),
                createParaxial(vm::vec3(-161.0, 603.0, 128.0), vm::vec3(-161.0, 672.0, 128.0), vm::vec3(-217.0, 672.0, 128.0)),
            }).value();
            
            REQUIRE(brush.fullySpecified());
            CHECK(brush.faceCount() == 7u);
        }

        TEST_CASE("BrushTest.constructWithFailingFaces2", "[BrushTest]") {
            /* from ne_ruins
             {
             ( 3488 1152 1340 ) ( 3488 1248 1344 ) ( 3488 1344 1340 ) *lavaskip 0 0 0 1 1 // right face (normal 1 0 0)
             ( 3232 1344 1576 ) ( 3232 1152 1576 ) ( 3232 1152 1256 ) *lavaskip 0 0 0 1 1 // left face (normal -1 0 0)
             ( 3488 1344 1576 ) ( 3264 1344 1576 ) ( 3264 1344 1256 ) *lavaskip 0 0 0 1 1 // back face (normal 0 1 0)
             ( 3280 1152 1576 ) ( 3504 1152 1576 ) ( 3504 1152 1256 ) *lavaskip 0 0 0 1 1 // front face (normal 0 -1 0)
             ( 3488 1248 1344 ) ( 3488 1152 1340 ) ( 3232 1152 1340 ) *lavaskip 0 0 0 1 1 // top triangle facing front
             ( 3488 1248 1344 ) ( 3232 1248 1344 ) ( 3232 1344 1340 ) *lavaskip 0 0 0 1 1 // top triangle facing back
             ( 3488 1152 1340 ) ( 3360 1152 1344 ) ( 3424 1344 1342 ) *lavaskip 0 0 0 1 1 // top triangle facing right
             ( 3360 1152 1344 ) ( 3232 1152 1340 ) ( 3296 1344 1342 ) *lavaskip 0 0 0 1 1 // top triangle facing left --> clip algorithm cannot find the initial edge
             ( 3504 1344 1280 ) ( 3280 1344 1280 ) ( 3280 1152 1280 ) *lavaskip 0 0 0 1 1 // bottom face (normal 0 0 -1)
             }
             */

            const vm::bbox3 worldBounds(4096.0);

            const Brush brush = Brush::create(worldBounds, {
                createParaxial(vm::vec3(3488.0, 1152.0, 1340.0), vm::vec3(3488.0, 1248.0, 1344.0), vm::vec3(3488.0, 1344.0, 1340.0)),
                createParaxial(vm::vec3(3232.0, 1344.0, 1576.0), vm::vec3(3232.0, 1152.0, 1576.0), vm::vec3(3232.0, 1152.0, 1256.0)),
                createParaxial(vm::vec3(3488.0, 1344.0, 1576.0), vm::vec3(3264.0, 1344.0, 1576.0), vm::vec3(3264.0, 1344.0, 1256.0)),
                createParaxial(vm::vec3(3280.0, 1152.0, 1576.0), vm::vec3(3504.0, 1152.0, 1576.0), vm::vec3(3504.0, 1152.0, 1256.0)),
                createParaxial(vm::vec3(3488.0, 1248.0, 1344.0), vm::vec3(3488.0, 1152.0, 1340.0), vm::vec3(3232.0, 1152.0, 1340.0)),
                createParaxial(vm::vec3(3488.0, 1248.0, 1344.0), vm::vec3(3232.0, 1248.0, 1344.0), vm::vec3(3232.0, 1344.0, 1340.0)),
                createParaxial(vm::vec3(3488.0, 1152.0, 1340.0), vm::vec3(3360.0, 1152.0, 1344.0), vm::vec3(3424.0, 1344.0, 1342.0)),
                createParaxial(vm::vec3(3360.0, 1152.0, 1344.0), vm::vec3(3232.0, 1152.0, 1340.0), vm::vec3(3296.0, 1344.0, 1342.0)),
                createParaxial(vm::vec3(3504.0, 1344.0, 1280.0), vm::vec3(3280.0, 1344.0, 1280.0), vm::vec3(3280.0, 1152.0, 1280.0)),
            }).value();
            
            REQUIRE(brush.fullySpecified());
            CHECK(brush.faceCount() == 9u);
        }

        TEST_CASE("BrushTest.constructWithFailingFaces3", "[BrushTest]") {
            /* from ne_ruins
             {
             ( -32 -1088 896 ) ( -64 -1120 896 ) ( -64 -1120 912 ) trims2b 0 0 0 1 1  // front face
             ( -32 -832 896 ) ( -32 -1088 896 ) ( -32 -1088 912 ) trims2b 128 0 0 1 1 // right face
             ( -64 -848 912 ) ( -64 -1120 912 ) ( -64 -1120 896 ) trims2b 128 0 0 1 1 // left face
             ( -32 -896 896 ) ( -32 -912 912 ) ( -64 -912 912 ) trims2b 128 16 0 1 1  // back face
             ( -64 -1088 912 ) ( -64 -848 912 ) ( -32 -848 912 ) e7trim32 0 0 90 1 1  // top face
             ( -64 -864 896 ) ( -32 -864 896 ) ( -32 -832 896 ) trims2b 128 16 0 1 1  // bottom face
             }
             */

            const vm::bbox3 worldBounds(4096.0);

            const Brush brush = Brush::create(worldBounds, {
                createParaxial(vm::vec3(-32.0, -1088.0, 896.0), vm::vec3(-64.0, -1120.0, 896.0), vm::vec3(-64.0, -1120.0, 912.0)),
                createParaxial(vm::vec3(-32.0, -832.0, 896.0), vm::vec3(-32.0, -1088.0, 896.0), vm::vec3(-32.0, -1088.0, 912.0)),
                createParaxial(vm::vec3(-64.0, -848.0, 912.0), vm::vec3(-64.0, -1120.0, 912.0), vm::vec3(-64.0, -1120.0, 896.0)),
                createParaxial(vm::vec3(-32.0, -896.0, 896.0), vm::vec3(-32.0, -912.0, 912.0), vm::vec3(-64.0, -912.0, 912.0)),
                createParaxial(vm::vec3(-64.0, -1088.0, 912.0), vm::vec3(-64.0, -848.0, 912.0), vm::vec3(-32.0, -848.0, 912.0)),
                createParaxial(vm::vec3(-64.0, -864.0, 896.0), vm::vec3(-32.0, -864.0, 896.0), vm::vec3(-32.0, -832.0, 896.0)),
            }).value();
            
            REQUIRE(brush.fullySpecified());
            CHECK(brush.faceCount() == 6u);
        }

        TEST_CASE("BrushTest.constructWithFailingFaces4", "[BrushTest]") {
            /* from ne_ruins
             {
             ( -1268 272 2524 ) ( -1268 272 2536 ) ( -1268 288 2540 ) wall1_128 0 0 0 0.5 0.5      faces right
             ( -1280 265 2534 ) ( -1268 272 2524 ) ( -1268 288 2528 ) wall1_128 128 128 0 0.5 0.5  faces left / down, there's just a minimal difference between this and the next face
             ( -1268 288 2528 ) ( -1280 288 2540 ) ( -1280 265 2534 ) wall1_128 128 128 0 0.5 0.5  faces left / up
             ( -1268 288 2540 ) ( -1280 288 2540 ) ( -1280 288 2536 ) wall1_128 128 0 0 0.5 0.5    faces back
             ( -1268 265 2534 ) ( -1280 265 2534 ) ( -1280 288 2540 ) wall1_128 128 128 0 0.5 0.5  faces front / up
             ( -1268 265 2534 ) ( -1268 272 2524 ) ( -1280 265 2534 ) wall1_128 128 0 0 0.5 0.5    faces front / down
             }
             */

            const vm::bbox3 worldBounds(4096.0);

            const Brush brush = Brush::create(worldBounds, {
                createParaxial(vm::vec3(-1268.0, 272.0, 2524.0), vm::vec3(-1268.0, 272.0, 2536.0), vm::vec3(-1268.0, 288.0, 2540.0)),
                createParaxial(vm::vec3(-1280.0, 265.0, 2534.0), vm::vec3(-1268.0, 272.0, 2524.0), vm::vec3(-1268.0, 288.0, 2528.0)),
                createParaxial(vm::vec3(-1268.0, 288.0, 2528.0), vm::vec3(-1280.0, 288.0, 2540.0), vm::vec3(-1280.0, 265.0, 2534.0)),
                createParaxial(vm::vec3(-1268.0, 288.0, 2540.0), vm::vec3(-1280.0, 288.0, 2540.0), vm::vec3(-1280.0, 288.0, 2536.0)),
                createParaxial(vm::vec3(-1268.0, 265.0, 2534.0), vm::vec3(-1280.0, 265.0, 2534.0), vm::vec3(-1280.0, 288.0, 2540.0)),
                createParaxial(vm::vec3(-1268.0, 265.0, 2534.0), vm::vec3(-1268.0, 272.0, 2524.0), vm::vec3(-1280.0, 265.0, 2534.0)),
            }).value();
            
            REQUIRE(brush.fullySpecified());
            CHECK(brush.faceCount() == 6u);
        }

        TEST_CASE("BrushTest.constructWithFailingFaces5", "[BrushTest]") {
            /* from jam6_ericwtronyn
             Interestingly, the order in which the faces appear in the map file is okay, but when they get reordered during load, the resulting order
             leads to a crash. The order below is the reordered one.
             {
             ( 1296 896 944 ) ( 1296 1008 1056 ) ( 1280 1008 1008 ) rock18clean 0 0 0 1 1 // bottom
             ( 1296 1008 1168 ) ( 1296 1008 1056 ) ( 1296 896 944 ) rock18clean 0 64 0 1 1 // right
             ( 1280 1008 1008 ) ( 1280 1008 1168 ) ( 1280 896 1056 ) rock18clean 0 64 0 1 1 // left, fails here
             ( 1280 1008 1168 ) ( 1280 1008 1008 ) ( 1296 1008 1056 ) rock18clean 0 64 0 1 1 // back
             ( 1296 1008 1168 ) ( 1296 896 1056 ) ( 1280 896 1056 ) rock18clean 0 64 0 1 1 // top
             ( 1280 896 896 ) ( 1280 896 1056 ) ( 1296 896 1056 ) rock18clean 0 64 0 1 1 // front
             }
             */

            const vm::bbox3 worldBounds(4096.0);

            const Brush brush = Brush::create(worldBounds, {
                createParaxial(vm::vec3(1296.0, 896.0, 944.0), vm::vec3(1296.0, 1008.0, 1056.0), vm::vec3(1280.0, 1008.0, 1008.0)),
                createParaxial(vm::vec3(1296.0, 1008.0, 1168.0), vm::vec3(1296.0, 1008.0, 1056.0), vm::vec3(1296.0, 896.0, 944.0)),
                createParaxial(vm::vec3(1280.0, 1008.0, 1008.0), vm::vec3(1280.0, 1008.0, 1168.0), vm::vec3(1280.0, 896.0, 1056.0)),
                createParaxial(vm::vec3(1280.0, 1008.0, 1168.0), vm::vec3(1280.0, 1008.0, 1008.0), vm::vec3(1296.0, 1008.0, 1056.0)),
                createParaxial(vm::vec3(1296.0, 1008.0, 1168.0), vm::vec3(1296.0, 896.0, 1056.0), vm::vec3(1280.0, 896.0, 1056.0)),
                createParaxial(vm::vec3(1280.0, 896.0, 896.0), vm::vec3(1280.0, 896.0, 1056.0), vm::vec3(1296.0, 896.0, 1056.0)),
            }).value();
            
            REQUIRE(brush.fullySpecified());
            CHECK(brush.faceCount() == 6u);
        }

        TEST_CASE("BrushTest.constructWithFailingFaces6", "[BrushTest]") {
            /* from 768_negke
             {
             ( -80 -80 -3840  ) ( -80 -80 -3824  ) ( -32 -32 -3808 ) mmetal1_2b 0 0 0 1 1 // front / right
             ( -96 -32 -3840  ) ( -96 -32 -3824  ) ( -80 -80 -3824 ) mmetal1_2 0 0 0 1 1 // left
             ( -96 -32 -3824  ) ( -32 -32 -3808  ) ( -80 -80 -3824 ) mmetal1_2b 0 0 0 1 1 // top
             ( -32 -32 -3840  ) ( -32 -32 -3808  ) ( -96 -32 -3824 ) mmetal1_2b 0 0 0 1 1 // back
             ( -32 -32 -3840  ) ( -96 -32 -3840  ) ( -80 -80 -3840 ) mmetal1_2b 0 0 0 1 1 // bottom
             }
             */

            const vm::bbox3 worldBounds(4096.0);

            const Brush brush = Brush::create(worldBounds, {
                createParaxial(vm::vec3(-80.0, -80.0, -3840.0), vm::vec3(-80.0, -80.0, -3824.0), vm::vec3(-32.0, -32.0, -3808.0)),
                createParaxial(vm::vec3(-96.0, -32.0, -3840.0), vm::vec3(-96.0, -32.0, -3824.0), vm::vec3(-80.0, -80.0, -3824.0)),
                createParaxial(vm::vec3(-96.0, -32.0, -3824.0), vm::vec3(-32.0, -32.0, -3808.0), vm::vec3(-80.0, -80.0, -3824.0)),
                createParaxial(vm::vec3(-32.0, -32.0, -3840.0), vm::vec3(-32.0, -32.0, -3808.0), vm::vec3(-96.0, -32.0, -3824.0)),
                createParaxial(vm::vec3(-32.0, -32.0, -3840.0), vm::vec3(-96.0, -32.0, -3840.0), vm::vec3(-80.0, -80.0, -3840.0)),
            }).value();
            
            REQUIRE(brush.fullySpecified());
            CHECK(brush.faceCount() == 5u);
        }

        TEST_CASE("BrushTest.constructBrushWithManySides", "[BrushTest]") {
            /*
             See https://github.com/TrenchBroom/TrenchBroom/issues/1153
             The faces have been reordered according to Model::BrushFace::sortFaces and all non-interesting faces
             have been removed from the brush.

             {
             ( 624 688 -456 ) ( 656 760 -480 ) ( 624 680 -480 ) face7 8 0 180 1 -1
             ( 536 792 -480 ) ( 536 792 -432 ) ( 488 720 -480 ) face12 48 0 180 1 -1
             ( 568 656 -464 ) ( 568 648 -480 ) ( 520 672 -456 ) face14 -32 0 -180 1 -1
             ( 520 672 -456 ) ( 520 664 -480 ) ( 488 720 -452 ) face15 8 0 180 1 -1
             ( 560 728 -440 ) ( 488 720 -452 ) ( 536 792 -432 ) face17 -32 -8 -180 1 1
             ( 568 656 -464 ) ( 520 672 -456 ) ( 624 688 -456 ) face19 -32 -8 -180 1 1
             ( 560 728 -440 ) ( 624 688 -456 ) ( 520 672 -456 ) face20 -32 -8 -180 1 1 // assert
             ( 600 840 -480 ) ( 536 792 -480 ) ( 636 812 -480 ) face22 -32 -8 -180 1 1
             }
             */

            const vm::bbox3 worldBounds(4096.0);

            const Brush brush = Brush::create(worldBounds, {
                createParaxial(vm::vec3(624.0, 688.0, -456.0), vm::vec3(656.0, 760.0, -480.0), vm::vec3(624.0, 680.0, -480.0), "face7"),
                createParaxial(vm::vec3(536.0, 792.0, -480.0), vm::vec3(536.0, 792.0, -432.0), vm::vec3(488.0, 720.0, -480.0), "face12"),
                createParaxial(vm::vec3(568.0, 656.0, -464.0), vm::vec3(568.0, 648.0, -480.0), vm::vec3(520.0, 672.0, -456.0), "face14"),
                createParaxial(vm::vec3(520.0, 672.0, -456.0), vm::vec3(520.0, 664.0, -480.0), vm::vec3(488.0, 720.0, -452.0), "face15"),
                createParaxial(vm::vec3(560.0, 728.0, -440.0), vm::vec3(488.0, 720.0, -452.0), vm::vec3(536.0, 792.0, -432.0), "face17"),
                createParaxial(vm::vec3(568.0, 656.0, -464.0), vm::vec3(520.0, 672.0, -456.0), vm::vec3(624.0, 688.0, -456.0), "face19"),
                createParaxial(vm::vec3(560.0, 728.0, -440.0), vm::vec3(624.0, 688.0, -456.0), vm::vec3(520.0, 672.0, -456.0), "face20"),
                createParaxial(vm::vec3(600.0, 840.0, -480.0), vm::vec3(536.0, 792.0, -480.0), vm::vec3(636.0, 812.0, -480.0), "face22"),
            }).value();
            
            REQUIRE(brush.fullySpecified());
            CHECK(brush.faceCount() == 8u);
        }

        TEST_CASE("BrushTest.constructBrushAfterRotateFail", "[BrushTest]") {
            /*
             See https://github.com/TrenchBroom/TrenchBroom/issues/1173

             This is the brush after rotation. Rebuilding the geometry should assert.

             {
             (-729.68857812925364 -128 2061.2927432882448) (-910.70791411301013 128 2242.3120792720015) (-820.19824612113155 -128 1970.7830752963655) 0 0 0 5 5
             (-639.17891013737574 -640 1970.7830752963669) (-729.68857812925364 -128 2061.2927432882448) (-729.68857812925364 -640 1880.2734073044885) 0 0 0 5 5
             (-639.17891013737574 -1024 1970.7830752963669) (-820.19824612113177 -640 2151.8024112801227) (-639.17891013737574 -640 1970.7830752963669) 0 0 0 5 5
             (-639.17891013737574 -1024 1970.7830752963669) (-639.17891013737574 -640 1970.7830752963669) (-729.68857812925364 -1024 1880.2734073044885) 0 0 0 5 5
             (-1001.2175821048878 -128 2151.8024112801222) (-910.70791411301013 -128 2242.3120792720015) (-910.70791411300991 -640 2061.2927432882443) 0 0 0 5 5
             (-639.17891013737574 -1024 1970.7830752963669) (-729.68857812925364 -1024 1880.2734073044885) (-820.19824612113177 -640 2151.8024112801227) 0 0 0 5 5
             (-1001.2175821048878 -128 2151.8024112801222) (-1001.2175821048878 128 2151.8024112801222) (-910.70791411301013 -128 2242.3120792720015) 0 0 0 5 5 // long upper face
             (-729.68857812925364 -1024 1880.2734073044885) (-729.68857812925364 -640 1880.2734073044885) (-910.70791411300991 -640 2061.2927432882443) 0 0 0 5 5 // lower face
             }
             */

            const vm::bbox3 worldBounds(4096.0);
            const Brush brush = Brush::create(worldBounds, {
                createParaxial(vm::vec3(-729.68857812925364, -128, 2061.2927432882448), vm::vec3(-910.70791411301013, 128, 2242.3120792720015), vm::vec3(-820.19824612113155, -128, 1970.7830752963655)),
                createParaxial(vm::vec3(-639.17891013737574, -640, 1970.7830752963669), vm::vec3(-729.68857812925364, -128, 2061.2927432882448), vm::vec3(-729.68857812925364, -640, 1880.2734073044885)),
                createParaxial(vm::vec3(-639.17891013737574, -1024, 1970.7830752963669), vm::vec3(-820.19824612113177, -640, 2151.8024112801227), vm::vec3(-639.17891013737574, -640, 1970.7830752963669)),
                createParaxial(vm::vec3(-639.17891013737574, -1024, 1970.7830752963669), vm::vec3(-639.17891013737574, -640, 1970.7830752963669), vm::vec3(-729.68857812925364, -1024, 1880.2734073044885)),
                createParaxial(vm::vec3(-1001.2175821048878, -128, 2151.8024112801222), vm::vec3(-910.70791411301013, -128, 2242.3120792720015), vm::vec3(-910.70791411300991, -640, 2061.2927432882443)),
                createParaxial(vm::vec3(-639.17891013737574, -1024, 1970.7830752963669), vm::vec3(-729.68857812925364, -1024, 1880.2734073044885), vm::vec3(-820.19824612113177, -640, 2151.8024112801227)), // assertion failure here
                createParaxial(vm::vec3(-1001.2175821048878, -128, 2151.8024112801222), vm::vec3(-1001.2175821048878, 128, 2151.8024112801222), vm::vec3(-910.70791411301013, -128, 2242.3120792720015)),
                createParaxial(vm::vec3(-729.68857812925364, -1024, 1880.2734073044885), vm::vec3(-729.68857812925364, -640, 1880.2734073044885), vm::vec3(-910.70791411300991, -640, 2061.2927432882443)),
            }).value();
            
            CHECK(brush.fullySpecified());
        }

        TEST_CASE("BrushTest.clip", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);

            const auto left = createParaxial(
                vm::vec3(0.0, 0.0, 0.0),
                vm::vec3(0.0, 1.0, 0.0),
                vm::vec3(0.0, 0.0, 1.0));
            const auto right = createParaxial(
                vm::vec3(16.0, 0.0, 0.0),
                vm::vec3(16.0, 0.0, 1.0),
                vm::vec3(16.0, 1.0, 0.0));
            const auto front = createParaxial(
                vm::vec3(0.0, 0.0, 0.0),
                vm::vec3(0.0, 0.0, 1.0),
                vm::vec3(1.0, 0.0, 0.0));
            const auto back = createParaxial(
                vm::vec3(0.0, 16.0, 0.0),
                vm::vec3(1.0, 16.0, 0.0),
                vm::vec3(0.0, 16.0, 1.0));
            const auto top = createParaxial(
                vm::vec3(0.0, 0.0, 16.0),
                vm::vec3(0.0, 1.0, 16.0),
                vm::vec3(1.0, 0.0, 16.0));
            const auto bottom = createParaxial(
                vm::vec3(0.0, 0.0, 0.0),
                vm::vec3(1.0, 0.0, 0.0),
                vm::vec3(0.0, 1.0, 0.0));

            // build a cube with length 16 at the origin
            Brush brush = Brush::create(worldBounds, { left, right, front, back, top, bottom }).value();

            BrushFace clip = createParaxial(
                vm::vec3(8.0, 0.0, 0.0),
                vm::vec3(8.0, 0.0, 1.0),
                vm::vec3(8.0, 1.0, 0.0));
            brush = brush.clip(worldBounds, clip).value();

            CHECK(brush.faceCount() == 6u);
            CHECK(brush.findFace(left.boundary()));
            CHECK(brush.findFace(clip.boundary()));
            CHECK(brush.findFace(front.boundary()));
            CHECK(brush.findFace(back.boundary()));
            CHECK(brush.findFace(top.boundary()));
            CHECK(brush.findFace(bottom.boundary()));
            CHECK_FALSE(brush.findFace(right.boundary()));
        }

        TEST_CASE("BrushTest.moveBoundary", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            Brush brush = Brush::create(worldBounds, {
                createParaxial(vm::vec3(0.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0), vm::vec3(1.0, 0.0, 1.0)), // left
                createParaxial(vm::vec3(16.0, 0.0, 0.0),  vm::vec3(15.0, 0.0, 1.0), vm::vec3(16.0, 1.0, 0.0)), // right
                createParaxial(vm::vec3(0.0, 0.0, 0.0),  vm::vec3(0.0, 0.0, 1.0), vm::vec3(1.0, 0.0, 0.0)), // front
                createParaxial(vm::vec3(0.0, 16.0, 0.0), vm::vec3(1.0, 16.0, 0.0), vm::vec3(0.0, 16.0, 1.0)), // back
                createParaxial(vm::vec3(0.0, 0.0, 6.0),vm::vec3(0.0, 1.0, 6.0), vm::vec3(1.0, 0.0, 6.0)), // top
                createParaxial(vm::vec3(0.0, 0.0, 0.0),   vm::vec3(1.0, 0.0, 0.0), vm::vec3(0.0, 1.0, 0.0)), // bottom
            }).value();

            REQUIRE(brush.faceCount() == 6u);

            const auto topFaceIndex = brush.findFace(vm::vec3::pos_z());
            REQUIRE(topFaceIndex);

            CHECK(!canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, +16.0)));
            CHECK(!canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, -16.0)));
            CHECK(!canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, +2.0)));
            CHECK(!canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, -6.0)));
            CHECK(canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, +1.0)));
            CHECK(canMoveBoundary(brush, worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, -5.0)));

            brush = brush.moveBoundary(worldBounds, *topFaceIndex, vm::vec3(0.0, 0.0, 1.0), false).value();
            CHECK(worldBounds.contains(brush.bounds()));
            
            CHECK(brush.faces().size() == 6u);
            CHECK(brush.bounds().size().z() == 7.0);
        }

        TEST_CASE("BrushTest.resizePastWorldBounds", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush brush1 = builder.createBrush(std::vector<vm::vec3>{vm::vec3(64, -64, 16), vm::vec3(64, 64, 16), vm::vec3(64, -64, -16), vm::vec3(64, 64, -16), vm::vec3(48, 64, 16), vm::vec3(48, 64, -16)}, "texture").value();

            const auto rightFaceIndex = brush1.findFace(vm::vec3::pos_x());
            REQUIRE(rightFaceIndex);

            CHECK(canMoveBoundary(brush1, worldBounds, *rightFaceIndex, vm::vec3(16, 0, 0)));
            CHECK(!canMoveBoundary(brush1, worldBounds, *rightFaceIndex, vm::vec3(8000, 0, 0)));
        }

        TEST_CASE("BrushTest.expand", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush brush1 = builder.createCuboid(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), "texture").value();
            const auto expandResult = brush1.expand(worldBounds, 6, true);
            CHECK(expandResult.is_success());
            brush1 = expandResult.value();

            const vm::bbox3 expandedBBox(vm::vec3(-70, -70, -70), vm::vec3(70, 70, 70));
            
            EXPECT_EQ(expandedBBox, brush1.bounds());
            EXPECT_COLLECTIONS_EQUIVALENT(expandedBBox.vertices(), brush1.vertexPositions());
        }

        TEST_CASE("BrushTest.contract", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush brush1 = builder.createCuboid(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), "texture").value();
            const auto expandResult = brush1.expand(worldBounds, -32, true);
            CHECK(expandResult.is_success());
            brush1 = expandResult.value();

            const vm::bbox3 expandedBBox(vm::vec3(-32, -32, -32), vm::vec3(32, 32, 32));

            EXPECT_EQ(expandedBBox, brush1.bounds());
            EXPECT_COLLECTIONS_EQUIVALENT(expandedBBox.vertices(), brush1.vertexPositions());
        }

        TEST_CASE("BrushTest.contractToZero", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Brush brush1 = builder.createCuboid(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), "texture").value();
            CHECK(brush1.expand(worldBounds, -64, true).is_error());
        }

        TEST_CASE("BrushTest.moveVertex", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom").value();

            const vm::vec3 p1(-32.0, -32.0, -32.0);
            const vm::vec3 p2(-32.0, -32.0, +32.0);
            const vm::vec3 p3(-32.0, +32.0, -32.0);
            const vm::vec3 p4(-32.0, +32.0, +32.0);
            const vm::vec3 p5(+32.0, -32.0, -32.0);
            const vm::vec3 p6(+32.0, -32.0, +32.0);
            const vm::vec3 p7(+32.0, +32.0, -32.0);
            const vm::vec3 p8(+32.0, +32.0, +32.0);
            const vm::vec3 p9(+16.0, +16.0, +32.0);

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            brush = brush.moveVertices(worldBounds, oldVertexPositions, p9 - p8).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + (p9 - p8));
            
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p9, newVertexPositions[0]);

            assertTexture("left", brush, p1, p2, p4, p3);
            assertTexture("right", brush, p5, p7, p6);
            assertTexture("right", brush, p6, p7, p9);
            assertTexture("front", brush, p1, p5, p6, p2);
            assertTexture("back", brush, p3, p4, p7);
            assertTexture("back", brush, p4, p9, p7);
            assertTexture("top", brush, p2, p6, p9, p4);
            assertTexture("bottom", brush, p1, p3, p7, p5);

            oldVertexPositions = std::move(newVertexPositions);
            brush = brush.moveVertices(worldBounds, oldVertexPositions, p8 - p9).value();
            newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + (p8 - p9));
            
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p8, newVertexPositions[0]);

            assertTexture("left", brush, p1, p2, p4, p3);
            assertTexture("right", brush, p5, p7, p8, p6);
            assertTexture("front", brush, p1, p5, p6, p2);
            assertTexture("back", brush, p3, p4, p8, p7);
            assertTexture("top", brush, p2, p6, p8, p4);
            assertTexture("bottom", brush, p1, p3, p7, p5);
        }

        TEST_CASE("BrushTest.moveTetrahedronVertexToOpposideSide", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const vm::vec3 top(0.0, 0.0, +16.0);

            std::vector<vm::vec3> points;
            points.push_back(vm::vec3(-16.0, -16.0, 0.0));
            points.push_back(vm::vec3(+16.0, -16.0, 0.0));
            points.push_back(vm::vec3(0.0, +16.0, 0.0));
            points.push_back(top);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(points, "some_texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({top});
            auto delta = vm::vec3(0.0, 0.0, -32.0);
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(vm::vec3(0.0, 0.0, -16.0), newVertexPositions[0]);
            ASSERT_TRUE(brush.fullySpecified());
        }

        TEST_CASE("BrushTest.moveVertexInwardWithoutMerges", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(+56.0, +56.0, +56.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            auto delta = p9 - p8;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p9, newVertexPositions[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(15u, brush.edgeCount());
            ASSERT_EQ(9u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));


            ASSERT_TRUE(brush.hasFace({p1, p5, p6, p2}));
            ASSERT_TRUE(brush.hasFace({p1, p2, p4, p3}));
            ASSERT_TRUE(brush.hasFace({p1, p3, p7, p5}));
            ASSERT_TRUE(brush.hasFace({p2, p6, p4}));
            ASSERT_TRUE(brush.hasFace({p5, p7, p6}));
            ASSERT_TRUE(brush.hasFace({p3, p4, p7}));
            ASSERT_TRUE(brush.hasFace({p9, p6, p7}));
            ASSERT_TRUE(brush.hasFace({p9, p4, p6}));
            ASSERT_TRUE(brush.hasFace({p9, p7, p4}));
        }

        TEST_CASE("BrushTest.moveVertexOutwardWithoutMerges", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(+72.0, +72.0, +72.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            auto delta = p9 - p8;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p9, newVertexPositions[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(15u, brush.edgeCount());
            ASSERT_EQ(9u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p6, p9})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p9, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p9})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p9, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p9, p6})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p9})));
        }

        TEST_CASE("BrushTest.moveVertexWithOneOuterNeighbourMerge", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+56.0, +56.0, +56.0);
            const vm::vec3d p9(+56.0, +56.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            auto delta = p9 - p8;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p9, newVertexPositions[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(14u, brush.edgeCount());
            ASSERT_EQ(8u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p6, p9, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p6})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p9, p6, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p9, p7, p4})));
        }

        TEST_CASE("BrushTest.moveVertexWithTwoOuterNeighbourMerges", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+56.0, +56.0, +56.0);
            const vm::vec3d p9(+64.0, +64.0, +56.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            auto delta = p9 - p8;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p9, newVertexPositions[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(13u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p9, p6})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p9, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p6, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p9, p4, p6})));
        }

        TEST_CASE("BrushTest.moveVertexWithAllOuterNeighbourMerges", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+56.0, +56.0, +56.0);
            const vm::vec3d p9(+64.0, +64.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            auto delta = p9 - p8;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p9, newVertexPositions[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(6u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p6, p9, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p9, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p9, p6})));
        }

        TEST_CASE("BrushTest.moveVertexWithAllInnerNeighbourMerge", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(0.0, 0.0, 0.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            auto delta = p9 - p8;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(0u, newVertexPositions.size());

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p7)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p6, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p6})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p4, p6, p7})));
        }

        TEST_CASE("BrushTest.moveVertexUpThroughPlane", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +56.0);
            const vm::vec3d p9(+64.0, +64.0, +72.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            auto delta = p9 - p8;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p9, newVertexPositions[0]);

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(13u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p9, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p9, p6})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p9, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p6, p9})));
        }

        TEST_CASE("BrushTest.moveVertexOntoEdge", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, 0.0);
            const vm::vec3d p9(0.0, 0.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            auto delta = p9 - p8;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(0u, newVertexPositions.size());

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p7)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p6, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p6})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p4, p6, p7})));
        }

        TEST_CASE("BrushTest.moveVertexOntoIncidentVertex", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            auto delta = p7 - p8;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p7, newVertexPositions[0]);

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p7)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p6, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p6})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p4, p6, p7})));
        }

        TEST_CASE("BrushTest.moveVertexOntoIncidentVertexInOppositeDirection", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p7});
            auto delta = p8 - p7;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p8, newVertexPositions[0]);

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p8));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p8)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p8)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p8)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p8)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p6, p8, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p8})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p8, p6})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p8, p5})));
        }

        TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithoutDeletingVertex", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(+80.0, +64.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p6});
            auto delta = p9 - p6;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p9, newVertexPositions[0]);

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p9, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p5, p9})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p9, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p9})));
        }

        TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithoutDeletingVertex2", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(+80.0, -64.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p8});
            auto delta = p9 - p8;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p9, newVertexPositions[0]);

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(7u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p9));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p9)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p9, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p9, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p4, p9, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p9})));
        }

        TEST_CASE("BrushTest.moveVertexAndMergeColinearEdgesWithDeletingVertex", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, -64.0);
            const vm::vec3d p2(-64.0, -64.0, +64.0);
            const vm::vec3d p3(-64.0, +64.0, -64.0);
            const vm::vec3d p4(-64.0, +64.0, +64.0);
            const vm::vec3d p5(+64.0, -64.0, -64.0);
            const vm::vec3d p6(+64.0, -64.0, +64.0);
            const vm::vec3d p7(+64.0, +64.0, -64.0);
            const vm::vec3d p8(+64.0, +64.0, +64.0);
            const vm::vec3d p9(+80.0, 0.0, +64.0);
            const vm::vec3d p10(+64.0, 0.0, +64.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);
            oldPositions.push_back(p5);
            oldPositions.push_back(p6);
            oldPositions.push_back(p7);
            oldPositions.push_back(p8);
            oldPositions.push_back(p9);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            auto oldVertexPositions = std::vector<vm::vec3>({p9});
            auto delta = p10 - p9;
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(0u, newVertexPositions.size());

            ASSERT_EQ(8u, brush.vertexCount());
            ASSERT_EQ(12u, brush.edgeCount());
            ASSERT_EQ(6u, brush.faceCount());

            ASSERT_TRUE(brush.hasVertex(p1));
            ASSERT_TRUE(brush.hasVertex(p2));
            ASSERT_TRUE(brush.hasVertex(p3));
            ASSERT_TRUE(brush.hasVertex(p4));
            ASSERT_TRUE(brush.hasVertex(p5));
            ASSERT_TRUE(brush.hasVertex(p6));
            ASSERT_TRUE(brush.hasVertex(p7));
            ASSERT_TRUE(brush.hasVertex(p8));

            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p4, p8)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p6, p8)));
            ASSERT_TRUE(brush.hasEdge(vm::segment3d(p7, p8)));

            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p2, p4, p3})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p3, p7, p5})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p1, p5, p6, p2})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p2, p6, p8, p4})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p3, p4, p8, p7})));
            ASSERT_TRUE(brush.hasFace(vm::polygon3d({p5, p7, p8, p6})));
        }

        TEST_CASE("BrushTest.moveVerticesPastWorldBounds", "[BrushTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Model::Brush brush = builder.createCube(128.0, "texture").value();

            std::vector<vm::vec3> allVertexPositions;
            for (const auto* vertex : brush.vertices()) {
                allVertexPositions.push_back(vertex->position());
            }

            EXPECT_TRUE(brush.canMoveVertices(worldBounds, allVertexPositions, vm::vec3(16, 0, 0)));
            EXPECT_FALSE(brush.canMoveVertices(worldBounds, allVertexPositions, vm::vec3(8192, 0, 0)));
        }

        static void assertCanMoveVertices(const Brush& brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            ASSERT_TRUE(brush.canMoveVertices(worldBounds, vertexPositions, delta));

            const Brush newBrush = brush.moveVertices(worldBounds, vertexPositions, delta).value();

            auto movedVertexPositions = newBrush.findClosestVertexPositions(vertexPositions + delta);
            movedVertexPositions = kdl::vec_sort_and_remove_duplicates(std::move(movedVertexPositions));

            auto expectedVertexPositions = vertexPositions + delta;
            expectedVertexPositions = kdl::vec_sort_and_remove_duplicates(std::move(expectedVertexPositions));

            ASSERT_EQ(expectedVertexPositions, movedVertexPositions);
        }

        // "Move point" tests

        static void assertMovingVerticesDeletes(const Brush& brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            ASSERT_TRUE(brush.canMoveVertices(worldBounds, vertexPositions, delta));

            const Brush newBrush = brush.moveVertices(worldBounds, vertexPositions, delta).value();
            const std::vector<vm::vec3> movedVertexPositions = newBrush.findClosestVertexPositions(vertexPositions + delta);
            ASSERT_TRUE(movedVertexPositions.empty());
        }

        static void assertCanNotMoveVertices(const Brush& brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);
            ASSERT_FALSE(brush.canMoveVertices(worldBounds, vertexPositions, delta));
        }

        static void assertCanMoveVertex(const Brush& brush, const vm::vec3 vertexPosition, const vm::vec3 delta) {
            assertCanMoveVertices(brush, std::vector<vm::vec3>{vertexPosition}, delta);
        }

        static void assertMovingVertexDeletes(const Brush& brush, const vm::vec3 vertexPosition, const vm::vec3 delta) {
            assertMovingVerticesDeletes(brush, std::vector<vm::vec3>{vertexPosition}, delta);
        }

        static void assertCanNotMoveVertex(const Brush& brush, const vm::vec3 vertexPosition, const vm::vec3 delta) {
            assertCanNotMoveVertices(brush, std::vector<vm::vec3>{vertexPosition}, delta);
        }

        // NOTE: Different than movePolygonRemainingPoint, because in this case we allow
        // point moves that flip the normal of the remaining polygon
        TEST_CASE("BrushTest.movePointRemainingPolygon", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const vm::vec3 peakPosition(0.0, 0.0, +64.0);
            const std::vector<vm::vec3> baseQuadVertexPositions{
                    vm::vec3(-64.0, -64.0, -64.0), // base quad
                    vm::vec3(-64.0, +64.0, -64.0),
                    vm::vec3(+64.0, +64.0, -64.0),
                    vm::vec3(+64.0, -64.0, -64.0)
            };
            const std::vector<vm::vec3> vertexPositions = kdl::vec_concat(std::vector<vm::vec3>{ peakPosition },
                baseQuadVertexPositions);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName).value();

            assertCanMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -127.0));
            assertCanNotMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -128.0)); // Onto the base quad plane
            assertCanMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -129.0)); // Through the other side of the base quad

            // More detailed testing of the last assertion
            {
                std::vector<vm::vec3> temp(baseQuadVertexPositions);
                std::reverse(temp.begin(), temp.end());
                const std::vector<vm::vec3> flippedBaseQuadVertexPositions(temp);

                const vm::vec3 delta(0.0, 0.0, -129.0);

                ASSERT_EQ(5u, brush.faceCount());
                ASSERT_TRUE(brush.findFace(vm::polygon3(baseQuadVertexPositions)));
                ASSERT_FALSE(brush.findFace(vm::polygon3(flippedBaseQuadVertexPositions)));
                ASSERT_TRUE(brush.findFace(vm::vec3::neg_z()));
                ASSERT_FALSE(brush.findFace(vm::vec3::pos_z()));

                const auto oldVertexPositions = std::vector<vm::vec3>({peakPosition});
                ASSERT_TRUE(brush.canMoveVertices(worldBounds, oldVertexPositions, delta));
                Brush newBrush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
                const auto newVertexPositions = newBrush.findClosestVertexPositions(oldVertexPositions + delta);
                ASSERT_EQ(oldVertexPositions + delta, newVertexPositions);

                ASSERT_EQ(5u, newBrush.faceCount());
                ASSERT_FALSE(newBrush.findFace(vm::polygon3(baseQuadVertexPositions)));
                ASSERT_TRUE(newBrush.findFace(vm::polygon3(flippedBaseQuadVertexPositions)));
                ASSERT_FALSE(newBrush.findFace(vm::vec3::neg_z()));
                ASSERT_TRUE(newBrush.findFace(vm::vec3::pos_z()));
            }

            assertCanMoveVertex(brush, peakPosition, vm::vec3(256.0, 0.0, -127.0));
            assertCanNotMoveVertex(brush, peakPosition, vm::vec3(256.0, 0.0, -128.0)); // Onto the base quad plane
            assertCanMoveVertex(brush, peakPosition, vm::vec3(256.0, 0.0, -129.0)); // Flips the normal of the base quad, without moving through it
        }

        TEST_CASE("BrushTest.movePointRemainingPolyhedron", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const vm::vec3 peakPosition(0.0, 0.0, 128.0);
            const std::vector<vm::vec3> vertexPositions {
                    vm::vec3(-64.0, -64.0, 0.0), // base quad
                    vm::vec3(-64.0, +64.0, 0.0),
                    vm::vec3(+64.0, +64.0, 0.0),
                    vm::vec3(+64.0, -64.0, 0.0),
                    vm::vec3(-64.0, -64.0, 64.0), // upper quad
                    vm::vec3(-64.0, +64.0, 64.0),
                    vm::vec3(+64.0, +64.0, 64.0),
                    vm::vec3(+64.0, -64.0, 64.0),
                    peakPosition
            };

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName).value();

            assertMovingVertexDeletes(brush, peakPosition, vm::vec3(0.0, 0.0, -65.0)); // Move inside the remaining cuboid
            assertCanMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -63.0)); // Slightly above the top of the cuboid is OK
            assertCanNotMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -129.0)); // Through and out the other side is disallowed
        }

        // add vertex tests

        // TODO: add tests for Brush::addVertex

        // remove vertex tests

        TEST_CASE("BrushTest.removeSingleVertex", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(64.0, "asdf").value();


            brush = brush.removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, +32.0, +32.0))).value();

            ASSERT_EQ(7u, brush.vertexCount());
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            brush = brush.removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, +32.0, -32.0))).value();

            ASSERT_EQ(6u, brush.vertexCount());
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            brush = brush.removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, -32.0, +32.0))).value();

            ASSERT_EQ(5u, brush.vertexCount());
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            brush = brush.removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, -32.0, -32.0))).value();

            ASSERT_EQ(4u, brush.vertexCount());
            ASSERT_FALSE(brush.hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush.hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush.hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            ASSERT_FALSE(brush.canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, -32.0, +32.0))));
            ASSERT_FALSE(brush.canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, +32.0, -32.0))));
            ASSERT_FALSE(brush.canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, +32.0, +32.0))));
            ASSERT_FALSE(brush.canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, -32.0, -32.0))));
        }


        TEST_CASE("BrushTest.removeMultipleVertices", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);
            BrushBuilder builder(&world, worldBounds);

            std::vector<vm::vec3> vertices;
            vertices.push_back(vm::vec3(-32.0, -32.0, -32.0));
            vertices.push_back(vm::vec3(-32.0, -32.0, +32.0));
            vertices.push_back(vm::vec3(-32.0, +32.0, -32.0));
            vertices.push_back(vm::vec3(-32.0, +32.0, +32.0));
            vertices.push_back(vm::vec3(+32.0, -32.0, -32.0));
            vertices.push_back(vm::vec3(+32.0, -32.0, +32.0));
            vertices.push_back(vm::vec3(+32.0, +32.0, -32.0));
            vertices.push_back(vm::vec3(+32.0, +32.0, +32.0));

            for (size_t i = 0; i < 6; ++i) {
                for (size_t j = i + 1; j < 7; ++j) {
                    for (size_t k = j + 1; k < 8; ++k) {
                        std::vector<vm::vec3> toRemove;
                        toRemove.push_back(vertices[i]);
                        toRemove.push_back(vertices[j]);
                        toRemove.push_back(vertices[k]);

                        Brush brush = builder.createBrush(vertices, "asdf").value();
                        ASSERT_TRUE(brush.canRemoveVertices(worldBounds, toRemove));
                        brush = brush.removeVertices(worldBounds, toRemove).value();

                        for (size_t l = 0; l < 8; ++l) {
                            if (l != i && l != j && l != k) {
                                ASSERT_TRUE(brush.hasVertex(vertices[l]));
                            }
                        }
                    }
                }
            }
        }

        // snap vertices tests

        static void assertCannotSnapTo(const std::string& data, const FloatType gridSize) {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            const std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            EXPECT_EQ(1u, nodes.size());

            Brush brush = static_cast<BrushNode*>(nodes.front())->brush();
            ASSERT_FALSE(brush.canSnapVertices(worldBounds, gridSize));

            kdl::col_delete_all(nodes);
        }

        static void assertCannotSnap(const std::string& data) {
            assertCannotSnapTo(data, 1.0);
        }

        static void assertSnapTo(const std::string& data, const FloatType gridSize) {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            const std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            EXPECT_EQ(1u, nodes.size());

            const Brush& brush = static_cast<BrushNode*>(nodes.front())->brush();
            ASSERT_TRUE(brush.canSnapVertices(worldBounds, gridSize));

            Brush newBrush = brush.snapVertices(worldBounds, gridSize).value();
            ASSERT_TRUE(newBrush.fullySpecified());

            // Ensure they were actually snapped
            {
                for (const Model::BrushVertex* vertex : newBrush.vertices()) {
                    const vm::vec3& pos = vertex->position();
                    ASSERT_TRUE(vm::is_integral(pos, 0.001));
                }
            }

            kdl::col_delete_all(nodes);
        }

        static void assertSnapToInteger(const std::string& data) {
            assertSnapTo(data, 1.0);
        }

        // TODO: add tests for Brush::snapVertices (there are some issue tests below)

        // "Move edge" tests

        TEST_CASE("BrushTest.moveEdge", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom").value();

            const vm::vec3 p1(-32.0, -32.0, -32.0);
            const vm::vec3 p2(-32.0, -32.0, +32.0);
            const vm::vec3 p3(-32.0, +32.0, -32.0);
            const vm::vec3 p4(-32.0, +32.0, +32.0);
            const vm::vec3 p5(+32.0, -32.0, -32.0);
            const vm::vec3 p6(+32.0, -32.0, +32.0);
            const vm::vec3 p7(+32.0, +32.0, -32.0);
            const vm::vec3 p8(+32.0, +32.0, +32.0);
            const vm::vec3 p1_2(-32.0, -32.0, -16.0);
            const vm::vec3 p2_2(-32.0, -32.0, +48.0);

            assertTexture("left", brush, p1, p2, p4, p3);
            assertTexture("right", brush, p5, p7, p8, p6);
            assertTexture("front", brush, p1, p5, p6, p2);
            assertTexture("back", brush, p3, p4, p8, p7);
            assertTexture("top", brush, p2, p6, p8, p4);
            assertTexture("bottom", brush, p1, p3, p7, p5);

            const auto originalEdge = vm::segment(p1, p2);
            auto oldEdgePositions = std::vector<vm::segment3>({originalEdge});
            auto delta = p1_2 - p1;
            brush = brush.moveEdges(worldBounds, oldEdgePositions, delta).value();
            auto newEdgePositions = brush.findClosestEdgePositions(kdl::vec_transform(oldEdgePositions, [&](const auto& s) {
                return s.translate(delta);
            }));

            ASSERT_EQ(1u, newEdgePositions.size());
            ASSERT_EQ(vm::segment3(p1_2, p2_2), newEdgePositions[0]);

            assertTexture("left", brush, p1_2, p2_2, p4, p3);
            assertTexture("right", brush, p5, p7, p8, p6);
            assertTexture("front", brush, p1_2, p5, p6, p2_2);
            assertTexture("back", brush, p3, p4, p8, p7);
            assertTexture("top", brush, p2_2, p6, p8);
            assertTexture("top", brush, p2_2, p8, p4);
            assertTexture("bottom", brush, p1_2, p3, p5);
            assertTexture("bottom", brush, p3, p7, p5);

            ASSERT_TRUE(brush.canMoveEdges(worldBounds, newEdgePositions, p1 - p1_2));

            oldEdgePositions = std::move(newEdgePositions);
            delta = p1 - p1_2;
            brush = brush.moveEdges(worldBounds, oldEdgePositions, delta).value();
            newEdgePositions = brush.findClosestEdgePositions(kdl::vec_transform(oldEdgePositions, [&](const auto& s) {
                return s.translate(delta);
            }));

            ASSERT_EQ(1u, newEdgePositions.size());
            ASSERT_EQ(originalEdge, newEdgePositions[0]);

            assertTexture("left", brush, p1, p2, p4, p3);
            assertTexture("right", brush, p5, p7, p8, p6);
            assertTexture("front", brush, p1, p5, p6, p2);
            assertTexture("back", brush, p3, p4, p8, p7);
            assertTexture("top", brush, p2, p6, p8, p4);
            assertTexture("bottom", brush, p1, p3, p7, p5);
        }

        static void assertCanMoveEdges(const Brush& brush, const std::vector<vm::segment3> edges, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            std::vector<vm::segment3> expectedMovedEdges;
            for (const vm::segment3& edge : edges) {
                expectedMovedEdges.push_back(vm::segment3(edge.start() + delta, edge.end() + delta));
            }

            ASSERT_TRUE(brush.canMoveEdges(worldBounds, edges, delta));
            const auto newBrush = brush.moveEdges(worldBounds, edges, delta).value();
            const auto movedEdges = newBrush.findClosestEdgePositions(kdl::vec_transform(edges, [&](const auto& s) { return s.translate(delta); }));
            ASSERT_EQ(expectedMovedEdges, movedEdges);
        }

        static void assertCanNotMoveEdges(const Brush& brush, const std::vector<vm::segment3> edges, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);
            ASSERT_FALSE(brush.canMoveEdges(worldBounds, edges, delta));
        }

        TEST_CASE("BrushTest.moveEdgeRemainingPolyhedron", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            // Taller than the cube, starts to the left of the +-64 unit cube
            const vm::segment3 edge(vm::vec3(-128, 0, -128), vm::vec3(-128, 0, +128));

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(128, Model::BrushFaceAttributes::NoTextureName).value();
            brush = brush.addVertex(worldBounds, edge.start()).value();
            brush = brush.addVertex(worldBounds, edge.end()).value();

            ASSERT_EQ(10u, brush.vertexCount());

            assertCanMoveEdges(brush, std::vector<vm::segment3>{edge}, vm::vec3(+63, 0, 0));
            assertCanNotMoveEdges(brush, std::vector<vm::segment3>{edge}, vm::vec3(+64, 0, 0)); // On the side of the cube
            assertCanNotMoveEdges(brush, std::vector<vm::segment3>{edge}, vm::vec3(+128, 0, 0)); // Center of the cube

            assertCanMoveVertices(brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+63, 0, 0));
            assertCanMoveVertices(brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+64, 0, 0));
            assertCanMoveVertices(brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+128, 0, 0));
        }

        // Same as above, but moving 2 edges
        TEST_CASE("BrushTest.moveEdgesRemainingPolyhedron", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            // Taller than the cube, starts to the left of the +-64 unit cube
            const vm::segment3 edge1(vm::vec3(-128, -32, -128), vm::vec3(-128, -32, +128));
            const vm::segment3 edge2(vm::vec3(-128, +32, -128), vm::vec3(-128, +32, +128));
            const std::vector<vm::segment3> movingEdges{edge1, edge2};

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(128, Model::BrushFaceAttributes::NoTextureName).value();
            brush = brush.addVertex(worldBounds, edge1.start()).value();
            brush = brush.addVertex(worldBounds, edge1.end()).value();
            brush = brush.addVertex(worldBounds, edge2.start()).value();
            brush = brush.addVertex(worldBounds, edge2.end()).value();

            ASSERT_EQ(12u, brush.vertexCount());

            assertCanMoveEdges(brush, movingEdges, vm::vec3(+63, 0, 0));
            assertCanNotMoveEdges(brush, movingEdges, vm::vec3(+64, 0, 0)); // On the side of the cube
            assertCanNotMoveEdges(brush, movingEdges, vm::vec3(+128, 0, 0)); // Center of the cube

            assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+63, 0, 0));
            assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+64, 0, 0));
            assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+128, 0, 0));
        }

        // "Move face" tests

        TEST_CASE("BrushTest.moveFace", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(64.0, "asdf").value();

            std::vector<vm::vec3> vertexPositions(4);
            vertexPositions[0] = vm::vec3(-32.0, -32.0, +32.0);
            vertexPositions[1] = vm::vec3(+32.0, -32.0, +32.0);
            vertexPositions[2] = vm::vec3(+32.0, +32.0, +32.0);
            vertexPositions[3] = vm::vec3(-32.0, +32.0, +32.0);

            const vm::polygon3 face(vertexPositions);

            ASSERT_TRUE(brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, face), vm::vec3(-16.0, -16.0, 0.0)));

            auto oldFacePositions = std::vector<vm::polygon3>({face});
            auto delta = vm::vec3(-16.0, -16.0, 0.0);
            brush = brush.moveFaces(worldBounds, oldFacePositions, delta).value();
            auto newFacePositions = brush.findClosestFacePositions(kdl::vec_transform(oldFacePositions, [&](const auto& f) { return f.translate(delta); }));

            ASSERT_EQ(1u, newFacePositions.size());
            ASSERT_TRUE(newFacePositions[0].hasVertex(vm::vec3(-48.0, -48.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].hasVertex(vm::vec3(-48.0, +16.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].hasVertex(vm::vec3(+16.0, +16.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].hasVertex(vm::vec3(+16.0, -48.0, +32.0)));

            oldFacePositions = std::move(newFacePositions);
            delta = vm::vec3(16.0, 16.0, 0.0);
            brush = brush.moveFaces(worldBounds, oldFacePositions, delta).value();
            newFacePositions = brush.findClosestFacePositions(kdl::vec_transform(oldFacePositions, [&](const auto& f) { return f.translate(delta); }));

            ASSERT_EQ(1u, newFacePositions.size());
            ASSERT_EQ(4u, newFacePositions[0].vertices().size());
            for (size_t i = 0; i < 4; ++i)
                ASSERT_TRUE(newFacePositions[0].hasVertex(face.vertices()[i]));
        }

        TEST_CASE("BrushNodeTest.cannotMoveFace", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCuboid(vm::vec3(128.0, 128.0, 32.0), Model::BrushFaceAttributes::NoTextureName).value();

            std::vector<vm::vec3> vertexPositions(4);
            vertexPositions[0] = vm::vec3(-64.0, -64.0, -16.0);
            vertexPositions[1] = vm::vec3(+64.0, -64.0, -16.0);
            vertexPositions[2] = vm::vec3(+64.0, -64.0, +16.0);
            vertexPositions[3] = vm::vec3(-64.0, -64.0, +16.0);

            const vm::polygon3 face(vertexPositions);

            ASSERT_FALSE(brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, face), vm::vec3(0.0, 128.0, 0.0)));
        }

        static void assertCanMoveFaces(const Brush& brush, const std::vector<vm::polygon3> movingFaces, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            std::vector<vm::polygon3> expectedMovedFaces;
            for (const vm::polygon3& polygon : movingFaces) {
                expectedMovedFaces.push_back(vm::polygon3(polygon.vertices() + delta));
            }

            ASSERT_TRUE(brush.canMoveFaces(worldBounds, movingFaces, delta));
            const auto newBrush = brush.moveFaces(worldBounds, movingFaces, delta).value();
            const auto movedFaces = newBrush.findClosestFacePositions(kdl::vec_transform(movingFaces, [&](const auto& f) { return f.translate(delta); }));
            ASSERT_EQ(expectedMovedFaces, movedFaces);
        }

        static void assertCanNotMoveFaces(const Brush& brush, const std::vector<vm::polygon3> movingFaces, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);
            ASSERT_FALSE(brush.canMoveFaces(worldBounds, movingFaces, delta));
        }

        static void assertCanMoveFace(const Brush& brush, const std::optional<size_t>& topFaceIndex, const vm::vec3 delta) {
            REQUIRE(topFaceIndex);
            const BrushFace& topFace = brush.face(*topFaceIndex);
            assertCanMoveFaces(brush, std::vector<vm::polygon3>{topFace.polygon()}, delta);
        }

        static void assertCanNotMoveFace(const Brush& brush, const std::optional<size_t>& topFaceIndex, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            REQUIRE(topFaceIndex);
            const BrushFace& topFace = brush.face(*topFaceIndex);
            ASSERT_FALSE(brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>{topFace.polygon()}, delta));
        }

        static void assertCanMoveTopFace(const Brush& brush, const vm::vec3 delta) {
            assertCanMoveFace(brush, brush.findFace(vm::vec3::pos_z()), delta);
        }

        static void assertCanNotMoveTopFace(const Brush& brush, const vm::vec3 delta) {
            assertCanNotMoveFace(brush, brush.findFace(vm::vec3::pos_z()), delta);
        }

        static void assertCanNotMoveTopFaceBeyond127UnitsDown(const Brush& brush) {
            assertCanMoveTopFace(brush, vm::vec3(0, 0, -127));
            assertCanNotMoveTopFace(brush, vm::vec3(0, 0, -128));
            assertCanNotMoveTopFace(brush, vm::vec3(0, 0, -129));

            assertCanMoveTopFace(brush, vm::vec3(256, 0, -127));
            assertCanNotMoveTopFace(brush, vm::vec3(256, 0, -128));
            assertCanNotMoveTopFace(brush, vm::vec3(256, 0, -129));
        }

        TEST_CASE("BrushTest.movePolygonRemainingPoint", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const std::vector<vm::vec3> vertexPositions{
                    vm::vec3(-64.0, -64.0, +64.0), // top quad
                    vm::vec3(-64.0, +64.0, +64.0),
                    vm::vec3(+64.0, -64.0, +64.0),
                    vm::vec3(+64.0, +64.0, +64.0),

                    vm::vec3(0.0, 0.0, -64.0), // bottom point
            };

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName).value();

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
        }

        TEST_CASE("BrushTest.movePolygonRemainingEdge", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const std::vector<vm::vec3> vertexPositions{
                    vm::vec3(-64.0, -64.0, +64.0), // top quad
                    vm::vec3(-64.0, +64.0, +64.0),
                    vm::vec3(+64.0, -64.0, +64.0),
                    vm::vec3(+64.0, +64.0, +64.0),

                    vm::vec3(-64.0, 0.0, -64.0), // bottom edge, on the z=-64 plane
                    vm::vec3(+64.0, 0.0, -64.0)
            };

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName).value();

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
        }

        TEST_CASE("BrushTest.movePolygonRemainingPolygon", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(128.0, Model::BrushFaceAttributes::NoTextureName).value();

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
        }

        TEST_CASE("BrushTest.movePolygonRemainingPolygon2", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            // Same brush as movePolygonRemainingPolygon, but this particular order of vertices triggers a failure in Brush::doCanMoveVertices
            // where the polygon inserted into the "remaining" BrushGeometry gets the wrong normal.
            const std::vector<vm::vec3> vertexPositions{
                    vm::vec3(+64.0, +64.0, +64.0),
                    vm::vec3(+64.0, -64.0, +64.0),
                    vm::vec3(+64.0, -64.0, -64.0),
                    vm::vec3(+64.0, +64.0, -64.0),
                    vm::vec3(-64.0, -64.0, +64.0),
                    vm::vec3(-64.0, -64.0, -64.0),
                    vm::vec3(-64.0, +64.0, -64.0),
                    vm::vec3(-64.0, +64.0, +64.0)};

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName).value();
            ASSERT_EQ(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), brush.bounds());

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
        }

        TEST_CASE("BrushTest.movePolygonRemainingPolygon_DisallowVertexCombining", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            //       z = +192  //
            // |\              //
            // | \             //
            // |  \  z = +64   //
            // |   |           //
            // |___| z = -64   //
            //                 //

            const std::vector<vm::vec3> vertexPositions{
                    vm::vec3(-64.0, -64.0, +192.0), // top quad, slanted
                    vm::vec3(-64.0, +64.0, +192.0),
                    vm::vec3(+64.0, -64.0, +64.0),
                    vm::vec3(+64.0, +64.0, +64.0),

                    vm::vec3(-64.0, -64.0, -64.0), // bottom quad
                    vm::vec3(-64.0, +64.0, -64.0),
                    vm::vec3(+64.0, -64.0, -64.0),
                    vm::vec3(+64.0, +64.0, -64.0),
            };

            const vm::vec3 topFaceNormal(sqrt(2.0) / 2.0, 0.0, sqrt(2.0) / 2.0);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName).value();

            const auto topFaceIndex = brush.findFace(topFaceNormal);
            assertCanMoveFace(brush, topFaceIndex, vm::vec3(0, 0, -127));
            assertCanMoveFace(brush, topFaceIndex, vm::vec3(0, 0, -128)); // Merge 2 verts of the moving polygon with 2 in the remaining polygon, should be allowed
            assertCanNotMoveFace(brush, topFaceIndex, vm::vec3(0, 0, -129));
        }

        TEST_CASE("BrushTest.movePolygonRemainingPolyhedron", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            //   _   z = +64   //
            //  / \            //
            // /   \           //
            // |   | z = -64   //
            // |   |           //
            // |___| z = -192  //
            //                 //

            const std::vector<vm::vec3> smallerTopPolygon{
                    vm::vec3(-32.0, -32.0, +64.0), // smaller top polygon
                    vm::vec3(-32.0, +32.0, +64.0),
                    vm::vec3(+32.0, -32.0, +64.0),
                    vm::vec3(+32.0, +32.0, +64.0)
            };
            const std::vector<vm::vec3> cubeTopFace{
                    vm::vec3(-64.0, -64.0, -64.0), // top face of cube
                    vm::vec3(-64.0, +64.0, -64.0),
                    vm::vec3(+64.0, -64.0, -64.0),
                    vm::vec3(+64.0, +64.0, -64.0),
            };
            const std::vector<vm::vec3> cubeBottomFace{
                    vm::vec3(-64.0, -64.0, -192.0), // bottom face of cube
                    vm::vec3(-64.0, +64.0, -192.0),
                    vm::vec3(+64.0, -64.0, -192.0),
                    vm::vec3(+64.0, +64.0, -192.0),
            };

            const std::vector<vm::vec3> vertexPositions = kdl::vec_concat(smallerTopPolygon, cubeTopFace,
                cubeBottomFace);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName).value();

            // Try to move the top face down along the Z axis
            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);
            assertCanNotMoveTopFace(brush, vm::vec3(0.0, 0.0, -257.0)); // Move top through the polyhedron and out the bottom

            // Move the smaller top polygon as 4 separate vertices
            assertCanMoveVertices(brush, smallerTopPolygon, vm::vec3(0, 0, -127));
            assertMovingVerticesDeletes(brush, smallerTopPolygon, vm::vec3(0, 0, -128));
            assertMovingVerticesDeletes(brush, smallerTopPolygon, vm::vec3(0, 0, -129));
            assertCanNotMoveVertices(brush, smallerTopPolygon, vm::vec3(0, 0, -257)); // Move through the polyhedron and out the bottom

            // Move top face along the X axis
            assertCanMoveTopFace(brush, vm::vec3(32.0, 0.0, 0.0));
            assertCanMoveTopFace(brush, vm::vec3(256, 0.0, 0.0));
            assertCanMoveTopFace(brush, vm::vec3(-32.0, -32.0, 0.0)); // Causes face merging and a vert to be deleted at z=-64
        }

        TEST_CASE("BrushTest.moveTwoFaces", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            //               //
            // |\    z = 64  //
            // | \           //
            // |  \          //
            //A|   \ z = 0   //
            // |   /         //
            // |__/C         //
            //  B    z = -64 //
            //               //


            const std::vector<vm::vec3> leftPolygon{ // A
                    vm::vec3(-32.0, -32.0, +64.0),
                    vm::vec3(-32.0, +32.0, +64.0),
                    vm::vec3(-32.0, +32.0, -64.0),
                    vm::vec3(-32.0, -32.0, -64.0),
            };
            const std::vector<vm::vec3> bottomPolygon{ // B
                    vm::vec3(-32.0, -32.0, -64.0),
                    vm::vec3(-32.0, +32.0, -64.0),
                    vm::vec3(+0.0, +32.0, -64.0),
                    vm::vec3(+0.0, -32.0, -64.0),
            };
            const std::vector<vm::vec3> bottomRightPolygon{ // C
                    vm::vec3(+0.0, -32.0, -64.0),
                    vm::vec3(+0.0, +32.0, -64.0),
                    vm::vec3(+32.0, +32.0, +0.0),
                    vm::vec3(+32.0, -32.0, +0.0),
            };

            const std::vector<vm::vec3> vertexPositions = kdl::vec_concat(leftPolygon, bottomPolygon,
                bottomRightPolygon);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName).value();

            EXPECT_TRUE(brush.hasFace(vm::polygon3(leftPolygon)));
            EXPECT_TRUE(brush.hasFace(vm::polygon3(bottomPolygon)));
            EXPECT_TRUE(brush.hasFace(vm::polygon3(bottomRightPolygon)));

            assertCanMoveFaces(brush, std::vector<vm::polygon3>{ vm::polygon3(leftPolygon), vm::polygon3(bottomPolygon) }, vm::vec3(0, 0, 63));
            assertCanNotMoveFaces(brush, std::vector<vm::polygon3>{ vm::polygon3(leftPolygon), vm::polygon3(bottomPolygon) }, vm::vec3(0, 0, 64)); // Merges B and C
        }

        // "Move polyhedron" tests

        TEST_CASE("BrushNodeTest.movePolyhedronRemainingEdge", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            // Edge to the left of the cube, shorter, extends down to Z=-256
            const vm::segment3 edge(vm::vec3(-128, 0, -256), vm::vec3(-128, 0, 0));

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(128, Model::BrushFaceAttributes::NoTextureName).value();
            brush = brush.addVertex(worldBounds, edge.start()).value();
            brush = brush.addVertex(worldBounds, edge.end()).value();

            ASSERT_EQ(10u, brush.vertexCount());

            const auto cubeTopIndex = brush.findFace(vm::vec3::pos_z());
            const auto cubeBottomIndex = brush.findFace(vm::vec3::neg_z());
            const auto cubeRightIndex = brush.findFace(vm::vec3::pos_x());
            const auto cubeLeftIndex = brush.findFace(vm::vec3::neg_x());
            const auto cubeBackIndex = brush.findFace(vm::vec3::pos_y());
            const auto cubeFrontIndex = brush.findFace(vm::vec3::neg_y());

            EXPECT_TRUE(cubeTopIndex);
            EXPECT_FALSE(cubeBottomIndex);  // no face here, part of the wedge connecting to `edge`
            EXPECT_TRUE(cubeRightIndex);
            EXPECT_FALSE(cubeLeftIndex); // no face here, part of the wedge connecting to `edge`
            EXPECT_TRUE(cubeFrontIndex);
            EXPECT_TRUE(cubeBackIndex);

            const BrushFace& cubeTop = brush.face(*cubeTopIndex);
            const BrushFace& cubeRight = brush.face(*cubeRightIndex);
            const BrushFace& cubeFront = brush.face(*cubeFrontIndex);
            const BrushFace& cubeBack = brush.face(*cubeBackIndex);

            const std::vector<vm::polygon3> movingFaces{
                cubeTop.polygon(),
                cubeRight.polygon(),
                cubeFront.polygon(),
                cubeBack.polygon(),
            };

            assertCanMoveFaces(brush, movingFaces, vm::vec3(32, 0, 0)); // away from `edge`
            assertCanMoveFaces(brush, movingFaces, vm::vec3(-63, 0, 0)); // towards `edge`, not touching
            assertCanMoveFaces(brush, movingFaces, vm::vec3(-64, 0, 0)); // towards `edge`, touching
            assertCanMoveFaces(brush, movingFaces, vm::vec3(-65, 0, 0)); // towards `edge`, covering

            // Move the cube down 64 units, so the top vertex of `edge` is on the same plane as `cubeTop`
            // This will turn `cubeTop` from a quad into a pentagon
            assertCanNotMoveFaces(brush, movingFaces, vm::vec3(0, 0, -64));
            assertCanMoveVertices(brush, asVertexList(movingFaces), vm::vec3(0, 0, -64));

            // Make edge poke through the top face
            assertCanNotMoveFaces(brush, movingFaces, vm::vec3(-192, 0, -128));
            assertCanNotMoveVertices(brush, asVertexList(movingFaces), vm::vec3(-192, 0, -128));
        }

        // UV Lock tests

        template<MapFormat F>
        class UVLockTest {
            MapFormat param = F;
        };

        TEST_CASE("moveFaceWithUVLock", "[UVLockTest]") {
            auto format = GENERATE(MapFormat::Valve, MapFormat::Standard);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), format);

            Assets::Texture testTexture("testTexture", 64, 64);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createCube(64.0, "").value();
            for (auto& face : brush.faces()) {
                face.setTexture(&testTexture);
            }

            const auto delta = vm::vec3(+8.0, 0.0, 0.0);
            const auto polygonToMove = vm::polygon3(brush.face(*brush.findFace(vm::vec3::pos_z())).vertexPositions());
            ASSERT_TRUE(brush.canMoveFaces(worldBounds, {polygonToMove}, delta));

            // move top face by x=+8
            const auto changed = brush.moveFaces(worldBounds, {polygonToMove}, delta, false).value();
            const auto changedWithUVLock = brush.moveFaces(worldBounds, {polygonToMove}, delta, true).value();

            // The move should be equivalent to shearing by this matrix
            const auto M = vm::shear_bbox_matrix(brush.bounds(), vm::vec3::pos_z(), delta);

            for (auto& oldFace : brush.faces()) {
                const auto oldTexCoords = kdl::vec_transform(oldFace.vertexPositions(),
                    [&](auto x) { return oldFace.textureCoords(x); });
                const auto shearedVertexPositions = kdl::vec_transform(oldFace.vertexPositions(),
                    [&](auto x) { return M * x; });
                const auto shearedPolygon = vm::polygon3(shearedVertexPositions);

                const auto normal = oldFace.boundary().normal;

                // The brush modified without texture lock is expected to have changed UV's on some faces, but not on others
                {
                    const auto newFaceIndex = changed.findFace(shearedPolygon);
                    REQUIRE(newFaceIndex);
                    const BrushFace& newFace = changed.face(*newFaceIndex);
                    const auto newTexCoords = kdl::vec_transform(shearedVertexPositions,
                        [&](auto x) { return newFace.textureCoords(x); });
                    if (normal == vm::vec3::pos_z()
                        || normal == vm::vec3::pos_y()
                        || normal == vm::vec3::neg_y()) {
                        EXPECT_FALSE(UVListsEqual(oldTexCoords, newTexCoords));
                        // TODO: actually check the UV's
                    } else {
                        EXPECT_TRUE(UVListsEqual(oldTexCoords, newTexCoords));
                    }
                }

                // UV's should all be the same when using texture lock (with Valve format).
                // Standard format can only do UV lock on the top face, which is not sheared.
                {
                    const auto newFaceWithUVLockIndex = changedWithUVLock.findFace(shearedPolygon);
                    REQUIRE(newFaceWithUVLockIndex);
                    const BrushFace& newFaceWithUVLock = changedWithUVLock.face(*newFaceWithUVLockIndex);
                    const auto newTexCoordsWithUVLock = kdl::vec_transform(shearedVertexPositions, [&](auto x) {
                        return newFaceWithUVLock.textureCoords(x);
                    });
                    if (normal == vm::vec3d::pos_z() || (format == MapFormat::Valve)) {
                        EXPECT_TRUE(UVListsEqual(oldTexCoords, newTexCoordsWithUVLock));
                    }
                }
            }
        }

        // Tests for failures and issues

        TEST_CASE("BrushTest.moveVertexFailing1", "[BrushTest]") {
            const vm::vec3d p1(-64.0, -64.0, 0.0);
            const vm::vec3d p2(+64.0, -64.0, 0.0);
            const vm::vec3d p3(0.0, +64.0, 0.0);
            const vm::vec3d p4(0.0, 0.0, +32.0);

            std::vector<vm::vec3d> oldPositions;
            oldPositions.push_back(p1);
            oldPositions.push_back(p2);
            oldPositions.push_back(p3);
            oldPositions.push_back(p4);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(oldPositions, "texture").value();

            for (size_t i = 0; i < oldPositions.size(); ++i) {
                for (size_t j = 0; j < oldPositions.size(); ++j) {
                    if (i != j) {
                        ASSERT_FALSE(brush.canMoveVertices(worldBounds, std::vector<vm::vec3d>(1, oldPositions[i]), oldPositions[j] - oldPositions[i]));
                    }
                }
            }
        }

        TEST_CASE("BrushTest.moveVertexFail_2158", "[BrushTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2158
            const std::string data("{\n"
                              "( 320 256 320 ) ( 384 192 320 ) ( 352 224 384 ) sky1 0 96 0 1 1\n"
                              "( 384 128 320 ) ( 320 64 320 ) ( 352 96 384 ) sky1 0 96 0 1 1\n"
                              "( 384 32 320 ) ( 384 32 384 ) ( 384 256 384 ) sky1 0 96 0 1 1\n"
                              "( 192 192 320 ) ( 256 256 320 ) ( 224 224 384 ) sky1 0 96 0 1 1\n"
                              "( 256 64 320 ) ( 192 128 320 ) ( 224 96 384 ) sky1 0 96 0 1 1\n"
                              "( 192 32 384 ) ( 192 32 320 ) ( 192 256 320 ) sky1 0 96 0 1 1\n"
                              "( 384 256 320 ) ( 384 256 384 ) ( 192 256 384 ) sky1 0 96 0 1 1\n"
                              "( 320 64 320 ) ( 256 64 320 ) ( 288 64 384 ) sky1 0 96 0 1 1\n"
                              "( 192 64 352 ) ( 192 240 352 ) ( 368 240 352 ) sky1 0 0 0 1 1\n"
                              "( 384 240 320 ) ( 208 240 320 ) ( 208 64 320 ) sky1 0 0 0 1 1\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;

            const std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            EXPECT_EQ(1u, nodes.size());

            Brush brush = static_cast<BrushNode*>(nodes.front())->brush();
            const vm::vec3 p(192.0, 128.0, 352.0);

            auto oldVertexPositions = std::vector<vm::vec3>({p});
            auto delta = 4.0 * 16.0 * vm::vec3::neg_y();
            brush = brush.moveVertices(worldBounds, oldVertexPositions, delta).value();
            auto newVertexPositions = brush.findClosestVertexPositions(oldVertexPositions + delta);

            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p + delta, newVertexPositions.front());

            kdl::col_delete_all(nodes);
        }

        TEST_CASE("BrushTest.moveVerticesFail_2158", "[BrushTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2158
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const std::string data = R"(
{
( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) rock4_2 30.92560005187988281 0.960906982421875 5.59741020202636719 0.98696297407150269 0.98029798269271851
( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) rock4_2 67.89600372314453125 -61.20909881591796875 13.658599853515625 0.85491102933883667 1.12606000900268555
( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) rock4_2 -3.77819991111755371 -44.42710113525390625 7.24881982803344727 0.95510202646255493 1.04886996746063232
( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) rock4_2 77.66159820556640625 -86.74199676513671875 173.0970001220703125 1.15471994876861572 -1.11249995231628418
( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) rock4_2 115.52100372314453125 55.40819931030273438 157.998992919921875 1.19368994235992432 -1.0113600492477417
( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) rock4_2 29.0522003173828125 16.1511993408203125 198.899993896484375 0.90696299076080322 -1.06921005249023438
( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1163.51855729802718997 -1820.79407602155902168 554.17919393113811566 ) rock4_2 -52.78820037841796875 -84.4026031494140625 200.2100067138671875 0.88777101039886475 -0.97177797555923462
( 1163.51855729802718997 -1820.79407602155902168 554.17919393113811566 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) rock4_2 72.63649749755859375 102.17099761962890625 80.11309814453125 0.87609797716140747 -1.61881005764007568
( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) rock4_2 -0.7561039924621582 32.18519973754882812 75.325897216796875 0.90074300765991211 -1.72079002857208252
( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) rock4_2 85.426300048828125 -37.61460113525390625 170.2440032958984375 0.94236099720001221 -1.08232998847961426
( 1169.17641562068342864 -1800.29610138592852309 568.7974852992444994 ) ( 1169.18149090383781186 -1800.30190582364161855 568.76530164709924975 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) rock4_2 -15.04969978332519531 -12.76039981842041016 176.2700042724609375 0.93921899795532227 -1.1466900110244751
( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1164.16844274448340002 -1797.72618014395857244 578.31529060850652968 ) rock4_2 -1.02465999126434326 60.25889968872070312 159.8549957275390625 0.78085201978683472 -1.21036994457244873
( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) rock4_2 86.87239837646484375 40.37289810180664062 129.878997802734375 0.66983801126480103 -2.06800007820129395
( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) ( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) rock4_2 77.13539886474609375 119.01000213623046875 358.319000244140625 1.14928996562957764 1.19559001922607422
( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) rock4_2 -19.27930068969726562 17.50340080261230469 148.16400146484375 1.01748001575469971 -0.89703798294067383
( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) rock4_2 -43.56299972534179688 -73.20639801025390625 350.87200927734375 0.98191499710083008 1.14552998542785645
( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) rock4_2 -57.5941009521484375 20.35930061340332031 349.8599853515625 0.91973602771759033 1.05388998985290527
( 718.09496664767948459 -1851.18753444490516813 378.79962463045302457 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) rock4_2 84.4087982177734375 44.97620010375976562 5.90301990509033203 0.94212800264358521 1.00434005260467529
( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 718.09496664767948459 -1851.18753444490516813 378.79962463045302457 ) rock4_2 -4.20452976226806641 26.958099365234375 7.14522981643676758 0.90771502256393433 1.01380002498626709
( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) rock4_2 -81.561798095703125 -95.4485015869140625 40.62070083618164062 0.5180240273475647 1.46343004703521729
( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) rock4_2 52.8777008056640625 -9.35947036743164062 58.6305999755859375 0.61474400758743286 1.24004995822906494
( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) rock4_2 -45.87020111083984375 -44.08499908447265625 41.31510162353515625 0.53462702035903931 1.54106998443603516
( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 718.09496664767948459 -1851.18753444490516813 378.79962463045302457 ) rock4_2 8.81488037109375 37.412200927734375 6.29719018936157227 0.96984899044036865 0.99895197153091431
( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) rock4_2 5.92700004577636719 4.41837978363037109 8.78011035919189453 0.7744939923286438 1.05709004402160645
( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) rock4_2 0.02703860029578209 11.37539958953857422 8.51169967651367188 0.77832400798797607 1.01610994338989258
( 647.29885930542945971 -1801.53486617151679638 462.0987669933149391 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) rock4_2 75.124298095703125 3.1680600643157959 8.79839038848876953 0.75931602716445923 1.01523995399475098
( 428.68162139174597769 -1687.29811786616778591 488.88114395300908654 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) rock4_2 -13.265899658203125 -8.93752956390380859 11.75290012359619141 0.59300100803375244 0.97339397668838501
( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) ( 383.59438380944988012 -1744.18320926297974438 267.01713311064645495 ) ( 394.84298436650840358 -1643.95107488440089583 473.74271495432344636 ) rock4_2 5.71436023712158203 66.92310333251953125 162.699005126953125 0.74939501285552979 -1.05348002910614014
( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 417.39145642527222435 -1674.70943252244819632 496.15546600960624346 ) rock4_2 47.94699859619140625 80.93849945068359375 350.2969970703125 0.99699199199676514 0.93575799465179443
( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 409.86763191010521723 -1638.4154691593678308 480.83629920333873997 ) rock4_2 -17.06769943237304688 76.29920196533203125 226.9109954833984375 0.86038202047348022 -0.97620397806167603
( 1164.16844274448340002 -1797.72618014395857244 578.31529060850652968 ) ( 1126.49874461573472217 -1839.25626760914360602 608.06151113412647646 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) rock4_2 17.15080070495605469 78.2032012939453125 226.90899658203125 0.86016601324081421 -0.97621601819992065
( 1164.16895096277721677 -1797.72592376172019613 578.31488545196270934 ) ( 1164.16844274448340002 -1797.72618014395857244 578.31529060850652968 ) ( 415.37140289843625851 -1630.10750076058616287 474.93304004273147712 ) rock4_2 67.65200042724609375 17.70070075988769531 124.0709991455078125 0.93583697080612183 0.99498897790908813
( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 1120.512868445862523 -1855.31927395340585463 574.535634983251839 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) rock4_2 34.074798583984375 -67.4031982421875 5.12918996810913086 0.89313501119613647 0.99598902463912964
( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 1183.69438641028636994 -1904.94288073521306615 311.88345805427366031 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) rock4_2 9.72570991516113281 95.0894012451171875 350.1099853515625 0.99535101652145386 0.97052502632141113
( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) rock4_2 -2.58533000946044922 7.69421005249023438 349.858001708984375 0.99317502975463867 0.99086099863052368
( 392.51561748944976671 -1758.13841025977330901 221.93166373893632226 ) ( 685.205227597987232 -1880.05386294480922516 267.14020489435648642 ) ( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) rock4_2 0.29211398959159851 -1.12084996700286865 349.87799072265625 0.99334698915481567 0.98575097322463989
( 1199.73437537143649934 -1850.52292721460958091 299.11555748386712139 ) ( 1195.3323608207340385 -1812.61180985669875554 293.31661882168685906 ) ( 404.63242807195160822 -1696.09174007488900315 211.96202895796943722 ) rock4_2 -3.78198003768920898 21.7248992919921875 349.865997314453125 0.9932439923286438 0.99966299533843994
}
)";

            IO::TestParserStatus status;

            auto nodes = IO::NodeReader::read(data, world, worldBounds, status);
            EXPECT_EQ(1u, nodes.size());

            Brush brush = static_cast<BrushNode*>(nodes.front())->brush();

            const std::vector<vm::vec3> vertexPositions {
                brush.findClosestVertexPosition(vm::vec3(1169.1764156206966, -1800.2961013859342, 568.79748529920892)),
                brush.findClosestVertexPosition(vm::vec3(1164.1689509627774, -1797.7259237617193, 578.31488545196294)),
                brush.findClosestVertexPosition(vm::vec3(1163.5185572994671, -1820.7940760208414, 554.17919392904093)),
                brush.findClosestVertexPosition(vm::vec3(1120.5128684458623, -1855.3192739534061, 574.53563498325116))
            };

            ASSERT_TRUE(brush.canMoveVertices(worldBounds, vertexPositions, vm::vec3(16.0, 0.0, 0.0)));
            ASSERT_NO_THROW(brush.moveVertices(worldBounds, vertexPositions, vm::vec3(16.0, 0.0, 0.0)));

            kdl::col_delete_all(nodes);
        }


        TEST_CASE("BrushTest.removeVertexWithCorrectTextures_2082", "[BrushTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2082

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Valve);

            const std::string data = R"(
{
( 32 -32 -0 ) ( -16 -32 -0 ) ( -16 -32 32 ) *04water1 [ -1 0 0 -0.941193 ] [ 0 0 -1 -0 ] 125.468 1 1
( -16 -32 32 ) ( -16 -32 -0 ) ( -32 -16 -0 ) *04mwat2 [ -1 0 0 -0.941193 ] [ 0 0 -1 -0 ] 125.468 1 1
( 32 32 -0 ) ( 32 -32 -0 ) ( 32 -32 32 ) *04water2 [ -2.22045e-16 -1 0 -24.9412 ] [ 0 0 -1 -0 ] 125.468 1 1
( 32 -32 32 ) ( -16 -32 32 ) ( 32 -0 64 ) *teleport [ 0 0 -1 -0 ] [ 1 0 0 0.999969 ] 270 1 1
( 32 -0 64 ) ( -16 -32 32 ) ( -32 -16 32 ) *slime1 [ 0 -1 -2.22045e-16 -0 ] [ 1 0 0 0.999969 ] 270 1 1
( 32 32 -0 ) ( -16 32 -0 ) ( -32 -16 -0 ) *lava1 [ 1 0 0 -0 ] [ 0 -1 0 0.999998 ] -0 1 1
( 32 -0 64 ) ( -16 32 32 ) ( 32 32 32 ) *slime [ 0 -1 2.22045e-16 -0 ] [ 1 0 0 0.999969 ] 270 1 1
( 32 32 32 ) ( -16 32 32 ) ( -16 32 -0 ) *04awater1 [ 0.894427 -0.447214 0 18.9966 ] [ 0 0 -1 -0 ] -0 1 1
( -16 32 -0 ) ( -16 32 32 ) ( -32 -16 32 ) *04mwat1 [ -2.22045e-16 1 0 39.0588 ] [ 0 0 -1 -0 ] 125.468 1 1
( -32 -16 32 ) ( -16 32 32 ) ( 32 -0 64 ) *slime0 [ -2.43359e-08 -1 0 0.999985 ] [ -1 2.43359e-08 0 -0 ] 90 1 1
}
)";

            IO::TestParserStatus status;

            std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            ASSERT_EQ(1u, nodes.size());

            Brush brush = static_cast<BrushNode*>(nodes.front())->brush();

            const vm::vec3 p1(32.0, 32.0, 0.0);
            const vm::vec3 p2(-16.0, 32.0, 0.0);
            const vm::vec3 p3(-32.0, -16.0, 0.0);
            const vm::vec3 p4(-16.0, -32.0, 0.0);
            const vm::vec3 p5(32.0, -32.0, 0.0);

            const vm::vec3 p6(32.0, 32.0, 32.0);
            const vm::vec3 p7(-16.0, 32.0, 32.0); // this vertex will be deleted
            const vm::vec3 p8(-32.0, -16.0, 32.0);
            const vm::vec3 p9(-16.0, -32.0, 32.0);
            const vm::vec3 p10(32.0, -32.0, 32.0);

            const vm::vec3 p11(32.0, 0.0, 64.0);

            // Make sure that the faces have the textures we expect before the vertex is deleted.

            // side faces
            assertTexture("*04awater1", brush, std::vector<vm::vec3d>{p1, p2, p7, p6});
            assertTexture("*04mwat1", brush, std::vector<vm::vec3d>{p2, p3, p8, p7});
            assertTexture("*04mwat2", brush, std::vector<vm::vec3d>{p3, p4, p9, p8});
            assertTexture("*04water1", brush, std::vector<vm::vec3d>{p4, p5, p10, p9});
            assertTexture("*04water2", brush, std::vector<vm::vec3d>{p5, p1, p6, p11, p10});

            // bottom face
            assertTexture("*lava1", brush, std::vector<vm::vec3d>{p5, p4, p3, p2, p1});

            // top faces
            assertTexture("*slime", brush, std::vector<vm::vec3d>{p6, p7, p11});
            assertTexture("*slime0", brush, std::vector<vm::vec3d>{p7, p8, p11});
            assertTexture("*slime1", brush, std::vector<vm::vec3d>{p8, p9, p11});
            assertTexture("*teleport", brush, std::vector<vm::vec3d>{p9, p10, p11});

            // delete the vertex
            ASSERT_TRUE(brush.canRemoveVertices(worldBounds, std::vector<vm::vec3d>{p7}));
            brush = brush.removeVertices(worldBounds, std::vector<vm::vec3d>{p7}).value();

            // assert the structure and textures

            // side faces
            assertTexture("*04awater1", brush, std::vector<vm::vec3d>{p1, p2, p6});
            assertTexture("*04mwat1", brush, std::vector<vm::vec3d>{p2, p3, p8});
            assertTexture("*04mwat2", brush, std::vector<vm::vec3d>{p3, p4, p9, p8});
            assertTexture("*04water1", brush, std::vector<vm::vec3d>{p4, p5, p10, p9});
            assertTexture("*04water2", brush, std::vector<vm::vec3d>{p5, p1, p6, p11, p10});

            // bottom face
            assertTexture("*lava1", brush, std::vector<vm::vec3d>{p5, p4, p3, p2, p1});

            // top faces
            assertTexture("*slime", brush, std::vector<vm::vec3d>{p6, p2, p11});
            assertTexture("*slime0", brush, std::vector<vm::vec3d>{p2, p8, p11});
            assertTexture("*slime1", brush, std::vector<vm::vec3d>{p8, p9, p11}); // failure, becomes *slime0
            assertTexture("*teleport", brush, std::vector<vm::vec3d>{p9, p10, p11});

            kdl::col_delete_all(nodes);
        }

        TEST_CASE("BrushTest.snapIssue1198", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1198
            const std::string data("{\n"
                              "( 167.63423 -46.88446 472.36551 ) ( 66.06285 -1.98675 573.93711 ) ( 139.12681 -168.36963 500.87299 ) rock_1736 -158 527 166.79401 0.97488 -0.85268 //TX1\n"
                              "( 208 -298.77704 309.53674 ) ( 208 -283.89740 159.77713 ) ( 208 -425.90924 294.65701 ) rock_1736 -261 -291 186.67561 1 1.17558 //TX1\n"
                              "( -495.37965 -970.19919 2420.40004 ) ( -369.12126 -979.60987 2439.22145 ) ( -516.42274 -1026.66357 2533.32892 ) skill_ground -2752 -44 100.55540 0.89744 -0.99664 //TX1\n"
                              "( 208 -103.52284 489.43151 ) ( 208 -63.04567 610.86296 ) ( 80 -103.52284 489.43151 ) rock_1736 208 516 0 -1 0.94868 //TX1\n"
                              "( -450.79344 -2050.77028 440.48261 ) ( -333.56544 -2071.81325 487.37381 ) ( -470.33140 -2177.02858 432.66743 ) skill_ground -2100 -142 261.20348 0.99813 0.93021 //TX1\n"
                              "( -192.25073 -2050.77026 159.49851 ) ( -135.78626 -2071.81323 272.42748 ) ( -201.66146 -2177.02856 140.67705 ) skill_ground -2010 513 188.47871 0.99729 -0.89685 //TX1\n"
                              "( 181.06874 -76.56186 495.11416 ) ( 172.37248 -56.19832 621.18438 ) ( 63.35341 -126.83229 495.11416 ) rock_1736 197 503 0 -0.91965 0.98492 //TX1\n"
                              "( 171.46251 -48.09583 474.98238 ) ( 129.03154 -21.91225 616.98017 ) ( 105.41315 -157.70143 477.82758 ) rock_1736 -71 425 178.51302 0.85658 -1.11429 //TX1\n"
                              "( -37.21422 -6.81390 22.01408 ) ( -12.34518 -24.34492 146.34503 ) ( -92.55376 -122.11616 16.82534 ) skill_ground -6 23 182.57664 0.90171 -0.97651 //TX1\n"
                              "( -975.92228 -1778.45799 1072.52401 ) ( -911.46425 -1772.13654 1182.92865 ) ( -1036.18913 -1883.59588 1113.72975 ) skill_ground -2320 426 158.59875 0.88222 -0.82108 //TX1\n"
                              "( -984.28431 -1006.06166 2136.35663 ) ( -881.58265 -976.76783 2206.91312 ) ( -1039.55007 -1059.19179 2238.85958 ) skill_ground -2580 152 118.33189 0.90978 -0.96784 //TX1\n"
                              "( -495.37960 -2050.77026 672 ) ( -369.12118 -2071.81323 672 ) ( -516.42263 -2177.02856 672 ) skill_ground -2104 -151 260.53769 1 1 //TX1\n"
                              "( 0 -192 512 ) ( 0 -192 640 ) ( 128 -192 512 ) skill_ground 0 512 0 1 1 //TX1\n"
                              "( 0 0 512 ) ( 0 -128 512 ) ( 128 0 512 ) skill_ground 0 0 0 1 -1 //TX1\n"
                              "}");
            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1202", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1202
            const std::string data("{\n"
                              "( -384 -1440 416 ) ( -384 -1440 544 ) ( -512 -1440 416 ) skip -384 416 0 -1 1 //TX1\n"
                              "( -479.20200 -1152 448 ) ( -388.69232 -1242.50967 448 ) ( -607.20203 -1152 448 ) skip -476 1631 -45 1 -0.70711 //TX2\n"
                              "( -202.75913 -1259.70123 365.61488 ) ( -293.26877 -1169.19156 365.61487 ) ( -288.09239 -1345.03450 408.28175 ) city6_8 747 1097 135 1 0.94281 //TX2\n"
                              "( -672 -1664 112 ) ( -800 -1664 112 ) ( -672 -1664 240 ) bricka2_4 -672 112 0 -1 1 //TX2\n"
                              "( -166.47095 -1535.24850 432 ) ( -294.41554 -1539.01482 432 ) ( -38.47095 -1663.24847 432 ) bricka2_4 -212 1487 181.68613 1 1.02899 //TX2\n"
                              "( 96 -2840.62573 176 ) ( 96 -3021.64502 176 ) ( 96 -2840.62573 304 ) bricka2_4 -2009 176 0 -1.41421 1 //TX2\n"
                              "( -128 -288 176 ) ( -128 -160 176 ) ( -128 -288 304 ) bricka2_4 288 176 0 1 1 //TX2\n"
                              "}");
            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1203", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1203
            const std::string data("{\n"
                              "( -2255.07542 -1621.75354 1184 ) ( -2340.26373 -1524.09826 1184 ) ( -2255.07542 -1621.75354 1312 ) metal5_6 2126 1184 0 0.76293 1 //TX2\n"
                              "( -2274.59294 -1572.67199 1077.14252 ) ( -2216.18139 -1643.55025 1214.27523 ) ( -2179.93925 -1486.72565 1086.37772 ) metal1_2 -86 -3857 66.92847 1.16449 -0.65206 //TX2\n"
                              "( -2294.68465 -1559.17687 1145.06418 ) ( -2209.49633 -1656.83209 1145.06409 ) ( -2226.47948 -1499.67881 1009.29941 ) metal1_2 -2044 -1080 180.00005 0.76293 1.06066 //TX2\n"
                              "( -2277.90664 -1569.35830 1229.87757 ) ( -2219.49502 -1640.23662 1092.74492 ) ( -2183.25294 -1483.41196 1220.64238 ) metal1_2 1738 -2475 -66.92843 1.16449 0.65206 //TX2\n"
                              "( -2291.16152 -1556.10351 1161.99537 ) ( -2205.97305 -1653.75857 1161.99532 ) ( -2222.95604 -1496.60517 1297.75964 ) metal1_2 -2040 1096 180.00003 0.76293 -1.06066 //TX2\n"
                              "( -2081.99036 -1805.83188 1184 ) ( -2022.45370 -1920.93607 1184 ) ( -2195.68224 -1864.63800 1184 ) skinsore -640 2679 -62.65012 1.01242 -1 //TX2\n"
                              "( -2243.07853 -1621.15697 1184 ) ( -2243.07799 -1621.15750 1312 ) ( -2152.56935 -1530.64682 1184 ) metal5_6 2293 1184 0 0.70711 1 //TX1\n"
                              "( -2288.33311 -1643.78464 1184 ) ( -2197.82344 -1553.27497 1184 ) ( -2288.33311 -1643.78464 1312 ) metal5_6 2325 1184 0 0.70711 1 //TX2\n"
                              "( -2243.76171 -1610.43983 1184 ) ( -2243.76171 -1610.43983 1312 ) ( -2327.90482 -1513.98290 1184 ) metal5_6 2137 1184 0 0.75357 1 //TX1\n"
                              "}");
            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1205", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1205
            const std::string data("{\n"
                              "( 304 -895.52890 1232 ) ( 304 -763.64662 1232 ) ( 304 -895.52890 1104 ) bookshelf1w 1232 -869 -90 1 1.03033 //TX1\n"
                              "( -23.76447 -759.76453 1232 ) ( 69.49032 -666.50962 1232 ) ( -23.76447 -759.76453 1104 ) bookshelf1w 1232 -1043 -90 1 0.72855 //TX1\n"
                              "( -139.64675 -480 1232 ) ( -7.76448 -480 1232 ) ( -139.64675 -480 1104 ) bookshelf1w 1232 -136 -90 1 1.03033 //TX1\n"
                              "( -42.50967 -245.49033 1232 ) ( 50.74518 -338.74518 1232 ) ( -42.50967 -245.49033 1104 ) bookshelf1w 1232 337 -90 1 -0.72855 //TX1\n"
                              "( 323.88225 -320 1232 ) ( 191.99998 -320 1232 ) ( 323.88225 -320 1104 ) bookshelf1w 1232 -314 -90 1 -1.03033 //TX1\n"
                              "( 144 -168.23550 1232 ) ( 144 -300.11777 1232 ) ( 144 -168.23550 1104 ) bookshelf1w 1232 163 -90 1 -1.03033 //TX1\n"
                              "( 303.99988 -432.00012 1248.00050 ) ( 278.89702 -432.00012 1373.51482 ) ( 303.99988 -304.00012 1248.00050 ) rfslte1 432 1273 0 1 0.98058 //TX1\n"
                              "( 303.99995 -367.99981 1248 ) ( 286.42119 -385.57861 1373.56263 ) ( 213.49015 -277.49027 1248 ) rfslte1 430 1272 0 -0.70711 0.98096 //TX1\n"
                              "( 256 -320 1247.99999 ) ( 256 -345.10286 1373.51432 ) ( 128 -320.00005 1247.99999 ) rfslte1 256 1273 0 -1 0.98058 //TX1\n"
                              "( 191.99988 -320.00012 1248.00049 ) ( 209.57867 -337.57891 1373.56311 ) ( 101.49021 -410.50979 1248.00049 ) rfslte1 -453 1272 0 -0.70711 0.98096 //TX1\n"
                              "( 144 -368 1248.00049 ) ( 169.10289 -368 1373.51481 ) ( 144 -496 1248.00049 ) rfslte1 -368 1273 0 -1 0.98058 //TX1\n"
                              "( 144 -432 1248.00049 ) ( 161.57879 -414.42121 1373.56311 ) ( 234.50967 -522.50967 1248.00049 ) rfslte1 -611 1272 0 -0.70711 0.98096 //TX1\n"
                              "( 192 -480 1248.00049 ) ( 192 -454.89711 1373.51481 ) ( 320 -480 1248.00049 ) rfslte1 -192 1273 0 1 0.98058 //TX1\n"
                              "( 256 -480 1248.00049 ) ( 238.42121 -462.42121 1373.56311 ) ( 346.50967 -389.49033 1248.00049 ) rfslte1 679 1272 0 0.70711 0.98096 //TX1\n"
                              "( 144 -320 1232 ) ( 144 -448 1232 ) ( 272 -320 1232 ) rfslte1 -144 320 0 1 -1 //TX1\n"
                              "( 285.25483 -226.74517 1232 ) ( 191.99999 -320.00001 1232 ) ( 285.25483 -226.74517 1104 ) bookshelf1w 1232 311 -90 1 -0.72855 //TX1\n"
                              "( 304 -368 1232 ) ( 210.74516 -274.74516 1232 ) ( 304 -368 1104 ) bookshelf1w 1232 -505 -90 1 0.72855 //TX1\n"
                              "}");
            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1206", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1206
            const std::string data("{\n"
                              "( -637.50000 1446.44631 1339.47316 ) ( -637.50000 1560.93298 1396.71649 ) ( -765.50000 1446.44631 1339.47316 ) column01_3 -638 1617 0 -1 0.89443 //TX1\n"
                              "( -632.50000 1438.33507 1340.33194 ) ( -632.50000 1538.28627 1260.37098 ) ( -760.50000 1438.33507 1340.33194 ) column01_3 -632 1842 0 -1 0.78087 //TX1\n"
                              "( -646 1397.33116 1362.08442 ) ( -646 1511.81782 1304.84109 ) ( -518 1397.33116 1362.08442 ) column01_3 646 1562 0 1 0.89443 //TX1\n"
                              "( -637.50000 1436 1338 ) ( -637.50000 1436 1466 ) ( -637.50000 1308 1338 ) column01_3 1436 1338 0 -1 1 //TX1\n"
                              "( -637 1438.91806 1338.87292 ) ( -637 1367.91644 1445.37534 ) ( -509 1438.91806 1338.87292 ) column01_3 637 1609 0 1 0.83205 //TX1\n"
                              "( -637 1440.50000 1338 ) ( -637 1440.50000 1466 ) ( -637 1568.50000 1338 ) column01_3 -1440 1338 0 1 1 //TX1\n"
                              "( -638 1435.27452 1340.35014 ) ( -638 1312.19946 1375.51444 ) ( -510 1435.27452 1340.35014 ) column01_3 638 -1493 0 1 -0.96152 //TX1\n"
                              "}");
            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1207", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1207
            const std::string data("{\n"
                              "( -635.50000 1442.50000 1353.50012 ) ( -763.50000 1442.50000 1353.50012 ) ( -635.50000 1314.50000 1353.50012 ) column01_3 1442 635 -90 1 -1 //TX1\n"
                              "( -635.50000 1442.50000 1355 ) ( -507.50000 1442.50000 1355 ) ( -635.50000 1314.50000 1355 ) column01_3 1442 -635 -90 1 1 //TX1\n"
                              "( -636 1442.50000 1354 ) ( -636 1442.50000 1482 ) ( -764 1442.50000 1354 ) column01_3 -636 1354 0 -1 1 //TX1\n"
                              "( -636 1438 1354 ) ( -636 1438 1482 ) ( -636 1310 1354 ) column01_3 1438 1354 0 -1 1 //TX1\n"
                              "( -635.50000 1438 1354 ) ( -635.50000 1438 1482 ) ( -507.50000 1438 1354 ) column01_3 636 1354 0 1 1 //TX1\n"
                              "( -635.50000 1442.50000 1354 ) ( -635.50000 1442.50000 1482 ) ( -635.50000 1570.50000 1354 ) column01_3 -1442 1354 0 1 1 //TX1\n"
                              "}\n");
            assertCannotSnap(data);
        }

        TEST_CASE("BrushTest.snapIssue1232", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1232
            const std::string data("{\n"
                              "  ( 2152.22540 381.27455 2072 ) ( 2152.22540 381.27455 2200 ) ( 2020.34268 513.15633 2072 ) wbord05 2089 2072 0 -1.03033 1 //TX1\n"
                              "  ( 2042 335.61771 2072 ) ( 2042 335.61771 2200 ) ( 2042 522.12738 2072 ) wbord05 -230 2072 0 1.45711 1 //TX1\n"
                              "  ( 1948.74515 374.24515 2072 ) ( 1948.74515 374.24515 2200 ) ( 2080.62741 506.12741 2072 ) wbord05 -363 2072 0 1.03033 1 //TX1\n"
                              "  ( 1916.74515 451.50000 2072 ) ( 1916.74515 451.50000 2200 ) ( 2103.25482 451.50000 2072 ) wbord05 -1315 2072 0 1.45711 1 //TX1\n"
                              "  ( 2043.56919 493.06919 2026.43074 ) ( 1969.66841 419.16841 2100.33167 ) ( 2134.07889 402.55957 2026.43079 ) kjwall2 -1096 -2197 -44.99997 1 -0.81650 //TX1\n"
                              "  ( 2028.72645 441.39868 2036.31307 ) ( 2140.35950 385.25273 2064.05640 ) ( 2063.24398 543.87358 2104.80712 ) kjwall2 -1262 1843 71.38448 0.84478 -0.96653 //TX1\n"
                              "  ( 1980.74480 497.22377 2022.51040 ) ( 2011.04246 392.71223 2089.91507 ) ( 2093.59579 549.47972 2052.80842 ) kjwall2 -2065 453 24.84662 0.97158 -0.84038 //TX1\n"
                              "  ( 2026.09563 451.97825 2028.19126 ) ( 1995.79798 556.48977 2095.59597 ) ( 1913.24475 399.72220 2058.48949 ) kjwall2 2088 -525 204.84669 0.97158 -0.84038 //TX1\n"
                              "  ( 1994 515.89878 2035.80067 ) ( 1994 401.41210 2093.04401 ) ( 2122 515.89859 2035.80028 ) kjwall2 -1994 -577 -0.00009 1 -0.89443 //TX1\n"
                              "  ( 2010 443.10126 2035.80060 ) ( 2010 557.58793 2093.04394 ) ( 1881.99999 443.10145 2035.80021 ) kjwall2 2010 495 179.99991 1 -0.89443 //TX1\n"
                              "  ( 2018.70638 436.61696 2056.35332 ) ( 2119.11026 375.11218 2106.55513 ) ( 2073.71821 548.87185 2083.85853 ) kjwall2 -1311 1770 63.89229 0.97664 -0.91582 //TX1\n"
                              "  ( 2034 453.83437 2044 ) ( 1982.79994 568.32105 2069.59989 ) ( 1931.59947 396.59103 2095.19895 ) kjwall2 2179 -611 209.20580 0.91652 -0.97590 //TX1\n"
                              "  ( 2018 507.50000 2072 ) ( 2018 507.50000 2200 ) ( 1831.49033 507.50000 2072 ) wbord05 1385 2072 0 -1.45711 1 //TX1\n"
                              "  ( 1986 530.12743 2072 ) ( 1986 530.12743 2200 ) ( 1986 343.61775 2072 ) wbord05 364 2072 0 -1.45711 1 //TX1\n"
                              "  ( 2010 479.50000 2072 ) ( 2010 607.50000 2072 ) ( 2138 479.50000 2072 ) kjwall2 -2010 480 0 1 1 //TX1\n"
                              "  ( 2010 479.50000 2060 ) ( 2010 351.50000 2060 ) ( 2138 479.50000 2060 ) kjwall2 -2010 -480 0 1 -1 //TX1\n"
                              "  ( 2013.31371 518.81371 2072 ) ( 2013.31371 518.81371 2200 ) ( 1881.43146 386.93146 2072 ) wbord05 504 2072 0 -1.03033 1 //TX1\n"
                              "  ( 1941.71572 511.78427 2072 ) ( 1941.71572 511.78427 2200 ) ( 2073.59785 379.90191 2072 ) wbord05 497 2072 0 -1.03033 1 //TX1\n"
                              " }\n");

            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1395_24202", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1395 brush at line 24202
            const std::string data("{\n"
                              "( -4 -325 952 ) ( -16 -356 1032 ) ( -44 -309 1016 ) rock3_8 -1.28601 -6.46194 113.395 0.943603 1.06043\n"
                              "( -17.57635498046875 -263.510009765625 988.9852294921875 ) ( -137.5655517578125 -375.941162109375 743.296875 ) ( 34.708740234375 -300.228759765625 1073.855712890625 ) rock3_8 -1.28595 -6.46191 113.395 0.943603 1.06043\n"
                              "( -135.7427978515625 -370.1265869140625 739.753173828125 ) ( -15.768181800842285 -257.6954345703125 985.42547607421875 ) ( -449.98324584960937 -364.254638671875 589.064697265625 ) rock3_8 -26.8653 -10.137 25.6205 1.15394 -1\n"
                              "( -399.50726318359375 -406.7877197265625 677.47894287109375 ) ( -137.5655517578125 -375.941162109375 743.296875 ) ( -451.79229736328125 -370.0692138671875 592.6083984375 ) rock3_8 26.1202 -7.68527 81.5004 0.875611 -1\n"
                              "( -280.1622314453125 -291.92608642578125 924.623779296875 ) ( -18.227519989013672 -261.07952880859375 990.43829345703125 ) ( -227.88420104980469 -328.64483642578125 1009.49853515625 ) rock3_8 -28.9783 0.638519 81.5019 0.875609 -1\n"
                              "( -195.9036865234375 -282.3568115234375 876.8590087890625 ) ( -143.6192626953125 -319.08740234375 961.7213134765625 ) ( -368.19818115234375 -358.08740234375 546.27716064453125 ) rock3_8 -25.9692 -19.1265 113.395 0.943603 1.06043\n"
                              "( -276.88287353515625 -332.21014404296875 930.47674560546875 ) ( -449.17929077148437 -407.92318725585937 599.90850830078125 ) ( -14.952971458435059 -301.37832641601562 996.28533935546875 ) rock3_8 -20.4888 -8.56413 -87.0938 1.30373 1.02112\n"
                              "( 37.161830902099609 -335.35406494140625 1080.605712890625 ) ( -135.12174987792969 -411.084716796875 750.062744140625 ) ( -224.79318237304687 -366.23345947265625 1014.8262329101562 ) rock3_8 8.91101 4.43578 -87.0938 1.30373 1.02112\n"
                              "( -290.354736328125 -397.304931640625 703.53790283203125 ) ( -470.618896484375 -265.4686279296875 632.53790283203125 ) ( -400.5767822265625 -391.6395263671875 703.53790283203125 ) rock3_8 8.25781 -11.1122 -165 0.865994 1\n"
                              "( -96 -299 1019 ) ( -96 -171 1019 ) ( 50 -400 1017 ) rock3_8 -28.9783 0.638519 81.5019 0.875609 -1\n"
                              "}\n");

            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapIssue1395_18995", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1395 brush at line 24202
            const std::string data("{\n"
                              "( 335 891 680 ) ( 314 881 665 ) ( 451 826 680 ) wswamp1_2 2 0 0 1 1\n"
                              "( 450 813 671 ) ( 451 826 680 ) ( 446 807 665 ) wswamp1_2 2 0 0 1 1\n"
                              "( 451 826 680 ) ( 314 881 665 ) ( 446 807 665 ) wswamp1_2 2 0 0 1 1\n"
                              "( 446 807 665 ) ( 446 754 665 ) ( 450 813 671 ) wswamp1_2 2 0 0 1 1\n"
                              "( 446 754 680 ) ( 451 826 680 ) ( 446 754 665 ) wswamp1_2 2 0 0 1 1\n"
                              "( 313 880 680 ) ( 310 879 677 ) ( 335 891 680 ) wswamp1_2 -16 0 0 1 1\n"
                              "( 304 876 670 ) ( 312 880 665 ) ( 310 879 677 ) wswamp1_2 -16 0 0 1 1\n"
                              "( 314 881 665 ) ( 335 891 680 ) ( 310 879 677 ) wswamp1_2 -16 0 0 1 1\n"
                              "( 330 754 667 ) ( 328 754 665 ) ( 342 757 680 ) wswamp1_2 2 0 0 1 1\n"
                              "( 342 757 680 ) ( 328 754 665 ) ( 310 879 677 ) wswamp1_2 2 0 0 1 1\n"
                              "( 304 876 670 ) ( 310 879 677 ) ( 328 754 665 ) wswamp1_2 2 0 0 1 1\n"
                              "( 312 823 665 ) ( 304 876 670 ) ( 328 754 665 ) wswamp1_2 2 0 0 1 1\n"
                              "( 310.50375366210937 879.1187744140625 676.45660400390625 ) ( 313.50375366210937 880.1187744140625 679.45660400390625 ) ( 342.50375366210937 757.1187744140625 679.45660400390625 ) wswamp1_2 2 0 0 1 1\n"
                              "( 308.35256958007812 876 676.95867919921875 ) ( 316.35256958007813 823 671.95867919921875 ) ( 316.35256958007813 880 671.95867919921875 ) wswamp1_2 2 0 0 1 1\n"
                              "( 342 757 680 ) ( 446 754 680 ) ( 330 754 667 ) wswamp1_2 -16 0 0 1 1\n"
                              "( 446 754 665 ) ( 328 754 665 ) ( 446 754 680 ) wswamp1_2 -16 0 0 1 1\n"
                              "( 446 754 680 ) ( 342 757 680 ) ( 451 826 680 ) wswamp1_2 -16 -2 0 1 1\n"
                              "( 446 754 665 ) ( 446 807 665 ) ( 328 754 665 ) wswamp1_2 -16 -2 0 1 1\n"
                              "}\n"
                              "\n");

            assertSnapToInteger(data);
        }

        TEST_CASE("BrushTest.snapToGrid64", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1415
            const std::string data("{\n"
                              "    ( 400 224 272 ) ( 416 272 224 ) ( 304 224 224 ) techrock 128 -0 -0 1 1\n"
                              "    ( 416 448 224 ) ( 416 272 224 ) ( 400 448 272 ) techrock 64 -0 -0 1 1\n"
                              "    ( 304 272 32 ) ( 304 832 48 ) ( 304 272 48 ) techrock 64 -0 -0 1 1\n"
                              "    ( 304 448 224 ) ( 416 448 224 ) ( 304 448 272 ) techrock 128 0 0 1 1\n"
                              "    ( 400 224 224 ) ( 304 224 224 ) ( 400 224 272 ) techrock 128 -0 -0 1 1\n"
                              "    ( 352 272 272 ) ( 400 832 272 ) ( 400 272 272 ) techrock 128 -64 -0 1 1\n"
                              "    ( 304 448 224 ) ( 304 224 224 ) ( 416 448 224 ) techrock 128 -64 0 1 1\n"
                              "}\n");

            // Seems reasonable for this to fail to snap to grid 64; it's only 48 units tall.
            // If it was able to snap, that would be OK too.
            assertCannotSnapTo(data, 64.0);
        }

        TEST_CASE("BrushNodeTest.moveEdgesFail_2361", "[BrushNodeTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2361

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const std::string data = R"(
{
( -5706.7302804991996 648 1090 ) ( -5730.7302769049566 730 1096 ) ( -5706.730280499035 722 1076 ) so_b4b 103.27 45.1201 180 1 1
( -5706.730280499035 722 1076 ) ( -5702.7302804990386 720 1070 ) ( -5706.7302804991996 648 1090 ) so_b4b -0 -0 -0 1 1
( -5706.730280499035 722 1076 ) ( -5712.7302804990295 720 1048 ) ( -5702.7302804990386 720 1070 ) so_b4b -1.26953 -0 -0 1 1
( -5734.7302804990386 722 1030 ) ( -5712.7302804990295 720 1048 ) ( -5730.7302769049566 730 1096 ) so_b4b -1.27002 -0 -0 1 1
( -5730.7302769049566 730 1096 ) ( -5712.7302804990295 720 1048 ) ( -5706.730280499035 722 1076 ) so_b4b -1.27002 -0 -0 1 1
( -5748.7302591202761 732 1100 ) ( -5730.7302769049566 730 1096 ) ( -5748.7302877821612 732 1104 ) so_b4b 99.2695 -56 180 1 1
( -5698.730280504652 488 1068 ) ( -5702.7302804975034 490 1062 ) ( -5736.7302805013023 496 1034 ) so_b4b -1.26953 -0 -0 1 1
( -5736.7302805013023 496 1034 ) ( -5702.7302804975034 490 1062 ) ( -5724.7302804992714 496 1042 ) so_b4b -1.26953 -46.4615 -0 1 1
( -5764.7302805002028 494 1030 ) ( -5698.730280504652 488 1068 ) ( -5736.7302805013023 496 1034 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7298890995526 484 1089.9989236411209 ) ( -5698.730280504652 488 1068 ) ( -5706.7298897136052 484 1086 ) so_b4b -21.27 -56 -0 1 -1
( -5730.7302804539777 574 1108 ) ( -5706.7302804991996 648 1090 ) ( -5706.7302804735173 484 1090 ) so_b4b -41.2695 6 -0 1 1
( -5702.7302804990386 720 1070 ) ( -5698.7302805012414 644 1068 ) ( -5706.7302804991996 648 1090 ) so_b4b -0 -0 -0 1 1
( -5748.7302877821612 732 1104 ) ( -5737.7302809186394 649 1108 ) ( -5772.7302805004565 732 1108 ) so_b4b -1.27002 -0 -0 1 1
( -5698.7302805012414 644 1068 ) ( -5698.730280504652 488 1068 ) ( -5706.7298890995526 484 1089.9989236411209 ) so_b4b 88 102 180 1 -1
( -5730.7302769049566 730 1096 ) ( -5737.7302809186394 649 1108 ) ( -5748.7302877821612 732 1104 ) so_b4b -1.27002 -0 -0 1 1
( -5706.7302804991996 648 1090 ) ( -5737.7302809186394 649 1108 ) ( -5730.7302769049566 730 1096 ) so_b4b -1.27002 -0 -0 1 1
( -5730.7302804539777 574 1108 ) ( -5737.7302809186394 649 1108 ) ( -5706.7302804991996 648 1090 ) so_b4b -41.27 2 -0 1 -1
( -5712.7302804990295 720 1048 ) ( -5698.7302805012414 644 1068 ) ( -5702.7302804990386 720 1070 ) so_b4b -0 -0 -0 1 1
( -5736.7302805013023 496 1034 ) ( -5734.7302804990195 638 1030 ) ( -5764.7302805002028 494 1030 ) so_b4b -1.26953 -0 -0 1 1
( -5710.7302804925348 636 1048 ) ( -5734.7302804990195 638 1030 ) ( -5724.7302804992714 496 1042 ) so_b4b -37.2695 6 -0 1 1
( -5734.7302804990386 722 1030 ) ( -5734.7302804990195 638 1030 ) ( -5710.7302804925348 636 1048 ) so_b4b -37.2695 6 -0 1 1
( -5724.7302804992714 496 1042 ) ( -5734.7302804990195 638 1030 ) ( -5736.7302805013023 496 1034 ) so_b4b -1.27002 -0 -0 1 1
( -5698.7302805012414 644 1068 ) ( -5710.7302804925348 636 1048 ) ( -5698.730280504652 488 1068 ) so_b4b -0 -0 -0 1 1
( -5698.730280504652 488 1068 ) ( -5710.7302804925348 636 1048 ) ( -5702.7302804975034 490 1062 ) so_b4b -0 -0 -0 1 1
( -5702.7302804975034 490 1062 ) ( -5710.7302804925348 636 1048 ) ( -5724.7302804992714 496 1042 ) so_b4b 103.232 -3.37415 -0 1 1
( -5734.7302804990386 722 1030 ) ( -5710.7302804925348 636 1048 ) ( -5712.7302804990295 720 1048 ) so_b4b 123.169 60.1836 -0 1 -1
( -5712.7302804990295 720 1048 ) ( -5710.7302804925348 636 1048 ) ( -5698.7302805012414 644 1068 ) so_b4b -0 -0 -0 1 1
( -5798.7302805036807 726 1034 ) ( -5842.7302804987194 726 1064 ) ( -5816.730280497547 724 1042 ) so_b4b -1.26953 -0 -0 1 1
( -5812.7302805003801 728 1108 ) ( -5834.7302796346403 726 1090 ) ( -5844.7302535491081 726 1070 ) so_b4b -1.26953 -0 -0 1 1
( -5832.7303385250107 490 1048 ) ( -5820.1222267769954 489.3996339666582 1040.4425460703619 ) ( -5808.7308828738051 492 1030 ) so_b4b 67.2695 -25.828 180 1 1
( -5814.730293347111 490 1096 ) ( -5832.7304033635619 490 1052.0010103828347 ) ( -5840.7302607871034 494 1072 ) so_b4b -1.26953 -0 -0 1 1
( -5840.7302607871034 494 1072 ) ( -5832.7304033635619 490 1052.0010103828347 ) ( -5832.7303385250107 490 1048 ) so_b4b 87.2695 34 180 1 -1
( -5814.730293347111 490 1096 ) ( -5836.7302804990259 642 1090 ) ( -5812.7302804995788 644 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5812.7302804995788 644 1108 ) ( -5836.7302804990259 642 1090 ) ( -5812.7302805003801 728 1108 ) so_b4b 63.2695 12 180 1 -1
( -5812.7302805003801 728 1108 ) ( -5836.7302804990259 642 1090 ) ( -5834.7302796346403 726 1090 ) so_b4b 119.763 -82.8022 -0 1 1
( -5834.7302796346403 726 1090 ) ( -5836.7302804990259 642 1090 ) ( -5844.7302535491081 726 1070 ) so_b4b -50 102 -0 1 1
( -5844.7302535491081 726 1070 ) ( -5836.7302804990259 642 1090 ) ( -5844.7303163465958 646 1070 ) so_b4b -50 102 -0 1 1
( -5844.7303163465958 646 1070 ) ( -5836.7302804990259 642 1090 ) ( -5840.7302607871034 494 1072 ) so_b4b -0 -0 -0 1 1
( -5840.7302607871034 494 1072 ) ( -5836.7302804990259 642 1090 ) ( -5814.730293347111 490 1096 ) so_b4b 111.887 -21.6224 -0 1 1
( -5812.7302804995788 644 1108 ) ( -5802.7302869960968 490 1104 ) ( -5814.730293347111 490 1096 ) so_b4b -1.27002 -0 -0 1 1
( -5774.7302805949284 488 1108 ) ( -5802.7302869960968 490 1104 ) ( -5812.7302804995788 644 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5832.7301202365989 642 1048 ) ( -5832.7303385250107 490 1048 ) ( -5808.7308828738051 492 1030 ) so_b4b 67.2695 -120 180 1 1
( -5832.7301202365989 642 1048 ) ( -5808.7308828738051 492 1030 ) ( -5808.7302827027843 640 1030 ) so_b4b 67.2695 -120 180 1 1
( -5832.7301202365989 642 1048 ) ( -5842.7302804987194 726 1064 ) ( -5844.73018187052 726 1066 ) so_b4b -85.6646 31.4945 -0 1 1
( -5816.730280497547 724 1042 ) ( -5842.7302804987194 726 1064 ) ( -5832.7301202365989 642 1048 ) so_b4b -1.26953 -0 -0 1 1
( -5844.73018187052 726 1066 ) ( -5832.7303385250107 490 1048 ) ( -5832.7301202365989 642 1048 ) so_b4b -0 -0 -0 1 1
( -5816.730280497547 724 1042 ) ( -5808.7302827027843 640 1030 ) ( -5798.7302805036807 726 1034 ) so_b4b -1.27002 -0 -0 1 1
( -5840.7302231706126 494 1068 ) ( -5832.7303385250107 490 1048 ) ( -5844.7302185478356 645.99772419238377 1066 ) so_b4b -0 -0 -0 1 1
( -5808.7302827027843 640 1030 ) ( -5774.7302970963183 726 1030 ) ( -5798.7302805036807 726 1034 ) so_b4b -1.27002 -0 -0 1 1
( -5832.7301202365989 642 1048 ) ( -5808.7302827027843 640 1030 ) ( -5816.730280497547 724 1042 ) so_b4b -1.26953 -0 -0 1 1
( -5844.7302185478356 645.99772419238377 1066 ) ( -5844.73018187052 726 1066 ) ( -5844.7302535491081 726 1070 ) so_b4b 56 12 270 1 1
( -5844.7302185478356 645.99772419238377 1066 ) ( -5844.7303163465958 646 1070 ) ( -5840.7302607871034 494 1072 ) so_b4b -0 -0 -0 1 1
( -5734.7302804990386 722 1030 ) ( -5730.7302769049566 730 1096 ) ( -5774.7302970963183 726 1030 ) so_b4b -1.26953 -0 -0 1 1
( -5774.7302970963183 726 1030 ) ( -5730.7302769049566 730 1096 ) ( -5748.7302591202761 732 1100 ) so_b4b -1.26953 -0 -0 1 1
( -5748.7302877821612 732 1104 ) ( -5772.7302805004565 732 1108 ) ( -5772.7302088121833 732 1104 ) so_b4b 95.2695 -56 180 1 1
( -5772.7302088121833 732 1104 ) ( -5772.7302805004565 732 1108 ) ( -5844.7302535491081 726 1070 ) so_b4b -1.26953 -0 -0 1 1
( -5798.7302805036807 726 1034 ) ( -5774.7302970963183 726 1030 ) ( -5748.7302591202761 732 1100 ) so_b4b -1.26953 -0 -0 1 1
( -5844.73018187052 726 1066 ) ( -5842.7302804987194 726 1064 ) ( -5772.7302088121833 732 1104 ) so_b4b -1.27002 -0 -0 1 1
( -5772.7302088121833 732 1104 ) ( -5842.7302804987194 726 1064 ) ( -5798.7302805036807 726 1034 ) so_b4b -1.26953 -0 -0 1 1
( -5772.7302805004565 732 1108 ) ( -5812.7302805003801 728 1108 ) ( -5844.7302535491081 726 1070 ) so_b4b -1.26953 -0 -0 1 1
( -5814.730293347111 490 1096 ) ( -5802.7302869960968 490 1104 ) ( -5774.7302805949284 488 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5820.1222267769954 489.3996339666582 1040.4425460703619 ) ( -5698.730280504652 488 1068 ) ( -5808.7308828738051 492 1030 ) so_b4b -1.27002 -0 -0 1 1
( -5808.7308828738051 492 1030 ) ( -5698.730280504652 488 1068 ) ( -5764.7302805002028 494 1030 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7298897136052 484 1086 ) ( -5698.730280504652 488 1068 ) ( -5820.1222267769954 489.3996339666582 1040.4425460703619 ) so_b4b -1.27002 -0 -0 1 1
( -5832.7303385250107 490 1048 ) ( -5832.7304033635619 490 1052.0010103828347 ) ( -5820.1222267769954 489.3996339666582 1040.4425460703619 ) so_b4b -1.26953 -0 -0 1 1
( -5774.7302805949284 488 1108 ) ( -5832.7304033635619 490 1052.0010103828347 ) ( -5814.730293347111 490 1096 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7362814612115 484 1090.0045006603091 ) ( -5774.7302805949284 488 1108 ) ( -5730.7302804977789 486 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7362814612115 484 1090.0045006603091 ) ( -5832.7304033635619 490 1052.0010103828347 ) ( -5774.7302805949284 488 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5772.7302805004565 732 1108 ) ( -5737.7302809186394 649 1108 ) ( -5730.7302804539777 574 1108 ) so_b4b -37.27 6 -0 1 1
( -5764.7302805002028 494 1030 ) ( -5734.7302804990195 638 1030 ) ( -5734.7302804990386 722 1030 ) so_b4b -33.27 6 -0 1 1
}
)";

            IO::TestParserStatus status;

            auto nodes = IO::NodeReader::read(data, world, worldBounds, status);
            REQUIRE(nodes.size() == 1u);

            Brush brush = static_cast<BrushNode*>(nodes.front())->brush();

            const auto vertex1 = brush.findClosestVertexPosition(vm::vec3(-5774.7302805949275, 488.0, 1108.0));
            const auto vertex2 = brush.findClosestVertexPosition(vm::vec3(-5730.730280440197,  486.0, 1108.0));
            const auto segment = vm::segment3(vertex1, vertex2);

            ASSERT_TRUE(brush.canMoveEdges(worldBounds, std::vector<vm::segment3>{ segment }, vm::vec3(0.0, -4.0, 0.0)));
            ASSERT_NO_THROW(brush.moveEdges(worldBounds, std::vector<vm::segment3>{ segment }, vm::vec3(0.0, -4.0, 0.0)));

            kdl::col_delete_all(nodes);
        }

        TEST_CASE("BrushTest.moveFaceFailure_1499", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1499

            const vm::vec3 p1(-4408.0, 16.0, 288.0);
            const vm::vec3 p2(-4384.0, 40.0, 288.0);
            const vm::vec3 p3(-4384.0, 64.0, 288.0);
            const vm::vec3 p4(-4416.0, 64.0, 288.0);
            const vm::vec3 p5(-4424.0, 48.0, 288.0); // left back  top
            const vm::vec3 p6(-4424.0, 16.0, 288.0); // left front top
            const vm::vec3 p7(-4416.0, 64.0, 224.0);
            const vm::vec3 p8(-4384.0, 64.0, 224.0);
            const vm::vec3 p9(-4384.0, 40.0, 224.0);
            const vm::vec3 p10(-4408.0, 16.0, 224.0);
            const vm::vec3 p11(-4424.0, 16.0, 224.0);
            const vm::vec3 p12(-4424.0, 48.0, 224.0);

            std::vector<vm::vec3> points;
            points.push_back(p1);
            points.push_back(p2);
            points.push_back(p3);
            points.push_back(p4);
            points.push_back(p5);
            points.push_back(p6);
            points.push_back(p7);
            points.push_back(p8);
            points.push_back(p9);
            points.push_back(p10);
            points.push_back(p11);
            points.push_back(p12);

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            Brush brush = builder.createBrush(points, "asdf").value();

            std::vector<vm::vec3> topFacePos;
            topFacePos.push_back(p1);
            topFacePos.push_back(p2);
            topFacePos.push_back(p3);
            topFacePos.push_back(p4);
            topFacePos.push_back(p5);
            topFacePos.push_back(p6);

            const vm::polygon3 topFace(topFacePos);

            ASSERT_TRUE(brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(+16.0, 0.0, 0.0)));
            ASSERT_TRUE(brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(-16.0, 0.0, 0.0)));
            ASSERT_TRUE(brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(0.0, +16.0, 0.0)));
            ASSERT_TRUE(brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(0.0, -16.0, 0.0)));
            ASSERT_TRUE(brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(0.0, 0.0, +16.0)));
            ASSERT_TRUE(brush.canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(0.0, 0.0, -16.0)));
        }
        
        TEST_CASE("BrushTest.convexMergeCrash_2789", "[BrushTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2789
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Valve);

            const auto path = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/test/Model/Brush/curvetut-crash.map");
            const std::string data = IO::Disk::readTextFile(path);
            REQUIRE(!data.empty());

            IO::TestParserStatus status;

            auto nodes = IO::NodeReader::read(data, world, worldBounds, status);
            REQUIRE(!nodes.empty());

            std::vector<vm::vec3> points;
            for (const auto* node : nodes) {
                if (const auto* brushNode = dynamic_cast<const BrushNode*>(node)) {
                    for (const auto* vertex : brushNode->brush().vertices()) {
                        points.push_back(vertex->position());
                    }
                }
            }

            const Polyhedron3 polyhedron(std::move(points));
            const std::vector<vm::vec3> expectedPositions {
                {40.000000, -144.000031, 180.999969},
                {40.000000, -144.000000, -0.000023},
                {55.996799, -111.999001, -0.000018},
                {55.996799, -111.999031, 178.999985},
                {16.000000, -168.000000, -0.000027},
                {16.000000, -168.000031, 183.999969},
                {16.000000, 39.999969, 184.000000},
                {16.000000, 40.000000, 0.000007},
                {-48.000000, 63.996498, 0.000010},
                {-80.000000, 64.000000, 0.000010},
                {-48.000000, -192.000031, 191.999969},
                {-80.000000, -192.000031, 195.999969},
                {-80.000000, -192.000000, -0.000031},
                {-48.000000, -192.000000, -0.000031},
                {-112.000000, 55.999966, 200.000015},
                {-112.000000, 56.000000, 0.000009},
                {-144.000000, 40.000000, 0.000007},
                {-144.000000, 39.999966, 204.000000},
                {-192.000000, -80.000031, 209.999985},
                {-192.000000, -48.000034, 209.999985},
                {-192.000000, -48.000000, -0.000008},
                {-192.000000, -80.000000, -0.000013},
                {-184.000000, -112.000031, 208.999985},
                {-184.000000, -112.000000, -0.000018},
                {-184.000000, -16.000034, 209.000000},
                {-184.000000, -16.000000, -0.000003},
                {-168.000000, -144.000031, 206.999969},
                {-168.000000, -144.000000, -0.000023},
                {-168.000000, 15.999967, 207.000000},
                {-168.000000, 16.000000, 0.000003},
                {-144.000000, -168.000031, 203.999969},
                {-144.000000, -168.000000, -0.000027},
                {-112.000000, -184.000031, 199.999969},
                {-112.000000, -184.000000, -0.000030},
                {-80.000000, 63.999969, 196.000015},
                {-48.000000, 63.996468, 192.000015},
                {-16.000000, -184.000031, 187.999969},
                {-16.000000, -184.000000, -0.000030},
                {-16.001301, 55.996799, 0.000009},
                {-16.001301, 55.996769, 188.000015},
                {40.000000, 15.999970, 181.000000},
                {40.000000, 16.000000, 0.000003},
                {56.000000, -16.000029, 179.000000},
                {56.000000, -16.000000, -0.000003},
                {63.996498, -80.000031, 177.999985},
                {63.996498, -80.000000, -0.000013},
                {64.000000, -48.000000, -0.000008},
                {64.000000, -48.000031, 177.999985},
            };
            // NOTE: The above was generated by manually cleaning up the output
            // in Blender. It's a 24-sided cylinder.
            // We currently generate some extra vertices/faces, so just check
            // that all vertices in the cleaned-up expected output exist in the
            // computed output.
            for (const auto& position : expectedPositions) {
                CHECK(polyhedron.hasVertex(position, 0.01));
            }

            kdl::col_delete_all(nodes);
        }

        TEST_CASE("BrushTest.convexMergeIncorrectResult_2789", "[BrushTest]") {
            // weirdcurvemerge.map from https://github.com/TrenchBroom/TrenchBroom/issues/2789
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Valve);

            const auto path = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/test/Model/Brush/weirdcurvemerge.map");
            const std::string data = IO::Disk::readTextFile(path);
            REQUIRE(!data.empty());

            IO::TestParserStatus status;

            const std::vector<Node*> nodes = IO::NodeReader::read(data, world, worldBounds, status);
            REQUIRE(nodes.size() == 28);

            std::vector<vm::vec3> points;
            for (const auto* node : nodes) {
                const auto* brushNode = dynamic_cast<const BrushNode*>(node);
                REQUIRE(brushNode != nullptr);
                for (const auto* vertex : brushNode->brush().vertices()) {
                    points.push_back(vertex->position());
                }
            }

            const Polyhedron3 polyhedron(std::move(points));

            // The result should be a 24-sided cylinder
            CHECK(polyhedron.faceCount() == 26);
            CHECK(polyhedron.edgeCount() == 72);
            CHECK(polyhedron.vertexCount() == 48);
            const std::vector<vm::vec3> expectedPositions {
                {383.997, -959.993, 875.0},
                {383.997,  959.993, 592.0},
                {383.997,  959.993, 875.0},
                {128.0,  -1024.0,   624.0},
                {128.0,  -1024.0,   907.0},
                {128.0,   1023.99,  624.0},
                {-1024.0, -128.0,   768.0},
                {-1024.0, -128.0,  1051.0},
                {-1024.0,  128.0,   768.0},
                {-1024.0,  128.0,  1051.0},
                {-960.0,  -384.0,   760.0},
                {-960.0,  -384.0,  1043.0},
                {-960.0,   384.0,   760.0},
                {-960.0,   384.0,  1043.0},
                {-832.0,  -640.0,   744.0},
                {-832.0,  -640.0,  1027.0},
                {-832.0,   640.0,   744.0},
                {-832.0,   640.0,  1027.0},
                {-640.0,  -832.0,   720.0},
                {-640.0,  -832.0,  1003.0},
                {-640.0,   832.0,   720.0},
                {-640.0,   832.0,  1003.0},
                {-384.0,  -960.0,   688.0},
                {-384.0,  -960.0,   971.0},
                {-384.0,   960.0,   688.0},
                {-384.0,   960.0,   971.0},
                {-128.0, -1024.0,   656.0},
                {-128.0, -1024.0,   939.0},
                {-128.0,  1023.99,  656.0},
                {-128.0,  1023.99,  939.0},
                {128.0,   1023.99,  907.0},
                {383.997, -959.993, 592.0},
                {640.0,   -832.0,   560.0},
                {640.0,   -832.0,   843.0},
                {640.0,    832.0,   560.0},
                {640.0,    832.0,   843.0},
                {832.0,   -640.0,   536.0},
                {832.0,   -640.0,   819.0},
                {832.0,    640.0,   536.0},
                {832.0,    640.0,   819.0},
                {960.0,   -384.0,   520.0},
                {960.0,   -384.0,   803.0},
                {960.0,    384.0,   520.0},
                {960.0,    384.0,   803.0},
                {1024.0,  -128.0,   512.0},
                {1024.0,  -128.0,   795.0},
                {1024.0,   128.0,   512.0},
                {1024.0,   128.0,   795.0}
            };
            CHECK(polyhedron.hasAllVertices(expectedPositions, 0.01));

            kdl::col_delete_all(nodes);
        }

        TEST_CASE("BrushTest.subtractCuboidFromCuboid", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const std::string minuendTexture("minuend");
            const std::string subtrahendTexture("subtrahend");
            const std::string defaultTexture("default");

            BrushBuilder builder(&world, worldBounds);
            const Brush minuend = builder.createCuboid(vm::bbox3(vm::vec3(-32.0, -16.0, -32.0), vm::vec3(32.0, 16.0, 32.0)), minuendTexture).value();
            const Brush subtrahend = builder.createCuboid(vm::bbox3(vm::vec3(-16.0, -32.0, -64.0), vm::vec3(16.0, 32.0, 0.0)), subtrahendTexture).value();

            const std::vector<Brush> result = minuend.subtract(world, worldBounds, defaultTexture, subtrahend).value();
            ASSERT_EQ(3u, result.size());

            const Brush* left = nullptr;
            const Brush* top = nullptr;
            const Brush* right = nullptr;

            for (const Brush& brush : result) {
                if (brush.findFace(vm::plane3(32.0, vm::vec3::neg_x()))) {
                    left = &brush;
                } else if (brush.findFace(vm::plane3(32.0, vm::vec3::pos_x()))) {
                    right = &brush;
                } else if (brush.findFace(vm::plane3(16.0, vm::vec3::neg_x()))) {
                    top = &brush;
                }
            }

            ASSERT_TRUE(left != nullptr);
            ASSERT_TRUE(top != nullptr);
            ASSERT_TRUE(right != nullptr);

            // left brush faces
            ASSERT_EQ(6u, left->faceCount());
            ASSERT_TRUE(left->findFace(vm::plane3(-16.0, vm::vec3::pos_x())));
            ASSERT_TRUE(left->findFace(vm::plane3(+32.0, vm::vec3::neg_x())));
            ASSERT_TRUE(left->findFace(vm::plane3(+16.0, vm::vec3::pos_y())));
            ASSERT_TRUE(left->findFace(vm::plane3(+16.0, vm::vec3::neg_y())));
            ASSERT_TRUE(left->findFace(vm::plane3(+32.0, vm::vec3::pos_z())));
            ASSERT_TRUE(left->findFace(vm::plane3(+32.0, vm::vec3::neg_z())));

            // left brush textures
            ASSERT_EQ(subtrahendTexture, left->face(*left->findFace(vm::vec3::pos_x())).attributes().textureName());
            ASSERT_EQ(minuendTexture, left->face(*left->findFace(vm::vec3::neg_x())).attributes().textureName());
            ASSERT_EQ(minuendTexture, left->face(*left->findFace(vm::vec3::pos_y())).attributes().textureName());
            ASSERT_EQ(minuendTexture, left->face(*left->findFace(vm::vec3::neg_y())).attributes().textureName());
            ASSERT_EQ(minuendTexture, left->face(*left->findFace(vm::vec3::pos_z())).attributes().textureName());
            ASSERT_EQ(minuendTexture, left->face(*left->findFace(vm::vec3::neg_z())).attributes().textureName());

            // top brush faces
            ASSERT_EQ(6u, top->faceCount());
            ASSERT_TRUE(top->findFace(vm::plane3(+16.0, vm::vec3::pos_x())));
            ASSERT_TRUE(top->findFace(vm::plane3(+16.0, vm::vec3::neg_x())));
            ASSERT_TRUE(top->findFace(vm::plane3(+16.0, vm::vec3::pos_y())));
            ASSERT_TRUE(top->findFace(vm::plane3(+16.0, vm::vec3::neg_y())));
            ASSERT_TRUE(top->findFace(vm::plane3(+32.0, vm::vec3::pos_z())));
            ASSERT_TRUE(top->findFace(vm::plane3(0.0, vm::vec3::neg_z())));

            // top brush textures
            ASSERT_EQ(defaultTexture, top->face(*top->findFace(vm::vec3::pos_x())).attributes().textureName());
            ASSERT_EQ(defaultTexture, top->face(*top->findFace(vm::vec3::neg_x())).attributes().textureName());
            ASSERT_EQ(minuendTexture, top->face(*top->findFace(vm::vec3::pos_y())).attributes().textureName());
            ASSERT_EQ(minuendTexture, top->face(*top->findFace(vm::vec3::neg_y())).attributes().textureName());
            ASSERT_EQ(minuendTexture, top->face(*top->findFace(vm::vec3::pos_z())).attributes().textureName());
            ASSERT_EQ(subtrahendTexture, top->face(*top->findFace(vm::vec3::neg_z())).attributes().textureName());

            // right brush faces
            ASSERT_EQ(6u, right->faceCount());
            ASSERT_TRUE(right->findFace(vm::plane3(+32.0, vm::vec3::pos_x())));
            ASSERT_TRUE(right->findFace(vm::plane3(-16.0, vm::vec3::neg_x())));
            ASSERT_TRUE(right->findFace(vm::plane3(+16.0, vm::vec3::pos_y())));
            ASSERT_TRUE(right->findFace(vm::plane3(+16.0, vm::vec3::neg_y())));
            ASSERT_TRUE(right->findFace(vm::plane3(+32.0, vm::vec3::pos_z())));
            ASSERT_TRUE(right->findFace(vm::plane3(+32.0, vm::vec3::neg_z())));

            // right brush textures
            ASSERT_EQ(minuendTexture, right->face(*right->findFace(vm::vec3::pos_x())).attributes().textureName());
            ASSERT_EQ(subtrahendTexture, right->face(*right->findFace(vm::vec3::neg_x())).attributes().textureName());
            ASSERT_EQ(minuendTexture, right->face(*right->findFace(vm::vec3::pos_y())).attributes().textureName());
            ASSERT_EQ(minuendTexture, right->face(*right->findFace(vm::vec3::neg_y())).attributes().textureName());
            ASSERT_EQ(minuendTexture, right->face(*right->findFace(vm::vec3::pos_z())).attributes().textureName());
            ASSERT_EQ(minuendTexture, right->face(*right->findFace(vm::vec3::neg_z())).attributes().textureName());
        }

        TEST_CASE("BrushTest.subtractDisjoint", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const vm::bbox3 brush1Bounds(vm::vec3::fill(-8.0), vm::vec3::fill(+8.0));
            const vm::bbox3 brush2Bounds(vm::vec3(124.0, 124.0, -4.0), vm::vec3(132.0, 132.0, +4.0));
            ASSERT_FALSE(brush1Bounds.intersects(brush2Bounds));

            BrushBuilder builder(&world, worldBounds);
            const Brush brush1 = builder.createCuboid(brush1Bounds, "texture").value();
            const Brush brush2 = builder.createCuboid(brush2Bounds, "texture").value();

            const std::vector<Brush> result = brush1.subtract(world, worldBounds, "texture", brush2).value();
            ASSERT_EQ(1u, result.size());

            const Brush& subtraction = result.at(0);
            ASSERT_COLLECTIONS_EQUIVALENT(brush1.vertexPositions(), subtraction.vertexPositions());
        }

        TEST_CASE("BrushTest.subtractEnclosed", "[BrushTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Entity(), MapFormat::Standard);

            const vm::bbox3 brush1Bounds(vm::vec3::fill(-8.0), vm::vec3::fill(+8.0));
            const vm::bbox3 brush2Bounds(vm::vec3::fill(-9.0), vm::vec3::fill(+9.0));
            ASSERT_TRUE(brush1Bounds.intersects(brush2Bounds));

            BrushBuilder builder(&world, worldBounds);
            const Brush brush1 = builder.createCuboid(brush1Bounds, "texture").value();
            const Brush brush2 = builder.createCuboid(brush2Bounds, "texture").value();

            const std::vector<Brush> result = brush1.subtract(world, worldBounds, "texture", brush2).value();
            ASSERT_EQ(0u, result.size());
        }

        TEST_CASE("BrushTest.subtractTruncatedCones", "[BrushTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1469

            const std::string minuendStr(R"({
                ( 29.393876913416079 -16.970562748463635 32 ) ( 16.970562748495468 29.393876913411077 32 ) ( 11.313708499003496 19.595917942278447 -16 ) __TB_empty [ -0.258819 0.965926 0 -0.507559 ] [ -0.158797 -0.0425496 -0.986394 -0.257094 ] -0 1 1
                ( 32.784609690844263 -8.784609690813113 32 ) ( 8.7846096908451727 32.784609690839488 32 ) ( 5.856406460569815 21.856406460564131 -16 ) __TB_empty [ -0.5 0.866025 0 -0.77533 ] [ -0.142374 -0.0821995 -0.986394 -0.0887003 ] -0 1 1
                ( 33.94112549697229 -0 32 ) ( -0 33.941125496967288 32 ) ( -0 22.627416997982664 -16 ) __TB_empty [ -0.707107 0.707107 0 -0.176551 ] [ -0.116248 -0.116248 -0.986394 -0.46579 ] -0 1 1
                ( 32.784609690844718 8.7846096908399431 32 ) ( -8.7846096908083382 32.784609690839488 32 ) ( -5.8564064605325257 21.856406460564131 -16 ) __TB_empty [ -0.866025 0.5 0 -0.0124664 ] [ -0.0821995 -0.142374 -0.986394 -0.870919 ] -0 1 1
                ( 29.393876913416534 16.970562748490465 32 ) ( -16.970562748458633 29.393876913411304 32 ) ( -11.313708498966207 19.595917942278675 -16 ) __TB_empty [ -0.965926 0.258819 0 -0.373029 ] [ -0.0425496 -0.158797 -0.986394 -0.805874 ] -0 1 1
                ( -11.313708498966662 -19.595917942252527 -16 ) ( -16.970562748458633 -29.393876913384929 32 ) ( 29.393876913416079 -16.970562748463635 32 ) __TB_empty [ -0.0425496 0.158797 -0.986394 -0.30125 ] [ -0.965926 -0.258819 0 -0.00242329 ] -0 1 1
                ( -5.8564064605325257 -21.85640646053821 -16 ) ( -8.7846096908078835 -32.784609690813113 32 ) ( 32.784609690844263 -8.784609690813113 32 ) __TB_empty [ -0.0821995 0.142374 -0.986394 -0.474954 ] [ -0.866025 -0.5 0 -0.0709991 ] -0 1 1
                ( -0 -22.627416997956516 -16 ) ( -0 -33.941125496940913 32 ) ( 33.94112549697229 -0 32 ) __TB_empty [ -0.116248 0.116248 -0.986394 -0.298004 ] [ -0.707107 -0.707107 0 -0.689445 ] -0 1 1
                ( 5.856406460569815 -21.856406460537755 -16 ) ( 8.7846096908451727 -32.784609690813113 32 ) ( 32.784609690844718 8.7846096908399431 32 ) __TB_empty [ -0.142374 0.0821995 -0.986394 -0.219636 ] [ -0.5 -0.866025 0 -0.872314 ] -0 1 1
                ( 11.313708499003496 -19.595917942252072 -16 ) ( 16.970562748495922 -29.393876913384702 32 ) ( 29.393876913416534 16.970562748490465 32 ) __TB_empty [ -0.158797 0.0425496 -0.986394 -0.818881 ] [ -0.258819 -0.965926 0 -0.590811 ] -0 1 1
                ( 16 -16 -16 ) ( 24 -24 32 ) ( 24 24 32 ) __TB_empty [ -0.164399 0 -0.986394 -0.283475 ] [ 0 -1 0 -0 ] -0 1 1
                ( 16.970562748495468 29.393876913411077 32 ) ( -29.3938769133797 16.970562748490465 32 ) ( -19.595917942246615 11.313708498997812 -16 ) __TB_empty [ -0.0425496 0.158797 0.986394 0.0475388 ] [ -0.965926 -0.258819 0 -0.238751 ] -0 1 1
                ( 8.7846096908451727 32.784609690839488 32 ) ( -32.784609690807883 8.7846096908399431 32 ) ( -21.856406460532071 5.8564064605641306 -16 ) __TB_empty [ -0.0821995 0.142374 0.986394 -0.902102 ] [ -0.866025 -0.5 0 -0.660111 ] -0 1 1
                ( -0 33.941125496967288 32 ) ( -33.941125496935911 -0 32 ) ( -22.627416997950604 -0 -16 ) __TB_empty [ -0.116248 0.116248 0.986394 -0.50108 ] [ -0.707107 -0.707107 0 -0.631095 ] -0 1 1
                ( -8.7846096908083382 32.784609690839488 32 ) ( -32.784609690807883 -8.7846096908135678 32 ) ( -21.856406460532071 -5.8564064605377553 -16 ) __TB_empty [ -0.142374 0.0821995 0.986394 -0.198669 ] [ -0.5 -0.866025 0 -0.166748 ] -0 1 1
                ( -16.970562748458633 29.393876913411304 32 ) ( -29.393876913379245 -16.970562748463863 32 ) ( -19.595917942246615 -11.313708498971437 -16 ) __TB_empty [ -0.158797 0.0425496 0.986394 -0.573831 ] [ -0.258819 -0.965926 0 -0.238028 ] -0 1 1
                ( -29.3938769133797 16.970562748490465 32 ) ( -16.970562748458633 -29.393876913384929 32 ) ( -11.313708498966662 -19.595917942252527 -16 ) __TB_empty [ -0.258819 0.965926 0 -0.271353 ] [ -0.158797 -0.0425496 0.986394 -0.908333 ] -0 1 1
                ( -32.784609690807883 8.7846096908399431 32 ) ( -8.7846096908078835 -32.784609690813113 32 ) ( -5.8564064605325257 -21.85640646053821 -16 ) __TB_empty [ -0.5 0.866025 0 -0.18634 ] [ -0.142374 -0.0821995 0.986394 -0.51593 ] -0 1 1
                ( -33.941125496935911 -0 32 ) ( -0 -33.941125496940913 32 ) ( -0 -22.627416997956516 -16 ) __TB_empty [ -0.707107 0.707107 0 -0.234839 ] [ -0.116248 -0.116248 0.986394 -0.668957 ] -0 1 1
                ( -32.784609690807883 -8.7846096908135678 32 ) ( 8.7846096908451727 -32.784609690813113 32 ) ( 5.856406460569815 -21.856406460537755 -16 ) __TB_empty [ -0.866025 0.5 0 -0.717973 ] [ -0.0821995 -0.142374 0.986394 -0.849948 ] -0 1 1
                ( -29.393876913379245 -16.970562748463863 32 ) ( 16.970562748495922 -29.393876913384702 32 ) ( 11.313708499003496 -19.595917942252072 -16 ) __TB_empty [ -0.965926 0.258819 0 -0.72569 ] [ -0.0425496 -0.158797 0.986394 -0.560825 ] -0 1 1
                ( -24 24 32 ) ( -24 -24 32 ) ( -16 -16 -16 ) __TB_empty [ -0.164399 0 0.986394 -0.81431 ] [ 0 -1 0 -0 ] -0 1 1
                ( 24 24 32 ) ( -24 24 32 ) ( -16 16 -16 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 -0.986394 -0.827715 ] -0 1 1
                ( -24 -24 32 ) ( 24 -24 32 ) ( 16 -16 -16 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 0.986394 0.641451 ] -0 1 1
                ( 24 24 32 ) ( 24 -24 32 ) ( -24 -24 32 ) __TB_empty [ 1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1
                ( -16 -16 -16 ) ( 16 16 -16 ) ( -16 16 -16 ) __TB_empty [ -1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1
            })");

            const std::string subtrahendStr(R"({
                ( 29.393876913416079 -16.970562748463635 48 ) ( 16.970562748495468 29.393876913411077 48 ) ( 11.313708499003496 19.595917942278447 -0 ) __TB_empty [ -0.258819 0.965926 0 -0.507559 ] [ -0.158797 -0.0425496 -0.986394 -0.474791 ] -0 1 1
                ( 32.784609690844263 -8.784609690813113 48 ) ( 8.7846096908451727 32.784609690839488 48 ) ( 5.856406460569815 21.856406460564131 -0 ) __TB_empty [ -0.5 0.866025 0 -0.77533 ] [ -0.142374 -0.0821995 -0.986394 -0.306396 ] -0 1 1
                ( 33.94112549697229 -0 48 ) ( -0 33.941125496967288 48 ) ( -0 22.627416997982664 -0 ) __TB_empty [ -0.707107 0.707107 0 -0.176551 ] [ -0.116248 -0.116248 -0.986394 -0.683485 ] -0 1 1
                ( 32.784609690844718 8.7846096908399431 48 ) ( -8.7846096908083382 32.784609690839488 48 ) ( -5.8564064605325257 21.856406460564131 -0 ) __TB_empty [ -0.866025 0.5 0 -0.0124664 ] [ -0.0821995 -0.142374 -0.986394 -0.0886002 ] -0 1 1
                ( 29.393876913416534 16.970562748490465 48 ) ( -16.970562748458633 29.393876913411304 48 ) ( -11.313708498966207 19.595917942278675 -0 ) __TB_empty [ -0.965926 0.258819 0 -0.373029 ] [ -0.0425496 -0.158797 -0.986394 -0.0235691 ] -0 1 1
                ( -11.313708498966662 -19.595917942252527 -0 ) ( -16.970562748458633 -29.393876913384929 48 ) ( 29.393876913416079 -16.970562748463635 48 ) __TB_empty [ -0.0425496 0.158797 -0.986394 -0.5189 ] [ -0.965926 -0.258819 0 -0.00242329 ] -0 1 1
                ( -5.8564064605325257 -21.85640646053821 -0 ) ( -8.7846096908078835 -32.784609690813113 48 ) ( 32.784609690844263 -8.784609690813113 48 ) __TB_empty [ -0.0821995 0.142374 -0.986394 -0.692604 ] [ -0.866025 -0.5 0 -0.0709991 ] -0 1 1
                ( -0 -22.627416997956516 -0 ) ( -0 -33.941125496940913 48 ) ( 33.94112549697229 -0 48 ) __TB_empty [ -0.116248 0.116248 -0.986394 -0.515699 ] [ -0.707107 -0.707107 0 -0.689445 ] -0 1 1
                ( 5.856406460569815 -21.856406460537755 -0 ) ( 8.7846096908451727 -32.784609690813113 48 ) ( 32.784609690844718 8.7846096908399431 48 ) __TB_empty [ -0.142374 0.0821995 -0.986394 -0.437332 ] [ -0.5 -0.866025 0 -0.872314 ] -0 1 1
                ( 11.313708499003496 -19.595917942252072 -0 ) ( 16.970562748495922 -29.393876913384702 48 ) ( 29.393876913416534 16.970562748490465 48 ) __TB_empty [ -0.158797 0.0425496 -0.986394 -0.0365772 ] [ -0.258819 -0.965926 0 -0.590811 ] -0 1 1
                ( 16 -16 -0 ) ( 24 -24 48 ) ( 24 24 48 ) __TB_empty [ -0.164399 0 -0.986394 -0.501169 ] [ 0 -1 0 -0 ] -0 1 1
                ( 16.970562748495468 29.393876913411077 48 ) ( -29.3938769133797 16.970562748490465 48 ) ( -19.595917942246615 11.313708498997812 -0 ) __TB_empty [ -0.0425496 0.158797 0.986394 0.265238 ] [ -0.965926 -0.258819 0 -0.238751 ] -0 1 1
                ( 8.7846096908451727 32.784609690839488 48 ) ( -32.784609690807883 8.7846096908399431 48 ) ( -21.856406460532071 5.8564064605641306 -0 ) __TB_empty [ -0.0821995 0.142374 0.986394 -0.684406 ] [ -0.866025 -0.5 0 -0.660111 ] -0 1 1
                ( -0 33.941125496967288 48 ) ( -33.941125496935911 -0 48 ) ( -22.627416997950604 -0 -0 ) __TB_empty [ -0.116248 0.116248 0.986394 -0.283369 ] [ -0.707107 -0.707107 0 -0.631095 ] -0 1 1
                ( -8.7846096908083382 32.784609690839488 48 ) ( -32.784609690807883 -8.7846096908135678 48 ) ( -21.856406460532071 -5.8564064605377553 -0 ) __TB_empty [ -0.142374 0.0821995 0.986394 -0.980953 ] [ -0.5 -0.866025 0 -0.166748 ] -0 1 1
                ( -16.970562748458633 29.393876913411304 48 ) ( -29.393876913379245 -16.970562748463863 48 ) ( -19.595917942246615 -11.313708498971437 -0 ) __TB_empty [ -0.158797 0.0425496 0.986394 -0.35615 ] [ -0.258819 -0.965926 0 -0.238028 ] -0 1 1
                ( -29.3938769133797 16.970562748490465 48 ) ( -16.970562748458633 -29.393876913384929 48 ) ( -11.313708498966662 -19.595917942252527 -0 ) __TB_empty [ -0.258819 0.965926 0 -0.271353 ] [ -0.158797 -0.0425496 0.986394 -0.690683 ] -0 1 1
                ( -32.784609690807883 8.7846096908399431 48 ) ( -8.7846096908078835 -32.784609690813113 48 ) ( -5.8564064605325257 -21.85640646053821 -0 ) __TB_empty [ -0.5 0.866025 0 -0.18634 ] [ -0.142374 -0.0821995 0.986394 -0.298214 ] -0 1 1
                ( -33.941125496935911 -0 48 ) ( -0 -33.941125496940913 48 ) ( -0 -22.627416997956516 -0 ) __TB_empty [ -0.707107 0.707107 0 -0.234839 ] [ -0.116248 -0.116248 0.986394 -0.451246 ] -0 1 1
                ( -32.784609690807883 -8.7846096908135678 48 ) ( 8.7846096908451727 -32.784609690813113 48 ) ( 5.856406460569815 -21.856406460537755 -0 ) __TB_empty [ -0.866025 0.5 0 -0.717973 ] [ -0.0821995 -0.142374 0.986394 -0.632298 ] -0 1 1
                ( -29.393876913379245 -16.970562748463863 48 ) ( 16.970562748495922 -29.393876913384702 48 ) ( 11.313708499003496 -19.595917942252072 -0 ) __TB_empty [ -0.965926 0.258819 0 -0.72569 ] [ -0.0425496 -0.158797 0.986394 -0.343115 ] -0 1 1
                ( -24 24 48 ) ( -24 -24 48 ) ( -16 -16 -0 ) __TB_empty [ -0.164399 0 0.986394 -0.596628 ] [ 0 -1 0 -0 ] -0 1 1
                ( 24 24 48 ) ( -24 24 48 ) ( -16 16 -0 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 -0.986394 -0.0454121 ] -0 1 1
                ( -24 -24 48 ) ( 24 -24 48 ) ( 16 -16 -0 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 0.986394 0.859102 ] -0 1 1
                ( 24 24 48 ) ( 24 -24 48 ) ( -24 -24 48 ) __TB_empty [ 1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1
                ( -16 -16 -0 ) ( 16 16 -0 ) ( -16 16 -0 ) __TB_empty [ -1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1
            })");

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Valve);

            IO::TestParserStatus status;
            const std::vector<Node*> minuendNodes = IO::NodeReader::read(minuendStr, world, worldBounds, status);
            const std::vector<Node*> subtrahendNodes = IO::NodeReader::read(subtrahendStr, world, worldBounds, status);

            const Brush& minuend = static_cast<BrushNode*>(minuendNodes.front())->brush();
            const Brush& subtrahend = static_cast<BrushNode*>(subtrahendNodes.front())->brush();

            const std::vector<Brush> result = minuend.subtract(world, worldBounds, "some_texture", subtrahend).value();
            ASSERT_FALSE(result.empty());

            kdl::col_delete_all(minuendNodes);
            kdl::col_delete_all(subtrahendNodes);
        }

        TEST_CASE("BrushTest.subtractDome", "[BrushTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2707

            const std::string minuendStr(R"({
                ( -1598.09391534391647838 -277.57717407067275417 -20 ) ( -1598.09391534391647838 54.02274375211438695 -20 ) ( -1598.09391534391647838 -277.57717407067275417 -12 ) 128_gold_2 -14.94120025634765625 -108 -0 0.72087001800537109 1
                ( -1178.96031746031826515 -277.57717407067275417 -20 ) ( -1598.09391534391647838 -277.57717407067275417 -20 ) ( -1178.96031746031826515 -277.57717407067275417 -12 ) 128_gold_2 28.92790031433105469 -108 -0 0.8250659704208374 1
                ( -1178.96031746031826515 54.02274375211438695 -20 ) ( -1598.09391534391647838 54.02274375211438695 -20 ) ( -1178.96031746031826515 -277.57717407067275417 -20 ) 128_gold_2 -28.98690032958984375 -4.01778984069824219 -0 0.77968800067901611 0.65970498323440552
                ( -1178.96031746031826515 -277.57717407067275417 -12 ) ( -1598.09391534391647838 -277.57717407067275417 -12 ) ( -1178.96031746031826515 54.02274375211438695 -12 ) 128_gold_2 -28.98690032958984375 -4.01778984069824219 -0 0.77968800067901611 0.65970498323440552
                ( -1598.09391534391647838 54.02274375211438695 -20 ) ( -1178.96031746031826515 54.02274375211438695 -20 ) ( -1598.09391534391647838 54.02274375211438695 -12 ) 128_gold_2 28.92790031433105469 -108 -0 0.8250659704208374 1
                ( -1178 54.02274375211438695 -20 ) ( -1178 -277.57717407067275417 -20 ) ( -1178 54.02274375211438695 -12 ) 128_gold_2 -14.94120025634765625 -108 -0 0.72087001800537109 1
            })");


            const auto subtrahendPath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/test/Model/Brush/subtrahend.map");
            std::ifstream stream = openPathAsInputStream(subtrahendPath);
            std::stringstream subtrahendStr;
            subtrahendStr << stream.rdbuf();

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;
            const std::vector<Node*> minuendNodes = IO::NodeReader::read(minuendStr, world, worldBounds, status);
            const std::vector<Node*> subtrahendNodes = IO::NodeReader::read(subtrahendStr.str(), world, worldBounds, status);

            const Brush& minuend = static_cast<BrushNode*>(minuendNodes.front())->brush();
            const Brush& subtrahend = static_cast<BrushNode*>(subtrahendNodes.front())->brush();

            const auto result = minuend.subtract(world, worldBounds, "some_texture", subtrahend);

            kdl::col_delete_all(minuendNodes);
            kdl::col_delete_all(subtrahendNodes);
        }

        TEST_CASE("BrushTest.subtractPipeFromCubeWithMissingFragments", "[BrushTest]") {
            // see https://github.com/TrenchBroom/TrenchBroom/pull/1764#issuecomment-296341588
            // subtract creates missing fragments

            const std::string minuendStr("{\n"
                                    "( -64 -64 -48 ) ( -64 -63 -48 ) ( -64 -64 -47 ) __TB_empty -0 -0 -0 1 1\n"
                                    "( 64 64 -16 ) ( 64 64 -15 ) ( 64 65 -16 ) __TB_empty -0 -0 -0 1 1\n"
                                    "( -64 -64 -48 ) ( -64 -64 -47 ) ( -63 -64 -48 ) __TB_empty -0 -0 -0 1 1\n"
                                    "( 64 64 -16 ) ( 65 64 -16 ) ( 64 64 -15 ) __TB_empty -0 -0 -0 1 1\n"
                                    "( 64 64 48 ) ( 64 65 48 ) ( 65 64 48 ) __TB_empty -0 -0 -0 1 1\n"
                                    "( -64 -64 -48 ) ( -63 -64 -48 ) ( -64 -63 -48 ) __TB_empty -0 -0 -0 1 1\n"
                                    "}\n");

            const std::string subtrahendStr("{\n"
                                       "( 174.71990352490863074 -62.14359353944905706 75.16563707012221585 ) ( 175.1529162268008406 -62.39359353944905706 76.03166247390666399 ) ( 175.60378700139182229 -61.83740732160116238 74.81208367952893923 ) __TB_empty 0.78229904174804688 -0.29628753662109375 338.198577880859375 0.95197159051895142 0.96824586391448975\n"
                                       "( 36.41270357552525638 -34.54767559718354875 115.33507514292870155 ) ( 36.84571627741747335 -34.79767559718354875 116.2011005467131497 ) ( 36.58948027082188759 -35.46623425072723279 114.98152175233542494 ) __TB_empty -0.04352569580078125 0.71729850769042969 201.0517425537109375 0.98425096273422241 -0.90138787031173706\n"
                                       "( 199.8900184844443686 -128.93134736624534753 80.25103299325476769 ) ( 200.77390196092756014 -128.62516114839746706 79.89747960266149107 ) ( 200.0667951797410069 -129.84990601978904579 79.89747960266149107 ) __TB_empty -0.59069061279296875 -0.1404876708984375 280.89337158203125 0.93541437387466431 0.93541431427001953\n"
                                       "( -116.00776749053582648 53.45232440281647257 -189.5058669891937484 ) ( -115.83099079523915975 52.53376574927277431 -189.85942037978702501 ) ( -115.12388401405260652 53.75851062066436725 -189.85942037978702501 ) __TB_empty -0.02112197875976562 -0.22997283935546875 280.89337158203125 0.93541437387466431 0.93541431427001953\n"
                                       "( 72.6107978708658095 -94.6384909672807737 153.79013823665565042 ) ( 145.00698646154697258 -136.4364499384135172 253.32768142207908113 ) ( 89.58136061934294503 -104.43644993841348878 142.47642973767091235 ) __TB_empty 0.93064975738525391 -0.637969970703125 326.3099365234375 1.27475488185882568 0.96824580430984497\n"
                                       "( 69.78237074611962498 -79.94155251058168687 159.44699248614801945 ) ( 81.0960792451044199 -60.34563456831627803 159.44699248614801945 ) ( 136.52170508730841902 -92.34563456831628514 270.29824417055618824 ) __TB_empty 0.81418228149414062 0.05062103271484375 -0 1.22474479675292969 0.90138781070709229\n"
                                       "( 81.0960792451044199 -60.34563456831627803 159.44699248614801945 ) ( 95.23821486883537091 -55.4466550827499276 153.79013823665565042 ) ( 150.66384071103937003 -87.44665508274994181 264.6413899210638192 ) __TB_empty 0.67885684967041016 -0.27746772766113281 338.198577880859375 0.95197159051895142 0.96824586391448975\n"
                                       "( 95.23821486883537091 -55.4466550827499276 153.79013823665565042 ) ( 112.20877761731250644 -65.24461405388265689 142.47642973767091235 ) ( 167.63440345951653399 -97.2446140538826711 253.32768142207908113 ) __TB_empty 0.16141700744628906 -0.67490577697753906 326.3099365234375 1.27475488185882568 0.96824580430984497\n"
                                       "( 112.20877761731250644 -65.24461405388265689 142.47642973767091235 ) ( 115.03720474205866253 -79.9415525105817153 136.81957548817854331 ) ( 170.46283058426269008 -111.94155251058172951 247.67082717258671209 ) __TB_empty -0.30159759521484375 0.28987884521484375 201.0517425537109375 0.98425096273422241 -0.90138787031173706\n"
                                       "( 115.03720474205866253 -79.9415525105817153 136.81957548817854331 ) ( 103.72349624307389604 -99.53747045284714545 136.81957548817854331 ) ( 159.14912208527792359 -131.53747045284714545 247.67082717258671209 ) __TB_empty 0.81418418884277344 0.94775390625 -0 1.22474479675292969 0.90138781070709229\n"
                                       "}\n");


            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Entity(), MapFormat::Standard);

            IO::TestParserStatus status;
            const std::vector<Node*> minuendNodes = IO::NodeReader::read(minuendStr, world, worldBounds, status);
            const std::vector<Node*> subtrahendNodes = IO::NodeReader::read(subtrahendStr, world, worldBounds, status);

            const Brush& minuend = static_cast<BrushNode*>(minuendNodes.front())->brush();
            const Brush& subtrahend = static_cast<BrushNode*>(subtrahendNodes.front())->brush();

            const std::vector<Brush> result = minuend.subtract(world, worldBounds, "some_texture", subtrahend).value();
            ASSERT_EQ(8u, result.size());

            kdl::col_delete_all(minuendNodes);
            kdl::col_delete_all(subtrahendNodes);
        }

        // TODO: add tests for Brush::intersect
    }
}
