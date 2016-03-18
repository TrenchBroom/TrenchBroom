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
        
        TEST(BrushTest, constructBrushAfterRotateFail) {
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
            ASSERT_TRUE(brush.fullySpecified());
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
            ASSERT_EQ(1u, nodes.size());
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
            ASSERT_EQ(1u, nodes.size());
        }
        
        TEST(BrushTest, buildBrushWithShortEdges) {
            /*
             See https://github.com/kduske/TrenchBroom/issues/1194
             */
            
            const String data("{\n"
                              "( -1248 -2144 1168 ) ( -1120 -2144 1168 ) ( -1248 -2272 1168 ) rock_1732 1248 2144 0 1 -1 //TX2\n"
                              "( -1248 -2224 1141.33333 ) ( -1248 -2224 1013.33333 ) ( -1120 -2224 1056 ) rock_1732 1391 -309 -33.69007 1.20185 -0.83205 //TX1\n"
                              "( -1408 -2144 1328 ) ( -1408 -2272 1328 ) ( -1408 -2144 1456 ) rock_1732 -1328 2144 90 1 1 //TX1\n"
                              "( -1472 -2256 1434.66667 ) ( -1472 -2256 1562.66667 ) ( -1344 -2256 1349.33334 ) skip 1681 453 -33.69007 1.20185 0.83205 //TX1\n"
                              "( -1248.00004 -2144 1061.33328 ) ( -1248.00004 -2272 1061.33328 ) ( -1120 -2144 976 ) rock_1732 1248 2144 0 1 -1 //TX1\n"
                              "}\n");

            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            IO::NodeReader reader(data, &world);
            const NodeList nodes = reader.read(worldBounds);
            ASSERT_TRUE(nodes.empty());
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
        
        TEST(BrushTest, testAlmostDegenerateBrush) {
            // https://github.com/kduske/TrenchBroom/issues/1194
            const String data("{\n"
                              "( -1248 -2144 1168 ) ( -1120 -2144 1168 ) ( -1248 -2272 1168 ) rock_1732 1248 2144 0 1 -1 //TX2\n"
                              "( -1248 -2224 1141.33333 ) ( -1248 -2224 1013.33333 ) ( -1120 -2224 1056 ) rock_1732 1391 -309 -33.69007 1.20185 -0.83205 //TX1\n"
                              "( -1408 -2144 1328 ) ( -1408 -2272 1328 ) ( -1408 -2144 1456 ) rock_1732 -1328 2144 90 1 1 //TX1\n"
                              "( -1472 -2256 1434.66667 ) ( -1472 -2256 1562.66667 ) ( -1344 -2256 1349.33334 ) skip 1681 453 -33.69007 1.20185 0.83205 //TX1\n"
                              "( -1248.00004 -2144 1061.33328 ) ( -1248.00004 -2272 1061.33328 ) ( -1120 -2144 976 ) rock_1732 1248 2144 0 1 -1 //TX1\n"
                              "}");
            
            // This brush is almost degenerate. It should be rejected by the map loader.
            
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            IO::NodeReader reader(data, &world);
            const NodeList nodes = reader.read(worldBounds);
            ASSERT_EQ(0, nodes.size());
        }
        
        static Vec3::List vertexPositions(Brush *brush) {
            Vec3::List initialPositions;
            const Brush::VertexList vertices = brush->vertices();
            Brush::VertexList::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                initialPositions.push_back((*it)->position());
            }
            return initialPositions;
        }
        
        static void assertSnapToInteger(const String &data) {
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            IO::NodeReader reader(data, &world);
            const NodeList nodes = reader.read(worldBounds);
            ASSERT_EQ(1, nodes.size());
            
            Brush* brush = static_cast<Brush*>(nodes.front());
            Vec3::List initialPositions = vertexPositions(brush);

            ASSERT_TRUE(brush->canSnapVertices(worldBounds, initialPositions, 1));
            
            Vec3::List newPositions = brush->snapVertices(worldBounds, initialPositions, 1);
            
            // Ensure they were actually snapped
            {
                const Brush::VertexList vertices = brush->vertices();
                Brush::VertexList::const_iterator it, end;
                for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                    Vec3 pos = (*it)->position();
                    ASSERT_TRUE(pos.isInteger());
                }
            }
        }
        
        TEST(BrushTest, snapIssue1198) {
            // https://github.com/kduske/TrenchBroom/issues/1198
            const String data("{\n"
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
        
        TEST(BrushTest, snapIssue1202) {
            // https://github.com/kduske/TrenchBroom/issues/1202
            const String data("{\n"
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
        
        TEST(BrushTest, snapIssue1203) {
            // https://github.com/kduske/TrenchBroom/issues/1203
            const String data("{\n"
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
        
        TEST(BrushTest, snapIssue1205) {
            // https://github.com/kduske/TrenchBroom/issues/1205
            const String data("{\n"
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
        
        TEST(BrushTest, snapIssue1206) {
            // https://github.com/kduske/TrenchBroom/issues/1206
            const String data("{\n"
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
        
        TEST(BrushTest, snapIssue1207) {
            // https://github.com/kduske/TrenchBroom/issues/1207
            const String data("{\n"
                              "( -635.50000 1442.50000 1353.50012 ) ( -763.50000 1442.50000 1353.50012 ) ( -635.50000 1314.50000 1353.50012 ) column01_3 1442 635 -90 1 -1 //TX1\n"
                              "( -635.50000 1442.50000 1355 ) ( -507.50000 1442.50000 1355 ) ( -635.50000 1314.50000 1355 ) column01_3 1442 -635 -90 1 1 //TX1\n"
                              "( -636 1442.50000 1354 ) ( -636 1442.50000 1482 ) ( -764 1442.50000 1354 ) column01_3 -636 1354 0 -1 1 //TX1\n"
                              "( -636 1438 1354 ) ( -636 1438 1482 ) ( -636 1310 1354 ) column01_3 1438 1354 0 -1 1 //TX1\n"
                              "( -635.50000 1438 1354 ) ( -635.50000 1438 1482 ) ( -507.50000 1438 1354 ) column01_3 636 1354 0 1 1 //TX1\n"
                              "( -635.50000 1442.50000 1354 ) ( -635.50000 1442.50000 1482 ) ( -635.50000 1570.50000 1354 ) column01_3 -1442 1354 0 1 1 //TX1\n"
                              "}\n");

            // This case is expected to fail to snap
            
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            IO::NodeReader reader(data, &world);
            const NodeList nodes = reader.read(worldBounds);
            ASSERT_EQ(1, nodes.size());
            
            Brush* brush = static_cast<Brush*>(nodes.front());
            Vec3::List initialPositions = vertexPositions(brush);
            
            ASSERT_FALSE(brush->canSnapVertices(worldBounds, initialPositions, 1));
        }
        
        TEST(BrushTest, snapIssue1232) {
            // https://github.com/kduske/TrenchBroom/issues/1232
            const String data("{\n"
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

            // This case is expected to fail to snap
            
            const BBox3 worldBounds(4096.0);
            World world(MapFormat::Standard, NULL, worldBounds);
            IO::NodeReader reader(data, &world);
            const NodeList nodes = reader.read(worldBounds);
            ASSERT_EQ(1, nodes.size());
            
            Brush* brush = static_cast<Brush*>(nodes.front());
            Vec3::List initialPositions = vertexPositions(brush);
            
            ASSERT_FALSE(brush->canSnapVertices(worldBounds, initialPositions, 1));
        }
    }
}
