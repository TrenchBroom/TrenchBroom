/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "StringUtils.h"
#include "IO/QuakeMapParser.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Map.h"

namespace TrenchBroom {
    namespace IO {
        inline Model::BrushFace::Ptr findFaceByPoints(const Model::BrushFace::List& faces, const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            Model::BrushFace::List::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const Model::BrushFace::Ptr face = *it;
                if (face->points()[0] == point0 &&
                    face->points()[1] == point1 &&
                    face->points()[2] == point2)
                    return face;
            }
            return Model::BrushFace::Ptr();
        }
        
        TEST(QuakeMapParserTest, parseEmptyMap) {
            const String data("");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::Map::Ptr map = parser.parseMap(worldBounds);
            
            ASSERT_TRUE(map->entities().empty());
        }
        
        TEST(QuakeMapParserTest, parseMapWithEmptyEntity) {
            const String data("{}");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::Map::Ptr map = parser.parseMap(worldBounds);

            ASSERT_EQ(1u, map->entities().size());
        }
        
        TEST(QuakeMapParserTest, parseMapWithWorldspawn) {
            const String data("{"
                              "\"classname\" \"worldspawn\""
                              "}");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::Map::Ptr map = parser.parseMap(worldBounds);
            
            const Model::Entity::List& entities = map->entities();
            ASSERT_EQ(1u, entities.size());
            
            const Model::Entity::Ptr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
        }

