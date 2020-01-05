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

#include "Exceptions.h"
#include "TestUtils.h"
#include "Assets/EntityDefinition.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/HitQuery.h"
#include "Model/Layer.h"
#include "Model/BrushFace.h"
#include "Model/BrushBuilder.h"
#include "Model/MapFormat.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Model/TestGame.h"
#include "Model/World.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/PasteType.h"
#include "View/SelectionTool.h"

#include <vecmath/bbox.h>
#include <vecmath/scalar.h>
#include <vecmath/ray.h>

namespace TrenchBroom {
    namespace View {
        MapDocumentTest::MapDocumentTest() :
        MapDocumentTest(Model::MapFormat::Standard) {}

        MapDocumentTest::MapDocumentTest(const Model::MapFormat mapFormat) :
        ::testing::Test(),
        m_mapFormat(mapFormat),
        m_pointEntityDef(nullptr),
        m_brushEntityDef(nullptr) {}

        void MapDocumentTest::SetUp() {
            game = std::make_shared<Model::TestGame>();
            document = MapDocumentCommandFacade::newMapDocument();
            document->newDocument(m_mapFormat, vm::bbox3(8192.0), game);

            // create two entity definitions
            m_pointEntityDef = new Assets::PointEntityDefinition("point_entity", Color(), vm::bbox3(16.0), "this is a point entity", {}, {});
            m_brushEntityDef = new Assets::BrushEntityDefinition("brush_entity", Color(), "this is a brush entity", {});

            document->setEntityDefinitions(std::vector<Assets::EntityDefinition*>{ m_pointEntityDef, m_brushEntityDef });
        }

        void MapDocumentTest::TearDown() {
            m_pointEntityDef = nullptr;
            m_brushEntityDef = nullptr;
        }

        Model::Brush* MapDocumentTest::createBrush(const std::string& textureName) {
            Model::BrushBuilder builder(document->world(), document->worldBounds(), document->game()->defaultFaceAttribs());
            return builder.createCube(32.0, textureName);
        }

        static void checkPlanePointsIntegral(const Model::Brush *brush) {
            for (const Model::BrushFace* face : brush->faces()) {
                for (size_t i=0; i<3; i++) {
                    vm::vec3 point = face->points()[i];
                    ASSERT_POINT_INTEGRAL(point);
                }
            }
        }

        static void checkVerticesIntegral(const Model::Brush *brush) {
            for (const Model::BrushVertex* vertex : brush->vertices())
                ASSERT_POINT_INTEGRAL(vertex->position());
        }

        static void checkBoundsIntegral(const Model::Brush *brush) {
            ASSERT_POINT_INTEGRAL(brush->logicalBounds().min);
            ASSERT_POINT_INTEGRAL(brush->logicalBounds().max);
        }

        static void checkBrushIntegral(const Model::Brush *brush) {
            checkPlanePointsIntegral(brush);
            checkVerticesIntegral(brush);
            checkBoundsIntegral(brush);
        }

