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

#include "IO/WorldReader.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        inline Model::BrushFace* findFaceByPoints(const Model::BrushFaceList& faces, const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                if (face->points()[0] == point0 &&
                    face->points()[1] == point1 &&
                    face->points()[2] == point2)
                    return face;
            }
            return NULL;
        }
        
        TEST(WorldReaderTest, parseEmptyMap) {
            const String data("");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);
            
            ASSERT_TRUE(world != NULL);
            ASSERT_EQ(1u, world->childCount());
            ASSERT_FALSE(world->children().front()->hasChildren());
        }

        TEST(WorldReaderTest, parseMapWithEmptyEntity) {
            const String data("{}");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);
            
            ASSERT_TRUE(world != NULL);
            ASSERT_EQ(1u, world->childCount());
            ASSERT_EQ(1u, world->children().front()->childCount());
        }

        TEST(WorldReaderTest, parseMapWithWorldspawn) {
            const String data("{"
                              "\"classname\" \"worldspawn\""
                              "\"message\" \"yay\""
                              "}");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);
            
            ASSERT_TRUE(world != NULL);
            ASSERT_EQ(1u, world->childCount());
            ASSERT_FALSE(world->children().front()->hasChildren());
            
            ASSERT_TRUE(world->hasAttribute(Model::AttributeNames::Classname));
            ASSERT_STREQ("yay", world->attribute("message").c_str());
        }

        TEST(WorldReaderTest, parseMapWithWorldspawnAndOneMoreEntity) {
            const String data("{"
                              "\"classname\" \"worldspawn\""
                              "\"message\" \"yay\""
                              "}"
                              "{"
                              "\"classname\" \"info_player_deathmatch\""
                              "\"origin\" \"1 22 -3\""
                              "\"angle\" \" -1 \""
                              "}");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);
            
            ASSERT_TRUE(world != NULL);
            ASSERT_TRUE(world->hasAttribute(Model::AttributeNames::Classname));
            ASSERT_STREQ("yay", world->attribute("message").c_str());

            ASSERT_EQ(1u, world->childCount());
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(1u, defaultLayer->childCount());
            
            Model::Entity* entity = static_cast<Model::Entity*>(defaultLayer->children().front());
            ASSERT_TRUE(entity->hasAttribute("classname"));
            ASSERT_STREQ("info_player_deathmatch", entity->attribute("classname").c_str());
            ASSERT_TRUE(entity->hasAttribute("origin"));
            ASSERT_STREQ("1 22 -3", entity->attribute("origin").c_str());
            ASSERT_TRUE(entity->hasAttribute("angle"));
            ASSERT_STREQ(" -1 ", entity->attribute("angle").c_str());
        }

        TEST(WorldReaderTest, parseMapWithWorldspawnAndOneBrush) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) tex1 1 2 3 4 5\n"
                              "( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) tex2 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) tex3 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) tex4 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) tex5 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) tex6 0 0 0 1 1\n"
                              "}\n"
                              "}\n");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);

            ASSERT_EQ(1u, world->childCount());
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(1u, defaultLayer->childCount());
            
            Model::Brush* brush = static_cast<Model::Brush*>(defaultLayer->children().front());
            const Model::BrushFaceList& faces = brush->faces();
            ASSERT_EQ(6u, faces.size());
            
            const Model::BrushFace* face1 = findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,   0.0,   0.0), Vec3( 64.0,   0.0, -16.0));
            ASSERT_TRUE(face1 != NULL);
            ASSERT_STREQ("tex1", face1->textureName().c_str());
            ASSERT_FLOAT_EQ(1.0, face1->xOffset());
            ASSERT_FLOAT_EQ(2.0, face1->yOffset());
            ASSERT_FLOAT_EQ(3.0, face1->rotation());
            ASSERT_FLOAT_EQ(4.0, face1->xScale());
            ASSERT_FLOAT_EQ(5.0, face1->yScale());
            
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0), Vec3(  0.0,   0.0,   0.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3( 64.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3(  0.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0), Vec3( 64.0,   0.0,   0.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,   0.0,   0.0), Vec3(  0.0,  64.0,   0.0)) != NULL);
        }

        TEST(WorldReaderTest, parseMapAndCheckFaceFlags) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) none 22 -3 56.2 1.03433 -0.55\n"
                              "( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1\n"
                              "}\n"
                              "}\n");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);
            
            ASSERT_EQ(1u, world->childCount());
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(1u, defaultLayer->childCount());
            
            Model::Brush* brush = static_cast<Model::Brush*>(defaultLayer->children().front());
            const Model::BrushFaceList& faces = brush->faces();
            ASSERT_EQ(6u, faces.size());
            
            Model::BrushFace* face = findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,   0.0,   0.0), Vec3( 64.0,   0.0, -16.0));
            ASSERT_TRUE(face != NULL);
            ASSERT_FLOAT_EQ(22.0f, face->xOffset());
            ASSERT_FLOAT_EQ(22.0f, face->xOffset());
            ASSERT_FLOAT_EQ(56.2f, face->rotation());
            ASSERT_FLOAT_EQ(1.03433f, face->xScale());
            ASSERT_FLOAT_EQ(-0.55f, face->yScale());
        }

        TEST(WorldReaderTest, parseBrushWithCurlyBraceInTextureName) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) {none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1\n"
                              "}\n"
                              "}\n");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);
            
            ASSERT_EQ(1u, world->childCount());
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(1u, defaultLayer->childCount());
            
            Model::Brush* brush = static_cast<Model::Brush*>(defaultLayer->children().front());
            const Model::BrushFaceList faces = brush->faces();
            ASSERT_EQ(6u, faces.size());
            
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,   0.0,   0.0), Vec3( 64.0,   0.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0), Vec3(  0.0,   0.0,   0.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3( 64.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3(  0.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0), Vec3( 64.0,   0.0,   0.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,   0.0,   0.0), Vec3(  0.0,  64.0,   0.0)) != NULL);
        }

        TEST(WorldReaderTest, parseProblematicBrush1) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( 308 108 176 ) ( 308 132 176 ) ( 252 132 176 ) mt_sr_v13 -59 13 -90 1 1\n"
                              "( 252 132 208 ) ( 308 132 208 ) ( 308 108 208 ) mt_sr_v13 -59 13 -90 1 1\n"
                              "( 288 152 176 ) ( 288 152 208 ) ( 288 120 208 ) mt_sr_v13 -59 -110 -180 1 1\n"
                              "( 288 122 176 ) ( 288 122 208 ) ( 308 102 208 ) mt_sr_v13 -37 -111 -180 1 1\n"
                              "( 308 100 176 ) ( 308 100 208 ) ( 324 116 208 ) mt_sr_v13 -100 -111 0 1 -1\n"
                              "( 287 152 208 ) ( 287 152 176 ) ( 323 116 176 ) mt_sr_v13 -65 -111 -180 1 1\n"
                              "}\n"
                              "}\n");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);
            
            ASSERT_EQ(1u, world->childCount());
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(1u, defaultLayer->childCount());
            
            Model::Brush* brush = static_cast<Model::Brush*>(defaultLayer->children().front());
            const Model::BrushFaceList faces = brush->faces();
            ASSERT_EQ(6u, faces.size());
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(308.0, 108.0, 176.0), Vec3(308.0, 132.0, 176.0), Vec3(252.0, 132.0, 176.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(252.0, 132.0, 208.0), Vec3(308.0, 132.0, 208.0), Vec3(308.0, 108.0, 208.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(288.0, 152.0, 176.0), Vec3(288.0, 152.0, 208.0), Vec3(288.0, 120.0, 208.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(288.0, 122.0, 176.0), Vec3(288.0, 122.0, 208.0), Vec3(308.0, 102.0, 208.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(308.0, 100.0, 176.0), Vec3(308.0, 100.0, 208.0), Vec3(324.0, 116.0, 208.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(287.0, 152.0, 208.0), Vec3(287.0, 152.0, 176.0), Vec3(323.0, 116.0, 176.0)) != NULL);
        }

        TEST(WorldReaderTest, parseProblematicBrush2) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -572 1078 128 ) ( -594 1088 128 ) ( -597 1072 96 ) mt_sr_v16 -64 0 -180 1 -1\n"
                              "( -572 1078 160 ) ( -572 1078 128 ) ( -590 1051 128 ) b_rc_v4 32 0 90 1 1\n"
                              "( -601 1056 160 ) ( -601 1056 128 ) ( -594 1088 128 ) b_rc_v4 32 0 90 1 1\n"
                              "( -590 1051 160 ) ( -590 1051 128 ) ( -601 1056 128 ) b_rc_v4 32 -16 90 1 1\n"
                              "( -512 1051 128 ) ( -624 1051 128 ) ( -568 1088 128 ) b_rc_v4 0 -16 90 1 1\n"
                              "( -559 1090 96 ) ( -598 1090 96 ) ( -598 1055 96 ) mt_sr_v13 -16 0 0 1 1\n"
                              "}\n"
                              "}\n");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);
            
            ASSERT_EQ(1u, world->childCount());
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(1u, defaultLayer->childCount());
        }
        
        TEST(WorldReaderTest, parseProblematicBrush3) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( 256 1152 -96 ) ( 160 1152 -96 ) ( 160 1120 -96 ) b_rc_v4 31 -31 90 1 1\n"
                              "( -64 1120 64 ) ( -64 1184 64 ) ( -32 1184 32 ) b_rc_v4 31 -31 90 1 1\n"
                              "( -112 1120 32 ) ( 224 1120 32 ) ( 224 1120 -96 ) b_rc_v4 0 0 90 1 1\n"
                              "( -112 1184 -96 ) ( 264 1184 -96 ) ( 264 1184 32 ) b_rc_v4 -127 -32 90 1 1\n"
                              "( -64 1184 64 ) ( -64 1120 64 ) ( -64 1120 -96 ) b_rc_v4 -127 32 90 1 1\n"
                              "( -32 1136 32 ) ( -32 1152 -96 ) ( -32 1120 -96 ) b_rc_v4 0 32 90 1 1\n"
                              "}\n"
                              "}\n");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);
            
            ASSERT_EQ(1u, world->childCount());
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(1u, defaultLayer->childCount());
        }

        TEST(WorldReaderTest, parseValveBrush) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) METAL4_5 [ 1 0 0 64 ] [ 0 -1 0 0 ] 0 1 1\n"
                              "( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) METAL4_5 [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1 \n"
                              "( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) METAL4_5 [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1 \n"
                              "( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) METAL4_5 [ 1 0 0 64 ] [ 0 0 -1 0 ] 0 1 1 \n"
                              "( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) METAL4_5 [ 1 0 0 64 ] [ 0 0 -1 0 ] 0 1 1 \n"
                              "( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) METAL4_5 [ 1 0 0 64 ] [ 0 -1 0 0 ] 0 1 1 \n"
                              "}\n"
                              "}\n");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Valve, worldBounds);
            
            ASSERT_EQ(1u, world->childCount());
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(1u, defaultLayer->childCount());
        }
        
        TEST(WorldReaderTest, parseQuake2Brush) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) rtz/c_mf_v3c 56 -32 0 1 1 0 0 0\n"
                              "( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) rtz/b_rc_v16w 32 32 0 1 1\n"
                              "( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) rtz/c_mf_v3c 16 96 0 1 1\n" // The flags are optional.
                              "( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1 0 0 0\n"
                              "( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1 0 0 0\n"
                              "( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1 0 0 0\n"
                              "}\n"
                              "}\n");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Quake2, worldBounds);
            
            ASSERT_EQ(1u, world->childCount());
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(1u, defaultLayer->childCount());
        }
        
        TEST(WorldReaderTest, parseQuakeBrushWithNumericalTextureName) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) c_mf_v3c 56 -32 0 1 1\n"
                              "( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) b_rc_v16w 32 32 0 1 1\n"
                              "( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) 666 16 96 0 1 1\n"
                              "( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) c_mf_v3c 56 96 0 1 1\n"
                              "( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) c_mf_v3c 56 96 0 1 1\n"
                              "( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) c_mf_v3c 16 96 0 1 1\n"
                              "}\n"
                              "}\n");
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Standard, worldBounds);
            
            ASSERT_EQ(1u, world->childCount());
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(1u, defaultLayer->childCount());
        }
        
        TEST(WorldReaderTest, parseBrushesWithLayer) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1\n"
                              "}\n"
                              "{\n"
                              "( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) rtz/b_rc_v16w 32 32 0 1 1\n"
                              "( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) rtz/c_mf_v3c 16 96 0 1 1\n"
                              "( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1\n"
                              "( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1\n"
                              "( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1\n"
                              "}\n"
                              "}\n"
                              "{\n"
                              "\"classname\" \"func_group\"\n"
                              "\"_tb_type\" \"_tb_layer\"\n"
                              "\"_tb_name\" \"My Layer\"\n"
                              "\"_tb_id\" \"1\"\n"
                              "{\n"
                              "( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "}\n"
                              "}\n"
                              );
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Quake2, worldBounds);
            
            ASSERT_EQ(2u, world->childCount());
            ASSERT_EQ(2u, world->children().front()->childCount());
            ASSERT_EQ(1u, world->children().back()->childCount());
        }
        
        TEST(WorldReaderTest, parseEntitiesAndBrushesWithLayer) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1\n"
                              "}\n"
                              "{\n"
                              "( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) rtz/b_rc_v16w 32 32 0 1 1\n"
                              "( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) rtz/c_mf_v3c 16 96 0 1 1\n"
                              "( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1\n"
                              "( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1\n"
                              "( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1\n"
                              "}\n"
                              "}\n"
                              "{\n"
                              "\"classname\" \"func_group\"\n"
                              "\"_tb_type\" \"_tb_layer\"\n"
                              "\"_tb_name\" \"My Layer\"\n"
                              "\"_tb_id\" \"1\"\n"
                              "{\n"
                              "( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "}\n"
                              "}\n"
                              "{\n"
                              "\"classname\" \"func_door\"\n"
                              "\"_tb_layer\" \"1\"\n"
                              "{\n"
                              "( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "}\n"
                              "}\n"
                              );
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Quake2, worldBounds);
            
            ASSERT_EQ(2u, world->childCount());
            ASSERT_EQ(2u, world->children().front()->childCount()); // default layer
            ASSERT_EQ(2u, world->children().back()->childCount()); // My Layer
            ASSERT_EQ(1u, world->children().back()->children().back()->childCount());
        }
        
        TEST(WorldReaderTest, parseEntitiesAndBrushesWithGroup) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1\n"
                              "}\n"
                              "{\n"
                              "( -712 1280 -448 ) ( -904 1280 -448 ) ( -904 992 -448 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -904 992 -416 ) ( -904 1280 -416 ) ( -712 1280 -416 ) rtz/b_rc_v16w 32 32 0 1 1\n"
                              "( -832 968 -416 ) ( -832 1256 -416 ) ( -832 1256 -448 ) rtz/c_mf_v3c 16 96 0 1 1\n"
                              "( -920 1088 -448 ) ( -920 1088 -416 ) ( -680 1088 -416 ) rtz/c_mf_v3c 56 96 0 1 1\n"
                              "( -968 1152 -448 ) ( -920 1152 -448 ) ( -944 1152 -416 ) rtz/c_mf_v3c 56 96 0 1 1\n"
                              "( -896 1056 -416 ) ( -896 1056 -448 ) ( -896 1344 -448 ) rtz/c_mf_v3c 16 96 0 1 1\n"
                              "}\n"
                              "}\n"
                              "{\n"
                              "\"classname\" \"func_group\"\n"
                              "\"_tb_type\" \"_tb_group\"\n"
                              "\"_tb_name\" \"My Group\"\n"
                              "\"_tb_id\" \"1\"\n"
                              "{\n"
                              "( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "}\n"
                              "}\n"
                              "{\n"
                              "\"classname\" \"func_door\"\n"
                              "\"_tb_group\" \"1\"\n"
                              "{\n"
                              "( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "}\n"
                              "}\n"
                              "{\n"
                              "\"classname\" \"func_group\"\n"
                              "\"_tb_type\" \"_tb_group\"\n"
                              "\"_tb_name\" \"My Subroup\"\n"
                              "\"_tb_id\" \"2\"\n"
                              "\"_tb_group\" \"1\"\n"
                              "{\n"
                              "( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1\n"
                              "}\n"
                              "}\n"
                              );
            BBox3 worldBounds(8192);
            
            WorldReader reader(data, NULL);
            Model::World* world = reader.read(Model::MapFormat::Quake2, worldBounds);
            
            ASSERT_EQ(1u, world->childCount());
            
            Model::Node* defaultLayer = world->children().front();
            ASSERT_EQ(3u, defaultLayer->childCount());
            
            Model::Node* myGroup = defaultLayer->children().back();
            ASSERT_EQ(3u, myGroup->childCount());
            
            Model::Node* mySubGroup = myGroup->children().back();
            ASSERT_EQ(1u, mySubGroup->childCount());
        }

        /*
        TEST(WorldReaderTest, parseIssueIgnoreFlags) {
            const String data("{"
                              "\"classname\" \"worldspawn\""
                              "{\n"
                              "/// hideIssues 2\n"
                              "( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1\n"
                              "}\n"
                              "}"
                              "{"
                              "/// hideIssues 3\n"
                              "\"classname\" \"info_player_deathmatch\""
                              "\"origin\" \"1 22 -3\""
                              "\"angle\" \" -1 \""
                              "}");
            BBox3 worldBounds(-8192, 8192);
            
            using namespace testing;
            Model::MockGamePtr game = Model::MockGame::newGame();
            EXPECT_CALL(*game, doBrushContentTypes()).WillOnce(ReturnRef(Model::BrushContentType::EmptyList));
            
            StandardMapParser parser(data, game.get());
            Model::Map* map = parser.parseMap(worldBounds);
            
            const Model::EntityList& entities = map->entities();
            ASSERT_EQ(2u, entities.size());
            
            const Model::Entity* firstEntity = entities[0];
            ASSERT_EQ(0u, firstEntity->hiddenIssues());
            
            const Model::BrushList& brushes = firstEntity->brushes();
            ASSERT_EQ(1u, brushes.size());
            
            const Model::Brush* brush = brushes[0];
            ASSERT_EQ(2u, brush->hiddenIssues());
            
            const Model::Entity* secondEntity = entities[1];
            ASSERT_EQ(3u, secondEntity->hiddenIssues());
        }
         */
    }
}
