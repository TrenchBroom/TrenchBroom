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

#include "IO/NodeReader.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushContentTypeBuilder.h"
#include "Model/BrushFace.h"
#include "Model/Hit.h"
#include "Model/MapFormat.h"
#include "Model/ModelFactoryImpl.h"
#include "Model/PickResult.h"
#include "Model/World.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        TEST(BrushTest, constructBrushWithRedundantFaces) {
            const BBox3 worldBounds(4096.0);
            
            BrushFaceList faces;
            faces.push_back(BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                      Vec3(1.0, 0.0, 0.0),
                                                      Vec3(0.0, 1.0, 0.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                      Vec3(1.0, 0.0, 0.0),
                                                      Vec3(0.0, 1.0, 0.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                      Vec3(1.0, 0.0, 0.0),
                                                      Vec3(0.0, 1.0, 0.0)));
            
            ASSERT_THROW(Brush(worldBounds, faces), GeometryException);
        }
        
        TEST(BrushTest, constructBrushWithFaces) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const BrushFaceList& brushFaces = brush.faces();
            ASSERT_EQ(6u, brushFaces.size());
            for (size_t i = 0; i < faces.size(); i++)
                ASSERT_EQ(faces[i], brushFaces[i]);
        }
        
        /*
         Regex to turn a face definition into a c++ statement to add a face to a vector of faces:
         Find: \(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*\(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*\(\s*(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s+(-?[\d\.+-]+)\s*\)\s*[^\n]+
         Replace: faces.push_back(BrushFace::createParaxial(Vec3($1, $2, $3), Vec3($4, $5, $6), Vec3($7, $8, $9)));
         */
        
        TEST(BrushTest, constructWithFailingFaces) {
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
            
            const BBox3 worldBounds(4096.0);
            
            BrushFaceList faces;
            faces.push_back(BrushFace::createParaxial(Vec3(-192.0, 704.0, 128.0), Vec3(-156.0, 650.0, 128.0), Vec3(-156.0, 650.0, 160.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-202.0, 604.0, 160.0), Vec3(-164.0, 664.0, 128.0), Vec3(-216.0, 613.0, 128.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-156.0, 650.0, 128.0), Vec3(-202.0, 604.0, 128.0), Vec3(-202.0, 604.0, 160.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-192.0, 704.0, 160.0), Vec3(-256.0, 640.0, 160.0), Vec3(-256.0, 640.0, 128.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-256.0, 640.0, 160.0), Vec3(-202.0, 604.0, 160.0), Vec3(-202.0, 604.0, 128.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-217.0, 672.0, 160.0), Vec3(-161.0, 672.0, 160.0), Vec3(-161.0, 603.0, 160.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-161.0, 603.0, 128.0), Vec3(-161.0, 672.0, 128.0), Vec3(-217.0, 672.0, 128.0)));
            
            Brush brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const BrushFaceList& brushFaces = brush.faces();
            ASSERT_EQ(7u, brushFaces.size());
        }
        
        TEST(BrushTest, constructWithFailingFaces2) {
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
            
            const BBox3 worldBounds(4096.0);
            
            BrushFaceList faces;
            faces.push_back(BrushFace::createParaxial(Vec3(3488.0, 1152.0, 1340.0), Vec3(3488.0, 1248.0, 1344.0), Vec3(3488.0, 1344.0, 1340.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(3232.0, 1344.0, 1576.0), Vec3(3232.0, 1152.0, 1576.0), Vec3(3232.0, 1152.0, 1256.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(3488.0, 1344.0, 1576.0), Vec3(3264.0, 1344.0, 1576.0), Vec3(3264.0, 1344.0, 1256.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(3280.0, 1152.0, 1576.0), Vec3(3504.0, 1152.0, 1576.0), Vec3(3504.0, 1152.0, 1256.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(3488.0, 1248.0, 1344.0), Vec3(3488.0, 1152.0, 1340.0), Vec3(3232.0, 1152.0, 1340.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(3488.0, 1248.0, 1344.0), Vec3(3232.0, 1248.0, 1344.0), Vec3(3232.0, 1344.0, 1340.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(3488.0, 1152.0, 1340.0), Vec3(3360.0, 1152.0, 1344.0), Vec3(3424.0, 1344.0, 1342.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(3360.0, 1152.0, 1344.0), Vec3(3232.0, 1152.0, 1340.0), Vec3(3296.0, 1344.0, 1342.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(3504.0, 1344.0, 1280.0), Vec3(3280.0, 1344.0, 1280.0), Vec3(3280.0, 1152.0, 1280.0)));
            
            Brush brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const BrushFaceList& brushFaces = brush.faces();
            ASSERT_EQ(9u, brushFaces.size());
        }
        
        TEST(BrushTest, constructWithFailingFaces3) {
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
            
            const BBox3 worldBounds(4096.0);
            
            BrushFaceList faces;
            faces.push_back(BrushFace::createParaxial(Vec3(-32.0, -1088.0, 896.0), Vec3(-64.0, -1120.0, 896.0), Vec3(-64.0, -1120.0, 912.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-32.0, -832.0, 896.0), Vec3(-32.0, -1088.0, 896.0), Vec3(-32.0, -1088.0, 912.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-64.0, -848.0, 912.0), Vec3(-64.0, -1120.0, 912.0), Vec3(-64.0, -1120.0, 896.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-32.0, -896.0, 896.0), Vec3(-32.0, -912.0, 912.0), Vec3(-64.0, -912.0, 912.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-64.0, -1088.0, 912.0), Vec3(-64.0, -848.0, 912.0), Vec3(-32.0, -848.0, 912.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-64.0, -864.0, 896.0), Vec3(-32.0, -864.0, 896.0), Vec3(-32.0, -832.0, 896.0)));
            
            Brush brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const BrushFaceList& brushFaces = brush.faces();
            ASSERT_EQ(6u, brushFaces.size());
        }
        
        TEST(BrushTest, constructWithFailingFaces4) {
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
            
            const BBox3 worldBounds(4096.0);
            
            BrushFaceList faces;
            faces.push_back(BrushFace::createParaxial(Vec3(-1268.0, 272.0, 2524.0), Vec3(-1268.0, 272.0, 2536.0), Vec3(-1268.0, 288.0, 2540.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-1280.0, 265.0, 2534.0), Vec3(-1268.0, 272.0, 2524.0), Vec3(-1268.0, 288.0, 2528.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-1268.0, 288.0, 2528.0), Vec3(-1280.0, 288.0, 2540.0), Vec3(-1280.0, 265.0, 2534.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-1268.0, 288.0, 2540.0), Vec3(-1280.0, 288.0, 2540.0), Vec3(-1280.0, 288.0, 2536.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-1268.0, 265.0, 2534.0), Vec3(-1280.0, 265.0, 2534.0), Vec3(-1280.0, 288.0, 2540.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-1268.0, 265.0, 2534.0), Vec3(-1268.0, 272.0, 2524.0), Vec3(-1280.0, 265.0, 2534.0)));
            
            Brush brush(worldBounds, faces);
            const BrushFaceList& brushFaces = brush.faces();
            ASSERT_EQ(6u, brushFaces.size());
        }
        
        TEST(BrushTest, constructWithFailingFaces5) {
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
            
            const BBox3 worldBounds(4096.0);
            
            BrushFaceList faces;
            faces.push_back(BrushFace::createParaxial(Vec3(1296.0, 896.0, 944.0), Vec3(1296.0, 1008.0, 1056.0), Vec3(1280.0, 1008.0, 1008.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(1296.0, 1008.0, 1168.0), Vec3(1296.0, 1008.0, 1056.0), Vec3(1296.0, 896.0, 944.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(1280.0, 1008.0, 1008.0), Vec3(1280.0, 1008.0, 1168.0), Vec3(1280.0, 896.0, 1056.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(1280.0, 1008.0, 1168.0), Vec3(1280.0, 1008.0, 1008.0), Vec3(1296.0, 1008.0, 1056.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(1296.0, 1008.0, 1168.0), Vec3(1296.0, 896.0, 1056.0), Vec3(1280.0, 896.0, 1056.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(1280.0, 896.0, 896.0), Vec3(1280.0, 896.0, 1056.0), Vec3(1296.0, 896.0, 1056.0)));
            
            Brush brush(worldBounds, faces);
            assert(brush.fullySpecified());

            const BrushFaceList& brushFaces = brush.faces();
            ASSERT_EQ(6u, brushFaces.size());
        }
        
        TEST(BrushTest, constructWithFailingFaces6) {
            /* from 768_negke
             {
             ( -80 -80 -3840  ) ( -80 -80 -3824  ) ( -32 -32 -3808 ) mmetal1_2b 0 0 0 1 1 // front / right
             ( -96 -32 -3840  ) ( -96 -32 -3824  ) ( -80 -80 -3824 ) mmetal1_2 0 0 0 1 1 // left
             ( -96 -32 -3824  ) ( -32 -32 -3808  ) ( -80 -80 -3824 ) mmetal1_2b 0 0 0 1 1 // top
             ( -32 -32 -3840  ) ( -32 -32 -3808  ) ( -96 -32 -3824 ) mmetal1_2b 0 0 0 1 1 // back
             ( -32 -32 -3840  ) ( -96 -32 -3840  ) ( -80 -80 -3840 ) mmetal1_2b 0 0 0 1 1 // bottom
             }
             */
            
            const BBox3 worldBounds(4096.0);
            
            BrushFaceList faces;
            faces.push_back(BrushFace::createParaxial(Vec3(-80.0, -80.0, -3840.0), Vec3(-80.0, -80.0, -3824.0), Vec3(-32.0, -32.0, -3808.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-96.0, -32.0, -3840.0), Vec3(-96.0, -32.0, -3824.0), Vec3(-80.0, -80.0, -3824.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-96.0, -32.0, -3824.0), Vec3(-32.0, -32.0, -3808.0), Vec3(-80.0, -80.0, -3824.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-32.0, -32.0, -3840.0), Vec3(-32.0, -32.0, -3808.0), Vec3(-96.0, -32.0, -3824.0)));
            faces.push_back(BrushFace::createParaxial(Vec3(-32.0, -32.0, -3840.0), Vec3(-96.0, -32.0, -3840.0), Vec3(-80.0, -80.0, -3840.0)));
            
            Brush brush(worldBounds, faces);
            assert(brush.fullySpecified());
            
            const BrushFaceList& brushFaces = brush.faces();
            ASSERT_EQ(5u, brushFaces.size());
        }

        TEST(BrushTest, constructBrushWithManySides) {
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
            
            const BBox3 worldBounds(4096.0);
            
            BrushFaceList faces;
            faces.push_back(BrushFace::createParaxial(Vec3(624.0, 688.0, -456.0), Vec3(656.0, 760.0, -480.0), Vec3(624.0, 680.0, -480.0), "face7"));
            faces.push_back(BrushFace::createParaxial(Vec3(536.0, 792.0, -480.0), Vec3(536.0, 792.0, -432.0), Vec3(488.0, 720.0, -480.0), "face12"));
            faces.push_back(BrushFace::createParaxial(Vec3(568.0, 656.0, -464.0), Vec3(568.0, 648.0, -480.0), Vec3(520.0, 672.0, -456.0), "face14"));
            faces.push_back(BrushFace::createParaxial(Vec3(520.0, 672.0, -456.0), Vec3(520.0, 664.0, -480.0), Vec3(488.0, 720.0, -452.0), "face15"));
            faces.push_back(BrushFace::createParaxial(Vec3(560.0, 728.0, -440.0), Vec3(488.0, 720.0, -452.0), Vec3(536.0, 792.0, -432.0), "face17"));
            faces.push_back(BrushFace::createParaxial(Vec3(568.0, 656.0, -464.0), Vec3(520.0, 672.0, -456.0), Vec3(624.0, 688.0, -456.0), "face19"));
            faces.push_back(BrushFace::createParaxial(Vec3(560.0, 728.0, -440.0), Vec3(624.0, 688.0, -456.0), Vec3(520.0, 672.0, -456.0), "face20"));
            faces.push_back(BrushFace::createParaxial(Vec3(600.0, 840.0, -480.0), Vec3(536.0, 792.0, -480.0), Vec3(636.0, 812.0, -480.0), "face22"));
            
            Brush brush(worldBounds, faces);
            assert(brush.fullySpecified());
            
            const BrushFaceList& brushFaces = brush.faces();
            ASSERT_EQ(8u, brushFaces.size());
        }

        TEST(BrushTest, buildBrushAfterRotateFail) {
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

            BrushFaceList faces;
            faces.push_back(BrushFace::createParaxial(Vec3(-729.68857812925364, -128, 2061.2927432882448), Vec3(-910.70791411301013, 128, 2242.3120792720015), Vec3(-820.19824612113155, -128, 1970.7830752963655)));
            faces.push_back(BrushFace::createParaxial(Vec3(-639.17891013737574, -640, 1970.7830752963669), Vec3(-729.68857812925364, -128, 2061.2927432882448), Vec3(-729.68857812925364, -640, 1880.2734073044885)));
            faces.push_back(BrushFace::createParaxial(Vec3(-639.17891013737574, -1024, 1970.7830752963669), Vec3(-820.19824612113177, -640, 2151.8024112801227), Vec3(-639.17891013737574, -640, 1970.7830752963669)));
            faces.push_back(BrushFace::createParaxial(Vec3(-639.17891013737574, -1024, 1970.7830752963669), Vec3(-639.17891013737574, -640, 1970.7830752963669), Vec3(-729.68857812925364, -1024, 1880.2734073044885)));
            faces.push_back(BrushFace::createParaxial(Vec3(-1001.2175821048878, -128, 2151.8024112801222), Vec3(-910.70791411301013, -128, 2242.3120792720015), Vec3(-910.70791411300991, -640, 2061.2927432882443)));
            faces.push_back(BrushFace::createParaxial(Vec3(-639.17891013737574, -1024, 1970.7830752963669), Vec3(-729.68857812925364, -1024, 1880.2734073044885), Vec3(-820.19824612113177, -640, 2151.8024112801227))); // assertion failure here
            faces.push_back(BrushFace::createParaxial(Vec3(-1001.2175821048878, -128, 2151.8024112801222), Vec3(-1001.2175821048878, 128, 2151.8024112801222), Vec3(-910.70791411301013, -128, 2242.3120792720015)));
            faces.push_back(BrushFace::createParaxial(Vec3(-729.68857812925364, -1024, 1880.2734073044885), Vec3(-729.68857812925364, -640, 1880.2734073044885), Vec3(-910.70791411300991, -640, 2061.2927432882443)));
            
            const BBox3 worldBounds(4096.0);
            Brush brush(worldBounds, faces);
            assert(brush.fullySpecified());
        }
        
        TEST(BrushTest, buildBrushFail) {
            /*
             See https://github.com/kduske/TrenchBroom/issues/1186
             This crash was caused by the correction of newly created vertices in Polyhedron::Edge::split - it would nudge vertices such that their plane status changed, resulting in problems when building the seam.
             */
            
            const String data("{\n"
                              "( 656 976 672 ) ( 656 1104 672 ) ( 656 976 800 ) black -976 672 0 1 1 //TX2\n"
                              "( 632 496.00295 640 ) ( 632 688.00137 768 ) ( 504 496.00295 640 ) doortrim2 632 331 0 -1 1.49999 //TX1\n"
                              "( 666.74516 848 928 ) ( 666.74516 826.95693 1054.25842 ) ( 794.74516 848 928 ) woodplank1 -941 667 90 0.98639 -1 //TX2\n"
                              "( 672 880 416 ) ( 672 880 544 ) ( 672 1008 416 ) wswamp2_1 -880 416 0 1 1 //TX1\n"
                              "( 656 754.57864 1021.42136 ) ( -84592 754.57864 1021.42136 ) ( 656 61034.01582 -59258.01582 ) skip 1 2 0 -666 470.93310 //TX2\n"
                              "}\n");

            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            IO::NodeReader reader(data, &world);
            const NodeList nodes = reader.read(worldBounds);
            assert(nodes.size() == 1);
        }
        
        TEST(BrushTest, buildBrushFail2) {
            /*
             See https://github.com/kduske/TrenchBroom/issues/1185
             
             The cause for the endless loop was, like above, the vertex correction in Polyhedron::Edge::split.
             */
            
            const String data("{\n"
                              "( 32 1392 960 ) ( 32 1392 1088 ) ( 32 1264 960 ) black 1392 960 0 -1 1 //TX1\n"
                              "( 64 1137.02125 916.65252 ) ( 64 1243.52363 845.65079 ) ( -64 1137.02125 916.65252 ) woodplank1 64 1367 0 -1 0.83205 //TX1\n"
                              "( 5.25484 1296 864 ) ( 5.25484 1317.04307 990.25842 ) ( -122.74516 1296 864 ) woodplank1 -876 -5 90 0.98639 1 //TX2\n"
                              "( 64 1184 819.77710 ) ( 64 1184 947.77710 ) ( 64 1312 819.77710 ) woodplank1 -820 1184 90 1 -1 //TX2\n"
                              "( 16 1389.42136 957.42136 ) ( 85264 1389.42136 957.42136 ) ( 16 -58890.01582 -59322.01582 ) skip 0 -3 0 666 -470.93310 //TX2\n"
                              "}\n");

            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            IO::NodeReader reader(data, &world);
            const NodeList nodes = reader.read(worldBounds);
            assert(nodes.size() == 1);
        }
        
        TEST(BrushTest, pick) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush brush(worldBounds, faces);
            
            PickResult hits1;
            brush.pick(Ray3(Vec3(8.0, -8.0, 8.0), Vec3::PosY), hits1);
            ASSERT_EQ(1u, hits1.size());
            
            Hit hit1 = hits1.all().front();
            ASSERT_DOUBLE_EQ(8.0, hit1.distance());
            BrushFace* face1 = hit1.target<BrushFace*>();
            ASSERT_EQ(front, face1);
            
            PickResult hits2;
            brush.pick(Ray3(Vec3(8.0, -8.0, 8.0), Vec3::NegY), hits2);
            ASSERT_TRUE(hits2.empty());
        }
        
        TEST(BrushTest, partialSelectionAfterAdd) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush brush(worldBounds, faces);
            ASSERT_FALSE(brush.descendantSelected());
            left->select();
            ASSERT_TRUE(brush.descendantSelected());
            right->select();
            left->deselect();
            ASSERT_TRUE(brush.descendantSelected());
            right->deselect();
            ASSERT_FALSE(brush.descendantSelected());
        }
        
        TEST(BrushTest, partialSelectionBeforeAdd) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            left->select();
            right->select();
            
            Brush brush(worldBounds, faces);
            ASSERT_TRUE(brush.descendantSelected());
            left->deselect();
            ASSERT_TRUE(brush.descendantSelected());
            right->deselect();
            ASSERT_FALSE(brush.descendantSelected());
        }
        
        struct MatchFace {
        private:
            const BrushFace& m_face;
        public:
            MatchFace(const BrushFace& face) :
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
        
        static void assertHasFace(const Brush& brush, const BrushFace& face) {
            const BrushFaceList& faces = brush.faces();
            const BrushFaceList::const_iterator it = std::find_if(faces.begin(), faces.end(), MatchFace(face));
            ASSERT_TRUE(it != faces.end());
        }
        
        TEST(BrushTest, clone) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush original(worldBounds, faces);
            Brush* clone = original.clone(worldBounds);
            
            assertHasFace(*clone, *left);
            assertHasFace(*clone, *right);
            assertHasFace(*clone, *front);
            assertHasFace(*clone, *back);
            assertHasFace(*clone, *top);
            assertHasFace(*clone, *bottom);
            
            delete clone;
        }
        
        TEST(BrushTest, clip) {
            const BBox3 worldBounds(4096.0);
            
            // build a cube with length 16 at the origin
            BrushFace* left = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                        Vec3(0.0, 1.0, 0.0),
                                                        Vec3(0.0, 0.0, 1.0));
            BrushFace* right = BrushFace::createParaxial(Vec3(16.0, 0.0, 0.0),
                                                         Vec3(16.0, 0.0, 1.0),
                                                         Vec3(16.0, 1.0, 0.0));
            BrushFace* front = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                         Vec3(0.0, 0.0, 1.0),
                                                         Vec3(1.0, 0.0, 0.0));
            BrushFace* back = BrushFace::createParaxial(Vec3(0.0, 16.0, 0.0),
                                                        Vec3(1.0, 16.0, 0.0),
                                                        Vec3(0.0, 16.0, 1.0));
            BrushFace* top = BrushFace::createParaxial(Vec3(0.0, 0.0, 16.0),
                                                       Vec3(0.0, 1.0, 16.0),
                                                       Vec3(1.0, 0.0, 16.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3(0.0, 0.0, 0.0),
                                                          Vec3(1.0, 0.0, 0.0),
                                                          Vec3(0.0, 1.0, 0.0));
            BrushFace* clip = BrushFace::createParaxial(Vec3(8.0, 0.0, 0.0),
                                                        Vec3(8.0, 0.0, 1.0),
                                                        Vec3(8.0, 1.0, 0.0));
            
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush brush(worldBounds, faces);
            ASSERT_TRUE(brush.clip(worldBounds, clip));
            
            ASSERT_EQ(6u, brush.faces().size());
            assertHasFace(brush, *left);
            assertHasFace(brush, *clip);
            assertHasFace(brush, *front);
            assertHasFace(brush, *back);
            assertHasFace(brush, *top);
            assertHasFace(brush, *bottom);
        }
        
        TEST(BrushTest, moveBoundary) {
            const BBox3 worldBounds(4096.0);
            
            // left and right a are slanted!
            BrushFace* left   = BrushFace::createParaxial(Vec3( 0.0,  0.0, 0.0),
                                                          Vec3( 0.0,  1.0, 0.0),
                                                          Vec3( 1.0,  0.0, 1.0));
            BrushFace* right  = BrushFace::createParaxial(Vec3(16.0,  0.0, 0.0),
                                                          Vec3(15.0,  0.0, 1.0),
                                                          Vec3(16.0,  1.0, 0.0));
            BrushFace* front  = BrushFace::createParaxial(Vec3( 0.0,  0.0, 0.0),
                                                          Vec3( 0.0,  0.0, 1.0),
                                                          Vec3( 1.0,  0.0, 0.0));
            BrushFace* back   = BrushFace::createParaxial(Vec3( 0.0, 16.0, 0.0),
                                                          Vec3( 1.0, 16.0, 0.0),
                                                          Vec3( 0.0, 16.0, 1.0));
            BrushFace* top    = BrushFace::createParaxial(Vec3( 0.0,  0.0, 6.0),
                                                          Vec3( 0.0,  1.0, 6.0),
                                                          Vec3( 1.0,  0.0, 6.0));
            BrushFace* bottom = BrushFace::createParaxial(Vec3( 0.0,  0.0, 0.0),
                                                          Vec3( 1.0,  0.0, 0.0),
                                                          Vec3( 0.0,  1.0, 0.0));
            BrushFaceList faces;
            faces.push_back(left);
            faces.push_back(right);
            faces.push_back(front);
            faces.push_back(back);
            faces.push_back(top);
            faces.push_back(bottom);
            
            Brush brush(worldBounds, faces);
            ASSERT_EQ(6u, brush.faces().size());
            
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, +16.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, -16.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, +2.0)));
            ASSERT_FALSE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, -6.0)));
            ASSERT_TRUE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, +1.0)));
            ASSERT_TRUE(brush.canMoveBoundary(worldBounds, top, Vec3(0.0, 0.0, -5.0)));
            
            brush.moveBoundary(worldBounds, top, Vec3(0.0, 0.0, 1.0), false);
            ASSERT_EQ(6u, brush.faces().size());
            ASSERT_DOUBLE_EQ(7.0, brush.bounds().size().z());
        }
        
        TEST(BrushTest, moveVertex) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCube(64.0, "asdf");
            
            const Vec3 vertex(32.0, 32.0, 32.0);
            Vec3::List newVertexPositions = brush->moveVertices(worldBounds, Vec3::List(1, vertex), Vec3(-16.0, -16.0, 0.0));
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(Vec3(16.0, 16.0, 32.0), newVertexPositions[0]);
            
            newVertexPositions = brush->moveVertices(worldBounds, newVertexPositions, Vec3(16.0, 16.0, 0.0));
            ASSERT_EQ(1u, newVertexPositions.size());
            ASSERT_VEC_EQ(vertex, newVertexPositions[0]);
            
            delete brush;
        }
        
        TEST(BrushTest, moveEdge) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCube(64.0, "asdf");
            
            const Edge3 edge(Vec3(-32.0, -32.0, -32.0), Vec3(32.0, -32.0, -32.0));
            Edge3::List newEdgePositions = brush->moveEdges(worldBounds, Edge3::List(1, edge), Vec3(-16.0, -16.0, 0.0));
            ASSERT_EQ(1u, newEdgePositions.size());
            ASSERT_EQ(Edge3(Vec3(-48.0, -48.0, -32.0), Vec3(16.0, -48.0, -32.0)), newEdgePositions[0]);
            
            newEdgePositions = brush->moveEdges(worldBounds, newEdgePositions, Vec3(16.0, 16.0, 0.0));
            ASSERT_EQ(1u, newEdgePositions.size());
            ASSERT_EQ(edge, newEdgePositions[0]);
            
            delete brush;
        }
        
        TEST(BrushTest, splitEdge) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCube(64.0, "asdf");
            
            const Edge3 edge(Vec3(-32.0, -32.0, -32.0), Vec3(32.0, -32.0, -32.0));
            const Vec3 newVertexPosition = brush->splitEdge(worldBounds, edge, Vec3(-16.0, -16.0, 0.0));
            
            ASSERT_VEC_EQ(Vec3(-16.0, -48.0, -32.0), newVertexPosition);
            ASSERT_EQ(9u, brush->vertexCount());
            ASSERT_EQ(15u, brush->edgeCount());
            
            delete brush;
        }
        
        TEST(BrushTest, moveFace) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCube(64.0, "asdf");
            
            Vec3::List vertexPositions(4);
            vertexPositions[0] = Vec3(-32.0, -32.0, +32.0);
            vertexPositions[1] = Vec3(+32.0, -32.0, +32.0);
            vertexPositions[2] = Vec3(+32.0, +32.0, +32.0);
            vertexPositions[3] = Vec3(-32.0, +32.0, +32.0);
            
            const Polygon3 face(vertexPositions);
            
            Polygon3::List newFacePositions = brush->moveFaces(worldBounds, Polygon3::List(1, face), Vec3(-16.0, -16.0, 0.0));
            ASSERT_EQ(1u, newFacePositions.size());
            ASSERT_TRUE(newFacePositions[0].contains(Vec3(-48.0, -48.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].contains(Vec3(-48.0, +16.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].contains(Vec3(+16.0, +16.0, +32.0)));
            ASSERT_TRUE(newFacePositions[0].contains(Vec3(+16.0, -48.0, +32.0)));
            
            newFacePositions = brush->moveFaces(worldBounds, newFacePositions, Vec3(16.0, 16.0, 0.0));
            ASSERT_EQ(1u, newFacePositions.size());
            ASSERT_EQ(4u, newFacePositions[0].vertices().size());
            for (size_t i = 0; i < 4; ++i)
                ASSERT_TRUE(newFacePositions[0].contains(face.vertices()[i]));
            
            delete brush;
        }
        
        TEST(BrushTest, moveFaceDownFailure) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCuboid(Vec3(128.0, 128.0, 32.0), Model::BrushFace::NoTextureName);
            
            Vec3::List vertexPositions(4);
            vertexPositions[0] = Vec3(-64.0, -64.0, -16.0);
            vertexPositions[1] = Vec3(+64.0, -64.0, -16.0);
            vertexPositions[2] = Vec3(+64.0, -64.0, +16.0);
            vertexPositions[3] = Vec3(-64.0, -64.0, +16.0);
            
            const Polygon3 face(vertexPositions);
            
            ASSERT_FALSE(brush->canMoveFaces(worldBounds, Polygon3::List(1, face), Vec3(0.0, 128.0, 0.0)));
            delete brush;
        }
        
        TEST(BrushTest, splitFace) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            
            BrushBuilder builder(&world, worldBounds);
            Brush* brush = builder.createCube(64.0, "asdf");
            
            Vec3::List vertexPositions(4);
            vertexPositions[0] = Vec3(-32.0, -32.0, +32.0);
            vertexPositions[1] = Vec3(+32.0, -32.0, +32.0);
            vertexPositions[2] = Vec3(+32.0, +32.0, +32.0);
            vertexPositions[3] = Vec3(-32.0, +32.0, +32.0);
            
            const Polygon3 face(vertexPositions);
            
            const Vec3 newVertexPosition = brush->splitFace(worldBounds, face, Vec3(-16.0, +8.0, +4.0));
            
            ASSERT_VEC_EQ(Vec3(-16.0, +8.0, 36.0), newVertexPosition);
            ASSERT_EQ(9u, brush->vertexCount());
            ASSERT_EQ(16u, brush->edgeCount());
            
            delete brush;
        }
        
        TEST(BrushTest, moveVertexFail) {
            const String data("{\n"
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
            
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            IO::NodeReader reader(data, &world);
            const NodeList nodes = reader.read(worldBounds);
            assert(nodes.size() == 1);
            
            Brush* brush = static_cast<Brush*>(nodes.front());
            const Vec3 p(192.0, 128.0, 352.0);
            const Vec3 d = 4.0 * 16.0 * Vec3::NegY;
            const Vec3::List newPositions = brush->moveVertices(worldBounds, Vec3::List(1, p), d);
            ASSERT_EQ(1u, newPositions.size());
            ASSERT_VEC_EQ(p + d, newPositions.front());
        }
        
        TEST(BrushTest, subtractCuboidFromCuboid) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            
            const String minuendTexture("minuend");
            const String subtrahendTexture("subtrahend");
            const String defaultTexture("default");
            
            BrushBuilder builder(&world, worldBounds);
            Brush* minuend    = builder.createCuboid(BBox3(Vec3(-32.0, -16.0, -32.0), Vec3(32.0, 16.0, 32.0)), minuendTexture);
            Brush* subtrahend = builder.createCuboid(BBox3(Vec3(-16.0, -32.0, -64.0), Vec3(16.0, 32.0,  0.0)), subtrahendTexture);

            const BrushList result = minuend->subtract(world, worldBounds, defaultTexture, subtrahend);
            ASSERT_EQ(3u, result.size());

            const Vec3 leftTopNormal  = Vec3( 2.0, 0.0,  1.0).normalized();
            const Vec3 rightTopNormal = Vec3(-2.0, 0.0,  1.0).normalized();
            const Vec3 topLeftNormal  = Vec3(-2.0, 0.0, -1.0).normalized();
            const Vec3 topRightNormal = Vec3( 2.0, 0.0, -1.0).normalized();
            Brush* left = NULL;
            Brush* top = NULL;
            Brush* right = NULL;
            BrushList::const_iterator it, end;
            for (it = result.begin(), end = result.end(); it != end; ++it) {
                Brush* brush = *it;
                if (brush->findFaceByNormal(Vec3::PosZ) != NULL)
                    top = brush;
                else if (brush->findFaceByNormal(leftTopNormal) != NULL)
                    left = brush;
                else if (brush->findFaceByNormal(rightTopNormal) != NULL)
                    right = brush;
            }
            
            ASSERT_TRUE(left != NULL && top != NULL && right != NULL);
            
            // left brush
            ASSERT_EQ(subtrahendTexture, left->findFaceByNormal(Vec3::PosX)->textureName());
            ASSERT_EQ(minuendTexture,    left->findFaceByNormal(Vec3::NegX)->textureName());
            ASSERT_EQ(minuendTexture,    left->findFaceByNormal(Vec3::PosY)->textureName());
            ASSERT_EQ(minuendTexture,    left->findFaceByNormal(Vec3::NegY)->textureName());
            ASSERT_EQ(defaultTexture,    left->findFaceByNormal(leftTopNormal)->textureName());
            ASSERT_EQ(minuendTexture,    left->findFaceByNormal(Vec3::NegZ)->textureName());
            
            // top brush
            ASSERT_EQ(defaultTexture,    top->findFaceByNormal(topLeftNormal)->textureName());
            ASSERT_EQ(defaultTexture,    top->findFaceByNormal(topRightNormal)->textureName());
            ASSERT_EQ(minuendTexture,    top->findFaceByNormal(Vec3::PosY)->textureName());
            ASSERT_EQ(minuendTexture,    top->findFaceByNormal(Vec3::NegY)->textureName());
            ASSERT_EQ(minuendTexture,    top->findFaceByNormal(Vec3::PosZ)->textureName());
            ASSERT_EQ(subtrahendTexture, top->findFaceByNormal(Vec3::NegZ)->textureName());
            
            // right brush
            ASSERT_EQ(minuendTexture,    right->findFaceByNormal(Vec3::PosX)->textureName());
            ASSERT_EQ(subtrahendTexture, right->findFaceByNormal(Vec3::NegX)->textureName());
            ASSERT_EQ(minuendTexture,    right->findFaceByNormal(Vec3::PosY)->textureName());
            ASSERT_EQ(minuendTexture,    right->findFaceByNormal(Vec3::NegY)->textureName());
            ASSERT_EQ(defaultTexture,    right->findFaceByNormal(rightTopNormal)->textureName());
            ASSERT_EQ(minuendTexture,    right->findFaceByNormal(Vec3::NegZ)->textureName());
        }
    }
}