        TEST_F(MapDocumentTest, flip) {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush* brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0.0, 0.0, 0.0), vm::vec3(30.0, 31.0, 31.0)), "texture");
            Model::Brush* brush2 = builder.createCuboid(vm::bbox3(vm::vec3(30.0, 0.0, 0.0), vm::vec3(31.0, 31.0, 31.0)), "texture");

            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);

            document->addNode(brush1, document->currentParent());
            document->addNode(brush2, document->currentParent());

            std::vector<Model::Node*> brushes;
            brushes.push_back(brush1);
            brushes.push_back(brush2);
            document->select(brushes);

            const vm::vec3 boundsCenter = document->selectionBounds().center();
            ASSERT_EQ(vm::vec3(15.5, 15.5, 15.5), boundsCenter);

            document->flipObjects(boundsCenter, vm::axis::x);

            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);

            ASSERT_EQ(vm::bbox3(vm::vec3(1.0, 0.0, 0.0), vm::vec3(31.0, 31.0, 31.0)), brush1->logicalBounds());
            ASSERT_EQ(vm::bbox3(vm::vec3(0.0, 0.0, 0.0), vm::vec3(1.0, 31.0, 31.0)), brush2->logicalBounds());
        }

        TEST_F(MapDocumentTest, rotate) {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0.0, 0.0, 0.0), vm::vec3(30.0, 31.0, 31.0)), "texture");
            Model::Brush *brush2 = builder.createCuboid(vm::bbox3(vm::vec3(30.0, 0.0, 0.0), vm::vec3(31.0, 31.0, 31.0)), "texture");

            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);

            document->addNode(brush1, document->currentParent());
            document->addNode(brush2, document->currentParent());

            std::vector<Model::Node*> brushes;
            brushes.push_back(brush1);
            brushes.push_back(brush2);
            document->select(brushes);

            vm::vec3 boundsCenter = document->selectionBounds().center();
            ASSERT_EQ(vm::vec3(15.5, 15.5, 15.5), boundsCenter);

            // 90 degrees CCW about the Z axis through the center of the selection
            document->rotateObjects(boundsCenter, vm::vec3::pos_z(), vm::to_radians(90.0));

            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);

            const vm::bbox3 brush1ExpectedBounds(vm::vec3(0.0, 0.0, 0.0), vm::vec3(31.0, 30.0, 31.0));
            const vm::bbox3 brush2ExpectedBounds(vm::vec3(0.0, 30.0, 0.0), vm::vec3(31.0, 31.0, 31.0));

            // these should be exactly integral
            ASSERT_EQ(brush1ExpectedBounds, brush1->logicalBounds());
            ASSERT_EQ(brush2ExpectedBounds, brush2->logicalBounds());
        }

        TEST_F(MapDocumentTest, shearCube) {
            const vm::bbox3 initialBBox(vm::vec3(100,100,100), vm::vec3(200,200,200));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(initialBBox, "texture");

            document->addNode(brush1, document->currentParent());
            document->select(std::vector<Model::Node*>{brush1});

            const std::vector<vm::vec3> initialPositions{
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
            ASSERT_COLLECTIONS_EQUIVALENT(initialPositions, brush1->vertexPositions());

            // Shear the -Y face by (50, 0, 0). That means the verts with Y=100 will get sheared.
            ASSERT_TRUE(document->shearObjects(initialBBox, vm::vec3::neg_y(), vm::vec3(50,0,0)));

            const std::vector<vm::vec3> shearedPositions{
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
            ASSERT_COLLECTIONS_EQUIVALENT(shearedPositions, brush1->vertexPositions());
        }

        TEST_F(MapDocumentTest, shearPillar) {
            const vm::bbox3 initialBBox(vm::vec3(0,0,0), vm::vec3(100,100,400));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(initialBBox, "texture");

            document->addNode(brush1, document->currentParent());
            document->select(std::vector<Model::Node*>{brush1});

            const std::vector<vm::vec3> initialPositions{
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
            ASSERT_COLLECTIONS_EQUIVALENT(initialPositions, brush1->vertexPositions());

            // Shear the +Z face by (50, 0, 0). That means the verts with Z=400 will get sheared.
            ASSERT_TRUE(document->shearObjects(initialBBox, vm::vec3::pos_z(), vm::vec3(50,0,0)));

            const std::vector<vm::vec3> shearedPositions{
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
            ASSERT_COLLECTIONS_EQUIVALENT(shearedPositions, brush1->vertexPositions());
        }

        TEST_F(MapDocumentTest, scaleObjects) {
            const vm::bbox3 initialBBox(vm::vec3(-100,-100,-100), vm::vec3(100,100,100));
            const vm::bbox3 doubleBBox(2.0 * initialBBox.min, 2.0 * initialBBox.max);
            const vm::bbox3 invalidBBox(vm::vec3(0,-100,-100), vm::vec3(0,100,100));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(initialBBox, "texture");

            document->addNode(brush1, document->currentParent());
            document->select(std::vector<Model::Node*>{brush1});

            ASSERT_EQ(vm::vec3(200,200,200), brush1->logicalBounds().size());
            ASSERT_EQ(vm::plane3(100.0, vm::vec3::pos_z()), brush1->findFace(vm::vec3::pos_z())->boundary());

            // attempting an invalid scale has no effect
            ASSERT_FALSE(document->scaleObjects(initialBBox, invalidBBox));
            ASSERT_EQ(vm::vec3(200,200,200), brush1->logicalBounds().size());
            ASSERT_EQ(vm::plane3(100.0, vm::vec3::pos_z()), brush1->findFace(vm::vec3::pos_z())->boundary());

            ASSERT_TRUE(document->scaleObjects(initialBBox, doubleBBox));
            ASSERT_EQ(vm::vec3(400,400,400), brush1->logicalBounds().size());
            ASSERT_EQ(vm::plane3(200.0, vm::vec3::pos_z()), brush1->findFace(vm::vec3::pos_z())->boundary());
        }

        TEST_F(MapDocumentTest, scaleObjectsInGroup) {
            const vm::bbox3 initialBBox(vm::vec3(-100, -100, -100), vm::vec3(100, 100, 100));
            const vm::bbox3 doubleBBox(2.0 * initialBBox.min, 2.0 * initialBBox.max);
            const vm::bbox3 invalidBBox(vm::vec3(0, -100, -100), vm::vec3(0, 100, 100));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(initialBBox, "texture");

            document->addNode(brush1, document->currentParent());
            document->select(std::vector<Model::Node*>{ brush1 });
            [[maybe_unused]] Model::Group* group = document->groupSelection("my group");

            // attempting an invalid scale has no effect
            ASSERT_FALSE(document->scaleObjects(initialBBox, invalidBBox));
            ASSERT_EQ(vm::vec3(200, 200, 200), brush1->logicalBounds().size());

            ASSERT_TRUE(document->scaleObjects(initialBBox, doubleBBox));
            ASSERT_EQ(vm::vec3(400, 400, 400), brush1->logicalBounds().size());
        }

        TEST_F(MapDocumentTest, scaleObjectsWithCenter) {
            const vm::bbox3 initialBBox(vm::vec3(0,0,0), vm::vec3(100,100,400));
            const vm::bbox3 expectedBBox(vm::vec3(-50,0,0), vm::vec3(150,100,400));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCuboid(initialBBox, "texture");

            document->addNode(brush1, document->currentParent());
            document->select(std::vector<Model::Node*>{brush1});

            const vm::vec3 boundsCenter = initialBBox.center();
            ASSERT_TRUE(document->scaleObjects(boundsCenter, vm::vec3(2.0, 1.0, 1.0)));
            ASSERT_EQ(expectedBBox, brush1->logicalBounds());
        }

        TEST_F(MapDocumentTest, csgConvexMergeBrushes) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());

            auto* brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture");
            auto* brush2 = builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture");
            document->addNode(brush1, entity);
            document->addNode(brush2, document->currentParent());
            ASSERT_EQ(1u, entity->children().size());

            document->select(std::vector<Model::Node*> { brush1, brush2 });
            ASSERT_TRUE(document->csgConvexMerge());
            ASSERT_EQ(1u, entity->children().size()); // added to the parent of the first brush

            auto* brush3 = entity->children().front();
            ASSERT_EQ(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), brush3->logicalBounds());
        }

        TEST_F(MapDocumentTest, csgConvexMergeFaces) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());

            auto* brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture");
            auto* brush2 = builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture");
            document->addNode(brush1, entity);
            document->addNode(brush2, document->currentParent());
            ASSERT_EQ(1u, entity->children().size());

            auto* face1 = brush1->faces().front();
            auto* face2 = brush2->faces().front();

            document->select(std::vector<Model::BrushFace*>{ face1, face2 });
            ASSERT_TRUE(document->csgConvexMerge());
            ASSERT_EQ(2u, entity->children().size()); // added to the parent of the first brush, original brush is not deleted

            auto* brush3 = entity->children().back();

            // check our assumption about the order of the entities' children
            assert(brush3 != brush1);
            assert(brush3 != brush2);

            const auto face1Verts = face1->vertexPositions();
            const auto face2Verts = face2->vertexPositions();

            const auto bounds = vm::merge(
                vm::bbox3::merge_all(std::begin(face1Verts), std::end(face1Verts)),
                vm::bbox3::merge_all(std::begin(face2Verts), std::end(face2Verts))
            );

            ASSERT_EQ(bounds, brush3->logicalBounds());
        }

        TEST_F(MapDocumentTest, setTextureNull) {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::Brush *brush1 = builder.createCube(64.0, Model::BrushFaceAttributes::NoTextureName);

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

            Model::ParallelTexCoordSystem texAlignment(vm::vec3(1, 0, 0), vm::vec3(0, 1, 0));
            auto texAlignmentSnapshot = texAlignment.takeSnapshot();

            Model::Brush* brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture");
            Model::Brush* brush2 = builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture");
            brush1->findFace(vm::vec3::pos_z())->restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);
            brush2->findFace(vm::vec3::pos_z())->restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);
            document->addNode(brush1, entity);
            document->addNode(brush2, entity);
            ASSERT_EQ(2u, entity->children().size());

            document->select(std::vector<Model::Node*> { brush1, brush2 });
            ASSERT_TRUE(document->csgConvexMerge());
            ASSERT_EQ(1u, entity->children().size());

            Model::Brush* brush3 = static_cast<Model::Brush*>(entity->children()[0]);
            Model::BrushFace* top = brush3->findFace(vm::vec3::pos_z());
            ASSERT_EQ(vm::vec3(1, 0, 0), top->textureXAxis());
            ASSERT_EQ(vm::vec3(0, 1, 0), top->textureYAxis());
        }

        TEST_F(ValveMapDocumentTest, csgSubtractTexturing) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());

            Model::ParallelTexCoordSystem texAlignment(vm::vec3(1, 0, 0), vm::vec3(0, 1, 0));
            auto texAlignmentSnapshot = texAlignment.takeSnapshot();

            Model::Brush* brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture");
            Model::Brush* brush2 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 32)), "texture");
            brush2->findFace(vm::vec3::pos_z())->restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);
            document->addNode(brush1, entity);
            document->addNode(brush2, entity);
            ASSERT_EQ(2u, entity->children().size());

            // we want to compute brush1 - brush2
            document->select(std::vector<Model::Node*> { brush2 });
            ASSERT_TRUE(document->csgSubtract());
            ASSERT_EQ(1u, entity->children().size());

            Model::Brush* brush3 = static_cast<Model::Brush*>(entity->children()[0]);
            ASSERT_EQ(vm::bbox3(vm::vec3(0, 0, 32), vm::vec3(64, 64, 64)), brush3->logicalBounds());

            // the texture alignment from the top of brush2 should have transferred
            // to the bottom face of brush3
            Model::BrushFace* top = brush3->findFace(vm::vec3::neg_z());
            ASSERT_EQ(vm::vec3(1, 0, 0), top->textureXAxis());
            ASSERT_EQ(vm::vec3(0, 1, 0), top->textureYAxis());
        }

        TEST_F(MapDocumentTest, csgSubtractMultipleBrushes) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());

            Model::Brush* minuend = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture");
            Model::Brush* subtrahend1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 32, 64)), "texture");
            Model::Brush* subtrahend2 = builder.createCuboid(vm::bbox3(vm::vec3(32, 32, 0), vm::vec3(64, 64, 64)), "texture");

            document->addNodes(std::vector<Model::Node*>{minuend, subtrahend1, subtrahend2}, entity);
            ASSERT_EQ(3u, entity->children().size());

            // we want to compute minuend - {subtrahend1, subtrahend2}
            document->select(std::vector<Model::Node*>{subtrahend1, subtrahend2});
            ASSERT_TRUE(document->csgSubtract());
            ASSERT_EQ(2u, entity->children().size());

            auto* remainder1 = dynamic_cast<Model::Brush*>(entity->children()[0]);
            auto* remainder2 = dynamic_cast<Model::Brush*>(entity->children()[1]);
            ASSERT_NE(nullptr, remainder1);
            ASSERT_NE(nullptr, remainder2);

            const auto expectedBBox1 = vm::bbox3(vm::vec3(0, 32, 0), vm::vec3(32, 64, 64));
            const auto expectedBBox2 = vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 32, 64));

            if (remainder1->logicalBounds() != expectedBBox1) {
                std::swap(remainder1, remainder2);
            }

            EXPECT_EQ(expectedBBox1, remainder1->logicalBounds());
            EXPECT_EQ(expectedBBox2, remainder2->logicalBounds());
        }

        TEST_F(MapDocumentTest, csgSubtractAndUndoRestoresSelection) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());

            Model::Brush* subtrahend1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture");
            document->addNodes(std::vector<Model::Node*>{subtrahend1}, entity);

            document->select(std::vector<Model::Node*>{subtrahend1});
            ASSERT_TRUE(document->csgSubtract());
            ASSERT_EQ(0u, entity->children().size());
            EXPECT_TRUE(document->selectedNodes().empty());

            // check that the selection is restored after undo
            document->undoCommand();

            EXPECT_TRUE(document->selectedNodes().hasOnlyBrushes());
            EXPECT_EQ(std::vector<Model::Brush*>({ subtrahend1 }), document->selectedNodes().brushes());
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
            document->select(std::vector<Model::Node*> {innerEnt1, innerEnt2});

            Model::Group* inner = document->groupSelection("Inner");

            document->deselectAll();
            document->addNode(outerEnt1, document->currentParent());
            document->addNode(outerEnt2, document->currentParent());
            document->select(std::vector<Model::Node*> {inner, outerEnt1, outerEnt2});

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
            document->select(std::vector<Model::Node*> {ent1});

            Model::Group* group = document->groupSelection("Group");
            ASSERT_EQ((std::vector<Model::Node*> {group}), document->selectedNodes().nodes());

            document->ungroupSelection();
            ASSERT_EQ((std::vector<Model::Node*> {ent1}), document->selectedNodes().nodes());
        }

        TEST_F(MapDocumentTest, ungroupLeavesBrushEntitySelected) {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            Model::Entity* ent1 = new Model::Entity();
            document->addNode(ent1, document->currentParent());

            Model::Brush* brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture");
            document->addNode(brush1, ent1);
            document->select(std::vector<Model::Node*>{ent1});
            ASSERT_EQ((std::vector<Model::Node*> {brush1}), document->selectedNodes().nodes());
            ASSERT_FALSE(ent1->selected());
            ASSERT_TRUE(brush1->selected());

            Model::Group* group = document->groupSelection("Group");
            ASSERT_EQ((std::vector<Model::Node*> {ent1}), group->children());
            ASSERT_EQ((std::vector<Model::Node*> {group}), document->selectedNodes().nodes());

            document->ungroupSelection();
            ASSERT_EQ((std::vector<Model::Node*> {brush1}), document->selectedNodes().nodes());
            ASSERT_FALSE(ent1->selected());
            ASSERT_TRUE(brush1->selected());
        }

        TEST_F(MapDocumentTest, mergeGroups) {
            document->selectAllNodes();
            document->deleteObjects();

            Model::Entity* ent1 = new Model::Entity();
            document->addNode(ent1, document->currentParent());
            document->deselectAll();
            document->select(std::vector<Model::Node*> {ent1});
            Model::Group* group1 = document->groupSelection("group1");

            Model::Entity* ent2 = new Model::Entity();
            document->addNode(ent2, document->currentParent());
            document->deselectAll();
            document->select(std::vector<Model::Node*> {ent2});
            Model::Group* group2 = document->groupSelection("group2");

            ASSERT_COLLECTIONS_EQUIVALENT(std::vector<Model::Node*>({ group1, group2 }), document->currentLayer()->children());

            document->select(std::vector<Model::Node*> {group1, group2});
            document->mergeSelectedGroupsWithGroup(group2);

            ASSERT_EQ((std::vector<Model::Node*> {group2}), document->selectedNodes().nodes());
            ASSERT_EQ((std::vector<Model::Node*> {group2}), document->currentLayer()->children());

            ASSERT_COLLECTIONS_EQUIVALENT(std::vector<Model::Node*>({}), group1->children());
            ASSERT_COLLECTIONS_EQUIVALENT(std::vector<Model::Node*>({ ent1, ent2 }), group2->children());
        }

        TEST_F(MapDocumentTest, pickSingleBrush) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture");
            document->addNode(brush1, document->currentParent());

            Model::PickResult pickResult;
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.query().all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vm::vec3::neg_x()), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::neg_x()), pickResult);
            ASSERT_TRUE(pickResult.query().all().empty());
        }

        TEST_F(MapDocumentTest, pickSingleEntity) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::Entity* ent1 = new Model::Entity();
            document->addNode(ent1, document->currentParent());

            const auto origin = ent1->origin();
            const auto bounds = ent1->logicalBounds();

            const auto rayOrigin = origin + vm::vec3(-32.0, bounds.size().y() / 2.0, bounds.size().z() / 2.0);

            Model::PickResult pickResult;
            document->pick(vm::ray3(rayOrigin, vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.query().all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(ent1, hits.front().target<Model::Entity*>());
            ASSERT_DOUBLE_EQ(32.0 - bounds.size().x() / 2.0, hits.front().distance());

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::neg_x()), pickResult);
            ASSERT_TRUE(pickResult.query().all().empty());
        }

        TEST_F(MapDocumentTest, pickSimpleGroup) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture");
            document->addNode(brush1, document->currentParent());

            auto* brush2 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture");
            document->addNode(brush2, document->currentParent());

            document->selectAllNodes();
            auto* group = document->groupSelection("test");

            Model::PickResult pickResult;
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            // picking a grouped object when the containing group is closed should return the object,
            // which is converted to the group when hitsToNodesWithGroupPicking() is used.
            auto hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vm::vec3::neg_x()), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            ASSERT_EQ(std::vector<Model::Node*>{ group }, hitsToNodesWithGroupPicking(hits));

            // hitting both objects in the group should return the group only once
            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(32, 32, -32), vm::vec3::pos_z()), pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(2u, hits.size());

            ASSERT_EQ(std::vector<Model::Node*>{ group }, hitsToNodesWithGroupPicking(hits));

            // hitting the group bounds doesn't count as a hit
            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 96), vm::vec3::pos_x()), pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_TRUE(hits.empty());

            // hitting a grouped object when the containing group is open should return the object only
            document->openGroup(group);

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vm::vec3::neg_x()), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            ASSERT_EQ(std::vector<Model::Node*>{ brush1 }, hitsToNodesWithGroupPicking(hits));
        }

        TEST_F(MapDocumentTest, pickNestedGroup) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture");
            document->addNode(brush1, document->currentParent());

            auto* brush2 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture");
            document->addNode(brush2, document->currentParent());

            document->selectAllNodes();
            auto* inner = document->groupSelection("inner");

            document->deselectAll();
            auto* brush3 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 256)), "texture");
            document->addNode(brush3, document->currentParent());

            document->selectAllNodes();
            auto* outer = document->groupSelection("outer");

            const vm::ray3 highRay(vm::vec3(-32, 0, 256+32), vm::vec3::pos_x());
            const vm::ray3  lowRay(vm::vec3(-32, 0,    +32), vm::vec3::pos_x());

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

            ASSERT_EQ(brush3->findFace(vm::vec3::neg_x()), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            ASSERT_EQ(std::vector<Model::Node*>{ brush3 }, hitsToNodesWithGroupPicking(hits));

            // hitting the brush in the inner group should return the inner group when hitsToNodesWithGroupPicking() is used
            pickResult.clear();
            document->pick(lowRay, pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vm::vec3::neg_x()), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());
            ASSERT_EQ(std::vector<Model::Node*>{ inner }, hitsToNodesWithGroupPicking(hits));

            // open the inner group, too. hitsToNodesWithGroupPicking() should no longer return groups, since all groups are open.
            document->openGroup(inner);

            /*
             * world
             * * outer (open)
             *   * inner (open)
             *     * brush1
             *     * brush2
             *   * brush3
             */

            ASSERT_TRUE(inner->opened());
            ASSERT_FALSE(outer->opened());
            ASSERT_TRUE(outer->hasOpenedDescendant());

            // pick a brush in the outer group
            pickResult.clear();
            document->pick(highRay, pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush3->findFace(vm::vec3::neg_x()), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());
            ASSERT_EQ(std::vector<Model::Node*>{ brush3 }, hitsToNodesWithGroupPicking(hits));

            // pick a brush in the inner group
            pickResult.clear();
            document->pick(lowRay, pickResult);

            hits = pickResult.query().type(Model::Brush::BrushHit).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vm::vec3::neg_x()), hits.front().target<Model::BrushFace*>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());
            ASSERT_EQ(std::vector<Model::Node*>{ brush1 }, hitsToNodesWithGroupPicking(hits));
        }

        TEST_F(MapDocumentTest, pickBrushEntity) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto *brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture");
            document->addNode(brush1, document->currentParent());

            auto *brush2 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)),
                                                "texture");
            document->addNode(brush2, document->currentParent());

            document->selectAllNodes();

            document->createBrushEntity(m_brushEntityDef);
            document->deselectAll();

            Model::PickResult pickResult;

            // picking entity brushes should only return the brushes and not the entity
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.query().all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1->findFace(vm::vec3::neg_x()), hits.front().target<Model::BrushFace *>());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());
        }

        TEST_F(MapDocumentTest, throwExceptionDuringCommand) {
            ASSERT_THROW(document->throwExceptionDuringCommand(), GeometryException);
        }

        // https://github.com/kduske/TrenchBroom/issues/2476
        TEST_F(MapDocumentTest, selectTouching) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto *brush1 = builder.createCuboid(box, "texture");
            document->addNode(brush1, document->currentParent());

            auto *brush2 = builder.createCuboid(box.translate(vm::vec3(1, 1, 1)), "texture");
            document->addNode(brush2, document->currentParent());

            document->selectAllNodes();

            EXPECT_EQ((std::vector<Model::Brush *>{brush1, brush2}), document->selectedNodes().brushes());
            EXPECT_EQ((std::vector<Model::Node *>{brush1, brush2}), document->currentLayer()->children());

            document->selectTouching(true);

            // only this next line was failing
            EXPECT_EQ(std::vector<Model::Brush *>{}, document->selectedNodes().brushes());
            EXPECT_EQ(std::vector<Model::Node *>{}, document->currentLayer()->children());

            // brush1 and brush2 are deleted
            EXPECT_EQ(nullptr, brush1->parent());
            EXPECT_EQ(nullptr, brush2->parent());
        }

        // https://github.com/kduske/TrenchBroom/issues/2776
        TEST_F(MapDocumentTest, pasteAndTranslateGroup) {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto *brush1 = builder.createCuboid(box, "texture");
            document->addNode(brush1, document->currentParent());
            document->select(brush1);

            const auto groupName = std::string("testGroup");

            auto* group = document->groupSelection(groupName);
            ASSERT_NE(nullptr, group);
            document->select(group);

            const std::string copied = document->serializeSelectedNodes();

            const auto delta = vm::vec3(16, 16, 16);
            ASSERT_EQ(PasteType::Node, document->paste(copied));
            ASSERT_EQ(1u, document->selectedNodes().groupCount());
            ASSERT_EQ(groupName, document->selectedNodes().groups().at(0)->name());
            ASSERT_TRUE(document->translateObjects(delta));
            ASSERT_EQ(box.translate(delta), document->selectionBounds());
        }
    }
}
