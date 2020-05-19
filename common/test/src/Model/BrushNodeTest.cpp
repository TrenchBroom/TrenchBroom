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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "TestUtils.h"

#include "Assets/Texture.h"
#include "IO/DiskIO.h"
#include "IO/NodeReader.h"
#include "IO/Path.h"
#include "IO/TestParserStatus.h"
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushSnapshot.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/MapFormat.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Model/WorldNode.h"

#include <kdl/collection_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/vec.h>
#include <vecmath/segment.h>
#include <vecmath/polygon.h>
#include <vecmath/ray.h>

#include <algorithm>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        std::vector<vm::vec3> asVertexList(const std::vector<vm::segment3>& edges);
        std::vector<vm::vec3> asVertexList(const std::vector<vm::polygon3>& faces);

        TEST_CASE("BrushNodeTest.constructBrushWithRedundantFaces", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);

            std::vector<BrushFace*> faces;
            faces.push_back(BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                      vm::vec3(1.0, 0.0, 0.0),
                                                      vm::vec3(0.0, 1.0, 0.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                      vm::vec3(1.0, 0.0, 0.0),
                                                      vm::vec3(0.0, 1.0, 0.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                      vm::vec3(1.0, 0.0, 0.0),
                                                      vm::vec3(0.0, 1.0, 0.0)));

            ASSERT_THROW(BrushNode(worldBounds, faces), GeometryException);
        }

        TEST_CASE("BrushNodeTest.constructBrushWithFaces", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);

            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                        vm::vec3(0.0, 1.0, 0.0),
                                                        vm::vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(vm::vec3(16.0, 0.0, 0.0),
                                                         vm::vec3(16.0, 0.0, 1.0),
                                                         vm::vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                         vm::vec3(0.0, 0.0, 1.0),
                                                         vm::vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(vm::vec3(0.0, 16.0, 0.0),
                                                        vm::vec3(1.0, 16.0, 0.0),
                                                        vm::vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 16.0),
                                                       vm::vec3(0.0, 1.0, 16.0),
                                                       vm::vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                          vm::vec3(1.0, 0.0, 0.0),
                                                          vm::vec3(0.0, 1.0, 0.0));

            std::vector<BrushFace*> faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);

            BrushNode brush(worldBounds, faces);
            assert(brush.fullySpecified());

            // sort the faces by the weight of their plane normals like QBSP does
            Model::BrushFace::sortFaces(faces);

            const std::vector<BrushFace*>& brushFaces = brush.faces();
            ASSERT_EQ(6u, brushFaces.size());
            for (size_t i = 0; i < faces.size(); i++)
                ASSERT_EQ(faces[i], brushFaces[i]);
        }

        /*
         Regex to turn a face definition into a c++ statement to add a face to a vector of faces:
         Find: \(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*\(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*\(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*[^\n]+
         Replace: faces.push_back(BrushFace::createParaxial(vm::vec3($1, $2, $3), vm::vec3($4, $5, $6), vm::vec3($7, $8, $9)));
         */

        TEST_CASE("BrushNodeTest.constructWithFailingFaces", "[BrushNodeTest]") {
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

            std::vector<BrushFace*> faces;
            faces.push_back(BrushFace::createParaxial(vm::vec3(-192.0, 704.0, 128.0), vm::vec3(-156.0, 650.0, 128.0), vm::vec3(-156.0, 650.0, 160.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-202.0, 604.0, 160.0), vm::vec3(-164.0, 664.0, 128.0), vm::vec3(-216.0, 613.0, 128.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-156.0, 650.0, 128.0), vm::vec3(-202.0, 604.0, 128.0), vm::vec3(-202.0, 604.0, 160.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-192.0, 704.0, 160.0), vm::vec3(-256.0, 640.0, 160.0), vm::vec3(-256.0, 640.0, 128.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-256.0, 640.0, 160.0), vm::vec3(-202.0, 604.0, 160.0), vm::vec3(-202.0, 604.0, 128.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-217.0, 672.0, 160.0), vm::vec3(-161.0, 672.0, 160.0), vm::vec3(-161.0, 603.0, 160.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-161.0, 603.0, 128.0), vm::vec3(-161.0, 672.0, 128.0), vm::vec3(-217.0, 672.0, 128.0)));

            BrushNode brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const std::vector<BrushFace*>& brushFaces = brush.faces();
            ASSERT_EQ(7u, brushFaces.size());
        }

        TEST_CASE("BrushNodeTest.constructWithFailingFaces2", "[BrushNodeTest]") {
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

            std::vector<BrushFace*> faces;
            faces.push_back(BrushFace::createParaxial(vm::vec3(3488.0, 1152.0, 1340.0), vm::vec3(3488.0, 1248.0, 1344.0), vm::vec3(3488.0, 1344.0, 1340.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(3232.0, 1344.0, 1576.0), vm::vec3(3232.0, 1152.0, 1576.0), vm::vec3(3232.0, 1152.0, 1256.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(3488.0, 1344.0, 1576.0), vm::vec3(3264.0, 1344.0, 1576.0), vm::vec3(3264.0, 1344.0, 1256.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(3280.0, 1152.0, 1576.0), vm::vec3(3504.0, 1152.0, 1576.0), vm::vec3(3504.0, 1152.0, 1256.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(3488.0, 1248.0, 1344.0), vm::vec3(3488.0, 1152.0, 1340.0), vm::vec3(3232.0, 1152.0, 1340.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(3488.0, 1248.0, 1344.0), vm::vec3(3232.0, 1248.0, 1344.0), vm::vec3(3232.0, 1344.0, 1340.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(3488.0, 1152.0, 1340.0), vm::vec3(3360.0, 1152.0, 1344.0), vm::vec3(3424.0, 1344.0, 1342.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(3360.0, 1152.0, 1344.0), vm::vec3(3232.0, 1152.0, 1340.0), vm::vec3(3296.0, 1344.0, 1342.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(3504.0, 1344.0, 1280.0), vm::vec3(3280.0, 1344.0, 1280.0), vm::vec3(3280.0, 1152.0, 1280.0)));

            BrushNode brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const std::vector<BrushFace*>& brushFaces = brush.faces();
            ASSERT_EQ(9u, brushFaces.size());
        }

        TEST_CASE("BrushNodeTest.constructWithFailingFaces3", "[BrushNodeTest]") {
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

            std::vector<BrushFace*> faces;
            faces.push_back(BrushFace::createParaxial(vm::vec3(-32.0, -1088.0, 896.0), vm::vec3(-64.0, -1120.0, 896.0), vm::vec3(-64.0, -1120.0, 912.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-32.0, -832.0, 896.0), vm::vec3(-32.0, -1088.0, 896.0), vm::vec3(-32.0, -1088.0, 912.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-64.0, -848.0, 912.0), vm::vec3(-64.0, -1120.0, 912.0), vm::vec3(-64.0, -1120.0, 896.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-32.0, -896.0, 896.0), vm::vec3(-32.0, -912.0, 912.0), vm::vec3(-64.0, -912.0, 912.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-64.0, -1088.0, 912.0), vm::vec3(-64.0, -848.0, 912.0), vm::vec3(-32.0, -848.0, 912.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-64.0, -864.0, 896.0), vm::vec3(-32.0, -864.0, 896.0), vm::vec3(-32.0, -832.0, 896.0)));

            BrushNode brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const std::vector<BrushFace*>& brushFaces = brush.faces();
            ASSERT_EQ(6u, brushFaces.size());
        }

        TEST_CASE("BrushNodeTest.constructWithFailingFaces4", "[BrushNodeTest]") {
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

            std::vector<BrushFace*> faces;
            faces.push_back(BrushFace::createParaxial(vm::vec3(-1268.0, 272.0, 2524.0), vm::vec3(-1268.0, 272.0, 2536.0), vm::vec3(-1268.0, 288.0, 2540.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-1280.0, 265.0, 2534.0), vm::vec3(-1268.0, 272.0, 2524.0), vm::vec3(-1268.0, 288.0, 2528.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-1268.0, 288.0, 2528.0), vm::vec3(-1280.0, 288.0, 2540.0), vm::vec3(-1280.0, 265.0, 2534.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-1268.0, 288.0, 2540.0), vm::vec3(-1280.0, 288.0, 2540.0), vm::vec3(-1280.0, 288.0, 2536.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-1268.0, 265.0, 2534.0), vm::vec3(-1280.0, 265.0, 2534.0), vm::vec3(-1280.0, 288.0, 2540.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-1268.0, 265.0, 2534.0), vm::vec3(-1268.0, 272.0, 2524.0), vm::vec3(-1280.0, 265.0, 2534.0)));

            BrushNode brush(worldBounds, faces);
            const std::vector<BrushFace*>& brushFaces = brush.faces();
            ASSERT_EQ(6u, brushFaces.size());
        }

        TEST_CASE("BrushNodeTest.constructWithFailingFaces5", "[BrushNodeTest]") {
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

            std::vector<BrushFace*> faces;
            faces.push_back(BrushFace::createParaxial(vm::vec3(1296.0, 896.0, 944.0), vm::vec3(1296.0, 1008.0, 1056.0), vm::vec3(1280.0, 1008.0, 1008.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(1296.0, 1008.0, 1168.0), vm::vec3(1296.0, 1008.0, 1056.0), vm::vec3(1296.0, 896.0, 944.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(1280.0, 1008.0, 1008.0), vm::vec3(1280.0, 1008.0, 1168.0), vm::vec3(1280.0, 896.0, 1056.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(1280.0, 1008.0, 1168.0), vm::vec3(1280.0, 1008.0, 1008.0), vm::vec3(1296.0, 1008.0, 1056.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(1296.0, 1008.0, 1168.0), vm::vec3(1296.0, 896.0, 1056.0), vm::vec3(1280.0, 896.0, 1056.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(1280.0, 896.0, 896.0), vm::vec3(1280.0, 896.0, 1056.0), vm::vec3(1296.0, 896.0, 1056.0)));

            BrushNode brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const std::vector<BrushFace*>& brushFaces = brush.faces();
            ASSERT_EQ(6u, brushFaces.size());
        }

        TEST_CASE("BrushNodeTest.constructWithFailingFaces6", "[BrushNodeTest]") {
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

            std::vector<BrushFace*> faces;
            faces.push_back(BrushFace::createParaxial(vm::vec3(-80.0, -80.0, -3840.0), vm::vec3(-80.0, -80.0, -3824.0), vm::vec3(-32.0, -32.0, -3808.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-96.0, -32.0, -3840.0), vm::vec3(-96.0, -32.0, -3824.0), vm::vec3(-80.0, -80.0, -3824.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-96.0, -32.0, -3824.0), vm::vec3(-32.0, -32.0, -3808.0), vm::vec3(-80.0, -80.0, -3824.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-32.0, -32.0, -3840.0), vm::vec3(-32.0, -32.0, -3808.0), vm::vec3(-96.0, -32.0, -3824.0)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-32.0, -32.0, -3840.0), vm::vec3(-96.0, -32.0, -3840.0), vm::vec3(-80.0, -80.0, -3840.0)));

            BrushNode brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const std::vector<BrushFace*>& brushFaces = brush.faces();
            ASSERT_EQ(5u, brushFaces.size());
        }

        TEST_CASE("BrushNodeTest.constructBrushWithManySides", "[BrushNodeTest]") {
            /*
             See https://github.com/kduske/TrenchBroom/issues/1153
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

            std::vector<BrushFace*> faces;
            faces.push_back(BrushFace::createParaxial(vm::vec3(624.0, 688.0, -456.0), vm::vec3(656.0, 760.0, -480.0), vm::vec3(624.0, 680.0, -480.0), "face7"));
            faces.push_back(BrushFace::createParaxial(vm::vec3(536.0, 792.0, -480.0), vm::vec3(536.0, 792.0, -432.0), vm::vec3(488.0, 720.0, -480.0), "face12"));
            faces.push_back(BrushFace::createParaxial(vm::vec3(568.0, 656.0, -464.0), vm::vec3(568.0, 648.0, -480.0), vm::vec3(520.0, 672.0, -456.0), "face14"));
            faces.push_back(BrushFace::createParaxial(vm::vec3(520.0, 672.0, -456.0), vm::vec3(520.0, 664.0, -480.0), vm::vec3(488.0, 720.0, -452.0), "face15"));
            faces.push_back(BrushFace::createParaxial(vm::vec3(560.0, 728.0, -440.0), vm::vec3(488.0, 720.0, -452.0), vm::vec3(536.0, 792.0, -432.0), "face17"));
            faces.push_back(BrushFace::createParaxial(vm::vec3(568.0, 656.0, -464.0), vm::vec3(520.0, 672.0, -456.0), vm::vec3(624.0, 688.0, -456.0), "face19"));
            faces.push_back(BrushFace::createParaxial(vm::vec3(560.0, 728.0, -440.0), vm::vec3(624.0, 688.0, -456.0), vm::vec3(520.0, 672.0, -456.0), "face20"));
            faces.push_back(BrushFace::createParaxial(vm::vec3(600.0, 840.0, -480.0), vm::vec3(536.0, 792.0, -480.0), vm::vec3(636.0, 812.0, -480.0), "face22"));

            BrushNode brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const std::vector<BrushFace*>& brushFaces = brush.faces();
            ASSERT_EQ(8u, brushFaces.size());
        }

        TEST_CASE("BrushNodeTest.constructBrushAfterRotateFail", "[BrushNodeTest]") {
            /*
             See https://github.com/kduske/TrenchBroom/issues/1173

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

            std::vector<BrushFace*> faces;
            faces.push_back(BrushFace::createParaxial(vm::vec3(-729.68857812925364, -128, 2061.2927432882448), vm::vec3(-910.70791411301013, 128, 2242.3120792720015), vm::vec3(-820.19824612113155, -128, 1970.7830752963655)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-639.17891013737574, -640, 1970.7830752963669), vm::vec3(-729.68857812925364, -128, 2061.2927432882448), vm::vec3(-729.68857812925364, -640, 1880.2734073044885)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-639.17891013737574, -1024, 1970.7830752963669), vm::vec3(-820.19824612113177, -640, 2151.8024112801227), vm::vec3(-639.17891013737574, -640, 1970.7830752963669)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-639.17891013737574, -1024, 1970.7830752963669), vm::vec3(-639.17891013737574, -640, 1970.7830752963669), vm::vec3(-729.68857812925364, -1024, 1880.2734073044885)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-1001.2175821048878, -128, 2151.8024112801222), vm::vec3(-910.70791411301013, -128, 2242.3120792720015), vm::vec3(-910.70791411300991, -640, 2061.2927432882443)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-639.17891013737574, -1024, 1970.7830752963669), vm::vec3(-729.68857812925364, -1024, 1880.2734073044885), vm::vec3(-820.19824612113177, -640, 2151.8024112801227))); // assertion failure here
            faces.push_back(BrushFace::createParaxial(vm::vec3(-1001.2175821048878, -128, 2151.8024112801222), vm::vec3(-1001.2175821048878, 128, 2151.8024112801222), vm::vec3(-910.70791411301013, -128, 2242.3120792720015)));
            faces.push_back(BrushFace::createParaxial(vm::vec3(-729.68857812925364, -1024, 1880.2734073044885), vm::vec3(-729.68857812925364, -640, 1880.2734073044885), vm::vec3(-910.70791411300991, -640, 2061.2927432882443)));

            const vm::bbox3 worldBounds(4096.0);
            BrushNode brush(worldBounds, faces);
            ASSERT_TRUE(brush.fullySpecified());
        }

        TEST_CASE("BrushNodeTest.buildBrushFail", "[BrushNodeTest]") {
            /*
             See https://github.com/kduske/TrenchBroom/issues/1186
             This crash was caused by the correction of newly created vertices in Polyhedron::Edge::split - it would nudge vertices such that their plane status changed, resulting in problems when building the seam.
             */

            const std::string data("{\n"
                              "( 656 976 672 ) ( 656 1104 672 ) ( 656 976 800 ) black -976 672 0 1 1 //TX2\n"
                              "( 632 496.00295 640 ) ( 632 688.00137 768 ) ( 504 496.00295 640 ) doortrim2 632 331 0 -1 1.49999 //TX1\n"
                              "( 666.74516 848 928 ) ( 666.74516 826.95693 1054.25842 ) ( 794.74516 848 928 ) woodplank1 -941 667 90 0.98639 -1 //TX2\n"
                              "( 672 880 416 ) ( 672 880 544 ) ( 672 1008 416 ) wswamp2_1 -880 416 0 1 1 //TX1\n"
                              "( 656 754.57864 1021.42136 ) ( -84592 754.57864 1021.42136 ) ( 656 61034.01582 -59258.01582 ) skip 1 2 0 -666 470.93310 //TX2\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            ASSERT_EQ(1u, nodes.size());
        }

        TEST_CASE("BrushNodeTest.buildBrushFail2", "[BrushNodeTest]") {
            /*
             See https://github.com/kduske/TrenchBroom/issues/1185

             The cause for the endless loop was, like above, the vertex correction in Polyhedron::Edge::split.
             */

            const std::string data("{\n"
                              "( 32 1392 960 ) ( 32 1392 1088 ) ( 32 1264 960 ) black 1392 960 0 -1 1 //TX1\n"
                              "( 64 1137.02125 916.65252 ) ( 64 1243.52363 845.65079 ) ( -64 1137.02125 916.65252 ) woodplank1 64 1367 0 -1 0.83205 //TX1\n"
                              "( 5.25484 1296 864 ) ( 5.25484 1317.04307 990.25842 ) ( -122.74516 1296 864 ) woodplank1 -876 -5 90 0.98639 1 //TX2\n"
                              "( 64 1184 819.77710 ) ( 64 1184 947.77710 ) ( 64 1312 819.77710 ) woodplank1 -820 1184 90 1 -1 //TX2\n"
                              "( 16 1389.42136 957.42136 ) ( 85264 1389.42136 957.42136 ) ( 16 -58890.01582 -59322.01582 ) skip 0 -3 0 666 -470.93310 //TX2\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            ASSERT_EQ(1u, nodes.size());
        }

        TEST_CASE("BrushNodeTest.buildBrushFail3", "[BrushNodeTest]") {
            // From https://github.com/kduske/TrenchBroom/issues/1697

            /*
             This brush is broken beyond repair. When building the polyhedron, we run into problems where no seam can be
             computed. We opt to just throw an exception that case and expect it to fail without crashing.
             */

            /*
             Update after fixing issue https://github.com/kduske/TrenchBroom/issues/2611
             With the revised face sort order (sort by normal), this brush can now be built.
             */

            const std::string data("{\n"
                              "( -24 1844 112.527 ) ( -24 1844 112 ) ( -24 1844.27 113.544 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20 1848.53 112.527 ) ( -20 1848.53 112 ) ( -20 1847.47 112.526 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.4797 1844 112.092 ) ( -23.4797 1844 112 ) ( -23.6766 1844 112.421 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -24 1852 112.526 ) ( -24 1852 112 ) ( -23.9258 1852 112.526 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.4559 1851.73 112 ) ( -23.4732 1852 112 ) ( -21.5439 1851.2 112 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.4727 1848.53 116 ) ( -23.4727 1847.47 116 ) ( -24 1848.53 116 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.405 1844.27 113.439 ) ( -23.7974 1844 112.491 ) ( -23.7971 1844.27 113.544 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.7971 1844.27 113.544 ) ( -23.9311 1844 112.527 ) ( -24 1844.27 113.544 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.5781 1845.54 115.201 ) ( -23.6762 1844.8 114.456 ) ( -24 1845.54 115.201 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.5099 1846.46 115.728 ) ( -23.5792 1845.54 115.201 ) ( -24 1846.46 115.727 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.6779 1844.8 114.456 ) ( -23.798 1844.27 113.545 ) ( -24 1844.8 114.456 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.4727 1847.47 116 ) ( -23.5085 1846.46 115.728 ) ( -24 1847.47 116 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 15.9999 ] 90 1 -1\n"
                              "( -23.5786 1850.46 115.201 ) ( -23.5092 1849.54 115.728 ) ( -24 1850.46 115.201 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.9316 1852 112.526 ) ( -23.7979 1851.73 113.545 ) ( -24 1852 112.526 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.5085 1849.54 115.728 ) ( -23.4726 1848.53 116 ) ( -24 1849.54 115.727 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 15.9999 ] 90 1 -1\n"
                              "( -23.4037 1851.73 113.439 ) ( -23.7965 1851.73 113.544 ) ( -23.7975 1852 112.491 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.6777 1851.2 114.457 ) ( -23.5797 1850.46 115.201 ) ( -24 1851.2 114.457 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 1 0 -0 ] 180 1 -1\n"
                              "( -23.7974 1851.73 113.544 ) ( -23.6772 1851.2 114.457 ) ( -24 1851.73 113.544 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.7994 1848.53 114.456 ) ( -20.2725 1848.53 113.544 ) ( -20.7993 1847.47 114.456 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.544 1848.53 115.201 ) ( -20.7995 1848.53 114.456 ) ( -21.5442 1847.47 115.201 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.2725 1848.53 113.544 ) ( -20 1848.53 112.527 ) ( -20.2726 1847.47 113.544 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.4566 1847.47 115.728 ) ( -23.4727 1847.47 116 ) ( -22.4567 1848.53 115.728 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -22.4567 1848.53 115.728 ) ( -21.5439 1848.53 115.201 ) ( -22.4452 1847.46 115.721 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -21.5444 1844.8 112.324 ) ( -21.5444 1844.8 112 ) ( -22.456 1844.27 112.204 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.2727 1846.46 112.491 ) ( -20.2727 1846.46 112 ) ( -20.799 1845.54 112.421 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.799 1845.54 112.421 ) ( -20.799 1845.54 112 ) ( -21.544 1844.8 112.323 ) O_METAL1_19AD [ 0 -1 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.4559 1844.27 112.204 ) ( -22.4559 1844.27 112 ) ( -23.4738 1844 112.07 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20 1847.47 112.527 ) ( -20 1847.47 112 ) ( -20.2727 1846.46 112.491 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5608 1844.27 112.594 ) ( -22.4564 1844.27 112.205 ) ( -23.5091 1844 112.203 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5615 1846.46 115.474 ) ( -22.7649 1845.54 114.983 ) ( -23.5089 1846.46 115.727 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.0512 1844.8 114.288 ) ( -23.677 1844.8 114.456 ) ( -22.7637 1845.54 114.982 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5618 1846.46 115.474 ) ( -23.5086 1846.46 115.727 ) ( -22.4567 1847.47 115.728 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -22.0364 1844.8 113.51 ) ( -21.7108 1844.8 112.946 ) ( -22.7661 1844.27 112.95 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.4392 1845.54 113.966 ) ( -21.0138 1846.47 114.293 ) ( -21.0168 1845.54 113.235 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7646 1844.27 112.948 ) ( -22.5612 1844.27 112.595 ) ( -23.5787 1844 112.323 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.0176 1846.46 114.289 ) ( -20.7995 1847.47 114.456 ) ( -20.5267 1846.46 113.438 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7113 1844.8 112.948 ) ( -21.5438 1844.8 112.323 ) ( -22.5613 1844.27 112.596 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.5159 1846.47 113.427 ) ( -20.27 1846.47 112.503 ) ( -21.0173 1845.54 113.236 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.0171 1845.55 113.237 ) ( -20.7981 1845.55 112.42 ) ( -21.7127 1844.8 112.949 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.527 1846.46 113.439 ) ( -20.2725 1847.47 113.544 ) ( -20.2728 1846.46 112.49 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.0347 1844.8 113.508 ) ( -21.4382 1845.54 113.965 ) ( -21.7115 1844.8 112.948 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.4921 1844.8 113.966 ) ( -22.0342 1844.8 113.508 ) ( -23.0526 1844.27 113.235 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7105 1846.46 114.982 ) ( -21.0178 1846.46 114.289 ) ( -22.0341 1845.54 114.561 ) O_METAL1_19AD [ -1 0 0 -0 ] [ 0 1 0 -0 ] 180 1 -1\n"
                              "( -22.0365 1845.54 114.562 ) ( -21.4377 1845.54 113.964 ) ( -22.4934 1844.8 113.967 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.0525 1844.27 113.235 ) ( -22.7657 1844.27 112.949 ) ( -23.6769 1844 112.422 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7119 1846.46 114.983 ) ( -21.5441 1847.47 115.201 ) ( -21.0172 1846.46 114.288 ) O_METAL1_19AD [ 0 -0 -1 -0 ] [ 0 -1 0 16 ] 90 1 -1\n"
                              "( -23.0525 1844.8 114.29 ) ( -22.4921 1844.8 113.966 ) ( -23.405 1844.27 113.439 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5618 1846.46 115.474 ) ( -21.7115 1846.46 114.983 ) ( -22.7644 1845.54 114.982 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -22.7646 1845.54 114.983 ) ( -22.0349 1845.54 114.561 ) ( -23.0523 1844.8 114.289 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.0523 1844.27 113.235 ) ( -23.6767 1844 112.421 ) ( -23.4045 1844.27 113.439 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5618 1846.46 115.474 ) ( -22.4567 1847.47 115.728 ) ( -21.7115 1846.46 114.983 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.0519 1844.8 114.289 ) ( -23.4042 1844.27 113.438 ) ( -23.6773 1844.8 114.457 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.4733 1852 112.069 ) ( -23.4733 1852 112 ) ( -22.4557 1851.73 112.202 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.2725 1849.54 112.491 ) ( -20.2725 1849.54 112 ) ( -20 1848.53 112.527 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.5438 1851.2 112.324 ) ( -21.5438 1851.2 112 ) ( -20.7997 1850.46 112.422 ) O_METAL1_19AD [ 0 -1 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.4557 1851.73 112.202 ) ( -22.4557 1851.73 112 ) ( -21.5433 1851.2 112.322 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.7995 1850.46 112.421 ) ( -20.7995 1850.46 112 ) ( -20.2725 1849.54 112.491 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.5615 1851.73 112.597 ) ( -23.5097 1852 112.204 ) ( -22.4559 1851.73 112.203 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.5273 1849.54 113.439 ) ( -21.0177 1849.54 114.289 ) ( -21.0178 1850.46 113.236 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7626 1851.73 112.947 ) ( -22.5616 1851.73 112.599 ) ( -22.0352 1851.2 113.507 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.0175 1850.46 113.235 ) ( -21.4388 1850.46 113.965 ) ( -21.7056 1851.19 112.95 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7643 1850.46 114.982 ) ( -23.0516 1851.2 114.289 ) ( -22.0348 1850.46 114.561 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7112 1849.54 114.983 ) ( -21.5439 1848.53 115.201 ) ( -22.562 1849.54 115.474 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -21.4402 1850.46 113.967 ) ( -22.035 1850.46 114.561 ) ( -22.0353 1851.2 113.51 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7646 1850.46 114.983 ) ( -22.5611 1849.54 115.474 ) ( -23.5787 1850.46 115.201 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -23.0504 1851.2 114.289 ) ( -23.6777 1851.2 114.457 ) ( -23.4026 1851.73 113.438 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -23.0511 1851.73 113.236 ) ( -22.7626 1851.73 112.947 ) ( -22.4919 1851.2 113.965 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.456 1848.53 115.727 ) ( -23.4729 1848.53 116 ) ( -22.5611 1849.54 115.474 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -21.7112 1849.54 114.983 ) ( -22.0347 1850.46 114.561 ) ( -21.0175 1849.54 114.289 ) O_METAL1_19AD [ -1 0 0 -0 ] [ 0 1 0 -0 ] 180 1 -1\n"
                              "( -23 1851.73 113.212 ) ( -23.4023 1851.73 113.439 ) ( -23.6625 1852 112.413 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.0177 1849.54 114.289 ) ( -20.7998 1848.53 114.457 ) ( -21.7127 1849.54 114.984 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -22.5611 1849.54 115.474 ) ( -22.7646 1850.46 114.983 ) ( -21.7113 1849.54 114.983 ) O_METAL1_19AD [ -0 -1 0 -0 ] [ -1 0 0 16 ] 90 1 -1\n"
                              "( -22.492 1851.2 113.965 ) ( -23.0499 1851.2 114.288 ) ( -23.051 1851.73 113.236 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7616 1851.73 112.946 ) ( -23.0571 1851.73 113.234 ) ( -23.5769 1852 112.32 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.2724 1849.54 112.491 ) ( -20 1848.53 112.526 ) ( -20.5263 1849.54 113.438 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7101 1851.2 112.947 ) ( -21.543 1851.2 112.323 ) ( -21.0175 1850.46 113.234 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.7109 1851.2 112.947 ) ( -22.5613 1851.73 112.596 ) ( -21.5437 1851.2 112.323 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -21.0173 1850.46 113.235 ) ( -20.7994 1850.46 112.421 ) ( -20.5265 1849.54 113.438 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7611 1851.73 112.945 ) ( -23.5758 1852 112.32 ) ( -22.5621 1851.73 112.596 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -20.5264 1849.54 113.438 ) ( -20.2725 1848.53 113.544 ) ( -21.0175 1849.54 114.289 ) O_METAL1_19AD [ 0 -1 0 -0 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "( -22.7647 1850.46 114.982 ) ( -23.5781 1850.46 115.2 ) ( -23.0501 1851.2 114.289 ) O_METAL1_19AD [ -1 0 0 -16 ] [ 0 0 1 -0 ] 180 1 -1\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Valve);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            ASSERT_EQ(1u, nodes.size());
        }

        TEST_CASE("BrushNodeTest.buildBrushWithShortEdges", "[BrushNodeTest]") {
            /*
             See https://github.com/kduske/TrenchBroom/issues/1194
             */

            const std::string data("{\n"
                              "( -1248 -2144 1168 ) ( -1120 -2144 1168 ) ( -1248 -2272 1168 ) rock_1732 1248 2144 0 1 -1 //TX2\n"
                              "( -1248 -2224 1141.33333 ) ( -1248 -2224 1013.33333 ) ( -1120 -2224 1056 ) rock_1732 1391 -309 -33.69007 1.20185 -0.83205 //TX1\n"
                              "( -1408 -2144 1328 ) ( -1408 -2272 1328 ) ( -1408 -2144 1456 ) rock_1732 -1328 2144 90 1 1 //TX1\n"
                              "( -1472 -2256 1434.66667 ) ( -1472 -2256 1562.66667 ) ( -1344 -2256 1349.33334 ) skip 1681 453 -33.69007 1.20185 0.83205 //TX1\n"
                              "( -1248.00004 -2144 1061.33328 ) ( -1248.00004 -2272 1061.33328 ) ( -1120 -2144 976 ) rock_1732 1248 2144 0 1 -1 //TX1\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            ASSERT_TRUE(nodes.empty());
        }

        TEST_CASE("BrushNodeTest.pick", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);

            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                        vm::vec3(0.0, 1.0, 0.0),
                                                        vm::vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(vm::vec3(16.0, 0.0, 0.0),
                                                         vm::vec3(16.0, 0.0, 1.0),
                                                         vm::vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                         vm::vec3(0.0, 0.0, 1.0),
                                                         vm::vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(vm::vec3(0.0, 16.0, 0.0),
                                                        vm::vec3(1.0, 16.0, 0.0),
                                                        vm::vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 16.0),
                                                       vm::vec3(0.0, 1.0, 16.0),
                                                       vm::vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                          vm::vec3(1.0, 0.0, 0.0),
                                                          vm::vec3(0.0, 1.0, 0.0));

            std::vector<BrushFace*> faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);

            BrushNode brush(worldBounds, faces);

            PickResult hits1;
            brush.pick(vm::ray3(vm::vec3(8.0, -8.0, 8.0), vm::vec3::pos_y()), hits1);
            ASSERT_EQ(1u, hits1.size());

            Hit hit1 = hits1.all().front();
            ASSERT_DOUBLE_EQ(8.0, hit1.distance());
            ASSERT_EQ(front, hitToFace(hit1));

            PickResult hits2;
            brush.pick(vm::ray3(vm::vec3(8.0, -8.0, 8.0), vm::vec3::neg_y()), hits2);
            ASSERT_TRUE(hits2.empty());
        }

        struct MatchFace {
        private:
            const BrushFace& m_face;
        public:
            explicit MatchFace(const BrushFace& face) :
                    m_face(face) {}

            bool operator()(const BrushFace* candidate) const {
                for (size_t i = 0; i < 3; ++i)
                    if (candidate->points()[i] != m_face.points()[i])
                        return false;
                if (candidate->selected() != m_face.selected())
                    return false;
                if (candidate->textureName() != m_face.textureName())
                    return false;
                if (candidate->texture() != m_face.texture())
                    return false;
                if (candidate->xOffset() != m_face.xOffset())
                    return false;
                if (candidate->yOffset() != m_face.yOffset())
                    return false;
                if (candidate->rotation() != m_face.rotation())
                    return false;
                if (candidate->xScale() != m_face.xScale())
                    return false;
                if (candidate->yScale() != m_face.yScale())
                    return false;
                if (candidate->surfaceContents() != m_face.surfaceContents())
                    return false;
                if (candidate->surfaceFlags() != m_face.surfaceFlags())
                    return false;
                if (candidate->surfaceValue() != m_face.surfaceValue())
                    return false;
                return true;
            }
        };

        static void assertHasFace(const BrushNode& brush, const BrushFace& face) {
            const std::vector<BrushFace*>& faces = brush.faces();
            const std::vector<BrushFace*>::const_iterator it = std::find_if(std::begin(faces), std::end(faces), MatchFace(face));
            ASSERT_TRUE(it != std::end(faces));
        }

        TEST_CASE("BrushNodeTest.clone", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);

            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                        vm::vec3(0.0, 1.0, 0.0),
                                                        vm::vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(vm::vec3(16.0, 0.0, 0.0),
                                                         vm::vec3(16.0, 0.0, 1.0),
                                                         vm::vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                         vm::vec3(0.0, 0.0, 1.0),
                                                         vm::vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(vm::vec3(0.0, 16.0, 0.0),
                                                        vm::vec3(1.0, 16.0, 0.0),
                                                        vm::vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 16.0),
                                                       vm::vec3(0.0, 1.0, 16.0),
                                                       vm::vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                          vm::vec3(1.0, 0.0, 0.0),
                                                          vm::vec3(0.0, 1.0, 0.0));

            std::vector<BrushFace*> faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);

            BrushNode original(worldBounds, faces);
            BrushNode* clone = original.clone(worldBounds);

            assertHasFace(*clone, *left);
            assertHasFace(*clone, *right);
            assertHasFace(*clone, *front);
            assertHasFace(*clone, *back);
            assertHasFace(*clone, *top);
            assertHasFace(*clone, *bottom);

            delete clone;
        }

        TEST_CASE("BrushNodeTest.subtractCuboidFromCuboid", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const std::string minuendTexture("minuend");
            const std::string subtrahendTexture("subtrahend");
            const std::string defaultTexture("default");

            BrushBuilder builder(&world, worldBounds);
            BrushNode* minuend = world.createBrush(builder.createCuboid(vm::bbox3(vm::vec3(-32.0, -16.0, -32.0), vm::vec3(32.0, 16.0, 32.0)), minuendTexture));
            BrushNode* subtrahend = world.createBrush(builder.createCuboid(vm::bbox3(vm::vec3(-16.0, -32.0, -64.0), vm::vec3(16.0, 32.0, 0.0)), subtrahendTexture));

            const std::vector<BrushNode*> result = minuend->subtract(world, worldBounds, defaultTexture, subtrahend);
            ASSERT_EQ(3u, result.size());

            BrushNode* left = nullptr;
            BrushNode* top = nullptr;
            BrushNode* right = nullptr;

            for (BrushNode* brush : result) {
                if (brush->findFace(vm::plane3(32.0, vm::vec3::neg_x())) != nullptr)
                    left = brush;
                else if (brush->findFace(vm::plane3(32.0, vm::vec3::pos_x())) != nullptr)
                    right = brush;
                else if (brush->findFace(vm::plane3(16.0, vm::vec3::neg_x())) != nullptr)
                    top = brush;
            }

            ASSERT_TRUE(left != nullptr);
            ASSERT_TRUE(top != nullptr);
            ASSERT_TRUE(right != nullptr);

            // left brush faces
            ASSERT_EQ(6u, left->faceCount());
            ASSERT_TRUE(left->findFace(vm::plane3(-16.0, vm::vec3::pos_x())) != nullptr);
            ASSERT_TRUE(left->findFace(vm::plane3(+32.0, vm::vec3::neg_x())) != nullptr);
            ASSERT_TRUE(left->findFace(vm::plane3(+16.0, vm::vec3::pos_y())) != nullptr);
            ASSERT_TRUE(left->findFace(vm::plane3(+16.0, vm::vec3::neg_y())) != nullptr);
            ASSERT_TRUE(left->findFace(vm::plane3(+32.0, vm::vec3::pos_z())) != nullptr);
            ASSERT_TRUE(left->findFace(vm::plane3(+32.0, vm::vec3::neg_z())) != nullptr);

            // left brush textures
            ASSERT_EQ(subtrahendTexture, left->findFace(vm::vec3::pos_x())->textureName());
            ASSERT_EQ(minuendTexture, left->findFace(vm::vec3::neg_x())->textureName());
            ASSERT_EQ(minuendTexture, left->findFace(vm::vec3::pos_y())->textureName());
            ASSERT_EQ(minuendTexture, left->findFace(vm::vec3::neg_y())->textureName());
            ASSERT_EQ(minuendTexture, left->findFace(vm::vec3::pos_z())->textureName());
            ASSERT_EQ(minuendTexture, left->findFace(vm::vec3::neg_z())->textureName());

            // top brush faces
            ASSERT_EQ(6u, top->faceCount());
            ASSERT_TRUE(top->findFace(vm::plane3(+16.0, vm::vec3::pos_x())) != nullptr);
            ASSERT_TRUE(top->findFace(vm::plane3(+16.0, vm::vec3::neg_x())) != nullptr);
            ASSERT_TRUE(top->findFace(vm::plane3(+16.0, vm::vec3::pos_y())) != nullptr);
            ASSERT_TRUE(top->findFace(vm::plane3(+16.0, vm::vec3::neg_y())) != nullptr);
            ASSERT_TRUE(top->findFace(vm::plane3(+32.0, vm::vec3::pos_z())) != nullptr);
            ASSERT_TRUE(top->findFace(vm::plane3(0.0, vm::vec3::neg_z())) != nullptr);

            // top brush textures
            ASSERT_EQ(defaultTexture, top->findFace(vm::vec3::pos_x())->textureName());
            ASSERT_EQ(defaultTexture, top->findFace(vm::vec3::neg_x())->textureName());
            ASSERT_EQ(minuendTexture, top->findFace(vm::vec3::pos_y())->textureName());
            ASSERT_EQ(minuendTexture, top->findFace(vm::vec3::neg_y())->textureName());
            ASSERT_EQ(minuendTexture, top->findFace(vm::vec3::pos_z())->textureName());
            ASSERT_EQ(subtrahendTexture, top->findFace(vm::vec3::neg_z())->textureName());

            // right brush faces
            ASSERT_EQ(6u, right->faceCount());
            ASSERT_TRUE(right->findFace(vm::plane3(+32.0, vm::vec3::pos_x())) != nullptr);
            ASSERT_TRUE(right->findFace(vm::plane3(-16.0, vm::vec3::neg_x())) != nullptr);
            ASSERT_TRUE(right->findFace(vm::plane3(+16.0, vm::vec3::pos_y())) != nullptr);
            ASSERT_TRUE(right->findFace(vm::plane3(+16.0, vm::vec3::neg_y())) != nullptr);
            ASSERT_TRUE(right->findFace(vm::plane3(+32.0, vm::vec3::pos_z())) != nullptr);
            ASSERT_TRUE(right->findFace(vm::plane3(+32.0, vm::vec3::neg_z())) != nullptr);

            // right brush textures
            ASSERT_EQ(minuendTexture, right->findFace(vm::vec3::pos_x())->textureName());
            ASSERT_EQ(subtrahendTexture, right->findFace(vm::vec3::neg_x())->textureName());
            ASSERT_EQ(minuendTexture, right->findFace(vm::vec3::pos_y())->textureName());
            ASSERT_EQ(minuendTexture, right->findFace(vm::vec3::neg_y())->textureName());
            ASSERT_EQ(minuendTexture, right->findFace(vm::vec3::pos_z())->textureName());
            ASSERT_EQ(minuendTexture, right->findFace(vm::vec3::neg_z())->textureName());

            delete minuend;
            delete subtrahend;
            kdl::col_delete_all(result);
        }

        TEST_CASE("BrushNodeTest.subtractDisjoint", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const vm::bbox3 brush1Bounds(vm::vec3::fill(-8.0), vm::vec3::fill(+8.0));
            const vm::bbox3 brush2Bounds(vm::vec3(124.0, 124.0, -4.0), vm::vec3(132.0, 132.0, +4.0));
            ASSERT_FALSE(brush1Bounds.intersects(brush2Bounds));

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush1 = world.createBrush(builder.createCuboid(brush1Bounds, "texture"));
            BrushNode* brush2 = world.createBrush(builder.createCuboid(brush2Bounds, "texture"));

            std::vector<BrushNode*> result = brush1->subtract(world, worldBounds, "texture", brush2);
            ASSERT_EQ(1u, result.size());

            BrushNode* subtraction = result.at(0);
            ASSERT_COLLECTIONS_EQUIVALENT(brush1->vertexPositions(), subtraction->vertexPositions());

            kdl::col_delete_all(result);
        }

        TEST_CASE("BrushNodeTest.subtractEnclosed", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const vm::bbox3 brush1Bounds(vm::vec3::fill(-8.0), vm::vec3::fill(+8.0));
            const vm::bbox3 brush2Bounds(vm::vec3::fill(-9.0), vm::vec3::fill(+9.0));
            ASSERT_TRUE(brush1Bounds.intersects(brush2Bounds));

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush1 = world.createBrush(builder.createCuboid(brush1Bounds, "texture"));
            BrushNode* brush2 = world.createBrush(builder.createCuboid(brush2Bounds, "texture"));

            std::vector<BrushNode*> result = brush1->subtract(world, worldBounds, "texture", brush2);
            ASSERT_EQ(0u, result.size());

            kdl::col_delete_all(result);
        }


        TEST_CASE("BrushNodeTest.subtractTruncatedCones", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1469

            const std::string minuendStr("{\n"
                                    "( 29.393876913416079 -16.970562748463635 32 ) ( 16.970562748495468 29.393876913411077 32 ) ( 11.313708499003496 19.595917942278447 -16 ) __TB_empty [ -0.258819 0.965926 0 -0.507559 ] [ -0.158797 -0.0425496 -0.986394 -0.257094 ] -0 1 1\n"
                                    "( 32.784609690844263 -8.784609690813113 32 ) ( 8.7846096908451727 32.784609690839488 32 ) ( 5.856406460569815 21.856406460564131 -16 ) __TB_empty [ -0.5 0.866025 0 -0.77533 ] [ -0.142374 -0.0821995 -0.986394 -0.0887003 ] -0 1 1\n"
                                    "( 33.94112549697229 -0 32 ) ( -0 33.941125496967288 32 ) ( -0 22.627416997982664 -16 ) __TB_empty [ -0.707107 0.707107 0 -0.176551 ] [ -0.116248 -0.116248 -0.986394 -0.46579 ] -0 1 1\n"
                                    "( 32.784609690844718 8.7846096908399431 32 ) ( -8.7846096908083382 32.784609690839488 32 ) ( -5.8564064605325257 21.856406460564131 -16 ) __TB_empty [ -0.866025 0.5 0 -0.0124664 ] [ -0.0821995 -0.142374 -0.986394 -0.870919 ] -0 1 1\n"
                                    "( 29.393876913416534 16.970562748490465 32 ) ( -16.970562748458633 29.393876913411304 32 ) ( -11.313708498966207 19.595917942278675 -16 ) __TB_empty [ -0.965926 0.258819 0 -0.373029 ] [ -0.0425496 -0.158797 -0.986394 -0.805874 ] -0 1 1\n"
                                    "( -11.313708498966662 -19.595917942252527 -16 ) ( -16.970562748458633 -29.393876913384929 32 ) ( 29.393876913416079 -16.970562748463635 32 ) __TB_empty [ -0.0425496 0.158797 -0.986394 -0.30125 ] [ -0.965926 -0.258819 0 -0.00242329 ] -0 1 1\n"
                                    "( -5.8564064605325257 -21.85640646053821 -16 ) ( -8.7846096908078835 -32.784609690813113 32 ) ( 32.784609690844263 -8.784609690813113 32 ) __TB_empty [ -0.0821995 0.142374 -0.986394 -0.474954 ] [ -0.866025 -0.5 0 -0.0709991 ] -0 1 1\n"
                                    "( -0 -22.627416997956516 -16 ) ( -0 -33.941125496940913 32 ) ( 33.94112549697229 -0 32 ) __TB_empty [ -0.116248 0.116248 -0.986394 -0.298004 ] [ -0.707107 -0.707107 0 -0.689445 ] -0 1 1\n"
                                    "( 5.856406460569815 -21.856406460537755 -16 ) ( 8.7846096908451727 -32.784609690813113 32 ) ( 32.784609690844718 8.7846096908399431 32 ) __TB_empty [ -0.142374 0.0821995 -0.986394 -0.219636 ] [ -0.5 -0.866025 0 -0.872314 ] -0 1 1\n"
                                    "( 11.313708499003496 -19.595917942252072 -16 ) ( 16.970562748495922 -29.393876913384702 32 ) ( 29.393876913416534 16.970562748490465 32 ) __TB_empty [ -0.158797 0.0425496 -0.986394 -0.818881 ] [ -0.258819 -0.965926 0 -0.590811 ] -0 1 1\n"
                                    "( 16 -16 -16 ) ( 24 -24 32 ) ( 24 24 32 ) __TB_empty [ -0.164399 0 -0.986394 -0.283475 ] [ 0 -1 0 -0 ] -0 1 1\n"
                                    "( 16.970562748495468 29.393876913411077 32 ) ( -29.3938769133797 16.970562748490465 32 ) ( -19.595917942246615 11.313708498997812 -16 ) __TB_empty [ -0.0425496 0.158797 0.986394 0.0475388 ] [ -0.965926 -0.258819 0 -0.238751 ] -0 1 1\n"
                                    "( 8.7846096908451727 32.784609690839488 32 ) ( -32.784609690807883 8.7846096908399431 32 ) ( -21.856406460532071 5.8564064605641306 -16 ) __TB_empty [ -0.0821995 0.142374 0.986394 -0.902102 ] [ -0.866025 -0.5 0 -0.660111 ] -0 1 1\n"
                                    "( -0 33.941125496967288 32 ) ( -33.941125496935911 -0 32 ) ( -22.627416997950604 -0 -16 ) __TB_empty [ -0.116248 0.116248 0.986394 -0.50108 ] [ -0.707107 -0.707107 0 -0.631095 ] -0 1 1\n"
                                    "( -8.7846096908083382 32.784609690839488 32 ) ( -32.784609690807883 -8.7846096908135678 32 ) ( -21.856406460532071 -5.8564064605377553 -16 ) __TB_empty [ -0.142374 0.0821995 0.986394 -0.198669 ] [ -0.5 -0.866025 0 -0.166748 ] -0 1 1\n"
                                    "( -16.970562748458633 29.393876913411304 32 ) ( -29.393876913379245 -16.970562748463863 32 ) ( -19.595917942246615 -11.313708498971437 -16 ) __TB_empty [ -0.158797 0.0425496 0.986394 -0.573831 ] [ -0.258819 -0.965926 0 -0.238028 ] -0 1 1\n"
                                    "( -29.3938769133797 16.970562748490465 32 ) ( -16.970562748458633 -29.393876913384929 32 ) ( -11.313708498966662 -19.595917942252527 -16 ) __TB_empty [ -0.258819 0.965926 0 -0.271353 ] [ -0.158797 -0.0425496 0.986394 -0.908333 ] -0 1 1\n"
                                    "( -32.784609690807883 8.7846096908399431 32 ) ( -8.7846096908078835 -32.784609690813113 32 ) ( -5.8564064605325257 -21.85640646053821 -16 ) __TB_empty [ -0.5 0.866025 0 -0.18634 ] [ -0.142374 -0.0821995 0.986394 -0.51593 ] -0 1 1\n"
                                    "( -33.941125496935911 -0 32 ) ( -0 -33.941125496940913 32 ) ( -0 -22.627416997956516 -16 ) __TB_empty [ -0.707107 0.707107 0 -0.234839 ] [ -0.116248 -0.116248 0.986394 -0.668957 ] -0 1 1\n"
                                    "( -32.784609690807883 -8.7846096908135678 32 ) ( 8.7846096908451727 -32.784609690813113 32 ) ( 5.856406460569815 -21.856406460537755 -16 ) __TB_empty [ -0.866025 0.5 0 -0.717973 ] [ -0.0821995 -0.142374 0.986394 -0.849948 ] -0 1 1\n"
                                    "( -29.393876913379245 -16.970562748463863 32 ) ( 16.970562748495922 -29.393876913384702 32 ) ( 11.313708499003496 -19.595917942252072 -16 ) __TB_empty [ -0.965926 0.258819 0 -0.72569 ] [ -0.0425496 -0.158797 0.986394 -0.560825 ] -0 1 1\n"
                                    "( -24 24 32 ) ( -24 -24 32 ) ( -16 -16 -16 ) __TB_empty [ -0.164399 0 0.986394 -0.81431 ] [ 0 -1 0 -0 ] -0 1 1\n"
                                    "( 24 24 32 ) ( -24 24 32 ) ( -16 16 -16 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 -0.986394 -0.827715 ] -0 1 1\n"
                                    "( -24 -24 32 ) ( 24 -24 32 ) ( 16 -16 -16 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 0.986394 0.641451 ] -0 1 1\n"
                                    "( 24 24 32 ) ( 24 -24 32 ) ( -24 -24 32 ) __TB_empty [ 1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1\n"
                                    "( -16 -16 -16 ) ( 16 16 -16 ) ( -16 16 -16 ) __TB_empty [ -1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1\n"
                                    "}\n");

            const std::string subtrahendStr("{\n"
                                       "( 29.393876913416079 -16.970562748463635 48 ) ( 16.970562748495468 29.393876913411077 48 ) ( 11.313708499003496 19.595917942278447 -0 ) __TB_empty [ -0.258819 0.965926 0 -0.507559 ] [ -0.158797 -0.0425496 -0.986394 -0.474791 ] -0 1 1\n"
                                       "( 32.784609690844263 -8.784609690813113 48 ) ( 8.7846096908451727 32.784609690839488 48 ) ( 5.856406460569815 21.856406460564131 -0 ) __TB_empty [ -0.5 0.866025 0 -0.77533 ] [ -0.142374 -0.0821995 -0.986394 -0.306396 ] -0 1 1\n"
                                       "( 33.94112549697229 -0 48 ) ( -0 33.941125496967288 48 ) ( -0 22.627416997982664 -0 ) __TB_empty [ -0.707107 0.707107 0 -0.176551 ] [ -0.116248 -0.116248 -0.986394 -0.683485 ] -0 1 1\n"
                                       "( 32.784609690844718 8.7846096908399431 48 ) ( -8.7846096908083382 32.784609690839488 48 ) ( -5.8564064605325257 21.856406460564131 -0 ) __TB_empty [ -0.866025 0.5 0 -0.0124664 ] [ -0.0821995 -0.142374 -0.986394 -0.0886002 ] -0 1 1\n"
                                       "( 29.393876913416534 16.970562748490465 48 ) ( -16.970562748458633 29.393876913411304 48 ) ( -11.313708498966207 19.595917942278675 -0 ) __TB_empty [ -0.965926 0.258819 0 -0.373029 ] [ -0.0425496 -0.158797 -0.986394 -0.0235691 ] -0 1 1\n"
                                       "( -11.313708498966662 -19.595917942252527 -0 ) ( -16.970562748458633 -29.393876913384929 48 ) ( 29.393876913416079 -16.970562748463635 48 ) __TB_empty [ -0.0425496 0.158797 -0.986394 -0.5189 ] [ -0.965926 -0.258819 0 -0.00242329 ] -0 1 1\n"
                                       "( -5.8564064605325257 -21.85640646053821 -0 ) ( -8.7846096908078835 -32.784609690813113 48 ) ( 32.784609690844263 -8.784609690813113 48 ) __TB_empty [ -0.0821995 0.142374 -0.986394 -0.692604 ] [ -0.866025 -0.5 0 -0.0709991 ] -0 1 1\n"
                                       "( -0 -22.627416997956516 -0 ) ( -0 -33.941125496940913 48 ) ( 33.94112549697229 -0 48 ) __TB_empty [ -0.116248 0.116248 -0.986394 -0.515699 ] [ -0.707107 -0.707107 0 -0.689445 ] -0 1 1\n"
                                       "( 5.856406460569815 -21.856406460537755 -0 ) ( 8.7846096908451727 -32.784609690813113 48 ) ( 32.784609690844718 8.7846096908399431 48 ) __TB_empty [ -0.142374 0.0821995 -0.986394 -0.437332 ] [ -0.5 -0.866025 0 -0.872314 ] -0 1 1\n"
                                       "( 11.313708499003496 -19.595917942252072 -0 ) ( 16.970562748495922 -29.393876913384702 48 ) ( 29.393876913416534 16.970562748490465 48 ) __TB_empty [ -0.158797 0.0425496 -0.986394 -0.0365772 ] [ -0.258819 -0.965926 0 -0.590811 ] -0 1 1\n"
                                       "( 16 -16 -0 ) ( 24 -24 48 ) ( 24 24 48 ) __TB_empty [ -0.164399 0 -0.986394 -0.501169 ] [ 0 -1 0 -0 ] -0 1 1\n"
                                       "( 16.970562748495468 29.393876913411077 48 ) ( -29.3938769133797 16.970562748490465 48 ) ( -19.595917942246615 11.313708498997812 -0 ) __TB_empty [ -0.0425496 0.158797 0.986394 0.265238 ] [ -0.965926 -0.258819 0 -0.238751 ] -0 1 1\n"
                                       "( 8.7846096908451727 32.784609690839488 48 ) ( -32.784609690807883 8.7846096908399431 48 ) ( -21.856406460532071 5.8564064605641306 -0 ) __TB_empty [ -0.0821995 0.142374 0.986394 -0.684406 ] [ -0.866025 -0.5 0 -0.660111 ] -0 1 1\n"
                                       "( -0 33.941125496967288 48 ) ( -33.941125496935911 -0 48 ) ( -22.627416997950604 -0 -0 ) __TB_empty [ -0.116248 0.116248 0.986394 -0.283369 ] [ -0.707107 -0.707107 0 -0.631095 ] -0 1 1\n"
                                       "( -8.7846096908083382 32.784609690839488 48 ) ( -32.784609690807883 -8.7846096908135678 48 ) ( -21.856406460532071 -5.8564064605377553 -0 ) __TB_empty [ -0.142374 0.0821995 0.986394 -0.980953 ] [ -0.5 -0.866025 0 -0.166748 ] -0 1 1\n"
                                       "( -16.970562748458633 29.393876913411304 48 ) ( -29.393876913379245 -16.970562748463863 48 ) ( -19.595917942246615 -11.313708498971437 -0 ) __TB_empty [ -0.158797 0.0425496 0.986394 -0.35615 ] [ -0.258819 -0.965926 0 -0.238028 ] -0 1 1\n"
                                       "( -29.3938769133797 16.970562748490465 48 ) ( -16.970562748458633 -29.393876913384929 48 ) ( -11.313708498966662 -19.595917942252527 -0 ) __TB_empty [ -0.258819 0.965926 0 -0.271353 ] [ -0.158797 -0.0425496 0.986394 -0.690683 ] -0 1 1\n"
                                       "( -32.784609690807883 8.7846096908399431 48 ) ( -8.7846096908078835 -32.784609690813113 48 ) ( -5.8564064605325257 -21.85640646053821 -0 ) __TB_empty [ -0.5 0.866025 0 -0.18634 ] [ -0.142374 -0.0821995 0.986394 -0.298214 ] -0 1 1\n"
                                       "( -33.941125496935911 -0 48 ) ( -0 -33.941125496940913 48 ) ( -0 -22.627416997956516 -0 ) __TB_empty [ -0.707107 0.707107 0 -0.234839 ] [ -0.116248 -0.116248 0.986394 -0.451246 ] -0 1 1\n"
                                       "( -32.784609690807883 -8.7846096908135678 48 ) ( 8.7846096908451727 -32.784609690813113 48 ) ( 5.856406460569815 -21.856406460537755 -0 ) __TB_empty [ -0.866025 0.5 0 -0.717973 ] [ -0.0821995 -0.142374 0.986394 -0.632298 ] -0 1 1\n"
                                       "( -29.393876913379245 -16.970562748463863 48 ) ( 16.970562748495922 -29.393876913384702 48 ) ( 11.313708499003496 -19.595917942252072 -0 ) __TB_empty [ -0.965926 0.258819 0 -0.72569 ] [ -0.0425496 -0.158797 0.986394 -0.343115 ] -0 1 1\n"
                                       "( -24 24 48 ) ( -24 -24 48 ) ( -16 -16 -0 ) __TB_empty [ -0.164399 0 0.986394 -0.596628 ] [ 0 -1 0 -0 ] -0 1 1\n"
                                       "( 24 24 48 ) ( -24 24 48 ) ( -16 16 -0 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 -0.986394 -0.0454121 ] -0 1 1\n"
                                       "( -24 -24 48 ) ( 24 -24 48 ) ( 16 -16 -0 ) __TB_empty [ -1 0 0 -0 ] [ 0 -0.164399 0.986394 0.859102 ] -0 1 1\n"
                                       "( 24 24 48 ) ( 24 -24 48 ) ( -24 -24 48 ) __TB_empty [ 1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1\n"
                                       "( -16 -16 -0 ) ( 16 16 -0 ) ( -16 16 -0 ) __TB_empty [ -1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1\n"
                                       "}\n");

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Model::MapFormat::Valve);

            IO::TestParserStatus status;
            BrushNode* minuend = static_cast<BrushNode*>(IO::NodeReader::read(minuendStr, world, worldBounds, status).front());
            BrushNode* subtrahend = static_cast<BrushNode*>(IO::NodeReader::read(subtrahendStr, world, worldBounds, status).front());

            const std::vector<BrushNode*> result = minuend->subtract(world, worldBounds, "some_texture", subtrahend);
            ASSERT_FALSE(result.empty());

            delete minuend;
            delete subtrahend;
            kdl::col_delete_all(result);
        }

        TEST_CASE("BrushNodeTest.subtractDome", "[BrushNodeTest]") {
            // see https://github.com/kduske/TrenchBroom/issues/2707

            const std::string minuendStr(R"({
                ( -1598.09391534391647838 -277.57717407067275417 -20 ) ( -1598.09391534391647838 54.02274375211438695 -20 ) ( -1598.09391534391647838 -277.57717407067275417 -12 ) 128_gold_2 -14.94120025634765625 -108 -0 0.72087001800537109 1
                ( -1178.96031746031826515 -277.57717407067275417 -20 ) ( -1598.09391534391647838 -277.57717407067275417 -20 ) ( -1178.96031746031826515 -277.57717407067275417 -12 ) 128_gold_2 28.92790031433105469 -108 -0 0.8250659704208374 1
                ( -1178.96031746031826515 54.02274375211438695 -20 ) ( -1598.09391534391647838 54.02274375211438695 -20 ) ( -1178.96031746031826515 -277.57717407067275417 -20 ) 128_gold_2 -28.98690032958984375 -4.01778984069824219 -0 0.77968800067901611 0.65970498323440552
                ( -1178.96031746031826515 -277.57717407067275417 -12 ) ( -1598.09391534391647838 -277.57717407067275417 -12 ) ( -1178.96031746031826515 54.02274375211438695 -12 ) 128_gold_2 -28.98690032958984375 -4.01778984069824219 -0 0.77968800067901611 0.65970498323440552
                ( -1598.09391534391647838 54.02274375211438695 -20 ) ( -1178.96031746031826515 54.02274375211438695 -20 ) ( -1598.09391534391647838 54.02274375211438695 -12 ) 128_gold_2 28.92790031433105469 -108 -0 0.8250659704208374 1
                ( -1178 54.02274375211438695 -20 ) ( -1178 -277.57717407067275417 -20 ) ( -1178 54.02274375211438695 -12 ) 128_gold_2 -14.94120025634765625 -108 -0 0.72087001800537109 1
            })");


            const auto subtrahendPath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/test/Model/Brush/subtrahend.map");
            std::ifstream stream(subtrahendPath.asString());
            std::stringstream subtrahendStr;
            subtrahendStr << stream.rdbuf();

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            const auto* minuend = static_cast<BrushNode*>(IO::NodeReader::read(minuendStr, world, worldBounds, status).front());
            const auto subtrahend = kdl::vec_element_cast<BrushNode*>(
                IO::NodeReader::read(subtrahendStr.str(), world, worldBounds, status));

            const auto result = minuend->subtract(world, worldBounds, "some_texture", subtrahend);

            delete minuend;
            kdl::col_delete_all(subtrahend);
            kdl::col_delete_all(result);
        }

        TEST_CASE("BrushNodeTest.subtractPipeFromCubeWithMissingFragments", "[BrushNodeTest]") {
            // see https://github.com/kduske/TrenchBroom/pull/1764#issuecomment-296341588
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
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            BrushNode* minuend = static_cast<BrushNode*>(IO::NodeReader::read(minuendStr, world, worldBounds, status).front());
            BrushNode* subtrahend = static_cast<BrushNode*>(IO::NodeReader::read(subtrahendStr, world, worldBounds, status).front());

            const std::vector<BrushNode*> result = minuend->subtract(world, worldBounds, "some_texture", subtrahend);
            ASSERT_EQ(8u, result.size());

            delete minuend;
            delete subtrahend;
            kdl::col_delete_all(result);
        }

        TEST_CASE("BrushNodeTest.testAlmostDegenerateBrush", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1194
            const std::string data("{\n"
                              "( -1248 -2144 1168 ) ( -1120 -2144 1168 ) ( -1248 -2272 1168 ) rock_1732 1248 2144 0 1 -1 //TX2\n"
                              "( -1248 -2224 1141.33333 ) ( -1248 -2224 1013.33333 ) ( -1120 -2224 1056 ) rock_1732 1391 -309 -33.69007 1.20185 -0.83205 //TX1\n"
                              "( -1408 -2144 1328 ) ( -1408 -2272 1328 ) ( -1408 -2144 1456 ) rock_1732 -1328 2144 90 1 1 //TX1\n"
                              "( -1472 -2256 1434.66667 ) ( -1472 -2256 1562.66667 ) ( -1344 -2256 1349.33334 ) skip 1681 453 -33.69007 1.20185 0.83205 //TX1\n"
                              "( -1248.00004 -2144 1061.33328 ) ( -1248.00004 -2272 1061.33328 ) ( -1120 -2144 976 ) rock_1732 1248 2144 0 1 -1 //TX1\n"
                              "}");

            // This brush is almost degenerate. It should be rejected by the map loader.

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            ASSERT_EQ(0u, nodes.size());
        }

        TEST_CASE("BrushNodeTest.invalidBrush1332", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1332
            const std::string data("{\n"
                              "( 91.428573608  0  4.57144165 ) ( 96 16  0 ) ( 82.285690308  0  0          ) rock5_2 0 0 0 1 1\n"
                              "( 95.238098145  0 16          ) ( 96  2 16 ) ( 91.428573608  0  4.57144165 ) rock5_2 0 0 0 1 1\n"
                              "( 96           16 16          ) ( 96 16  0 ) ( 96            2 16          ) rock5_2 0 0 0 1 1\n"
                              "(  0           16 16          ) (  0  0  0 ) ( 96           16  0          ) rock5_2 0 0 90 1 1\n"
                              "(  0            0 16          ) (  0  0  0 ) (  0           16 16          ) rock5_2 0 0 0 1 1\n"

                              // The next face causes an assertion failure. It's the back face, the normal is +Y.
                              "( 96           16 16          ) (  0 16 16 ) ( 96           16  0          ) rock5_2 0 0 90 1 1\n"

                              // Normal -Y (roughly)
                              "(  0            0  0          ) (  0  0 16 ) ( 82.285690308  0  0          ) rock5_2 0 0 0 1 1\n"

                              // Normal +Z (roughly)
                              "(  0            0 16          ) (  0 16 16 ) ( 95.238098145  0 16          ) rock5_2 0 0 0 1 1\n"

                              // Normal -Z (roughly)
                              "( 82.285690308  0  0          ) ( 96 16  0 ) (  0            0  0          ) rock5_2 0 0 0 1 1\n"
                              "}");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            std::vector<Node*> nodes = reader.read(worldBounds, status); // assertion failure
            kdl::vec_clear_and_delete(nodes);
        }


        TEST_CASE("BrushNodeTest.invalidBrush1395", "[BrushNodeTest]") {
            // Brush causes assertion to fail after having had its vertices snapped
            const std::string data("{\n"
                              "( -72 497 878 ) ( -77 465 878 ) ( -77 396 878 ) rock4_2 -30 1 0 1 1\n"
                              "( -72 497 878 ) ( -59 536 878 ) ( -65 536 905 ) rock4_2 -30 33 0 1 1\n"
                              "( -50 269 878 ) ( -59 279 878 ) ( -40 269 898 ) rock4_2 -1 33 0 1 1\n"
                              "( -67 328 878 ) ( -35 269 904 ) ( -59 279 878 ) rock4_2 -1 33 0 1 1\n"
                              "( -59 279 878 ) ( -35 269 904 ) ( -40 269 898 ) rock4_2 -1 33 0 1 1\n"
                              "( -40 269 898 ) ( -35 269 904 ) ( 28 269 911 ) rock4_2 -30 33 0 1 1\n"
                              "( 171 269 878 ) ( 169 269 884 ) ( 212 340 878 ) rock4_2 -30 1 0 1 1\n"
                              "( 212 340 878 ) ( 169 269 884 ) ( 192 315 893 ) rock4_2 -30 1 0 1 1\n"
                              "( 192 315 893 ) ( 169 269 884 ) ( 106 269 911 ) rock4_2 -30 1 0 1 1\n"
                              "( 28 269 911 ) ( -53 431 911 ) ( -67 524 911 ) rock4_2 -30 1 0 1 1\n"
                              "( -67 524 911 ) ( -53 431 911 ) ( -69 515 908 ) rock4_2 -30 1 0 1 1\n"
                              "( -69 515 908 ) ( -53 431 911 ) ( -35 269 904 ) rock4_2 -30 1 0 1 1\n"
                              "( -35 269 904 ) ( -53 431 911 ) ( 28 269 911 ) rock4_2 -30 33 0 1 1\n"
                              "( -65 536 911 ) ( -67 524 911 ) ( -69 515 908 ) rock4_2 -30 1 0 1 1\n"
                              "( 205 536 911 ) ( -65 536 911 ) ( -65 536 905 ) rock4_2 -30 33 0 1 1\n"
                              "( -65 536 905 ) ( -65 536 911 ) ( -69 515 908 ) rock4_2 -30 33 0 1 1\n"
                              "( 231 504 911 ) ( 205 536 911 ) ( 246 507 884 ) rock4_2 -30 1 0 1 1\n"
                              "( 246 507 884 ) ( 205 536 911 ) ( 226 536 878 ) rock4_2 -30 1 0 1 1\n"
                              "( 136 301 911 ) ( 231 504 911 ) ( 209 344 892 ) rock4_2 -30 1 0 1 1\n"
                              "( 209 344 892 ) ( 231 504 911 ) ( 237 499 908 ) rock4_2 -30 1 0 1 1\n"
                              "( 212 340 878 ) ( 192 315 893 ) ( 209 344 892 ) rock4_2 -30 1 0 1 1\n"
                              "( 209 344 892 ) ( 192 315 893 ) ( 136 301 911 ) rock4_2 -30 1 0 1 1\n"
                              "( 136 301 911 ) ( 192 315 893 ) ( 106 269 911 ) rock4_2 -30 1 0 1 1\n"
                              "( 212 340 878 ) ( 209 344 892 ) ( 246 498 878 ) rock4_2 -30 1 0 1 1\n"
                              "( 246 498 878 ) ( 209 344 892 ) ( 237 499 908 ) rock4_2 -1 33 0 1 1\n"
                              "( 246 511 878 ) ( 246 507 884 ) ( 226 536 878 ) rock4_2 -30 1 0 1 1\n"
                              "( 237 499 908 ) ( 246 507 884 ) ( 246 498 878 ) rock4_2 -1 33 0 1 1\n"
                              "( 246 498 878 ) ( 246 507 884 ) ( 246 511 878 ) rock4_2 -30 1 0 1 1\n"
                              "( -65 536 905 ) ( -69 515 908 ) ( -72 497 878 ) rock4_2 -30 33 0 1 1\n"
                              "( -67 328 878 ) ( -69 515 908 ) ( -35 269 904 ) rock4_2 -1 33 0 1 1\n"
                              "( -69 515 908 ) ( -77 465 890 ) ( -72 497 878 ) rock4_2 -30 33 0 1 1\n"
                              "( -72 497 878 ) ( -77 465 890 ) ( -77 465 878 ) rock4_2 -30 1 0 1 1\n"
                              "( -77 465 878 ) ( -77 465 890 ) ( -77 396 878 ) rock4_2 -30 1 0 1 1\n"
                              "( -77 396 878 ) ( -77 465 890 ) ( -67 328 878 ) rock4_2 -30 1 0 1 1\n"
                              "( -67 328 878 ) ( -77 465 890 ) ( -69 515 908 ) rock4_2 -1 33 0 1 1\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            std::vector<Node*> nodes = reader.read(worldBounds, status); // assertion failure
            kdl::vec_clear_and_delete(nodes);
        }

        TEST_CASE("BrushNodeTest.invalidBrush1801", "[BrushNodeTest]") {
            // see https://github.com/kduske/TrenchBroom/issues/1801
            // see PolyhedronTest::clipWithInvalidSeam

            const std::string data("{\n"
                              "( -484 1513 395 ) ( -483 1371 131 ) ( -483 1777 253 ) *water1 -0 -0 -0 1 1\n"
                              "( -483 1371 131 ) ( -459 1579 -115 ) ( -483 1777 253 ) *water1 -0 -0 -0 1 1\n"
                              "( -459 1579 -115 ) ( -483 1371 131 ) ( -184 1428 237 ) *water1 -0 -0 -0 1 1\n"
                              "( -459 1579 -115 ) ( -184 1428 237 ) ( -183 1692 95 ) *water1 -0 -0 -0 1 1\n"
                              "( -184 1428 237 ) ( -184 1513 396 ) ( -184 1777 254 ) *water1 -0 -0 -0 1 1\n"
                              "( -184 1513 396 ) ( -484 1513 395 ) ( -184 1777 254 ) *water1 -0 -0 -0 1 1\n"
                              "( -483 1371 131 ) ( -484 1513 395 ) ( -184 1513 396 ) *water1 -0 -0 -0 1 1\n"
                              "( -483 1371 131 ) ( -184 1513 396 ) ( -184 1428 237 ) *water1 -0 -0 -0 1 1\n"
                              "( -184 1777 254 ) ( -483 1777 253 ) ( -183 1692 95 ) *water1 -0 -0 -0 1 1\n"
                              "( -483 1777 253 ) ( -459 1579 -115 ) ( -183 1692 95 ) *water1 -0 -0 -0 1 1\n"
                              "}\n");

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            std::vector<Node*> nodes = reader.read(worldBounds, status); // assertion failure
            kdl::vec_clear_and_delete(nodes);
        }

        TEST_CASE("BrushNodeTest.snapshotTextureTest", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            BrushNode* cube = world.createBrush(builder.createCube(128.0, ""));
            BrushSnapshot* snapshot = nullptr;

            // Temporarily set a texture on `cube`, take a snapshot, then clear the texture
            {
                Assets::Texture texture("testTexture", 64, 64);
                for (BrushFace* face : cube->faces()) {
                    face->setTexture(&texture);
                }
                ASSERT_EQ(6U, texture.usageCount());

                snapshot = dynamic_cast<BrushSnapshot*>(cube->takeSnapshot());
                ASSERT_NE(nullptr, snapshot);
                ASSERT_EQ(6U, texture.usageCount());

                for (BrushFace* face : cube->faces()) {
                    face->unsetTexture();
                }
                ASSERT_EQ(0U, texture.usageCount());
            }

            // Check all textures are cleared
            for (BrushFace* face : cube->faces()) {
                EXPECT_EQ(BrushFaceAttributes::NoTextureName, face->textureName());
            }

            snapshot->restore(worldBounds);

            // Check just the texture names are restored
            for (BrushFace* face : cube->faces()) {
                EXPECT_EQ("testTexture", face->textureName());
                EXPECT_EQ(nullptr, face->texture());
            }

            delete snapshot;
            delete cube;
        }

        // https://github.com/kduske/TrenchBroom/issues/1893
        TEST_CASE("BrushNodeTest.intersectsIssue1893", "[BrushNodeTest]") {
            const std::string data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( 2368 173.07179676972467 525.07179676972441 ) ( 2368 194.92820323027539 530.92820323027559 ) ( 2368 186.92820323027561 517.07179676972441 ) mt_sr_v1x [ 0 0 1 -32 ] [ 0 -1 0 32 ] 0 1 1\n"
                              "( 2048 173.07179676972467 525.07179676972441 ) ( 2048 194.92820323027539 530.92820323027559 ) ( 2048 181.07179676972444 538.92820323027536 ) mt_sr_v1x [ 0 0 1 -32 ] [ 0 -1 0 32 ] 0 1 1\n"
                              "( 1680 181.07179676972444 538.92820323027536 ) ( 1664 194.92820323027539 530.92820323027559 ) ( 1680 194.92820323027539 530.92820323027559 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 184 539.31370849898462 ) ( 1664 195.31370849898465 528 ) ( 1680 195.31370849898465 528 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 186.92820323027561 538.92820323027536 ) ( 1664 194.92820323027539 525.07179676972441 ) ( 1680 194.92820323027539 525.07179676972441 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 189.65685424949245 537.79795897113263 ) ( 1664 193.79795897113243 522.34314575050757 ) ( 1680 193.79795897113243 522.34314575050757 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 178.3431457505076 537.79795897113263 ) ( 1664 193.79795897113266 533.65685424949243 ) ( 1680 193.79795897113266 533.65685424949243 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 186.92820323027561 517.07179676972441 ) ( 1664 194.92820323027539 530.92820323027559 ) ( 1664 186.92820323027561 517.07179676972441 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 184 516.68629150101515 ) ( 1664 195.31370849898465 528 ) ( 1664 184 516.68629150101515 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 181.07179676972444 517.07179676972441 ) ( 1664 194.92820323027539 525.07179676972441 ) ( 1664 181.07179676972444 517.07179676972441 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 178.34314575050738 518.20204102886714 ) ( 1664 193.79795897113243 522.34314575050757 ) ( 1664 178.34314575050738 518.20204102886714 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 189.65685424949245 518.20204102886737 ) ( 1664 193.79795897113266 533.65685424949243 ) ( 1664 189.65685424949245 518.20204102886737 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 192 520 ) ( 1664 192 536 ) ( 1664 192 520 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 173.07179676972467 525.07179676972441 ) ( 1664 181.07179676972444 538.92820323027536 ) ( 1680 181.07179676972444 538.92820323027536 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 172.68629150101518 528 ) ( 1664 184 539.31370849898462 ) ( 1680 184 539.31370849898462 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 173.07179676972444 530.92820323027559 ) ( 1664 186.92820323027561 538.92820323027536 ) ( 1680 186.92820323027561 538.92820323027536 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 174.20204102886717 533.65685424949243 ) ( 1664 189.65685424949245 537.79795897113263 ) ( 1680 189.65685424949245 537.79795897113263 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 174.2020410288674 522.34314575050757 ) ( 1664 178.3431457505076 537.79795897113263 ) ( 1680 178.3431457505076 537.79795897113263 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 173.07179676972467 525.07179676972441 ) ( 1664 186.92820323027561 517.07179676972441 ) ( 1664 173.07179676972467 525.07179676972441 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 172.68629150101518 528 ) ( 1664 184 516.68629150101515 ) ( 1664 172.68629150101518 528 ) mt_sr_v1x [ 0 0 -1 28.6864 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 173.07179676972444 530.92820323027559 ) ( 1664 181.07179676972444 517.07179676972441 ) ( 1664 173.07179676972444 530.92820323027559 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 174.20204102886717 533.65685424949243 ) ( 1664 178.34314575050738 518.20204102886714 ) ( 1664 174.20204102886717 533.65685424949243 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 174.2020410288674 522.34314575050757 ) ( 1664 189.65685424949245 518.20204102886737 ) ( 1664 174.2020410288674 522.34314575050757 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 176 520 ) ( 1664 176 536 ) ( 1680 176 536 ) mt_sr_v1x [ 0 0 1 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 176 536 ) ( 1664 192 536 ) ( 1680 192 536 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "( 1680 176 520 ) ( 1664 192 520 ) ( 1664 176 520 ) mt_sr_v1x [ 0 1 0 -32 ] [ 1 0 0 -48 ] 0 1 1\n"
                              "}\n"
                              "{\n"
                              "( 784 -624 656 ) ( 5536 -624 672 ) ( 5536 -624 656 ) __TB_empty [ 1 0 0 -0 ] [ 0 0 -1 -0 ] -0 1 1\n"
                              "( 784 -208 656 ) ( 784 4672 672 ) ( 784 -208 672 ) __TB_empty [ 0 -1 0 -0 ] [ 0 0 -1 -0 ] -0 1 1\n"
                              "( 784 -208 -1792 ) ( 5536 4672 -1792 ) ( 784 4672 -1792 ) __TB_empty [ -1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1\n"
                              "( 784 -208 1200 ) ( 5536 4672 1200 ) ( 5536 -208 1200 ) __TB_empty [ 1 0 0 -0 ] [ 0 -1 0 -0 ] -0 1 1\n"
                              "( 784 4672 656 ) ( 5536 4672 672 ) ( 784 4672 672 ) __TB_empty [ -1 0 0 -0 ] [ 0 0 -1 -0 ] -0 1 1\n"
                              "( 5536 -208 656 ) ( 5536 4672 672 ) ( 5536 4672 656 ) __TB_empty [ 0 1 0 -0 ] [ 0 0 -1 -0 ] -0 1 1\n"
                              "}\n"
                              "}\n");

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Model::MapFormat::Valve);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            std::vector<Node*> nodes = reader.read(worldBounds, status);
            ASSERT_EQ(1u, nodes.size());
            ASSERT_TRUE(nodes.at(0)->hasChildren());
            ASSERT_EQ(2u, nodes.at(0)->children().size());

            BrushNode* pipe = static_cast<BrushNode*>(nodes.at(0)->children().at(0));
            BrushNode* cube = static_cast<BrushNode*>(nodes.at(0)->children().at(1));

            ASSERT_TRUE(pipe->intersects(cube));
            ASSERT_TRUE(cube->intersects(pipe));
        }

        TEST_CASE("BrushNodeTest.loadBrushFail_2361", "[BrushNodeTest]") {
            // see https://github.com/kduske/TrenchBroom/pull/2372#issuecomment-432893836

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);

            const std::string data = R"(
{
( -5706.7302805113286 648 1090 ) ( -5730.730280567378 730 1100 ) ( -5706.7302804991377 722 1076 ) so_b4b -0 -0 -0 1 1
( -5730.7302804970022 574 1112 ) ( -5706.7302805113286 648 1090 ) ( -5706.7302621135759 484 1090 ) so_b4b -41.2695 6 -0 1 1
( -5734.7302801651795 722 1026 ) ( -5712.7302804989204 720 1048 ) ( -5730.7302803650427 730 1096 ) so_b4b -1.27002 -0 -0 1 1
( -5844.7302079667779 726 1066 ) ( -5768.7302088192082 732 1104 ) ( -5772.7302795053893 732 1112 ) so_b4b -1.26953 -0 -0 1 1
( -5812.7302805003419 728 1112 ) ( -5834.7302796344165 726 1090 ) ( -5844.7302796989916 726 1070 ) so_b4b -1.26953 -0 -0 1 1
( -5844.7302091123302 646 1066 ) ( -5844.7302079667779 726 1066 ) ( -5844.7302796989916 726 1070 ) so_b4b 56 12 270 1 1
( -5844.7302796989916 726 1070 ) ( -5844.7302079667779 726 1066 ) ( -5772.7302795053893 732 1112 ) so_b4b -1.26953 -0 -0 1 1
( -5794.7302805078398 710 1026 ) ( -5816.7302804990422 724 1038 ) ( -5808.7302832535743 624 1026 ) so_b4b -1.27002 -0 -0 1 1
( -5844.7302079667779 726 1066 ) ( -5838.7302804991104 726 1060 ) ( -5768.7302088192082 732 1104 ) so_b4b -1.27002 -0 -0 1 1
( -5768.7302088192082 732 1104 ) ( -5838.7302804991104 726 1060 ) ( -5774.73028677006 726 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5774.73028677006 726 1026 ) ( -5838.7302804991104 726 1060 ) ( -5816.7302804990422 724 1038 ) so_b4b -1.26953 126.71 -0 1 1
( -5816.7302804990422 724 1038 ) ( -5832.7301282194012 642 1048 ) ( -5808.7302832535743 624 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5808.7302832535743 624 1026 ) ( -5832.7301282194012 642 1048 ) ( -5832.7304881233285 490 1048 ) so_b4b 67.2695 -120 180 1 1
( -5832.7304881233285 490 1048 ) ( -5832.7301282194012 642 1048 ) ( -5844.7302079667779 726 1066 ) so_b4b -85.6646 31.4945 -0 1 1
( -5844.7302079667779 726 1066 ) ( -5832.7301282194012 642 1048 ) ( -5838.7302804991104 726 1060 ) so_b4b -97.377 98.5979 -0 1 1
( -5838.7302804991104 726 1060 ) ( -5832.7301282194012 642 1048 ) ( -5816.7302804990422 724 1038 ) so_b4b -1.26953 -0 -0 1 1
( -5774.73028677006 726 1026 ) ( -5816.7302804990422 724 1038 ) ( -5794.7302805078398 710 1026 ) so_b4b -1.27002 -0 -0 1 1
( -5832.7304881233285 490 1048 ) ( -5844.7302079667779 726 1066 ) ( -5844.7302091123302 646 1066 ) so_b4b -85.6646 31.4945 -0 1 1
( -5808.7302832535743 624 1026 ) ( -5832.7304881233285 490 1048 ) ( -5808.7302837141997 492 1026 ) so_b4b 67.2695 -120 180 1 1
( -5808.7302837141997 492 1026 ) ( -5832.7304881233285 490 1048 ) ( -5706.7302802080176 484 1086 ) so_b4b -1.26953 -0 -0 1 1
( -5832.7304881233285 490 1048 ) ( -5832.7302554422868 490 1052 ) ( -5706.7302621135759 484 1090 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7302621135759 484 1090 ) ( -5832.7302554422868 490 1052 ) ( -5774.730280496974 488 1112 ) so_b4b -1.26953 -0 -0 1 1
( -5774.730280496974 488 1112 ) ( -5832.7302554422868 490 1052 ) ( -5814.7302804944029 490 1100 ) so_b4b -1.26953 -0 -0 1 1
( -5814.7302804944029 490 1100 ) ( -5832.7302554422868 490 1052 ) ( -5840.7302829597875 494 1072 ) so_b4b -1.26953 -0 -0 1 1
( -5840.7302829597875 494 1072 ) ( -5832.7302554422868 490 1052 ) ( -5832.7304881233285 490 1048 ) so_b4b 87.2695 34 180 1 -1
( -5840.7302829597875 494 1072 ) ( -5832.7304881233285 490 1048 ) ( -5840.7302074378586 494 1068 ) so_b4b 87.2695 34 180 1 -1
( -5840.7302074378586 494 1068 ) ( -5832.7304881233285 490 1048 ) ( -5844.7302091123302 646 1066 ) so_b4b -128 -0 -0 1 1
( -5764.7302804806905 494 1026 ) ( -5736.7302804958917 496 1030 ) ( -5734.7302802830618 638 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5702.7302793858989 490 1062 ) ( -5724.7302804988412 496 1038 ) ( -5736.7302804958917 496 1030 ) so_b4b -1.26953 -0 -0 1 1
( -5736.7302804958917 496 1030 ) ( -5724.7302804988412 496 1038 ) ( -5734.7302802830618 638 1026 ) so_b4b -1.27002 128 -0 1 1
( -5706.7302621135759 484 1090 ) ( -5698.7302805883301 488 1068 ) ( -5706.7302802080176 484 1086 ) so_b4b -21.27 -56 -0 1 -1
( -5706.7302802080176 484 1086 ) ( -5698.7302805883301 488 1068 ) ( -5808.7302837141997 492 1026 ) so_b4b -1.27002 -0 -0 1 1
( -5808.7302837141997 492 1026 ) ( -5698.7302805883301 488 1068 ) ( -5764.7302804806905 494 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5764.7302804806905 494 1026 ) ( -5698.7302805883301 488 1068 ) ( -5736.7302804958917 496 1030 ) so_b4b -1.26953 -0 -0 1 1
( -5736.7302804958917 496 1030 ) ( -5698.7302805883301 488 1068 ) ( -5702.7302793858989 490 1062 ) so_b4b -1.26953 -0 -0 1 1
( -5844.7302091123302 646 1066 ) ( -5844.7302808445411 646 1070 ) ( -5840.7302829597875 494 1072 ) so_b4b -0 -0 -0 1 1
( -5814.7302804944029 490 1100 ) ( -5836.7302805003565 642 1090 ) ( -5812.7302805004229 644 1112 ) so_b4b -1.26953 -0 -0 1 1
( -5812.7302805004229 644 1112 ) ( -5836.7302805003565 642 1090 ) ( -5812.7302805003419 728 1112 ) so_b4b 63.2695 12 180 1 -1
( -5812.7302805003419 728 1112 ) ( -5836.7302805003565 642 1090 ) ( -5834.7302796344165 726 1090 ) so_b4b -15.7554 51.0244 -0 1 1
( -5834.7302796344165 726 1090 ) ( -5836.7302805003565 642 1090 ) ( -5844.7302796989916 726 1070 ) so_b4b -50 102 -0 1 1
( -5844.7302796989916 726 1070 ) ( -5836.7302805003565 642 1090 ) ( -5844.7302808445411 646 1070 ) so_b4b -50 102 -0 1 1
( -5844.7302808445411 646 1070 ) ( -5836.7302805003565 642 1090 ) ( -5840.7302829597875 494 1072 ) so_b4b -0 -0 -0 1 1
( -5840.7302829597875 494 1072 ) ( -5836.7302805003565 642 1090 ) ( -5814.7302804944029 490 1100 ) so_b4b -0 -0 -0 1 1
( -5814.7302804944029 490 1100 ) ( -5812.7302805004229 644 1112 ) ( -5802.7302804990823 490 1108 ) so_b4b -1.27002 128 -0 1 1
( -5802.7302804990823 490 1108 ) ( -5774.730280496974 488 1112 ) ( -5814.7302804944029 490 1100 ) so_b4b -1.26953 -0 -0 1 1
( -5706.7302621135759 484 1090 ) ( -5774.730280496974 488 1112 ) ( -5730.73028055137 486 1112 ) so_b4b -1.26953 -0 -0 1 1
( -5812.7302805004229 644 1112 ) ( -5774.730280496974 488 1112 ) ( -5802.7302804990823 490 1108 ) so_b4b -1.26953 -0 -0 1 1
( -5734.7302801651795 722 1026 ) ( -5774.73028677006 726 1026 ) ( -5794.7302805078398 710 1026 ) so_b4b -33.27 6 -0 1 1
( -5844.7302796989916 726 1070 ) ( -5772.7302795053893 732 1112 ) ( -5812.7302805003419 728 1112 ) so_b4b -1.27002 -0 -0 1 1
( -5734.7302801651795 722 1026 ) ( -5730.7302803650427 730 1096 ) ( -5774.73028677006 726 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5748.7302733109655 732 1104 ) ( -5748.7302735130534 732 1108 ) ( -5772.7302795053893 732 1112 ) so_b4b 95.2695 -56 180 1 1
( -5730.7302803650427 730 1096 ) ( -5748.7302733109655 732 1104 ) ( -5774.73028677006 726 1026 ) so_b4b -1.26953 -0 -0 1 1
( -5774.73028677006 726 1026 ) ( -5748.7302733109655 732 1104 ) ( -5768.7302088192082 732 1104 ) so_b4b -1.26953 -0 -0 1 1
( -5712.7302804989204 720 1048 ) ( -5706.7302804991377 722 1076 ) ( -5730.7302803650427 730 1096 ) so_b4b -1.26953 -0 -0 1 1
( -5702.7302804990277 720 1070 ) ( -5706.7302804991377 722 1076 ) ( -5712.7302804989204 720 1048 ) so_b4b -1.26953 -0 -0 1 1
( -5712.7302804989204 720 1048 ) ( -5710.7302804925857 636 1048 ) ( -5698.7302805012459 644 1068 ) so_b4b -0 -0 -0 1 1
( -5698.7302805012459 644 1068 ) ( -5710.7302804925857 636 1048 ) ( -5698.7302805883301 488 1068 ) so_b4b -128 -0 -0 1 1
( -5698.7302805883301 488 1068 ) ( -5710.7302804925857 636 1048 ) ( -5702.7302793858989 490 1062 ) so_b4b -0 -0 -0 1 1
( -5702.7302793858989 490 1062 ) ( -5710.7302804925857 636 1048 ) ( -5724.7302804988412 496 1038 ) so_b4b -0 -0 -0 1 1
( -5724.7302804988412 496 1038 ) ( -5710.7302804925857 636 1048 ) ( -5734.7302802830618 638 1026 ) so_b4b -37.2695 6 -0 1 1
( -5734.7302802830618 638 1026 ) ( -5710.7302804925857 636 1048 ) ( -5734.7302801651795 722 1026 ) so_b4b -37.2695 6 -0 1 1
( -5734.7302801651795 722 1026 ) ( -5710.7302804925857 636 1048 ) ( -5712.7302804989204 720 1048 ) so_b4b -9.75537 -38.9756 -0 1 -1
( -5712.7302804989204 720 1048 ) ( -5698.7302805012459 644 1068 ) ( -5702.7302804990277 720 1070 ) so_b4b -0 -0 -0 1 1
( -5706.7302621135759 484 1090 ) ( -5706.7302805113286 648 1090 ) ( -5698.7302805012459 644 1068 ) so_b4b 88 102 180 1 -1
( -5698.7302805012459 644 1068 ) ( -5706.7302805113286 648 1090 ) ( -5702.7302804990277 720 1070 ) so_b4b -0 -0 -0 1 1
( -5702.7302804990277 720 1070 ) ( -5706.7302805113286 648 1090 ) ( -5706.7302804991377 722 1076 ) so_b4b -0 -0 -0 1 1
( -5706.7302804991377 722 1076 ) ( -5730.730280567378 730 1100 ) ( -5730.7302803650427 730 1096 ) so_b4b 103.27 -56 180 1 1
( -5730.7302803650427 730 1096 ) ( -5730.730280567378 730 1100 ) ( -5748.7302735130534 732 1108 ) so_b4b 99.2695 -56 180 1 1
( -5730.7302804970022 574 1112 ) ( -5737.730280499567 649 1112 ) ( -5706.7302805113286 648 1090 ) so_b4b -41.27 -126 -0 1 -1
( -5706.7302805113286 648 1090 ) ( -5737.730280499567 649 1112 ) ( -5730.730280567378 730 1100 ) so_b4b -1.27002 -0 -0 1 1
( -5730.730280567378 730 1100 ) ( -5737.730280499567 649 1112 ) ( -5748.7302735130534 732 1108 ) so_b4b -1.27002 -0 -0 1 1
( -5748.7302735130534 732 1108 ) ( -5737.730280499567 649 1112 ) ( -5772.7302795053893 732 1112 ) so_b4b -1.27002 -0 -0 1 1
( -5772.7302795053893 732 1112 ) ( -5737.730280499567 649 1112 ) ( -5730.7302804970022 574 1112 ) so_b4b -37.27 6 -0 1 1
}
)";

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            ASSERT_NO_THROW(reader.read(worldBounds, status));
        }

        TEST_CASE("BrushNodeTest.loadBrushFail_2491", "[BrushNodeTest]") {
            // see https://github.com/kduske/TrenchBroom/issues/2491

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);

            const std::string data = R"(
            {
                ( -179 -179 -63 ) ( -158 -158 -69 ) ( 1.055125500745701e+154 1.0551255007456758e+154 -5.2756275037285048e+153 ) _core/tangerine -2.82843 -0 -0 0.0625 0.0625
                ( -132 -126.3431457505086 -60 ) ( -132 188 -60 ) ( -132 -126.34314575050865 -64 ) _core/tangerine 0 0 0 0.0625 0.0625
                ( -188 188 -60 ) ( -188 -182.34314575050769 -60 ) ( -188 188 -64 ) _core/tangerine 0 0 0 0.0625 0.0625
                ( -132 192 -60 ) ( -188 192 -60 ) ( -132 192 -64 ) _core/tangerine -0 -0 -0 0.0625 0.0625
                ( -188 188 -60 ) ( -132 188 -60 ) ( -188 -182.34314575050769 -60 ) _core/tangerine 32 -112 -0 0.0625 0.0625
                ( -132 188 -64 ) ( -188 188 -64 ) ( -132 -126.34314575050865 -64 ) _core/tangerine 32 -112 -0 0.0625 0.0625
            }
            )";

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            ASSERT_NO_THROW(reader.read(worldBounds, status));
        }

        TEST_CASE("BrushNodeTest.loadBrushFail_2686", "[BrushNodeTest]") {
            // see https://github.com/kduske/TrenchBroom/issues/2686

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(Model::MapFormat::Valve);

            const std::string data = R"(
{
( -751 -623.07933525052886 159.27097151882481 ) ( -753.39055027600557 -658.05150554216561 45.762341114124865 ) ( -752.04220703008457 -655.64774857459861 45.762340887734425 ) wood1_1 [ -0.499995 -0.836519 0.224145 8.59912 ] [ -0.0094517 -0.253533 -0.967281 -43.4648 ] 346.992 1 1
( -746.54446646023075 -654.12665614912589 45.762340832676934 ) ( -743.99141084100086 -655.64759047173152 45.762340853972965 ) ( -746.90192378883967 -622.0185651831514 158.98673884436587 ) wood1_1 [ -0.866028 0.482959 -0.129408 -4.96463 ] [ 0.00536862 -0.249822 -0.968277 -43.5033 ] 7.53177 1 1
( -746.90192378883967 -622.0185651831514 158.98673884436587 ) ( -743.99141084100086 -655.64759047173152 45.762340853972965 ) ( -745 -623.0792133033973 159.27093866053934 ) wood1_1 [ -0.866028 0.482959 -0.129408 -4.96463 ] [ 0.00536862 -0.249822 -0.968277 -43.5033 ] 7.53177 1 1
( -745 -623.0792133033973 159.27093866053934 ) ( -743.99141084100086 -655.64759047173152 45.762340853972965 ) ( -742.51072427503652 -658.28759504188008 45.762340891699573 ) wood1_1 [ -0.499995 0.836519 -0.224145 -8.59909 ] [ 0.00925779 -0.253641 -0.967254 -43.4641 ] 13.0082 1 1
( -753.39055027600557 -658.05150554216561 45.762341114124865 ) ( -752 -627.20176933038158 160.37557439373654 ) ( -753.40030222000973 -661.5816915717096 45.76234097597262 ) wood1_1 [ 0 -0.965926 0.258819 9.92938 ] [ -0.0106727 -0.258804 -0.965871 -43.4111 ] 345 1 1
( -753.40030222000973 -661.5816915717096 45.76234097597262 ) ( -752 -627.20176933038158 160.37557439373654 ) ( -751 -628.8747682432919 160.82385299770002 ) wood1_1 [ 0.500008 -0.836512 0.224143 8.59901 ] [ -0.0094517 -0.264075 -0.964456 -43.3565 ] 346.992 1 1
( -743.90192378794575 -624.91635477344664 159.76319924922808 ) ( -745 -623.0792133033973 159.27093866053934 ) ( -742.51072427503652 -658.28759504188008 45.762340891699573 ) wood1_1 [ -0.499995 0.836519 -0.224145 -8.59909 ] [ 0.00925779 -0.253641 -0.967254 -43.4641 ] 13.0082 1 1
( -752.04220703008457 -655.64774857459861 45.762340887734425 ) ( -749.09793039137571 -622.01856518315344 158.98673884435811 ) ( -751 -623.07933525052886 159.27097151882481 ) wood1_1 [ -0.866028 -0.482959 0.129408 4.96466 ] [ -0.00543319 -0.249714 -0.968304 -43.5042 ] 352.468 1 1
( -751 -623.07933525052886 159.27097151882481 ) ( -752 -624.75226938818867 159.71923270135312 ) ( -753.39055027600557 -658.05150554216561 45.762341114124865 ) wood1_1 [ -0.499995 -0.836519 0.224145 8.59912 ] [ -0.0094517 -0.253533 -0.967281 -43.4648 ] 346.992 1 1
( -753.39055027600557 -658.05150554216561 45.762341114124865 ) ( -752 -624.75226938818867 159.71923270135312 ) ( -752 -627.20176933038158 160.37557439373654 ) wood1_1 [ 0 -0.965926 0.258819 9.92938 ] [ -0.0106727 -0.258804 -0.965871 -43.4111 ] 345 1 1
( -746.90207063287346 -629.93555546737525 161.10809006388723 ) ( -745 -628.87474788883753 160.82384752100626 ) ( -743.97456390268746 -664 45.762340974536315 ) wood1_1 [ 0.866016 0.482978 -0.129414 -4.96484 ] [ 0.00536892 -0.267786 -0.963463 -43.3186 ] 7.53207 1 1
( -743.90192378813185 -627.03768398273758 160.33160771552403 ) ( -745 -628.87474788883753 160.82384752100626 ) ( -746.90207063287346 -629.93555546737525 161.10809006388723 ) wood1_1 [ 1 0 0 -0 ] [ 0 -0.965926 0.258819 9.92938 ] -0 1 1
( -751 -628.8747682432919 160.82385299770002 ) ( -749.09792934966106 -629.93555547773678 161.10809006665528 ) ( -752.05952711953228 -664 45.762340944544121 ) wood1_1 [ 0.866016 -0.482978 0.129414 4.96484 ] [ -0.00543343 -0.267894 -0.963433 -43.3173 ] 352.468 1 1
( -752.05952711953228 -664 45.762340944544121 ) ( -749.09792934966106 -629.93555547773678 161.10809006665528 ) ( -749.49773869956948 -665.53645570829394 45.762340998099269 ) wood1_1 [ 0.866016 -0.482978 0.129414 4.96484 ] [ -0.00543343 -0.267894 -0.963433 -43.3173 ] 352.468 1 1
( -746.90192378883967 -622.0185651831514 158.98673884436587 ) ( -749.09793039137571 -622.01856518315344 158.98673884435811 ) ( -749.4887863191035 -654.12665614891398 45.762340833436674 ) wood1_1 [ -1 0 0 -0 ] [ 0 -0.24837 -0.968665 -43.5181 ] -0 1 1
( -749.4887863191035 -654.12665614891398 45.762340833436674 ) ( -749.09793039137571 -622.01856518315344 158.98673884435811 ) ( -752.04220703008457 -655.64774857459861 45.762340887734425 ) wood1_1 [ -0.866028 -0.482959 0.129408 4.96466 ] [ -0.00543319 -0.249714 -0.968304 -43.5042 ] 352.468 1 1
( -743.90192378813185 -627.03768398273758 160.33160771552403 ) ( -743.90192378794575 -624.91635477344664 159.76319924922808 ) ( -742.51072427503652 -658.28759504188008 45.762340891699573 ) wood1_1 [ 0 0.965926 -0.258819 -9.92938 ] [ 0.0106727 -0.258804 -0.965871 -43.4111 ] 15 1 1
( -751 -628.8747682432919 160.82385299770002 ) ( -752.05952711953228 -664 45.762340944544121 ) ( -753.40030222000973 -661.5816915717096 45.76234097597262 ) wood1_1 [ 0.500008 -0.836512 0.224143 8.59901 ] [ -0.0094517 -0.264075 -0.964456 -43.3565 ] 346.992 1 1
( -743.97456390268746 -664 45.762340974536315 ) ( -746.53638375403534 -665.53645569340722 45.762340997376214 ) ( -746.90207063287346 -629.93555546737525 161.10809006388723 ) wood1_1 [ 0.866016 0.482978 -0.129414 -4.96484 ] [ 0.00536892 -0.267786 -0.963463 -43.3186 ] 7.53207 1 1
( -746.90207063287346 -629.93555546737525 161.10809006388723 ) ( -746.53638375403534 -665.53645569340722 45.762340997376214 ) ( -749.49773869956948 -665.53645570829394 45.762340998099269 ) wood1_1 [ 1 0 0 -0 ] [ 0 -0.269238 -0.963074 -43.3036 ] -0 1 1
( -742.51072427503652 -658.28759504188008 45.762340891699573 ) ( -742.50227651731177 -661.34478591957327 45.762340935828185 ) ( -743.90192378813185 -627.03768398273758 160.33160771552403 ) wood1_1 [ 0 0.965926 -0.258819 -9.92938 ] [ 0.0106727 -0.258804 -0.965871 -43.4111 ] 15 1 1
( -743.90192378813185 -627.03768398273758 160.33160771552403 ) ( -742.50227651731177 -661.34478591957327 45.762340935828185 ) ( -745 -628.87474788883753 160.82384752100626 ) wood1_1 [ 0.499998 0.836517 -0.224144 -8.59906 ] [ 0.00925781 -0.263967 -0.964487 -43.358 ] 13.0082 1 1
( -745 -628.87474788883753 160.82384752100626 ) ( -742.50227651731177 -661.34478591957327 45.762340935828185 ) ( -743.97456390268746 -664 45.762340974536315 ) wood1_1 [ 0.499998 0.836517 -0.224144 -8.59906 ] [ 0.00925781 -0.263967 -0.964487 -43.358 ] 13.0082 1 1
( -743.97456390268746 -664 45.762340974536315 ) ( -742.50227651731177 -661.34478591957327 45.762340935828185 ) ( -742.51072427503652 -658.28759504188008 45.762340891699573 ) wood1_1 [ -1 0 0 -0 ] [ 0 -1 0 9.92938 ] -0 1 1
}
            )";

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            ASSERT_NO_THROW(reader.read(worldBounds, status));
        }
    }
}
