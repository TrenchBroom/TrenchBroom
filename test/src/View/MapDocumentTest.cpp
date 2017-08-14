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

#include "MapDocumentTest.h"

#include "TestUtils.h"
#include "MathUtils.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/BrushFace.h"
#include "Model/BrushBuilder.h"
#include "Model/MapFormat.h"
#include "Model/TestGame.h"
#include "Model/World.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        MapDocumentTest::MapDocumentTest() :
        ::testing::Test(),
        m_mapFormat(Model::MapFormat::Standard) {}
        
        MapDocumentTest::MapDocumentTest(const Model::MapFormat::Type mapFormat) :
        ::testing::Test(),
        m_mapFormat(mapFormat) {}

        void MapDocumentTest::SetUp() {
            document = MapDocumentCommandFacade::newMapDocument();
            document->newDocument(m_mapFormat, BBox3(8192.0), Model::GameSPtr(new Model::TestGame()));
        }

        Model::Brush* MapDocumentTest::createBrush(const String& textureName) {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            return builder.createCube(32.0, textureName);
        }
        
        static void checkPlanePointsIntegral(const Model::Brush *brush) {
            for (const Model::BrushFace* face : brush->faces()) {
                for (size_t i=0; i<3; i++) {
                    Vec3 point = face->points()[i];
                    ASSERT_POINT_INTEGRAL(point);
                }
            }
        }
        
        static void checkVerticesIntegral(const Model::Brush *brush) {
            for (const Model::BrushVertex* vertex : brush->vertices())
                ASSERT_POINT_INTEGRAL(vertex->position());
        }
        
        static void checkBoundsIntegral(const Model::Brush *brush) {
            ASSERT_POINT_INTEGRAL(brush->bounds().min);
            ASSERT_POINT_INTEGRAL(brush->bounds().max);
        }
        
        static void checkBrushIntegral(const Model::Brush *brush) {
            checkPlanePointsIntegral(brush);
            checkVerticesIntegral(brush);
            checkBoundsIntegral(brush);
        }
        
        TEST_F(MapDocumentTest, flip) {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(BBox3(Vec3(0.0, 0.0, 0.0), Vec3(30.0, 31.0, 31.0)), "texture");
            Model::Brush *brush2 = builder.createCuboid(BBox3(Vec3(30.0, 0.0, 0.0), Vec3(31.0, 31.0, 31.0)), "texture");
            
            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);
            
            document->addNode(brush1, document->currentParent());
            document->addNode(brush2, document->currentParent());
            
            Model::NodeList brushes;
            brushes.push_back(brush1);
            brushes.push_back(brush2);
            document->select(brushes);
            
            Vec3 center = document->selectionBounds().center();
            ASSERT_EQ(Vec3(15.5, 15.5, 15.5), center);
            
            document->flipObjects(center, Math::Axis::AX);
            
            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);
         
            ASSERT_EQ(BBox3(Vec3(1.0, 0.0, 0.0), Vec3(31.0, 31.0, 31.0)), brush1->bounds());
            ASSERT_EQ(BBox3(Vec3(0.0, 0.0, 0.0), Vec3(1.0, 31.0, 31.0)), brush2->bounds());
        }
        
        TEST_F(MapDocumentTest, rotate) {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(BBox3(Vec3(0.0, 0.0, 0.0), Vec3(30.0, 31.0, 31.0)), "texture");
            Model::Brush *brush2 = builder.createCuboid(BBox3(Vec3(30.0, 0.0, 0.0), Vec3(31.0, 31.0, 31.0)), "texture");
            
            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);
            
            document->addNode(brush1, document->currentParent());
            document->addNode(brush2, document->currentParent());
            
            Model::NodeList brushes;
            brushes.push_back(brush1);
            brushes.push_back(brush2);
            document->select(brushes);
            
            Vec3 center = document->selectionBounds().center();
            ASSERT_EQ(Vec3(15.5, 15.5, 15.5), center);
            
            // 90 degrees CCW about the Z axis through the center of the selection
            document->rotateObjects(center, Vec3::PosZ, Math::radians(90.0));
            
            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);
            
            const BBox3 brush1ExpectedBounds(Vec3(0.0, 0.0, 0.0), Vec3(31.0, 30.0, 31.0));
            const BBox3 brush2ExpectedBounds(Vec3(0.0, 30.0, 0.0), Vec3(31.0, 31.0, 31.0));
            
            // these should be exactly integral
            ASSERT_EQ(brush1ExpectedBounds, brush1->bounds());
            ASSERT_EQ(brush2ExpectedBounds, brush2->bounds());
        }
        
        TEST_F(MapDocumentTest, csgConvexMerge) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());
            
            Model::Brush* brush1 = builder.createCuboid(BBox3(Vec3(0, 0, 0), Vec3(32, 64, 64)), "texture");
            Model::Brush* brush2 = builder.createCuboid(BBox3(Vec3(32, 0, 0), Vec3(64, 64, 64)), "texture");
            document->addNode(brush1, entity);
            document->addNode(brush2, entity);
            ASSERT_EQ(2, entity->children().size());
            
            document->select(Model::NodeList { brush1, brush2 });
            ASSERT_TRUE(document->csgConvexMerge());
            ASSERT_EQ(1, entity->children().size());
            
            Model::Node* brush3 = entity->children()[0];            
            ASSERT_EQ(BBox3(Vec3(0, 0, 0), Vec3(64, 64, 64)), brush3->bounds());
        }

        TEST_F(MapDocumentTest, setTextureNull) {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCube(64.0f, Model::BrushFace::NoTextureName);
            
            document->addNode(brush1, document->currentParent());
            document->select(brush1);
            
            document->setTexture(nullptr);
        }
    }
}
