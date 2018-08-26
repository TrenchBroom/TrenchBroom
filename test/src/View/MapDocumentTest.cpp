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
#include "Assets/EntityDefinition.h"
#include "Assets/ModelDefinition.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/BrushFace.h"
#include "Model/BrushBuilder.h"
#include "Model/Hit.h"
#include "Model/MapFormat.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/PickResult.h"
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
        m_mapFormat(mapFormat),
        m_pointEntityDef(nullptr),
        m_brushEntityDef(nullptr) {}

        void MapDocumentTest::SetUp() {
            document = MapDocumentCommandFacade::newMapDocument();
            document->newDocument(m_mapFormat, BBox3(8192.0), Model::GameSPtr(new Model::TestGame()));

            // create two entity definitions
            m_pointEntityDef = new Assets::PointEntityDefinition("point_entity", Color(), BBox3(16.0), "this is a point entity", Assets::AttributeDefinitionList(), Assets::ModelDefinition());
            m_brushEntityDef = new Assets::BrushEntityDefinition("point_entity", Color(), "this is a point entity", Assets::AttributeDefinitionList());

            document->setEntityDefinitions(Assets::EntityDefinitionList { m_pointEntityDef, m_brushEntityDef });
        }

        void MapDocumentTest::TearDown() {
            m_pointEntityDef = nullptr;
            m_brushEntityDef = nullptr;
        }

        Model::Brush* MapDocumentTest::createBrush(const String& textureName) {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            return builder.createCube(32.0, textureName);
        }
        
        static void checkPlanePointsIntegral(const Model::Brush *brush) {
            for (const Model::BrushFace* face : brush->faces()) {
                for (size_t i=0; i<3; i++) {
                    vec3 point = face->points()[i];
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
            Model::Brush *brush1 = builder.createCuboid(BBox3(vec3(0.0, 0.0, 0.0), vec3(30.0, 31.0, 31.0)), "texture");
            Model::Brush *brush2 = builder.createCuboid(BBox3(vec3(30.0, 0.0, 0.0), vec3(31.0, 31.0, 31.0)), "texture");
            
            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);
            
            document->addNode(brush1, document->currentParent());
            document->addNode(brush2, document->currentParent());
            
            Model::NodeList brushes;
            brushes.push_back(brush1);
            brushes.push_back(brush2);
            document->select(brushes);
            
            vec3 center = document->selectionBounds().center();
            ASSERT_EQ(vec3(15.5, 15.5, 15.5), center);
            
            document->flipObjects(center, Math::Axis::AX);
            
            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);
         
            ASSERT_EQ(BBox3(vec3(1.0, 0.0, 0.0), vec3(31.0, 31.0, 31.0)), brush1->bounds());
            ASSERT_EQ(BBox3(vec3(0.0, 0.0, 0.0), vec3(1.0, 31.0, 31.0)), brush2->bounds());
        }
        
        TEST_F(MapDocumentTest, rotate) {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(BBox3(vec3(0.0, 0.0, 0.0), vec3(30.0, 31.0, 31.0)), "texture");
            Model::Brush *brush2 = builder.createCuboid(BBox3(vec3(30.0, 0.0, 0.0), vec3(31.0, 31.0, 31.0)), "texture");
            
            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);
            
            document->addNode(brush1, document->currentParent());
            document->addNode(brush2, document->currentParent());
            
            Model::NodeList brushes;
            brushes.push_back(brush1);
            brushes.push_back(brush2);
            document->select(brushes);
            
            vec3 center = document->selectionBounds().center();
            ASSERT_EQ(vec3(15.5, 15.5, 15.5), center);
            
            // 90 degrees CCW about the Z axis through the center of the selection
            document->rotateObjects(center, vec3::pos_z, Math::radians(90.0));
            
            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);
            
            const BBox3 brush1ExpectedBounds(vec3(0.0, 0.0, 0.0), vec3(31.0, 30.0, 31.0));
            const BBox3 brush2ExpectedBounds(vec3(0.0, 30.0, 0.0), vec3(31.0, 31.0, 31.0));
            
            // these should be exactly integral
            ASSERT_EQ(brush1ExpectedBounds, brush1->bounds());
            ASSERT_EQ(brush2ExpectedBounds, brush2->bounds());
        }
        
        TEST_F(MapDocumentTest, shearCube) {
            const BBox3 initialBBox(vec3(100,100,100), vec3(200,200,200));
            
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(initialBBox, "texture");
            
            document->addNode(brush1, document->currentParent());
            document->select(Model::NodeList{brush1});
            
            const std::set<vec3> initialPositions{
                // bottom face
                {100,100,100},
                {200,100,100},
                {200,200,100},
                {100,200,100},
                // top face
                {100,100,200},
                {200,100,200},
                {200,200,200},
                {100,200,200},
            };
            ASSERT_EQ(initialPositions, SetUtils::makeSet(brush1->vertexPositions()));
            
            // Shear the -Y face by (50, 0, 0). That means the verts with Y=100 will get sheared.
            ASSERT_TRUE(document->shearObjects(initialBBox, vec3::neg_y, vec3(50,0,0)));
            
            const std::set<vec3> shearedPositions{
                // bottom face
                {150,100,100},
                {250,100,100},
                {200,200,100},
                {100,200,100},
                // top face
                {150,100,200},
                {250,100,200},
                {200,200,200},
                {100,200,200},
            };
            ASSERT_EQ(shearedPositions, SetUtils::makeSet(brush1->vertexPositions()));
        }
        
        TEST_F(MapDocumentTest, shearPillar) {
            const BBox3 initialBBox(vec3(0,0,0), vec3(100,100,400));
            
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(initialBBox, "texture");
            
            document->addNode(brush1, document->currentParent());
            document->select(Model::NodeList{brush1});
            
            const std::set<vec3> initialPositions{
                // bottom face
                {0,  0,  0},
                {100,0,  0},
                {100,100,0},
                {0,  100,0},
                // top face
                {0,  0,  400},
                {100,0,  400},
                {100,100,400},
                {0,  100,400},
            };
            ASSERT_EQ(initialPositions, SetUtils::makeSet(brush1->vertexPositions()));
            
            // Shear the +Z face by (50, 0, 0). That means the verts with Z=400 will get sheared.
            ASSERT_TRUE(document->shearObjects(initialBBox, vec3::pos_z, vec3(50,0,0)));
            
            const std::set<vec3> shearedPositions{
                // bottom face
                {0,  0,  0},
                {100,0,  0},
                {100,100,0},
                {0,  100,0},
                // top face
                {50, 0,  400},
                {150,0,  400},
                {150,100,400},
                {50, 100,400},
            };
            ASSERT_EQ(shearedPositions, SetUtils::makeSet(brush1->vertexPositions()));
        }

        TEST_F(MapDocumentTest, scaleObjects) {
            const BBox3 initialBBox(vec3(-100,-100,-100), vec3(100,100,100));
            const BBox3 doubleBBox(2.0 * initialBBox.min, 2.0 * initialBBox.max);
            const BBox3 invalidBBox(vec3(0,-100,-100), vec3(0,100,100));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(initialBBox, "texture");

            document->addNode(brush1, document->currentParent());
            document->select(Model::NodeList{brush1});

            ASSERT_EQ(vec3(200,200,200), brush1->bounds().size());
            ASSERT_EQ(Plane3(100.0, vec3::pos_z), brush1->findFace(vec3::pos_z)->boundary());

            // attempting an invalid scale has no effect
            ASSERT_FALSE(document->scaleObjects(initialBBox, invalidBBox));
            ASSERT_EQ(vec3(200,200,200), brush1->bounds().size());
            ASSERT_EQ(Plane3(100.0, vec3::pos_z), brush1->findFace(vec3::pos_z)->boundary());

            ASSERT_TRUE(document->scaleObjects(initialBBox, doubleBBox));
            ASSERT_EQ(vec3(400,400,400), brush1->bounds().size());
            ASSERT_EQ(Plane3(200.0, vec3::pos_z), brush1->findFace(vec3::pos_z)->boundary());
        }

        TEST_F(MapDocumentTest, scaleObjectsWithCenter) {
            const BBox3 initialBBox(vec3(0,0,0), vec3(100,100,400));
            const BBox3 expectedBBox(vec3(-50,0,0), vec3(150,100,400));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(initialBBox, "texture");

            document->addNode(brush1, document->currentParent());
            document->select(Model::NodeList{brush1});

            const vec3 center = initialBBox.center();
            ASSERT_TRUE(document->scaleObjects(center, vec3(2.0, 1.0, 1.0)));
            ASSERT_EQ(expectedBBox, brush1->bounds());
        }
        
        TEST_F(MapDocumentTest, csgConvexMerge) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());
            
            Model::Brush* brush1 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(32, 64, 64)), "texture");
            Model::Brush* brush2 = builder.createCuboid(BBox3(vec3(32, 0, 0), vec3(64, 64, 64)), "texture");
            document->addNode(brush1, entity);
            document->addNode(brush2, entity);
            ASSERT_EQ(2, entity->children().size());
            
            document->select(Model::NodeList { brush1, brush2 });
            ASSERT_TRUE(document->csgConvexMerge());
            ASSERT_EQ(1, entity->children().size());
            
            Model::Node* brush3 = entity->children()[0];            
            ASSERT_EQ(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)), brush3->bounds());
        }

        TEST_F(MapDocumentTest, setTextureNull) {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCube(64.0f, Model::BrushFace::NoTextureName);
            
            document->addNode(brush1, document->currentParent());
            document->select(brush1);
            
            document->setTexture(nullptr);
        }
        
        ValveMapDocumentTest::ValveMapDocumentTest() :
        MapDocumentTest(Model::MapFormat::Valve) {}
        
        TEST_F(ValveMapDocumentTest, csgConvexMergeTexturing) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());
            
            Model::ParallelTexCoordSystem texAlignment(vec3(1, 0, 0), vec3(0, 1, 0));
            Model::TexCoordSystemSnapshot* texAlignmentSnapshot = texAlignment.takeSnapshot();
            
            Model::Brush* brush1 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(32, 64, 64)), "texture");
            Model::Brush* brush2 = builder.createCuboid(BBox3(vec3(32, 0, 0), vec3(64, 64, 64)), "texture");
            brush1->findFace(vec3::pos_z)->restoreTexCoordSystemSnapshot(texAlignmentSnapshot);
            brush2->findFace(vec3::pos_z)->restoreTexCoordSystemSnapshot(texAlignmentSnapshot);
            document->addNode(brush1, entity);
            document->addNode(brush2, entity);
            ASSERT_EQ(2, entity->children().size());
            
            document->select(Model::NodeList { brush1, brush2 });
            ASSERT_TRUE(document->csgConvexMerge());
            ASSERT_EQ(1, entity->children().size());
            
            Model::Brush* brush3 = static_cast<Model::Brush*>(entity->children()[0]);
            Model::BrushFace* top = brush3->findFace(vec3::pos_z);
            ASSERT_EQ(vec3(1, 0, 0), top->textureXAxis());
            ASSERT_EQ(vec3(0, 1, 0), top->textureYAxis());
            
            delete texAlignmentSnapshot;
        }
        
        TEST_F(ValveMapDocumentTest, csgSubtractTexturing) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());
            
            Model::ParallelTexCoordSystem texAlignment(vec3(1, 0, 0), vec3(0, 1, 0));
            Model::TexCoordSystemSnapshot* texAlignmentSnapshot = texAlignment.takeSnapshot();
            
            Model::Brush* brush1 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)), "texture");
            Model::Brush* brush2 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 32)), "texture");
            brush2->findFace(vec3::pos_z)->restoreTexCoordSystemSnapshot(texAlignmentSnapshot);
            document->addNode(brush1, entity);
            document->addNode(brush2, entity);
            ASSERT_EQ(2, entity->children().size());
            
            document->select(Model::NodeList { brush1, brush2 });
            ASSERT_TRUE(document->csgSubtract());
            ASSERT_EQ(1, entity->children().size());
            
            Model::Brush* brush3 = static_cast<Model::Brush*>(entity->children()[0]);
            ASSERT_EQ(BBox3(vec3(0, 0, 32), vec3(64, 64, 64)), brush3->bounds());
            
            // the texture alignment from the top of brush2 should have transferred
            // to the bottom face of brush3
            Model::BrushFace* top = brush3->findFace(vec3::neg_z);
            ASSERT_EQ(vec3(1, 0, 0), top->textureXAxis());
            ASSERT_EQ(vec3(0, 1, 0), top->textureYAxis());
            
            delete texAlignmentSnapshot;
        }
        
        TEST_F(MapDocumentTest, newWithGroupOpen) {
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());
            document->select(entity);
            Model::Group* group = document->groupSelection("my group");
            document->openGroup(group);
            
            ASSERT_EQ(group, document->currentGroup());
            
            document->newDocument(Model::MapFormat::Valve, MapDocument::DefaultWorldBounds, document->game());
         	
            ASSERT_EQ(nullptr, document->currentGroup());
        }

        TEST_F(MapDocumentTest, ungroupInnerGroup) {
            // see https://github.com/kduske/TrenchBroom/issues/2050
            Model::Entity* outerEnt1 = new Model::Entity();
            Model::Entity* outerEnt2 = new Model::Entity();
            Model::Entity* innerEnt1 = new Model::Entity();
            Model::Entity* innerEnt2 = new Model::Entity();

            document->addNode(innerEnt1, document->currentParent());
            document->addNode(innerEnt2, document->currentParent());
            document->select(Model::NodeList {innerEnt1, innerEnt2});

            Model::Group* inner = document->groupSelection("Inner");

            document->deselectAll();
            document->addNode(outerEnt1, document->currentParent());
            document->addNode(outerEnt2, document->currentParent());
            document->select(Model::NodeList {inner, outerEnt1, outerEnt2});

            Model::Group* outer = document->groupSelection("Outer");
            document->deselectAll();

            // check our assumptions
            ASSERT_EQ(3u, outer->childCount());
            ASSERT_EQ(2u, inner->childCount());

            ASSERT_EQ(document->currentLayer(), outer->parent());

            ASSERT_EQ(outer, outerEnt1->parent());
            ASSERT_EQ(outer, outerEnt2->parent());
            ASSERT_EQ(outer, inner->parent());

            ASSERT_EQ(inner, innerEnt1->parent());
            ASSERT_EQ(inner, innerEnt2->parent());

            // open the outer group and ungroup the inner group
            document->openGroup(outer);
            document->select(inner);
            document->ungroupSelection();
            document->deselectAll();

            ASSERT_EQ(outer, innerEnt1->parent());
            ASSERT_EQ(outer, innerEnt2->parent());
        }
        
        TEST_F(MapDocumentTest, ungroupLeavesPointEntitySelected) {
            Model::Entity* ent1 = new Model::Entity();
            
            document->addNode(ent1, document->currentParent());
            document->select(Model::NodeList {ent1});
            
            Model::Group* group = document->groupSelection("Group");
            ASSERT_EQ((Model::NodeList {group}), document->selectedNodes().nodes());
            
            document->ungroupSelection();
            ASSERT_EQ((Model::NodeList {ent1}), document->selectedNodes().nodes());
        }
        
        TEST_F(MapDocumentTest, ungroupLeavesBrushEntitySelected) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            
            Model::Entity* ent1 = new Model::Entity();
            document->addNode(ent1, document->currentParent());
            
            Model::Brush* brush1 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)), "texture");
            document->addNode(brush1, ent1);
            document->select(Model::NodeList{ent1});
            ASSERT_EQ((Model::NodeList {brush1}), document->selectedNodes().nodes());
            ASSERT_FALSE(ent1->selected());
            ASSERT_TRUE(brush1->selected());
            
            Model::Group* group = document->groupSelection("Group");
            ASSERT_EQ((Model::NodeList {ent1}), group->children());
            ASSERT_EQ((Model::NodeList {group}), document->selectedNodes().nodes());
            
            document->ungroupSelection();
            ASSERT_EQ((Model::NodeList {brush1}), document->selectedNodes().nodes());
            ASSERT_FALSE(ent1->selected());
            ASSERT_TRUE(brush1->selected());
        }

        TEST_F(MapDocumentTest, mergeGroups) {
            document->selectAllNodes();
            document->deleteObjects();
            
            Model::Entity* ent1 = new Model::Entity();
            document->addNode(ent1, document->currentParent());
            document->deselectAll();
            document->select(Model::NodeList {ent1});
            Model::Group* group1 = document->groupSelection("group1");
            
            Model::Entity* ent2 = new Model::Entity();
            document->addNode(ent2, document->currentParent());
            document->deselectAll();
            document->select(Model::NodeList {ent2});
            Model::Group* group2 = document->groupSelection("group2");
            
            ASSERT_EQ((Model::NodeSet {group1, group2}), SetUtils::makeSet(document->currentLayer()->children()));
            
            document->select(Model::NodeList {group1, group2});
            document->mergeSelectedGroupsWithGroup(group2);
            
            ASSERT_EQ((Model::NodeList {group2}), document->selectedNodes().nodes());
            ASSERT_EQ((Model::NodeList {group2}), document->currentLayer()->children());
            
            ASSERT_EQ((Model::NodeSet {}), SetUtils::makeSet(group1->children()));
            ASSERT_EQ((Model::NodeSet {ent1, ent2}), SetUtils::makeSet(group2->children()));
        }

        TEST_F(MapDocumentTest, pickSingleBrush) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* brush1 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)), "texture");
            document->addNode(brush1, document->currentParent());

            Model::PickResult pickResult;
            document->pick(Ray3(vec3(-32, 0, 0), vec3::pos_x), pickResult);

            auto hits = pickResult.query().all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vec3::neg_x), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            pickResult.clear();
            document->pick(Ray3(vec3(-32, 0, 0), vec3::neg_x), pickResult);
            ASSERT_TRUE(pickResult.query().all().empty());
        }

        TEST_F(MapDocumentTest, pickSingleEntity) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::Entity* ent1 = new Model::Entity();
            document->addNode(ent1, document->currentParent());

            const auto origin = ent1->origin();
            const auto bounds = ent1->bounds();

            const auto rayOrigin = origin + vec3(-32.0, bounds.size().y() / 2.0, bounds.size().z() / 2.0);

            Model::PickResult pickResult;
            document->pick(Ray3(rayOrigin, vec3::pos_x), pickResult);

            auto hits = pickResult.query().all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(ent1, hits.front().target<Model::Entity*>());
            ASSERT_DOUBLE_EQ(32.0 - bounds.size().x() / 2.0, hits.front().distance());

            pickResult.clear();
            document->pick(Ray3(vec3(-32, 0, 0), vec3::neg_x), pickResult);
            ASSERT_TRUE(pickResult.query().all().empty());
        }

        TEST_F(MapDocumentTest, pickSimpleGroup) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* brush1 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)), "texture");
            document->addNode(brush1, document->currentParent());

            auto* brush2 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)).translate(vec3(0, 0, 128)), "texture");
            document->addNode(brush2, document->currentParent());

            document->selectAllNodes();
            auto* group = document->groupSelection("test");

            Model::PickResult pickResult;
            document->pick(Ray3(vec3(-32, 0, 0), vec3::pos_x), pickResult);

            // picking a grouped object when the containing group is closed should return both the object and the group
            auto hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vec3::neg_x), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            hits = pickResult.query().type(Model::Group::GroupHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(group, hits.front().target<Model::Group*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            // hitting both objects in the group should return the group only once
            pickResult.clear();
            document->pick(Ray3(vec3(32, 32, -32), vec3::pos_z), pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(2u, hits.size());

            hits = pickResult.query().type(Model::Group::GroupHit).all();
            ASSERT_EQ(1u, hits.size());

            // hitting the group bounds doesn't count as a hit
            pickResult.clear();
            document->pick(Ray3(vec3(-32, 0, 96), vec3::pos_x), pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_TRUE(hits.empty());

            hits = pickResult.query().type(Model::Group::GroupHit).all();
            ASSERT_TRUE(hits.empty());

            // hitting a grouped object when the containing group is open should return the object only
            document->openGroup(group);

            pickResult.clear();
            document->pick(Ray3(vec3(-32, 0, 0), vec3::pos_x), pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vec3::neg_x), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            hits = pickResult.query().type(Model::Group::GroupHit).all();
            ASSERT_TRUE(hits.empty());
        }

        TEST_F(MapDocumentTest, pickNestedGroup) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* brush1 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)), "texture");
            document->addNode(brush1, document->currentParent());

            auto* brush2 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)).translate(vec3(0, 0, 128)), "texture");
            document->addNode(brush2, document->currentParent());

            document->selectAllNodes();
            auto* inner = document->groupSelection("inner");

            document->deselectAll();
            auto* brush3 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)).translate(vec3(0, 0, 256)), "texture");
            document->addNode(brush3, document->currentParent());

            document->selectAllNodes();
            auto* outer = document->groupSelection("outer");

            const Ray3 highRay(vec3(-32, 0, 256+32), vec3::pos_x);
            const Ray3  lowRay(vec3(-32, 0,    +32), vec3::pos_x);

            /*
             *          Z
             *         /|\
             *          |
             *          | ______________
             *          | |   ______   |
             *  hiRay *-->|   | b3 |   |
             *          | |   |____|   |
             *          | |            |
             *          | |   outer    |
             *          | | __________ |
             *          | | | ______ | |
             *          | | | | b2 | | |
             *          | | | |____| | |
             *          | | |        | |
             *          | | |  inner | |
             *          | | | ______ | |
             * lowRay *-->| | | b1 | | |
             *        0_| | | |____| | |
             *          | | |________| |
             *          | |____________|
             * ---------|--------------------> X
             *                |
             *                0
             */

            /*
             * world
             * * outer (closed)
             *   * inner (closed)
             *     * brush1
             *     * brush2
             *   * brush3
             */

            Model::PickResult pickResult;

            // hitting a grouped object when the containing group is open should return the object only
            document->openGroup(outer);

            /*
             * world
             * * outer (open)
             *   * inner (closed)
             *     * brush1
             *     * brush2
             *   * brush3
             */

            pickResult.clear();
            document->pick(highRay, pickResult);

            auto hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush3->findFace(vec3::neg_x), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            hits = pickResult.query().type(Model::Group::GroupHit).all();
            ASSERT_TRUE(hits.empty());

            // hitting the brush in the inner group should return the inner group and the brush
            pickResult.clear();
            document->pick(lowRay, pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vec3::neg_x), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            hits = pickResult.query().type(Model::Group::GroupHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(inner, hits.front().target<Model::Group*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            // open the inner group, too
            document->openGroup(inner);

            /*
             * world
             * * outer (open)
             *   * inner (open)
             *     * brush1
             *     * brush2
             *   * brush3
             */

            // pick a brush in the outer group
            pickResult.clear();
            document->pick(highRay, pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush3->findFace(vec3::neg_x), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            hits = pickResult.query().type(Model::Group::GroupHit).all();
            ASSERT_TRUE(hits.empty());

            // pick a brush in the inner group
            pickResult.clear();
            document->pick(lowRay, pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vec3::neg_x), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            hits = pickResult.query().type(Model::Group::GroupHit).all();
            ASSERT_TRUE(hits.empty());
        }

        TEST_F(MapDocumentTest, pickBrushEntity) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto *brush1 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)), "texture");
            document->addNode(brush1, document->currentParent());

            auto *brush2 = builder.createCuboid(BBox3(vec3(0, 0, 0), vec3(64, 64, 64)).translate(vec3(0, 0, 128)),
                                                "texture");
            document->addNode(brush2, document->currentParent());

            document->selectAllNodes();

            document->createBrushEntity(m_brushEntityDef);
            document->deselectAll();

            Model::PickResult pickResult;

            // picking entity brushes should only return the brushes and not the entity
            document->pick(Ray3(vec3(-32, 0, 0), vec3::pos_x), pickResult);

            auto hits = pickResult.query().all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vec3::neg_x), hits.front().target<Model::BrushFace *>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());
        }

        TEST_F(MapDocumentTest, throwExceptionDuringCommand) {
            ASSERT_THROW(document->throwExceptionDuringCommand(), GeometryException);
        }
    }
}