        TEST(QuakeMapParserTest, parseMapWithWorldspawnAndOneMoreEntity) {
            const String data("{"
                              "\"classname\" \"worldspawn\""
                              "}"
                              "{"
                              "\"classname\" \"info_player_deathmatch\""
                              "\"origin\" \"1 22 -3\""
                              "\"angle\" \" -1 \""
                              "}");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::Map::Ptr map = parser.parseMap(worldBounds);
            
            const Model::Entity::List& entities = map->entities();
            ASSERT_EQ(2u, entities.size());
            
            const Model::Entity::Ptr first = entities.front();
            ASSERT_TRUE(first->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, first->property(Model::PropertyKeys::Classname));
            
            const Model::Entity::Ptr second = entities[1];
            ASSERT_TRUE(second->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(String("info_player_deathmatch"), second->property(Model::PropertyKeys::Classname));
            ASSERT_TRUE(second->hasProperty("origin"));
            ASSERT_EQ(String("1 22 -3"), second->property("origin"));
            ASSERT_TRUE(second->hasProperty("angle"));
            ASSERT_EQ(String(" -1 "), second->property("angle"));
        }
        
        TEST(QuakeMapParserTest, parseMapWithWorldspawnAndOneBrush) {
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
                              "}\n");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::Map::Ptr map = parser.parseMap(worldBounds);
            
            const Model::Entity::List& entities = map->entities();
            ASSERT_EQ(1u, entities.size());
            
            const Model::Entity::Ptr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
            
            const Model::Brush::List& brushes = entity->brushes();
            ASSERT_EQ(1u, brushes.size());
            
            const Model::Brush::Ptr brush = brushes.front();
            const Model::BrushFace::List faces = brush->faces();
            ASSERT_EQ(6u, faces.size());
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,   0.0,   0.0), Vec3( 64.0,   0.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0), Vec3(  0.0,   0.0,   0.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3( 64.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3(  0.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0), Vec3( 64.0,   0.0,   0.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,   0.0,   0.0), Vec3(  0.0,  64.0,   0.0)) != NULL);
        }
        
        TEST(QuakeMapParserTest, parseMapAndCheckFaceFlags) {
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
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::Map::Ptr map = parser.parseMap(worldBounds);
            
            const Model::Entity::List& entities = map->entities();
            ASSERT_EQ(1u, entities.size());
            
            const Model::Entity::Ptr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
            
            const Model::Brush::List& brushes = entity->brushes();
            ASSERT_EQ(1u, brushes.size());
            
            const Model::Brush::Ptr brush = brushes.front();
            const Model::BrushFace::List faces = brush->faces();
            ASSERT_EQ(6u, faces.size());
            
            Model::BrushFace::Ptr face = findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,   0.0,   0.0), Vec3( 64.0,   0.0, -16.0));
            ASSERT_TRUE(face != NULL);
            ASSERT_FLOAT_EQ(22.0f, face->xOffset());
            ASSERT_FLOAT_EQ(22.0f, face->xOffset());
            ASSERT_FLOAT_EQ(56.2f, face->rotation());
            ASSERT_FLOAT_EQ(1.03433f, face->xScale());
            ASSERT_FLOAT_EQ(-0.55f, face->yScale());
        }

        TEST(QuakeMapParserTest, parseBrushWithCurlyBraceInTextureName) {
            const String data("{\n"
                              "\"classname\" \"worldspawn\"\n"
                              "{\n"
                              "( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) \"{none\" 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) none 0 0 0 1 1\n"
                              "( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) none 0 0 0 1 1\n"
                              "( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) none 0 0 0 1 1\n"
                              "}\n"
                              "}\n");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::Map::Ptr map = parser.parseMap(worldBounds);
            
            const Model::Entity::List& entities = map->entities();
            ASSERT_EQ(1u, entities.size());
            
            const Model::Entity::Ptr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
            
            const Model::Brush::List& brushes = entity->brushes();
            ASSERT_EQ(1u, brushes.size());
            
            const Model::Brush::Ptr brush = brushes.front();
            const Model::BrushFace::List faces = brush->faces();
            ASSERT_EQ(6u, faces.size());
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,   0.0,   0.0), Vec3( 64.0,   0.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0), Vec3(  0.0,   0.0,   0.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3( 64.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3(  0.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0), Vec3( 64.0,   0.0,   0.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,   0.0,   0.0), Vec3(  0.0,  64.0,   0.0)) != NULL);
        }
        
        TEST(QuakeMapParserTest, parseProblematicBrush1) {
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
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::Map::Ptr map = parser.parseMap(worldBounds);
            
            const Model::Entity::List& entities = map->entities();
            ASSERT_EQ(1u, entities.size());
            
            const Model::Entity::Ptr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
            
            const Model::Brush::List& brushes = entity->brushes();
            ASSERT_EQ(1u, brushes.size());
            
            const Model::Brush::Ptr brush = brushes.front();
            const Model::BrushFace::List faces = brush->faces();
            ASSERT_EQ(6u, faces.size());
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(308.0, 108.0, 176.0), Vec3(308.0, 132.0, 176.0), Vec3(252.0, 132.0, 176.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(252.0, 132.0, 208.0), Vec3(308.0, 132.0, 208.0), Vec3(308.0, 108.0, 208.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(288.0, 152.0, 176.0), Vec3(288.0, 152.0, 208.0), Vec3(288.0, 120.0, 208.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(288.0, 122.0, 176.0), Vec3(288.0, 122.0, 208.0), Vec3(308.0, 102.0, 208.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(308.0, 100.0, 176.0), Vec3(308.0, 100.0, 208.0), Vec3(324.0, 116.0, 208.0)) != NULL);
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(287.0, 152.0, 208.0), Vec3(287.0, 152.0, 176.0), Vec3(323.0, 116.0, 176.0)) != NULL);
        }

        TEST(QuakeMapParserTest, parseProblematicBrush2) {
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
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::Map::Ptr map = parser.parseMap(worldBounds);
            
            const Model::Entity::List& entities = map->entities();
            ASSERT_EQ(1u, entities.size());
            
            const Model::Entity::Ptr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
            
            const Model::Brush::List& brushes = entity->brushes();
            ASSERT_EQ(1u, brushes.size());
        }

        TEST(QuakeMapParserTest, parseProblematicBrush3) {
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
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::Map::Ptr map = parser.parseMap(worldBounds);
            
            const Model::Entity::List& entities = map->entities();
            ASSERT_EQ(1u, entities.size());
            
            const Model::Entity::Ptr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
            
            const Model::Brush::List& brushes = entity->brushes();
            ASSERT_EQ(1u, brushes.size());
        }
    }
}
