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
#include "Model/EntityPropertyTypes.h"
#include "Model/Map.h"

namespace TrenchBroom {
    namespace IO {
        inline bool findFaceByPoints(const Model::BrushFaceList& faces, const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                const Model::BrushFacePtr face = *it;
                if (face->points()[0] == point0 &&
                    face->points()[1] == point1 &&
                    face->points()[2] == point2)
                    return true;
            }
            return false;
        }
        
        TEST(QuakeMapParserTest, ParseEmptyMap) {
            const String data("");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::MapPtr map = parser.parseMap(worldBounds);
            
            ASSERT_TRUE(map->entities().empty());
        }
        
        TEST(QuakeMapParserTest, ParseMapWithEmptyEntity) {
            const String data("{}");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::MapPtr map = parser.parseMap(worldBounds);

            ASSERT_EQ(1u, map->entities().size());
        }
        
        TEST(QuakeMapParserTest, ParseMapWithWorldspawn) {
            const String data("{"
                              "\"classname\" \"worldspawn\""
                              "}");
            BBox3 worldBounds(-8192, 8192);
            
            QuakeMapParser parser(data);
            Model::MapPtr map = parser.parseMap(worldBounds);
            
            const Model::EntityList& entities = map->entities();
            ASSERT_EQ(1u, entities.size());
            
            const Model::EntityPtr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
        }

        TEST(QuakeMapParserTest, ParseMapWithWorldspawnAndOneMoreEntity) {
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
            Model::MapPtr map = parser.parseMap(worldBounds);
            
            const Model::EntityList& entities = map->entities();
            ASSERT_EQ(2u, entities.size());
            
            const Model::EntityPtr first = entities.front();
            ASSERT_TRUE(first->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, first->property(Model::PropertyKeys::Classname));
            
            const Model::EntityPtr second = entities[1];
            ASSERT_TRUE(second->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(String("info_player_deathmatch"), second->property(Model::PropertyKeys::Classname));
            ASSERT_TRUE(second->hasProperty("origin"));
            ASSERT_EQ(String("1 22 -3"), second->property("origin"));
            ASSERT_TRUE(second->hasProperty("angle"));
            ASSERT_EQ(String(" -1 "), second->property("angle"));
        }
        
        TEST(QuakeMapParserTest, ParseMapWithWorldspawnAndOneBrush) {
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
            Model::MapPtr map = parser.parseMap(worldBounds);
            
            const Model::EntityList& entities = map->entities();
            ASSERT_EQ(1u, entities.size());
            
            const Model::EntityPtr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
            
            const Model::BrushList& brushes = entity->brushes();
            ASSERT_EQ(1u, brushes.size());
            
            const Model::BrushPtr brush = brushes.front();
            const Model::BrushFaceList faces = brush->faces();
            ASSERT_EQ(6u, faces.size());
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,   0.0,   0.0), Vec3( 64.0,   0.0, -16.0)));
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0), Vec3(  0.0,   0.0,   0.0)));
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3( 64.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0)));
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3(  0.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0)));
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0), Vec3( 64.0,   0.0,   0.0)));
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,   0.0,   0.0), Vec3(  0.0,  64.0,   0.0)));
        }

        TEST(QuakeMapParserTest, ParseBrushWithCurlyBraceInTextureName) {
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
            Model::MapPtr map = parser.parseMap(worldBounds);
            
            const Model::EntityList& entities = map->entities();
            ASSERT_EQ(1u, entities.size());
            
            const Model::EntityPtr entity = entities.front();
            ASSERT_TRUE(entity->hasProperty(Model::PropertyKeys::Classname));
            ASSERT_EQ(Model::PropertyValues::WorldspawnClassname, entity->property(Model::PropertyKeys::Classname));
            
            const Model::BrushList& brushes = entity->brushes();
            ASSERT_EQ(1u, brushes.size());
            
            const Model::BrushPtr brush = brushes.front();
            const Model::BrushFaceList faces = brush->faces();
            ASSERT_EQ(6u, faces.size());
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,   0.0,   0.0), Vec3( 64.0,   0.0, -16.0)));
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0), Vec3(  0.0,   0.0,   0.0)));
            ASSERT_TRUE(findFaceByPoints(faces, Vec3(  0.0,   0.0, -16.0), Vec3( 64.0,   0.0, -16.0), Vec3(  0.0,  64.0, -16.0)));
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3(  0.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0)));
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,  64.0, -16.0), Vec3( 64.0,   0.0,   0.0)));
            ASSERT_TRUE(findFaceByPoints(faces, Vec3( 64.0,  64.0,   0.0), Vec3( 64.0,   0.0,   0.0), Vec3(  0.0,  64.0,   0.0)));
        }
    }
}
