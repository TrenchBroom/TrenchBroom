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

        TEST_CASE("BrushNodeTest.clip", "[BrushNodeTest]") {
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
            BrushFace* clip = BrushFace::createParaxial(vm::vec3(8.0, 0.0, 0.0),
                                                        vm::vec3(8.0, 0.0, 1.0),
                                                        vm::vec3(8.0, 1.0, 0.0));

            std::vector<BrushFace*> faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);

            BrushNode brush(worldBounds, faces);
            ASSERT_TRUE(brush.clip(worldBounds, clip));

            ASSERT_EQ(6u, brush.faces().size());
            assertHasFace(brush, *left);
            assertHasFace(brush, *clip);
            assertHasFace(brush, *front);
            assertHasFace(brush, *back);
            assertHasFace(brush, *top);
            assertHasFace(brush, *bottom);
        }

        TEST_CASE("BrushNodeTest.moveBoundary", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);

            // left and right a are slanted!
            BrushFace* left = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                        vm::vec3(0.0, 1.0, 0.0),
                                                        vm::vec3(1.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(vm::vec3(16.0, 0.0, 0.0),
                                                         vm::vec3(15.0, 0.0, 1.0),
                                                         vm::vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 0.0),
                                                         vm::vec3(0.0, 0.0, 1.0),
                                                         vm::vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(vm::vec3(0.0, 16.0, 0.0),
                                                        vm::vec3(1.0, 16.0, 0.0),
                                                        vm::vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(vm::vec3(0.0, 0.0, 6.0),
                                                       vm::vec3(0.0, 1.0, 6.0),
                                                       vm::vec3(1.0, 0.0, 6.0));
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
            ASSERT_EQ(6u, brush.faces().size());

            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, +16.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, -16.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, +2.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, -6.0)));
            ASSERT_TRUE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, +1.0)));
            ASSERT_TRUE(brush.canMoveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, -5.0)));

            brush.moveBoundary(worldBounds, top, vm::vec3(0.0, 0.0, 1.0), false);
            ASSERT_EQ(6u, brush.faces().size());
            ASSERT_DOUBLE_EQ(7.0, brush.logicalBounds().size().z());
        }

        TEST_CASE("BrushNodeTest.moveVertex", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom"));

            const vm::vec3 p1(-32.0, -32.0, -32.0);
            const vm::vec3 p2(-32.0, -32.0, +32.0);
            const vm::vec3 p3(-32.0, +32.0, -32.0);
            const vm::vec3 p4(-32.0, +32.0, +32.0);
            const vm::vec3 p5(+32.0, -32.0, -32.0);
            const vm::vec3 p6(+32.0, -32.0, +32.0);
            const vm::vec3 p7(+32.0, +32.0, -32.0);
            const vm::vec3 p8(+32.0, +32.0, +32.0);
            const vm::vec3 p9(+16.0, +16.0, +32.0);

            std::vector<vm::vec3> newVertexPositions = brush->moveVertices(worldBounds, std::vector<vm::vec3>(1, p8), p9 - p8);
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

            newVertexPositions = brush->moveVertices(worldBounds, newVertexPositions, p8 - p9);
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(p8, newVertexPositions[0]);

            assertTexture("left", brush, p1, p2, p4, p3);
            assertTexture("right", brush, p5, p7, p8, p6);
            assertTexture("front", brush, p1, p5, p6, p2);
            assertTexture("back", brush, p3, p4, p8, p7);
            assertTexture("top", brush, p2, p6, p8, p4);
            assertTexture("bottom", brush, p1, p3, p7, p5);

            delete brush;
        }

        TEST_CASE("BrushNodeTest.moveTetrahedronVertexToOpposideSide", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const vm::vec3 top(0.0, 0.0, +16.0);

            std::vector<vm::vec3> points;
            points.push_back(vm::vec3(-16.0, -16.0, 0.0));
            points.push_back(vm::vec3(+16.0, -16.0, 0.0));
            points.push_back(vm::vec3(0.0, +16.0, 0.0));
            points.push_back(top);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(points, "some_texture"));

            std::vector<vm::vec3> newVertexPositions = brush->moveVertices(worldBounds, std::vector<vm::vec3>(1, top), vm::vec3(0.0, 0.0, -32.0));
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(vm::vec3(0.0, 0.0, -16.0), newVertexPositions[0]);
            ASSERT_TRUE(brush->fullySpecified());

            delete brush;
        }

        TEST_CASE("BrushNodeTest.moveEdge", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom"));

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

            const vm::segment3 edge(p1, p2);
            std::vector<vm::segment3> newEdgePositions = brush->moveEdges(worldBounds, std::vector<vm::segment3>(1, edge), p1_2 - p1);
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

            ASSERT_TRUE(brush->canMoveEdges(worldBounds, newEdgePositions, p1 - p1_2));

            newEdgePositions = brush->moveEdges(worldBounds, newEdgePositions, p1 - p1_2);
            ASSERT_EQ(1u, newEdgePositions.size());
            ASSERT_EQ(edge, newEdgePositions[0]);

            assertTexture("left", brush, p1, p2, p4, p3);
            assertTexture("right", brush, p5, p7, p8, p6);
            assertTexture("front", brush, p1, p5, p6, p2);
            assertTexture("back", brush, p3, p4, p8, p7);
            assertTexture("top", brush, p2, p6, p8, p4);
            assertTexture("bottom", brush, p1, p3, p7, p5);

            delete brush;
        }

        TEST_CASE("BrushNodeTest.moveFace", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createCube(64.0, "asdf"));

            std::vector<vm::vec3> vertexPositions(4);
            vertexPositions[0] = vm::vec3(-32.0, -32.0, +32.0);
            vertexPositions[1] = vm::vec3(+32.0, -32.0, +32.0);
            vertexPositions[2] = vm::vec3(+32.0, +32.0, +32.0);
            vertexPositions[3] = vm::vec3(-32.0, +32.0, +32.0);

            const vm::polygon3 face(vertexPositions);

            ASSERT_TRUE(brush->canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, face), vm::vec3(-16.0, -16.0, 0.0)));

            std::vector<vm::polygon3> newFacePositions = brush->moveFaces(worldBounds, std::vector<vm::polygon3>(1, face), vm::vec3(-16.0, -16.0, 0.0));
            ASSERT_EQ(1u, newFacePositions.size());
            ASSERT_TRUE(newFacePositions[0].hasVertex(vm::vec3(-48.0, -48.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].hasVertex(vm::vec3(-48.0, +16.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].hasVertex(vm::vec3(+16.0, +16.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].hasVertex(vm::vec3(+16.0, -48.0, +32.0)));

            newFacePositions = brush->moveFaces(worldBounds, newFacePositions, vm::vec3(16.0, 16.0, 0.0));
            ASSERT_EQ(1u, newFacePositions.size());
            ASSERT_EQ(4u, newFacePositions[0].vertices().size());
            for (size_t i = 0; i < 4; ++i)
                ASSERT_TRUE(newFacePositions[0].hasVertex(face.vertices()[i]));

            delete brush;
        }

        template<MapFormat F>
        class UVLockTest {
            MapFormat param = F;
        };

        TEST_CASE("moveFaceWithUVLock", "[UVLockTest]") {
            auto format = GENERATE(MapFormat::Valve, MapFormat::Standard);

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(format);

            Assets::Texture testTexture("testTexture", 64, 64);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createCube(64.0, ""));
            for (auto* face : brush->faces()) {
                face->setTexture(&testTexture);
            }

            // move top face by x=+8
            auto changed = std::shared_ptr<BrushNode>(brush->clone(worldBounds));
            auto changedWithUVLock = std::shared_ptr<BrushNode>(brush->clone(worldBounds));

            const auto delta = vm::vec3(+8.0, 0.0, 0.0);
            const auto polygonToMove = vm::polygon3(brush->findFace(vm::vec3::pos_z())->vertexPositions());
            ASSERT_TRUE(changedWithUVLock->canMoveFaces(worldBounds, {polygonToMove}, delta));

            [[maybe_unused]] auto result1 = changed->moveFaces(worldBounds, {polygonToMove}, delta, false);
            [[maybe_unused]] auto result2 = changedWithUVLock->moveFaces(worldBounds, {polygonToMove}, delta, true);

            // The move should be equivalent to shearing by this matrix
            const auto M = vm::shear_bbox_matrix(brush->logicalBounds(), vm::vec3::pos_z(), delta);

            for (auto* oldFace : brush->faces()) {
                const auto oldTexCoords = kdl::vec_transform(oldFace->vertexPositions(),
                    [&](auto x) { return oldFace->textureCoords(x); });
                const auto shearedVertexPositions = kdl::vec_transform(oldFace->vertexPositions(),
                    [&](auto x) { return M * x; });
                const auto shearedPolygon = vm::polygon3(shearedVertexPositions);

                const auto normal = oldFace->boundary().normal;

                // The brush modified without texture lock is expected to have changed UV's on some faces, but not on others
                {
                    const BrushFace *newFace = changed->findFace(shearedPolygon);
                    ASSERT_NE(nullptr, newFace);
                    const auto newTexCoords = kdl::vec_transform(shearedVertexPositions,
                        [&](auto x) { return newFace->textureCoords(x); });
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
                    const BrushFace *newFaceWithUVLock = changedWithUVLock->findFace(shearedPolygon);
                    ASSERT_NE(nullptr, newFaceWithUVLock);
                    const auto newTexCoordsWithUVLock = kdl::vec_transform(shearedVertexPositions, [&](auto x) {
                        return newFaceWithUVLock->textureCoords(x);
                    });
                    if (normal == vm::vec3d::pos_z() || (format == MapFormat::Valve)) {
                        EXPECT_TRUE(UVListsEqual(oldTexCoords, newTexCoordsWithUVLock));
                    }
                }
            }
        }

        TEST_CASE("BrushNodeTest.moveFaceDownFailure", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createCuboid(vm::vec3(128.0, 128.0, 32.0), Model::BrushFaceAttributes::NoTextureName));

            std::vector<vm::vec3> vertexPositions(4);
            vertexPositions[0] = vm::vec3(-64.0, -64.0, -16.0);
            vertexPositions[1] = vm::vec3(+64.0, -64.0, -16.0);
            vertexPositions[2] = vm::vec3(+64.0, -64.0, +16.0);
            vertexPositions[3] = vm::vec3(-64.0, -64.0, +16.0);

            const vm::polygon3 face(vertexPositions);

            ASSERT_FALSE(brush->canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, face), vm::vec3(0.0, 128.0, 0.0)));
            delete brush;
        }

        static void assertCanMoveEdges(const BrushNode* brush, const std::vector<vm::segment3> edges, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            std::vector<vm::segment3> expectedMovedEdges;
            for (const vm::segment3& edge : edges) {
                expectedMovedEdges.push_back(vm::segment3(edge.start() + delta, edge.end() + delta));
            }

            ASSERT_TRUE(brush->canMoveEdges(worldBounds, edges, delta));

            BrushNode* brushClone = brush->clone(worldBounds);
            const std::vector<vm::segment3> movedEdges = brushClone->moveEdges(worldBounds, edges, delta);

            ASSERT_EQ(expectedMovedEdges, movedEdges);

            delete brushClone;
        }

        static void assertCanNotMoveEdges(const BrushNode* brush, const std::vector<vm::segment3> edges, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);
            ASSERT_FALSE(brush->canMoveEdges(worldBounds, edges, delta));
        }

        static void assertCanMoveFaces(const BrushNode* brush, const std::vector<vm::polygon3> movingFaces, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            std::vector<vm::polygon3> expectedMovedFaces;
            for (const vm::polygon3& polygon : movingFaces) {
                expectedMovedFaces.push_back(vm::polygon3(polygon.vertices() + delta));
            }

            ASSERT_TRUE(brush->canMoveFaces(worldBounds, movingFaces, delta));

            BrushNode* brushClone = brush->clone(worldBounds);
            const std::vector<vm::polygon3> movedFaces = brushClone->moveFaces(worldBounds, movingFaces, delta);

            ASSERT_EQ(expectedMovedFaces, movedFaces);

            delete brushClone;
        }

        static void assertCanNotMoveFaces(const BrushNode* brush, const std::vector<vm::polygon3> movingFaces, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);
            ASSERT_FALSE(brush->canMoveFaces(worldBounds, movingFaces, delta));
        }

        static void assertCanMoveFace(const BrushNode* brush, const BrushFace* topFace, const vm::vec3 delta) {
            assertCanMoveFaces(brush, std::vector<vm::polygon3>{topFace->polygon()}, delta);
        }

        static void assertCanNotMoveFace(const BrushNode* brush, const BrushFace* topFace, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            ASSERT_NE(nullptr, topFace);

            ASSERT_FALSE(brush->canMoveFaces(worldBounds, std::vector<vm::polygon3>{topFace->polygon()}, delta));
        }

        static void assertCanMoveTopFace(const BrushNode* brush, const vm::vec3 delta) {
            assertCanMoveFace(brush, brush->findFace(vm::vec3::pos_z()), delta);
        }

        static void assertCanNotMoveTopFace(const BrushNode* brush, const vm::vec3 delta) {
            assertCanNotMoveFace(brush, brush->findFace(vm::vec3::pos_z()), delta);
        }

        static void assertCanNotMoveTopFaceBeyond127UnitsDown(BrushNode* brush) {
            assertCanMoveTopFace(brush, vm::vec3(0, 0, -127));
            assertCanNotMoveTopFace(brush, vm::vec3(0, 0, -128));
            assertCanNotMoveTopFace(brush, vm::vec3(0, 0, -129));

            assertCanMoveTopFace(brush, vm::vec3(256, 0, -127));
            assertCanNotMoveTopFace(brush, vm::vec3(256, 0, -128));
            assertCanNotMoveTopFace(brush, vm::vec3(256, 0, -129));
        }

        static void assertCanMoveVertices(const BrushNode* brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            ASSERT_TRUE(brush->canMoveVertices(worldBounds, vertexPositions, delta));

            BrushNode* brushClone = brush->clone(worldBounds);

            auto movedVertexPositions = brushClone->moveVertices(worldBounds, vertexPositions, delta);
            kdl::vec_sort_and_remove_duplicates(movedVertexPositions);

            auto expectedVertexPositions = vertexPositions + delta;
            kdl::vec_sort_and_remove_duplicates(expectedVertexPositions);

            ASSERT_EQ(expectedVertexPositions, movedVertexPositions);

            delete brushClone;
        }

        static void assertMovingVerticesDeletes(const BrushNode* brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);

            ASSERT_TRUE(brush->canMoveVertices(worldBounds, vertexPositions, delta));

            BrushNode* brushClone = brush->clone(worldBounds);
            const std::vector<vm::vec3> movedVertexPositions = brushClone->moveVertices(worldBounds, vertexPositions, delta);

            ASSERT_EQ(std::vector<vm::vec3>(), movedVertexPositions);

            delete brushClone;
        }

        static void assertCanNotMoveVertices(const BrushNode* brush, const std::vector<vm::vec3> vertexPositions, const vm::vec3 delta) {
            const vm::bbox3 worldBounds(4096.0);
            ASSERT_FALSE(brush->canMoveVertices(worldBounds, vertexPositions, delta));
        }

        static void assertCanMoveVertex(const BrushNode* brush, const vm::vec3 vertexPosition, const vm::vec3 delta) {
            assertCanMoveVertices(brush, std::vector<vm::vec3>{vertexPosition}, delta);
        }

        static void assertMovingVertexDeletes(const BrushNode* brush, const vm::vec3 vertexPosition, const vm::vec3 delta) {
            assertMovingVerticesDeletes(brush, std::vector<vm::vec3>{vertexPosition}, delta);
        }

        static void assertCanNotMoveVertex(const BrushNode* brush, const vm::vec3 vertexPosition, const vm::vec3 delta) {
            assertCanNotMoveVertices(brush, std::vector<vm::vec3>{vertexPosition}, delta);
        }

        // "Move point" tests

        // NOTE: Different than movePolygonRemainingPoint, because in this case we allow
        // point moves that flip the normal of the remaining polygon
        TEST_CASE("BrushNodeTest.movePointRemainingPolygon", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

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
            BrushNode* brush = world.createBrush(builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName));

            assertCanMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -127.0));
            assertCanNotMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -128.0)); // Onto the base quad plane
            assertCanMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -129.0)); // Through the other side of the base quad

            // More detailed testing of the last assertion
            {
                std::vector<vm::vec3> temp(baseQuadVertexPositions);
                std::reverse(temp.begin(), temp.end());
                const std::vector<vm::vec3> flippedBaseQuadVertexPositions(temp);

                const vm::vec3 delta(0.0, 0.0, -129.0);
                BrushNode* brushClone = brush->clone(worldBounds);

                ASSERT_EQ(5u, brushClone->faceCount());
                ASSERT_TRUE(brushClone->findFace(vm::polygon3(baseQuadVertexPositions)));
                ASSERT_FALSE(brushClone->findFace(vm::polygon3(flippedBaseQuadVertexPositions)));
                ASSERT_NE(nullptr, brushClone->findFace(vm::vec3::neg_z()));
                ASSERT_EQ(nullptr, brushClone->findFace(vm::vec3::pos_z()));

                ASSERT_TRUE(brushClone->canMoveVertices(worldBounds, std::vector<vm::vec3>{peakPosition}, delta));
                ASSERT_EQ(std::vector<vm::vec3>{peakPosition + delta}, brushClone->moveVertices(worldBounds, std::vector<vm::vec3>{peakPosition}, delta));

                ASSERT_EQ(5u, brushClone->faceCount());
                ASSERT_FALSE(brushClone->findFace(vm::polygon3(baseQuadVertexPositions)));
                ASSERT_TRUE(brushClone->findFace(vm::polygon3(flippedBaseQuadVertexPositions)));
                ASSERT_EQ(nullptr, brushClone->findFace(vm::vec3::neg_z()));
                ASSERT_NE(nullptr, brushClone->findFace(vm::vec3::pos_z()));

                delete brushClone;
            }

            assertCanMoveVertex(brush, peakPosition, vm::vec3(256.0, 0.0, -127.0));
            assertCanNotMoveVertex(brush, peakPosition, vm::vec3(256.0, 0.0, -128.0)); // Onto the base quad plane
            assertCanMoveVertex(brush, peakPosition, vm::vec3(256.0, 0.0, -129.0)); // Flips the normal of the base quad, without moving through it

            delete brush;
        }

        TEST_CASE("BrushNodeTest.movePointRemainingPolyhedron", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

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
            BrushNode* brush = world.createBrush(builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName));

            assertMovingVertexDeletes(brush, peakPosition, vm::vec3(0.0, 0.0, -65.0)); // Move inside the remaining cuboid
            assertCanMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -63.0)); // Slightly above the top of the cuboid is OK
            assertCanNotMoveVertex(brush, peakPosition, vm::vec3(0.0, 0.0, -129.0)); // Through and out the other side is disallowed

            delete brush;
        }

        // "Move edge" tests

        TEST_CASE("BrushNodeTest.moveEdgeRemainingPolyhedron", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            // Taller than the cube, starts to the left of the +-64 unit cube
            const vm::segment3 edge(vm::vec3(-128, 0, -128), vm::vec3(-128, 0, +128));

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createCube(128, Model::BrushFaceAttributes::NoTextureName));
            ASSERT_NE(nullptr, brush->addVertex(worldBounds, edge.start()));
            ASSERT_NE(nullptr, brush->addVertex(worldBounds, edge.end()));

            ASSERT_EQ(10u, brush->vertexCount());

            assertCanMoveEdges(brush, std::vector<vm::segment3>{edge}, vm::vec3(+63, 0, 0));
            assertCanNotMoveEdges(brush, std::vector<vm::segment3>{edge}, vm::vec3(+64, 0, 0)); // On the side of the cube
            assertCanNotMoveEdges(brush, std::vector<vm::segment3>{edge}, vm::vec3(+128, 0, 0)); // Center of the cube

            assertCanMoveVertices(brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+63, 0, 0));
            assertCanMoveVertices(brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+64, 0, 0));
            assertCanMoveVertices(brush, asVertexList(std::vector<vm::segment3>{edge}), vm::vec3(+128, 0, 0));

            delete brush;
        }

        // Same as above, but moving 2 edges
        TEST_CASE("BrushNodeTest.moveEdgesRemainingPolyhedron", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            // Taller than the cube, starts to the left of the +-64 unit cube
            const vm::segment3 edge1(vm::vec3(-128, -32, -128), vm::vec3(-128, -32, +128));
            const vm::segment3 edge2(vm::vec3(-128, +32, -128), vm::vec3(-128, +32, +128));
            const std::vector<vm::segment3> movingEdges{edge1, edge2};

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createCube(128, Model::BrushFaceAttributes::NoTextureName));
            ASSERT_NE(nullptr, brush->addVertex(worldBounds, edge1.start()));
            ASSERT_NE(nullptr, brush->addVertex(worldBounds, edge1.end()));
            ASSERT_NE(nullptr, brush->addVertex(worldBounds, edge2.start()));
            ASSERT_NE(nullptr, brush->addVertex(worldBounds, edge2.end()));

            ASSERT_EQ(12u, brush->vertexCount());

            assertCanMoveEdges(brush, movingEdges, vm::vec3(+63, 0, 0));
            assertCanNotMoveEdges(brush, movingEdges, vm::vec3(+64, 0, 0)); // On the side of the cube
            assertCanNotMoveEdges(brush, movingEdges, vm::vec3(+128, 0, 0)); // Center of the cube

            assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+63, 0, 0));
            assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+64, 0, 0));
            assertCanMoveVertices(brush, asVertexList(movingEdges), vm::vec3(+128, 0, 0));

            delete brush;
        }

        // "Move polygon" tests

        TEST_CASE("BrushNodeTest.movePolygonRemainingPoint", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const std::vector<vm::vec3> vertexPositions{
                    vm::vec3(-64.0, -64.0, +64.0), // top quad
                    vm::vec3(-64.0, +64.0, +64.0),
                    vm::vec3(+64.0, -64.0, +64.0),
                    vm::vec3(+64.0, +64.0, +64.0),

                    vm::vec3(0.0, 0.0, -64.0), // bottom point
            };

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName));

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);

            delete brush;
        }

        TEST_CASE("BrushNodeTest.movePolygonRemainingEdge", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            const std::vector<vm::vec3> vertexPositions{
                    vm::vec3(-64.0, -64.0, +64.0), // top quad
                    vm::vec3(-64.0, +64.0, +64.0),
                    vm::vec3(+64.0, -64.0, +64.0),
                    vm::vec3(+64.0, +64.0, +64.0),

                    vm::vec3(-64.0, 0.0, -64.0), // bottom edge, on the z=-64 plane
                    vm::vec3(+64.0, 0.0, -64.0)
            };

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName));

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);

            delete brush;
        }

        TEST_CASE("BrushNodeTest.movePolygonRemainingPolygon", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createCube(128.0, Model::BrushFaceAttributes::NoTextureName));

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);

            delete brush;
        }

        TEST_CASE("BrushNodeTest.movePolygonRemainingPolygon2", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

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
            BrushNode* brush = world.createBrush(builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName));
            ASSERT_EQ(vm::bbox3(vm::vec3(-64, -64, -64), vm::vec3(64, 64, 64)), brush->logicalBounds());

            assertCanNotMoveTopFaceBeyond127UnitsDown(brush);

            delete brush;
        }

        TEST_CASE("BrushNodeTest.movePolygonRemainingPolygon_DisallowVertexCombining", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

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
            BrushNode* brush = world.createBrush(builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName));

            BrushFace* topFace = brush->findFace(topFaceNormal);
            ASSERT_NE(nullptr, topFace);

            assertCanMoveFace(brush, topFace, vm::vec3(0, 0, -127));
            assertCanMoveFace(brush, topFace, vm::vec3(0, 0, -128)); // Merge 2 verts of the moving polygon with 2 in the remaining polygon, should be allowed
            assertCanNotMoveFace(brush, topFace, vm::vec3(0, 0, -129));

            delete brush;
        }

        TEST_CASE("BrushNodeTest.movePolygonRemainingPolyhedron", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

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
            BrushNode* brush = world.createBrush(builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName));

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

            delete brush;
        }

        TEST_CASE("BrushNodeTest.moveTwoFaces", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

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
            BrushNode* brush = world.createBrush(builder.createBrush(vertexPositions, Model::BrushFaceAttributes::NoTextureName));

            EXPECT_TRUE(brush->hasFace(vm::polygon3(leftPolygon)));
            EXPECT_TRUE(brush->hasFace(vm::polygon3(bottomPolygon)));
            EXPECT_TRUE(brush->hasFace(vm::polygon3(bottomRightPolygon)));

            assertCanMoveFaces(brush, std::vector<vm::polygon3>{ vm::polygon3(leftPolygon), vm::polygon3(bottomPolygon) }, vm::vec3(0, 0, 63));
            assertCanNotMoveFaces(brush, std::vector<vm::polygon3>{ vm::polygon3(leftPolygon), vm::polygon3(bottomPolygon) }, vm::vec3(0, 0, 64)); // Merges B and C

            delete brush;
        }

        // "Move polyhedron" tests

        TEST_CASE("BrushNodeTest.movePolyhedronRemainingEdge", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            // Edge to the left of the cube, shorter, extends down to Z=-256
            const vm::segment3 edge(vm::vec3(-128, 0, -256), vm::vec3(-128, 0, 0));

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createCube(128, Model::BrushFaceAttributes::NoTextureName));
            ASSERT_NE(nullptr, brush->addVertex(worldBounds, edge.start()));
            ASSERT_NE(nullptr, brush->addVertex(worldBounds, edge.end()));

            ASSERT_EQ(10u, brush->vertexCount());

            BrushFace* cubeTop = brush->findFace(vm::vec3::pos_z());
            BrushFace* cubeBottom = brush->findFace(vm::vec3::neg_z());
            BrushFace* cubeRight = brush->findFace(vm::vec3::pos_x());
            BrushFace* cubeLeft = brush->findFace(vm::vec3::neg_x());
            BrushFace* cubeBack = brush->findFace(vm::vec3::pos_y());
            BrushFace* cubeFront = brush->findFace(vm::vec3::neg_y());

            ASSERT_NE(nullptr, cubeTop);
            ASSERT_EQ(nullptr, cubeBottom); // no face here, part of the wedge connecting to `edge`
            ASSERT_NE(nullptr, cubeRight);
            ASSERT_EQ(nullptr, cubeLeft); // no face here, part of the wedge connecting to `edge`
            ASSERT_NE(nullptr, cubeFront);
            ASSERT_NE(nullptr, cubeBack);

            const std::vector<vm::polygon3> movingFaces{
                    cubeTop->polygon(),
                    cubeRight->polygon(),
                    cubeFront->polygon(),
                    cubeBack->polygon(),
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

            delete brush;
        }

        TEST_CASE("BrushNodeTest.moveFaceFailure", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1499

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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(points, "asdf"));

            std::vector<vm::vec3> topFacePos;
            topFacePos.push_back(p1);
            topFacePos.push_back(p2);
            topFacePos.push_back(p3);
            topFacePos.push_back(p4);
            topFacePos.push_back(p5);
            topFacePos.push_back(p6);

            const vm::polygon3 topFace(topFacePos);

            ASSERT_TRUE(brush->canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(+16.0, 0.0, 0.0)));
            ASSERT_TRUE(brush->canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(-16.0, 0.0, 0.0)));
            ASSERT_TRUE(brush->canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(0.0, +16.0, 0.0)));
            ASSERT_TRUE(brush->canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(0.0, -16.0, 0.0)));
            ASSERT_TRUE(brush->canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(0.0, 0.0, +16.0)));
            ASSERT_TRUE(brush->canMoveFaces(worldBounds, std::vector<vm::polygon3>(1, topFace), vm::vec3(0.0, 0.0, -16.0)));
        }

        TEST_CASE("BrushNodeTest.moveVertexFail", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            assert(nodes.size() == 1);

            BrushNode* brush = static_cast<BrushNode*>(nodes.front());
            const vm::vec3 p(192.0, 128.0, 352.0);
            const vm::vec3 d = 4.0 * 16.0 * vm::vec3::neg_y();
            const std::vector<vm::vec3> newPositions = brush->moveVertices(worldBounds, std::vector<vm::vec3>(1, p), d);
            ASSERT_EQ(1u, newPositions.size());
            ASSERT_VEC_EQ(p + d, newPositions.front());
        }

        TEST_CASE("BrushNodeTest.moveVertexInwardWithoutMerges", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush->vertexCount());
            ASSERT_EQ(15u, brush->edgeCount());
            ASSERT_EQ(9u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p7));
            ASSERT_TRUE(brush->hasVertex(p9));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p7, p9)));


            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p2, p6, p4));
            ASSERT_TRUE(brush->hasFace(p5, p7, p6));
            ASSERT_TRUE(brush->hasFace(p3, p4, p7));
            ASSERT_TRUE(brush->hasFace(p9, p6, p7));
            ASSERT_TRUE(brush->hasFace(p9, p4, p6));
            ASSERT_TRUE(brush->hasFace(p9, p7, p4));
        }

        TEST_CASE("BrushNodeTest.moveVertexOutwardWithoutMerges", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush->vertexCount());
            ASSERT_EQ(15u, brush->edgeCount());
            ASSERT_EQ(9u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p7));
            ASSERT_TRUE(brush->hasVertex(p9));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p2, p6, p9));
            ASSERT_TRUE(brush->hasFace(p2, p9, p4));
            ASSERT_TRUE(brush->hasFace(p3, p4, p9));
            ASSERT_TRUE(brush->hasFace(p3, p9, p7));
            ASSERT_TRUE(brush->hasFace(p5, p9, p6));
            ASSERT_TRUE(brush->hasFace(p5, p7, p9));
        }

        TEST_CASE("BrushNodeTest.moveVertexWithOneOuterNeighbourMerge", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush->vertexCount());
            ASSERT_EQ(14u, brush->edgeCount());
            ASSERT_EQ(8u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p7));
            ASSERT_TRUE(brush->hasVertex(p9));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p2, p6, p9, p4));
            ASSERT_TRUE(brush->hasFace(p5, p7, p6));
            ASSERT_TRUE(brush->hasFace(p3, p4, p7));
            ASSERT_TRUE(brush->hasFace(p9, p6, p7));
            ASSERT_TRUE(brush->hasFace(p9, p7, p4));
        }

        TEST_CASE("BrushNodeTest.moveVertexWithTwoOuterNeighbourMerges", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush->vertexCount());
            ASSERT_EQ(13u, brush->edgeCount());
            ASSERT_EQ(7u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p7));
            ASSERT_TRUE(brush->hasVertex(p9));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p5, p7, p9, p6));
            ASSERT_TRUE(brush->hasFace(p3, p4, p9, p7));
            ASSERT_TRUE(brush->hasFace(p2, p6, p4));
            ASSERT_TRUE(brush->hasFace(p9, p4, p6));
        }

        TEST_CASE("BrushNodeTest.moveVertexWithAllOuterNeighbourMerges", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush->vertexCount());
            ASSERT_EQ(12u, brush->edgeCount());
            ASSERT_EQ(6u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p7));
            ASSERT_TRUE(brush->hasVertex(p9));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p2, p6, p9, p4));
            ASSERT_TRUE(brush->hasFace(p3, p4, p9, p7));
            ASSERT_TRUE(brush->hasFace(p5, p7, p9, p6));
        }

        TEST_CASE("BrushNodeTest.moveVertexWithAllInnerNeighbourMerge", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(0u, result.size());

            ASSERT_EQ(7u, brush->vertexCount());
            ASSERT_EQ(12u, brush->edgeCount());
            ASSERT_EQ(7u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p7));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p7)));

            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p2, p6, p4));
            ASSERT_TRUE(brush->hasFace(p3, p4, p7));
            ASSERT_TRUE(brush->hasFace(p5, p7, p6));
            ASSERT_TRUE(brush->hasFace(p4, p6, p7));
        }

        TEST_CASE("BrushNodeTest.moveVertexUpThroughPlane", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(8u, brush->vertexCount());
            ASSERT_EQ(13u, brush->edgeCount());
            ASSERT_EQ(7u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p7));
            ASSERT_TRUE(brush->hasVertex(p9));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p3, p4, p9, p7));
            ASSERT_TRUE(brush->hasFace(p5, p7, p9, p6));
            ASSERT_TRUE(brush->hasFace(p2, p9, p4));
            ASSERT_TRUE(brush->hasFace(p2, p6, p9));
        }

        TEST_CASE("BrushNodeTest.moveVertexOntoEdge", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(0u, result.size());

            ASSERT_EQ(7u, brush->vertexCount());
            ASSERT_EQ(12u, brush->edgeCount());
            ASSERT_EQ(7u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p7));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p7)));

            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p2, p6, p4));
            ASSERT_TRUE(brush->hasFace(p3, p4, p7));
            ASSERT_TRUE(brush->hasFace(p5, p7, p6));
            ASSERT_TRUE(brush->hasFace(p4, p6, p7));
        }

        TEST_CASE("BrushNodeTest.moveVertexOntoIncidentVertex", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p7 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p7, result[0]);

            ASSERT_EQ(7u, brush->vertexCount());
            ASSERT_EQ(12u, brush->edgeCount());
            ASSERT_EQ(7u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p7));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p7)));

            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p2, p6, p4));
            ASSERT_TRUE(brush->hasFace(p3, p4, p7));
            ASSERT_TRUE(brush->hasFace(p5, p7, p6));
            ASSERT_TRUE(brush->hasFace(p4, p6, p7));
        }

        TEST_CASE("BrushNodeTest.moveVertexOntoIncidentVertexInOppositeDirection", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p7), p8 - p7);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p8, result[0]);

            ASSERT_EQ(7u, brush->vertexCount());
            ASSERT_EQ(12u, brush->edgeCount());
            ASSERT_EQ(7u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p8));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p8)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p8)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p8)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p8)));

            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p2, p6, p8, p4));
            ASSERT_TRUE(brush->hasFace(p1, p3, p5));
            ASSERT_TRUE(brush->hasFace(p3, p4, p8));
            ASSERT_TRUE(brush->hasFace(p5, p8, p6));
            ASSERT_TRUE(brush->hasFace(p3, p8, p5));
        }

        TEST_CASE("BrushNodeTest.moveVertexAndMergeColinearEdgesWithoutDeletingVertex", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p6), p9 - p6);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(7u, brush->vertexCount());
            ASSERT_EQ(12u, brush->edgeCount());
            ASSERT_EQ(7u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p7));
            ASSERT_TRUE(brush->hasVertex(p9));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p3, p4, p9, p7));
            ASSERT_TRUE(brush->hasFace(p1, p5, p2));
            ASSERT_TRUE(brush->hasFace(p2, p5, p9));
            ASSERT_TRUE(brush->hasFace(p2, p9, p4));
            ASSERT_TRUE(brush->hasFace(p5, p7, p9));
        }

        TEST_CASE("BrushNodeTest.moveVertexAndMergeColinearEdgesWithoutDeletingVertex2", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p8), p9 - p8);
            ASSERT_EQ(1u, result.size());
            ASSERT_VEC_EQ(p9, result[0]);

            ASSERT_EQ(7u, brush->vertexCount());
            ASSERT_EQ(12u, brush->edgeCount());
            ASSERT_EQ(7u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p7));
            ASSERT_TRUE(brush->hasVertex(p9));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p9)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p7, p9)));

            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p1, p5, p9, p2));
            ASSERT_TRUE(brush->hasFace(p2, p9, p4));
            ASSERT_TRUE(brush->hasFace(p3, p4, p7));
            ASSERT_TRUE(brush->hasFace(p4, p9, p7));
            ASSERT_TRUE(brush->hasFace(p5, p7, p9));
        }

        TEST_CASE("BrushNodeTest.moveVertexAndMergeColinearEdgesWithDeletingVertex", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            const std::vector<vm::vec3d> result = brush->moveVertices(worldBounds, std::vector<vm::vec3d>(1, p9), p10 - p9);
            ASSERT_EQ(0u, result.size());

            ASSERT_EQ(8u, brush->vertexCount());
            ASSERT_EQ(12u, brush->edgeCount());
            ASSERT_EQ(6u, brush->faceCount());

            ASSERT_TRUE(brush->hasVertex(p1));
            ASSERT_TRUE(brush->hasVertex(p2));
            ASSERT_TRUE(brush->hasVertex(p3));
            ASSERT_TRUE(brush->hasVertex(p4));
            ASSERT_TRUE(brush->hasVertex(p5));
            ASSERT_TRUE(brush->hasVertex(p6));
            ASSERT_TRUE(brush->hasVertex(p7));
            ASSERT_TRUE(brush->hasVertex(p8));

            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p2)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p3)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p1, p5)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p2, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p4)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p3, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p4, p8)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p6)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p5, p7)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p6, p8)));
            ASSERT_TRUE(brush->hasEdge(vm::segment3d(p7, p8)));

            ASSERT_TRUE(brush->hasFace(p1, p2, p4, p3));
            ASSERT_TRUE(brush->hasFace(p1, p3, p7, p5));
            ASSERT_TRUE(brush->hasFace(p1, p5, p6, p2));
            ASSERT_TRUE(brush->hasFace(p2, p6, p8, p4));
            ASSERT_TRUE(brush->hasFace(p3, p4, p8, p7));
            ASSERT_TRUE(brush->hasFace(p5, p7, p8, p6));
        }

        TEST_CASE("BrushNodeTest.moveVertexFailing1", "[BrushNodeTest]") {
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
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createBrush(oldPositions, "texture"));

            for (size_t i = 0; i < oldPositions.size(); ++i) {
                for (size_t j = 0; j < oldPositions.size(); ++j) {
                    if (i != j) {
                        ASSERT_FALSE(brush->canMoveVertices(worldBounds, std::vector<vm::vec3d>(1, oldPositions[i]), oldPositions[j] - oldPositions[i]));
                    }
                }
            }
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

        static void assertCannotSnapTo(const std::string& data, const FloatType gridSize) {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            ASSERT_EQ(1u, nodes.size());

            BrushNode* brush = static_cast<BrushNode*>(nodes.front());
            ASSERT_FALSE(brush->canSnapVertices(worldBounds, gridSize));
        }

        static void assertCannotSnap(const std::string& data) {
            assertCannotSnapTo(data, 1.0);
        }

        static void assertSnapTo(const std::string& data, const FloatType gridSize) {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);

            IO::TestParserStatus status;
            IO::NodeReader reader(data, world);

            const std::vector<Node*> nodes = reader.read(worldBounds, status);
            ASSERT_EQ(1u, nodes.size());

            BrushNode* brush = static_cast<BrushNode*>(nodes.front());
            ASSERT_TRUE(brush->canSnapVertices(worldBounds, gridSize));

            brush->snapVertices(worldBounds, gridSize);
            ASSERT_TRUE(brush->fullySpecified());

            // Ensure they were actually snapped
            {
                for (const Model::BrushVertex* vertex : brush->vertices()) {
                    const vm::vec3& pos = vertex->position();
                    ASSERT_TRUE(vm::is_integral(pos, 0.001));
                }
            }
        }

        static void assertSnapToInteger(const std::string& data) {
            assertSnapTo(data, 1.0);
        }

        TEST_CASE("BrushNodeTest.snapIssue1198", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1198
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

        TEST_CASE("BrushNodeTest.snapIssue1202", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1202
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

        TEST_CASE("BrushNodeTest.snapIssue1203", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1203
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

        TEST_CASE("BrushNodeTest.snapIssue1205", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1205
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

        TEST_CASE("BrushNodeTest.snapIssue1206", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1206
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

        TEST_CASE("BrushNodeTest.snapIssue1207", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1207
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

        TEST_CASE("BrushNodeTest.snapIssue1232", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1232
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

        TEST_CASE("BrushNodeTest.snapIssue1395_24202", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1395 brush at line 24202
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

        TEST_CASE("BrushNodeTest.snapIssue1395_18995", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1395 brush at line 24202
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

        TEST_CASE("BrushNodeTest.snapToGrid64", "[BrushNodeTest]") {
            // https://github.com/kduske/TrenchBroom/issues/1415
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

        TEST_CASE("BrushNodeTest.removeSingleVertex", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

            BrushBuilder builder(&world, worldBounds);
            BrushNode* brush = world.createBrush(builder.createCube(64.0, "asdf"));


            brush->removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, +32.0, +32.0)));

            ASSERT_EQ(7u, brush->vertexCount());
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush->hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            brush->removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, +32.0, -32.0)));

            ASSERT_EQ(6u, brush->vertexCount());
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_FALSE(brush->hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush->hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            brush->removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, -32.0, +32.0)));

            ASSERT_EQ(5u, brush->vertexCount());
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_FALSE(brush->hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_FALSE(brush->hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush->hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            brush->removeVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, -32.0, -32.0)));

            ASSERT_EQ(4u, brush->vertexCount());
            ASSERT_FALSE(brush->hasVertex(vm::vec3(-32.0, -32.0, -32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, -32.0, +32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, +32.0, -32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(-32.0, +32.0, +32.0)));
            ASSERT_TRUE (brush->hasVertex(vm::vec3(+32.0, -32.0, -32.0)));
            ASSERT_FALSE(brush->hasVertex(vm::vec3(+32.0, -32.0, +32.0)));
            ASSERT_FALSE(brush->hasVertex(vm::vec3(+32.0, +32.0, -32.0)));
            ASSERT_FALSE(brush->hasVertex(vm::vec3(+32.0, +32.0, +32.0)));


            ASSERT_FALSE(brush->canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, -32.0, +32.0))));
            ASSERT_FALSE(brush->canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, +32.0, -32.0))));
            ASSERT_FALSE(brush->canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(-32.0, +32.0, +32.0))));
            ASSERT_FALSE(brush->canRemoveVertices(worldBounds, std::vector<vm::vec3>(1, vm::vec3(+32.0, -32.0, -32.0))));

            delete brush;
        }


        TEST_CASE("BrushNodeTest.removeMultipleVertices", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);
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

                        BrushNode* brush = world.createBrush(builder.createBrush(vertices, "asdf"));
                        ASSERT_TRUE(brush->canRemoveVertices(worldBounds, toRemove));
                        brush->removeVertices(worldBounds, toRemove);

                        for (size_t l = 0; l < 8; ++l) {
                            if (l != i && l != j && l != k) {
                                ASSERT_TRUE(brush->hasVertex(vertices[l]));
                            }
                        }

                        delete brush;
                    }
                }
            }
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

        TEST_CASE("BrushNodeTest.resizePastWorldBounds", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Model::BrushNode* brush1 = world.createBrush(builder.createBrush(std::vector<vm::vec3>{vm::vec3(64, -64, 16), vm::vec3(64, 64, 16), vm::vec3(64, -64, -16), vm::vec3(64, 64, -16), vm::vec3(48, 64, 16), vm::vec3(48, 64, -16)}, "texture"));

            Model::BrushFace* rightFace = brush1->findFace(vm::vec3(1, 0, 0));
            ASSERT_NE(nullptr, rightFace);

            EXPECT_TRUE(brush1->canMoveBoundary(worldBounds, rightFace, vm::vec3(16, 0, 0)));
            EXPECT_FALSE(brush1->canMoveBoundary(worldBounds, rightFace, vm::vec3(8000, 0, 0)));
        }

        TEST_CASE("BrushNodeTest.moveVerticesPastWorldBounds", "[BrushNodeTest]") {
            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);
            const BrushBuilder builder(&world, worldBounds);

            Model::BrushNode* brush1 = world.createBrush(builder.createCube(128.0, "texture"));

            std::vector<vm::vec3> allVertexPositions;
            for (const auto* vertex : brush1->vertices()) {
                allVertexPositions.push_back(vertex->position());
            }

            EXPECT_TRUE(brush1->canMoveVertices(worldBounds, allVertexPositions, vm::vec3(16, 0, 0)));
            EXPECT_FALSE(brush1->canMoveVertices(worldBounds, allVertexPositions, vm::vec3(8192, 0, 0)));
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

        TEST_CASE("BrushNodeTest.removeVertexWithCorrectTextures", "[BrushNodeTest]") {
            // see https://github.com/kduske/TrenchBroom/issues/2082

            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(Model::MapFormat::Valve);

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
            IO::NodeReader reader(data, world);

            std::vector<Node*> nodes = reader.read(worldBounds, status);
            ASSERT_EQ(1u, nodes.size());

            BrushNode* brush = static_cast<BrushNode*>(nodes.front());

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
            ASSERT_TRUE(brush->canRemoveVertices(worldBounds, std::vector<vm::vec3d>{p7}));
            brush->removeVertices(worldBounds, std::vector<vm::vec3d>{p7});

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

            delete brush;
        }

        TEST_CASE("BrushNodeTest.moveVerticesFail_2158", "[BrushNodeTest]") {
            // see https://github.com/kduske/TrenchBroom/issues/2158
            const vm::bbox3 worldBounds(4096.0);
            WorldNode world(MapFormat::Standard);

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
            IO::NodeReader reader(data, world);

            auto nodes = reader.read(worldBounds, status);
            ASSERT_EQ(1u, nodes.size());

            std::unique_ptr<BrushNode> brush(static_cast<BrushNode*>(nodes.front()));

            const std::vector<vm::vec3> vertexPositions {
                brush->findClosestVertexPosition(vm::vec3(1169.1764156206966, -1800.2961013859342, 568.79748529920892)),
                brush->findClosestVertexPosition(vm::vec3(1164.1689509627774, -1797.7259237617193, 578.31488545196294)),
                brush->findClosestVertexPosition(vm::vec3(1163.5185572994671, -1820.7940760208414, 554.17919392904093)),
                brush->findClosestVertexPosition(vm::vec3(1120.5128684458623, -1855.3192739534061, 574.53563498325116))
            };

            ASSERT_TRUE(brush->canMoveVertices(worldBounds, vertexPositions, vm::vec3(16.0, 0.0, 0.0)));
            ASSERT_NO_THROW(brush->moveVertices(worldBounds, vertexPositions, vm::vec3(16.0, 0.0, 0.0)));
        }

        TEST_CASE("BrushNodeTest.moveEdgesFail_2361", "[BrushNodeTest]") {
            // see https://github.com/kduske/TrenchBroom/issues/2361

            const vm::bbox3 worldBounds(8192.0);
            WorldNode world(MapFormat::Standard);

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
            IO::NodeReader reader(data, world);

            auto nodes = reader.read(worldBounds, status);
            ASSERT_EQ(1u, nodes.size());

            std::unique_ptr<BrushNode> brush(static_cast<BrushNode*>(nodes.front()));

            const auto vertex1 = brush->findClosestVertexPosition(vm::vec3(-5774.7302805949275, 488.0, 1108.0));
            const auto vertex2 = brush->findClosestVertexPosition(vm::vec3(-5730.730280440197,  486.0, 1108.0));
            const auto segment = vm::segment3(vertex1, vertex2);

            ASSERT_TRUE(brush->canMoveEdges(worldBounds, std::vector<vm::segment3>{ segment }, vm::vec3(0.0, -4.0, 0.0)));
            ASSERT_NO_THROW(brush->moveEdges(worldBounds, std::vector<vm::segment3>{ segment }, vm::vec3(0.0, -4.0, 0.0)));
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

        std::vector<vm::vec3> asVertexList(const std::vector<vm::segment3>& edges) {
            std::vector<vm::vec3> result;
            vm::segment3::get_vertices(std::begin(edges), std::end(edges), std::back_inserter(result));
            return result;
        }

        std::vector<vm::vec3> asVertexList(const std::vector<vm::polygon3>& faces) {
            std::vector<vm::vec3> result;
            vm::polygon3::get_vertices(std::begin(faces), std::end(faces), std::back_inserter(result));
            return result;
        }
    }
}
