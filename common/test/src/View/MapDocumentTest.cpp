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
#include "Assets/EntityDefinition.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EmptyAttributeNameIssueGenerator.h"
#include "Model/EmptyAttributeValueIssueGenerator.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/HitAdapter.h"
#include "Model/HitQuery.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/MapFormat.h"
#include "Model/ModelUtils.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "Model/TestGame.h"
#include "Model/VisibilityState.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/PasteType.h"
#include "View/SelectionTool.h"

#include <kdl/result.h>
#include <kdl/overload.h>
#include <kdl/vector_utils.h>

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/ray.h>
#include <vecmath/ray_io.h>
#include <vecmath/scalar.h>

#include "TestUtils.h"

namespace TrenchBroom {
    namespace View {
        MapDocumentTest::MapDocumentTest() :
        MapDocumentTest(Model::MapFormat::Standard) {
            SetUp();
        }

        MapDocumentTest::MapDocumentTest(const Model::MapFormat mapFormat) :
        m_mapFormat(mapFormat),
        m_pointEntityDef(nullptr),
        m_brushEntityDef(nullptr) {
            SetUp();
        }

        void MapDocumentTest::SetUp() {
            game = std::make_shared<Model::TestGame>();
            document = MapDocumentCommandFacade::newMapDocument();
            document->newDocument(m_mapFormat, vm::bbox3(8192.0), game);

            // create two entity definitions
            m_pointEntityDef = new Assets::PointEntityDefinition("point_entity", Color(), vm::bbox3(16.0), "this is a point entity", {}, {});
            m_brushEntityDef = new Assets::BrushEntityDefinition("brush_entity", Color(), "this is a brush entity", {});

            document->setEntityDefinitions(std::vector<Assets::EntityDefinition*>{ m_pointEntityDef, m_brushEntityDef });
        }

        MapDocumentTest::~MapDocumentTest() {
            m_pointEntityDef = nullptr;
            m_brushEntityDef = nullptr;
        }

        Model::BrushNode* MapDocumentTest::createBrushNode(const std::string& textureName, const std::function<void(Model::Brush&)>& brushFunc) {
            const Model::WorldNode* world = document->world();
            Model::BrushBuilder builder(world, document->worldBounds(), document->game()->defaultFaceAttribs());
            Model::Brush brush = builder.createCube(32.0, textureName).value();
            brushFunc(brush);
            return world->createBrush(std::move(brush));
        }

        static void checkPlanePointsIntegral(const Model::BrushNode* brushNode) {
            for (const Model::BrushFace& face : brushNode->brush().faces()) {
                for (size_t i=0; i<3; i++) {
                    vm::vec3 point = face.points()[i];
                    ASSERT_POINT_INTEGRAL(point);
                }
            }
        }

        static void checkVerticesIntegral(const Model::BrushNode* brushNode) {
            const Model::Brush& brush = brushNode->brush();
            for (const Model::BrushVertex* vertex : brush.vertices()) {
                ASSERT_POINT_INTEGRAL(vertex->position());
            }
        }

        static void checkBoundsIntegral(const Model::BrushNode* brush) {
            ASSERT_POINT_INTEGRAL(brush->logicalBounds().min);
            ASSERT_POINT_INTEGRAL(brush->logicalBounds().max);
        }

        static void checkBrushIntegral(const Model::BrushNode* brush) {
            checkPlanePointsIntegral(brush);
            checkVerticesIntegral(brush);
            checkBoundsIntegral(brush);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.flip") {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::BrushNode* brush1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0.0, 0.0, 0.0), vm::vec3(30.0, 31.0, 31.0)), "texture").value());
            Model::BrushNode* brush2 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(30.0, 0.0, 0.0), vm::vec3(31.0, 31.0, 31.0)), "texture").value());

            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);

            document->addNode(brush1, document->parentForNodes());
            document->addNode(brush2, document->parentForNodes());

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

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.rotate") {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::BrushNode* brush1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0.0, 0.0, 0.0), vm::vec3(30.0, 31.0, 31.0)), "texture").value());
            Model::BrushNode* brush2 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(30.0, 0.0, 0.0), vm::vec3(31.0, 31.0, 31.0)), "texture").value());

            checkBrushIntegral(brush1);
            checkBrushIntegral(brush2);

            document->addNode(brush1, document->parentForNodes());
            document->addNode(brush2, document->parentForNodes());

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

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.shearCube") {
            const vm::bbox3 initialBBox(vm::vec3(100,100,100), vm::vec3(200,200,200));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::BrushNode* brushNode = document->world()->createBrush(builder.createCuboid(initialBBox, "texture").value());

            document->addNode(brushNode, document->parentForNodes());
            document->select(std::vector<Model::Node*>{brushNode});

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
            ASSERT_COLLECTIONS_EQUIVALENT(initialPositions, brushNode->brush().vertexPositions());

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
            ASSERT_COLLECTIONS_EQUIVALENT(shearedPositions, brushNode->brush().vertexPositions());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.shearPillar") {
            const vm::bbox3 initialBBox(vm::vec3(0,0,0), vm::vec3(100,100,400));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::BrushNode* brushNode = document->world()->createBrush(builder.createCuboid(initialBBox, "texture").value());

            document->addNode(brushNode, document->parentForNodes());
            document->select(std::vector<Model::Node*>{brushNode});

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
            ASSERT_COLLECTIONS_EQUIVALENT(initialPositions, brushNode->brush().vertexPositions());

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
            ASSERT_COLLECTIONS_EQUIVALENT(shearedPositions, brushNode->brush().vertexPositions());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.scaleObjects") {
            const vm::bbox3 initialBBox(vm::vec3(-100,-100,-100), vm::vec3(100,100,100));
            const vm::bbox3 doubleBBox(2.0 * initialBBox.min, 2.0 * initialBBox.max);
            const vm::bbox3 invalidBBox(vm::vec3(0,-100,-100), vm::vec3(0,100,100));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::BrushNode* brushNode = document->world()->createBrush(builder.createCuboid(initialBBox, "texture").value());
            const Model::Brush& brush = brushNode->brush();

            document->addNode(brushNode, document->parentForNodes());
            document->select(std::vector<Model::Node*>{brushNode});

            ASSERT_EQ(vm::vec3(200,200,200), brushNode->logicalBounds().size());
            ASSERT_EQ(vm::plane3(100.0, vm::vec3::pos_z()), brush.face(*brush.findFace(vm::vec3::pos_z())).boundary());

            // attempting an invalid scale has no effect
            ASSERT_FALSE(document->scaleObjects(initialBBox, invalidBBox));
            ASSERT_EQ(vm::vec3(200,200,200), brushNode->logicalBounds().size());
            ASSERT_EQ(vm::plane3(100.0, vm::vec3::pos_z()), brush.face(*brush.findFace(vm::vec3::pos_z())).boundary());

            ASSERT_TRUE(document->scaleObjects(initialBBox, doubleBBox));
            ASSERT_EQ(vm::vec3(400,400,400), brushNode->logicalBounds().size());
            ASSERT_EQ(vm::plane3(200.0, vm::vec3::pos_z()), brush.face(*brush.findFace(vm::vec3::pos_z())).boundary());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.scaleObjectsInGroup") {
            const vm::bbox3 initialBBox(vm::vec3(-100, -100, -100), vm::vec3(100, 100, 100));
            const vm::bbox3 doubleBBox(2.0 * initialBBox.min, 2.0 * initialBBox.max);
            const vm::bbox3 invalidBBox(vm::vec3(0, -100, -100), vm::vec3(0, 100, 100));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::BrushNode* brushNode = document->world()->createBrush(builder.createCuboid(initialBBox, "texture").value());

            document->addNode(brushNode, document->parentForNodes());
            document->select(std::vector<Model::Node*>{ brushNode });
            [[maybe_unused]] Model::GroupNode* group = document->groupSelection("my group");

            // attempting an invalid scale has no effect
            ASSERT_FALSE(document->scaleObjects(initialBBox, invalidBBox));
            ASSERT_EQ(vm::vec3(200, 200, 200), brushNode->logicalBounds().size());

            ASSERT_TRUE(document->scaleObjects(initialBBox, doubleBBox));
            ASSERT_EQ(vm::vec3(400, 400, 400), brushNode->logicalBounds().size());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.scaleObjectsWithCenter") {
            const vm::bbox3 initialBBox(vm::vec3(0,0,0), vm::vec3(100,100,400));
            const vm::bbox3 expectedBBox(vm::vec3(-50,0,0), vm::vec3(150,100,400));

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::BrushNode* brushNode = document->world()->createBrush(builder.createCuboid(initialBBox, "texture").value());

            document->addNode(brushNode, document->parentForNodes());
            document->select(std::vector<Model::Node*>{brushNode});

            const vm::vec3 boundsCenter = initialBBox.center();
            ASSERT_TRUE(document->scaleObjects(boundsCenter, vm::vec3(2.0, 1.0, 1.0)));
            ASSERT_EQ(expectedBBox, brushNode->logicalBounds());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.csgConvexMergeBrushes") {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* entity = new Model::EntityNode();
            document->addNode(entity, document->parentForNodes());

            auto* brush1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture").value());
            auto* brush2 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNode(brush1, entity);
            document->addNode(brush2, document->parentForNodes());
            ASSERT_EQ(1u, entity->children().size());

            document->select(std::vector<Model::Node*> { brush1, brush2 });
            ASSERT_TRUE(document->csgConvexMerge());
            ASSERT_EQ(1u, entity->children().size()); // added to the parent of the first brush

            auto* brush3 = entity->children().front();
            ASSERT_EQ(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), brush3->logicalBounds());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.csgConvexMergeFaces") {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* entity = new Model::EntityNode();
            document->addNode(entity, document->parentForNodes());

            auto* brushNode1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture").value());
            auto* brushNode2 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNode(brushNode1, entity);
            document->addNode(brushNode2, document->parentForNodes());
            ASSERT_EQ(1u, entity->children().size());

            const auto faceIndex = 0u;
            const auto& face1 = brushNode1->brush().face(faceIndex);
            const auto& face2 = brushNode2->brush().face(faceIndex);

            document->select({
                { brushNode1, faceIndex },
                { brushNode2, faceIndex }
            });
            ASSERT_TRUE(document->csgConvexMerge());
            ASSERT_EQ(2u, entity->children().size()); // added to the parent of the first brush, original brush is not deleted

            auto* brush3 = entity->children().back();

            // check our assumption about the order of the entities' children
            assert(brush3 != brushNode1);
            assert(brush3 != brushNode2);

            const auto face1Verts = face1.vertexPositions();
            const auto face2Verts = face2.vertexPositions();

            const auto bounds = vm::merge(
                vm::bbox3::merge_all(std::begin(face1Verts), std::end(face1Verts)),
                vm::bbox3::merge_all(std::begin(face2Verts), std::end(face2Verts))
            );

            ASSERT_EQ(bounds, brush3->logicalBounds());
        }

        ValveMapDocumentTest::ValveMapDocumentTest() :
        MapDocumentTest(Model::MapFormat::Valve) {}

        TEST_CASE_METHOD(ValveMapDocumentTest, "ValveMapDocumentTest.csgConvexMergeTexturing") {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, document->parentForNodes());

            Model::ParallelTexCoordSystem texAlignment(vm::vec3(1, 0, 0), vm::vec3(0, 1, 0));
            auto texAlignmentSnapshot = texAlignment.takeSnapshot();

            Model::Brush brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture").value();
            brush1.face(*brush1.findFace(vm::vec3::pos_z())).restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);

            Model::Brush brush2 = builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture").value();
            brush2.face(*brush2.findFace(vm::vec3::pos_z())).restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);

            Model::BrushNode* brushNode1 = document->world()->createBrush(std::move(brush1));
            Model::BrushNode* brushNode2 = document->world()->createBrush(std::move(brush2));
            
            document->addNode(brushNode1, entity);
            document->addNode(brushNode2, entity);
            ASSERT_EQ(2u, entity->children().size());

            document->select(std::vector<Model::Node*> { brushNode1, brushNode2 });
            ASSERT_TRUE(document->csgConvexMerge());
            ASSERT_EQ(1u, entity->children().size());

            Model::BrushNode* brushNode3 = static_cast<Model::BrushNode*>(entity->children()[0]);
            const Model::Brush& brush3 = brushNode3->brush();
            
            const Model::BrushFace& top = brush3.face(*brush3.findFace(vm::vec3::pos_z()));
            ASSERT_EQ(vm::vec3(1, 0, 0), top.textureXAxis());
            ASSERT_EQ(vm::vec3(0, 1, 0), top.textureYAxis());
        }

        TEST_CASE_METHOD(ValveMapDocumentTest, "ValveMapDocumentTest.csgSubtractTexturing") {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, document->parentForNodes());

            Model::ParallelTexCoordSystem texAlignment(vm::vec3(1, 0, 0), vm::vec3(0, 1, 0));
            auto texAlignmentSnapshot = texAlignment.takeSnapshot();

            Model::Brush brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value();
            Model::Brush brush2 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 32)), "texture").value();
            brush2.face(*brush2.findFace(vm::vec3::pos_z())).restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);

            Model::BrushNode* brushNode1 = document->world()->createBrush(std::move(brush1));
            Model::BrushNode* brushNode2 = document->world()->createBrush(std::move(brush2));
            
            document->addNode(brushNode1, entity);
            document->addNode(brushNode2, entity);
            ASSERT_EQ(2u, entity->children().size());

            // we want to compute brush1 - brush2
            document->select(std::vector<Model::Node*> { brushNode2 });
            ASSERT_TRUE(document->csgSubtract());
            ASSERT_EQ(1u, entity->children().size());

            Model::BrushNode* brushNode3 = static_cast<Model::BrushNode*>(entity->children()[0]);
            const Model::Brush& brush3 = brushNode3->brush();

            ASSERT_EQ(vm::bbox3(vm::vec3(0, 0, 32), vm::vec3(64, 64, 64)), brushNode3->logicalBounds());

            // the texture alignment from the top of brush2 should have transferred
            // to the bottom face of brush3
            const Model::BrushFace& top = brush3.face(*brush3.findFace(vm::vec3::neg_z()));
            ASSERT_EQ(vm::vec3(1, 0, 0), top.textureXAxis());
            ASSERT_EQ(vm::vec3(0, 1, 0), top.textureYAxis());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.csgSubtractMultipleBrushes") {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* entity = new Model::EntityNode();
            document->addNode(entity, document->parentForNodes());

            Model::BrushNode* minuend = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            Model::BrushNode* subtrahend1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 32, 64)), "texture").value());
            Model::BrushNode* subtrahend2 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(32, 32, 0), vm::vec3(64, 64, 64)), "texture").value());

            document->addNodes(std::vector<Model::Node*>{minuend, subtrahend1, subtrahend2}, entity);
            ASSERT_EQ(3u, entity->children().size());

            // we want to compute minuend - {subtrahend1, subtrahend2}
            document->select(std::vector<Model::Node*>{subtrahend1, subtrahend2});
            ASSERT_TRUE(document->csgSubtract());
            ASSERT_EQ(2u, entity->children().size());

            auto* remainder1 = dynamic_cast<Model::BrushNode*>(entity->children()[0]);
            auto* remainder2 = dynamic_cast<Model::BrushNode*>(entity->children()[1]);
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

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.csgSubtractAndUndoRestoresSelection") {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* entity = new Model::EntityNode();
            document->addNode(entity, document->parentForNodes());

            Model::BrushNode* subtrahend1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNodes(std::vector<Model::Node*>{subtrahend1}, entity);

            document->select(std::vector<Model::Node*>{subtrahend1});
            ASSERT_TRUE(document->csgSubtract());
            ASSERT_EQ(0u, entity->children().size());
            EXPECT_TRUE(document->selectedNodes().empty());

            // check that the selection is restored after undo
            document->undoCommand();

            EXPECT_TRUE(document->selectedNodes().hasOnlyBrushes());
            EXPECT_EQ(std::vector<Model::BrushNode*>({ subtrahend1 }), document->selectedNodes().brushes());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.newWithGroupOpen") {
            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, document->parentForNodes());
            document->select(entity);
            Model::GroupNode* group = document->groupSelection("my group");
            document->openGroup(group);

            ASSERT_EQ(group, document->currentGroup());

            document->newDocument(Model::MapFormat::Valve, MapDocument::DefaultWorldBounds, document->game());

            ASSERT_EQ(nullptr, document->currentGroup());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.ungroupInnerGroup") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2050
            Model::EntityNode* outerEnt1 = new Model::EntityNode();
            Model::EntityNode* outerEnt2 = new Model::EntityNode();
            Model::EntityNode* innerEnt1 = new Model::EntityNode();
            Model::EntityNode* innerEnt2 = new Model::EntityNode();

            document->addNode(innerEnt1, document->parentForNodes());
            document->addNode(innerEnt2, document->parentForNodes());
            document->select(std::vector<Model::Node*> {innerEnt1, innerEnt2});

            Model::GroupNode* inner = document->groupSelection("Inner");

            document->deselectAll();
            document->addNode(outerEnt1, document->parentForNodes());
            document->addNode(outerEnt2, document->parentForNodes());
            document->select(std::vector<Model::Node*> {inner, outerEnt1, outerEnt2});

            Model::GroupNode* outer = document->groupSelection("Outer");
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

            CHECK(document->currentGroup() == nullptr);
            CHECK(!outer->opened());
            CHECK(!inner->opened());

            CHECK(Model::findOutermostClosedGroup(innerEnt1) == outer);
            CHECK(Model::findOutermostClosedGroup(outerEnt1) == outer);

            CHECK(Model::findContainingGroup(innerEnt1) == inner);
            CHECK(Model::findContainingGroup(outerEnt1) == outer);

            // open the outer group and ungroup the inner group
            document->openGroup(outer);
            document->select(inner);
            document->ungroupSelection();
            document->deselectAll();

            ASSERT_EQ(outer, innerEnt1->parent());
            ASSERT_EQ(outer, innerEnt2->parent());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.ungroupLeavesPointEntitySelected") {
            Model::EntityNode* ent1 = new Model::EntityNode();

            document->addNode(ent1, document->parentForNodes());
            document->select(std::vector<Model::Node*> {ent1});

            Model::GroupNode* group = document->groupSelection("Group");
            ASSERT_EQ((std::vector<Model::Node*> {group}), document->selectedNodes().nodes());

            document->ungroupSelection();
            ASSERT_EQ((std::vector<Model::Node*> {ent1}), document->selectedNodes().nodes());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.ungroupLeavesBrushEntitySelected") {
            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            Model::EntityNode* ent1 = new Model::EntityNode();
            document->addNode(ent1, document->parentForNodes());

            Model::BrushNode* brush1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNode(brush1, ent1);
            document->select(std::vector<Model::Node*>{ent1});
            ASSERT_EQ((std::vector<Model::Node*> {brush1}), document->selectedNodes().nodes());
            ASSERT_FALSE(ent1->selected());
            ASSERT_TRUE(brush1->selected());

            Model::GroupNode* group = document->groupSelection("Group");
            ASSERT_EQ((std::vector<Model::Node*> {ent1}), group->children());
            ASSERT_EQ((std::vector<Model::Node*> {brush1}), ent1->children());
            ASSERT_EQ((std::vector<Model::Node*> {group}), document->selectedNodes().nodes());
            CHECK(document->selectedNodes().brushesRecursively() == std::vector<Model::BrushNode*>{brush1});
            CHECK(document->selectedNodes().hasBrushesRecursively());
            CHECK(!document->selectedNodes().hasBrushes());

            document->ungroupSelection();
            ASSERT_EQ((std::vector<Model::Node*> {brush1}), document->selectedNodes().nodes());
            ASSERT_FALSE(ent1->selected());
            ASSERT_TRUE(brush1->selected());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.mergeGroups") {
            document->selectAllNodes();
            document->deleteObjects();

            Model::EntityNode* ent1 = new Model::EntityNode();
            document->addNode(ent1, document->parentForNodes());
            document->deselectAll();
            document->select(std::vector<Model::Node*> {ent1});
            Model::GroupNode* group1 = document->groupSelection("group1");

            Model::EntityNode* ent2 = new Model::EntityNode();
            document->addNode(ent2, document->parentForNodes());
            document->deselectAll();
            document->select(std::vector<Model::Node*> {ent2});
            Model::GroupNode* group2 = document->groupSelection("group2");

            ASSERT_COLLECTIONS_EQUIVALENT(std::vector<Model::Node*>({ group1, group2 }), document->currentLayer()->children());

            document->select(std::vector<Model::Node*> {group1, group2});
            document->mergeSelectedGroupsWithGroup(group2);

            ASSERT_EQ((std::vector<Model::Node*> {group2}), document->selectedNodes().nodes());
            ASSERT_EQ((std::vector<Model::Node*> {group2}), document->currentLayer()->children());

            ASSERT_COLLECTIONS_EQUIVALENT(std::vector<Model::Node*>({}), group1->children());
            ASSERT_COLLECTIONS_EQUIVALENT(std::vector<Model::Node*>({ ent1, ent2 }), group2->children());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pickSingleBrush") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* brushNode1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNode(brushNode1, document->parentForNodes());

            Model::PickResult pickResult;
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.query().all();
            ASSERT_EQ(1u, hits.size());

            const auto& brush1 = brushNode1->brush();
            ASSERT_EQ(brush1.face(*brush1.findFace(vm::vec3::neg_x())), Model::hitToFaceHandle(hits.front())->face());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::neg_x()), pickResult);
            ASSERT_TRUE(pickResult.query().all().empty());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pickSingleEntity") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::EntityNode* ent1 = new Model::EntityNode();
            document->addNode(ent1, document->parentForNodes());

            const auto origin = ent1->entity().origin();
            const auto bounds = ent1->logicalBounds();

            const auto rayOrigin = origin + vm::vec3(-32.0, bounds.size().y() / 2.0, bounds.size().z() / 2.0);

            Model::PickResult pickResult;
            document->pick(vm::ray3(rayOrigin, vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.query().all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(ent1, hits.front().target<Model::EntityNode*>());
            ASSERT_DOUBLE_EQ(32.0 - bounds.size().x() / 2.0, hits.front().distance());

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::neg_x()), pickResult);
            ASSERT_TRUE(pickResult.query().all().empty());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pickSimpleGroup") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* brushNode1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNode(brushNode1, document->parentForNodes());

            auto* brushNode2 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture").value());
            document->addNode(brushNode2, document->parentForNodes());

            document->selectAllNodes();
            auto* group = document->groupSelection("test");

            Model::PickResult pickResult;
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            // picking a grouped object when the containing group is closed should return the object,
            // which is converted to the group when hitsToNodesWithGroupPicking() is used.
            auto hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            ASSERT_EQ(1u, hits.size());

            const auto& brush1 = brushNode1->brush();
            ASSERT_EQ(brush1.face(*brush1.findFace(vm::vec3::neg_x())), Model::hitToFaceHandle(hits.front())->face());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            ASSERT_EQ(std::vector<Model::Node*>{ group }, hitsToNodesWithGroupPicking(hits));

            // hitting both objects in the group should return the group only once
            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(32, 32, -32), vm::vec3::pos_z()), pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            ASSERT_EQ(2u, hits.size());

            ASSERT_EQ(std::vector<Model::Node*>{ group }, hitsToNodesWithGroupPicking(hits));

            // hitting the group bounds doesn't count as a hit
            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 96), vm::vec3::pos_x()), pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            ASSERT_TRUE(hits.empty());

            // hitting a grouped object when the containing group is open should return the object only
            document->openGroup(group);

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1.face(*brush1.findFace(vm::vec3::neg_x())), Model::hitToFaceHandle(hits.front())->face());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            ASSERT_EQ(std::vector<Model::Node*>{ brushNode1 }, hitsToNodesWithGroupPicking(hits));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pickNestedGroup") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* brushNode1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNode(brushNode1, document->parentForNodes());

            auto* brushNode2 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture").value());
            document->addNode(brushNode2, document->parentForNodes());

            document->selectAllNodes();
            auto* innerGroup = document->groupSelection("inner");

            document->deselectAll();
            auto* brushNode3 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 256)), "texture").value());
            document->addNode(brushNode3, document->parentForNodes());

            document->selectAllNodes();
            auto* outerGroup = document->groupSelection("outer");

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
            document->openGroup(outerGroup);

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

            auto hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            ASSERT_EQ(1u, hits.size());

            const auto& brush3 = brushNode3->brush();
            ASSERT_EQ(brush3.face(*brush3.findFace(vm::vec3::neg_x())), Model::hitToFaceHandle(hits.front())->face());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());

            ASSERT_EQ(std::vector<Model::Node*>{ brushNode3 }, hitsToNodesWithGroupPicking(hits));

            // hitting the brush in the inner group should return the inner group when hitsToNodesWithGroupPicking() is used
            pickResult.clear();
            document->pick(lowRay, pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            ASSERT_EQ(1u, hits.size());

            const auto& brush1 = brushNode1->brush();
            ASSERT_EQ(brush1.face(*brush1.findFace(vm::vec3::neg_x())), Model::hitToFaceHandle(hits.front())->face());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());
            ASSERT_EQ(std::vector<Model::Node*>{ innerGroup }, hitsToNodesWithGroupPicking(hits));

            // open the inner group, too. hitsToNodesWithGroupPicking() should no longer return groups, since all groups are open.
            document->openGroup(innerGroup);

            /*
             * world
             * * outer (open)
             *   * inner (open)
             *     * brush1
             *     * brush2
             *   * brush3
             */

            ASSERT_TRUE(innerGroup->opened());
            ASSERT_FALSE(outerGroup->opened());
            ASSERT_TRUE(outerGroup->hasOpenedDescendant());

            // pick a brush in the outer group
            pickResult.clear();
            document->pick(highRay, pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush3.face(*brush3.findFace(vm::vec3::neg_x())), Model::hitToFaceHandle(hits.front())->face());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());
            ASSERT_EQ(std::vector<Model::Node*>{ brushNode3 }, hitsToNodesWithGroupPicking(hits));

            // pick a brush in the inner group
            pickResult.clear();
            document->pick(lowRay, pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            ASSERT_EQ(1u, hits.size());

            ASSERT_EQ(brush1.face(*brush1.findFace(vm::vec3::neg_x())), Model::hitToFaceHandle(hits.front())->face());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());
            ASSERT_EQ(std::vector<Model::Node*>{ brushNode1 }, hitsToNodesWithGroupPicking(hits));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pickBrushEntity") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());

            auto* brushNode1 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNode(brushNode1, document->parentForNodes());

            auto* brushNode2 = document->world()->createBrush(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture").value());
            document->addNode(brushNode2, document->parentForNodes());

            document->selectAllNodes();

            document->createBrushEntity(m_brushEntityDef);
            document->deselectAll();

            Model::PickResult pickResult;

            // picking entity brushes should only return the brushes and not the entity
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.query().all();
            ASSERT_EQ(1u, hits.size());

            const auto& brush1 = brushNode1->brush();
            ASSERT_EQ(brush1.face(*brush1.findFace(vm::vec3::neg_x())), Model::hitToFaceHandle(hits.front())->face());
            ASSERT_DOUBLE_EQ(32.0, hits.front().distance());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.throwExceptionDuringCommand") {
            ASSERT_THROW(document->throwExceptionDuringCommand(), CommandProcessorException);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.selectTouching") {
            Model::BrushBuilder builder(document->world(), document->worldBounds());
            Model::BrushNode* brush1 = document->world()->createBrush(builder.createCube(64.0, "none").value());
            Model::BrushNode* brush2 = document->world()->createBrush(builder.createCube(64.0, "none").value());
            Model::BrushNode* brush3 = document->world()->createBrush(builder.createCube(64.0, "none").value());

            REQUIRE(brush2->transform(document->worldBounds(), vm::translation_matrix(vm::vec3(10.0, 0.0, 0.0)), false));
            REQUIRE(brush3->transform(document->worldBounds(), vm::translation_matrix(vm::vec3(100.0, 0.0, 0.0)), false));

            document->addNode(brush1, document->parentForNodes());
            document->addNode(brush2, document->parentForNodes());
            document->addNode(brush3, document->parentForNodes());

            REQUIRE(brush1->intersects(brush2));
            REQUIRE(brush2->intersects(brush1));

            REQUIRE(!brush1->intersects(brush3));
            REQUIRE(!brush3->intersects(brush1));

            document->select(brush1);
            document->selectTouching(false);

            using Catch::Matchers::UnorderedEquals;
            CHECK_THAT(document->selectedNodes().brushes(), UnorderedEquals(std::vector<Model::BrushNode*>{brush2}));
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/2476
        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.selectTouching_2476") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto *brush1 = document->world()->createBrush(builder.createCuboid(box, "texture").value());
            document->addNode(brush1, document->parentForNodes());

            auto *brush2 = document->world()->createBrush(builder.createCuboid(box.translate(vm::vec3(1, 1, 1)), "texture").value());
            document->addNode(brush2, document->parentForNodes());

            document->selectAllNodes();

            EXPECT_EQ((std::vector<Model::BrushNode* >{brush1, brush2}), document->selectedNodes().brushes());
            EXPECT_EQ((std::vector<Model::Node *>{brush1, brush2}), document->currentLayer()->children());

            document->selectTouching(true);

            // only this next line was failing
            EXPECT_EQ(std::vector<Model::BrushNode* >{}, document->selectedNodes().brushes());
            EXPECT_EQ(std::vector<Model::Node *>{}, document->currentLayer()->children());

            // brush1 and brush2 are deleted
            EXPECT_EQ(nullptr, brush1->parent());
            EXPECT_EQ(nullptr, brush2->parent());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.selectInverse") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto *brush1 = document->world()->createBrush(builder.createCuboid(box, "texture").value());
            document->addNode(brush1, document->parentForNodes());

            auto *brush2 = document->world()->createBrush(builder.createCuboid(box.translate(vm::vec3(1, 1, 1)), "texture").value());
            document->addNode(brush2, document->parentForNodes());

            auto *brush3 = document->world()->createBrush(builder.createCuboid(box.translate(vm::vec3(2, 2, 2)), "texture").value());
            document->addNode(brush3, document->parentForNodes());

            document->select(std::vector<Model::Node *>{brush1, brush2});
            Model::EntityNode* brushEnt = document->createBrushEntity(m_brushEntityDef);

            document->deselectAll();

            // worldspawn {
            //   brushEnt { brush1, brush2 },
            //   brush3
            // }

            document->select(brush1);
            CHECK( brush1->selected());
            CHECK(!brush2->selected());
            CHECK(!brush3->selected());
            CHECK(!brushEnt->selected());

            document->selectInverse();

            CHECK_THAT(document->selectedNodes().brushes(), Catch::UnorderedEquals(std::vector<Model::BrushNode *>{brush2, brush3}));
            CHECK(!brush1->selected());
            CHECK( brush2->selected());
            CHECK( brush3->selected());
            CHECK(!brushEnt->selected());
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/2776
        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pasteAndTranslateGroup") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto *brush1 = document->world()->createBrush(builder.createCuboid(box, "texture").value());
            document->addNode(brush1, document->parentForNodes());
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

        // https://github.com/TrenchBroom/TrenchBroom/issues/3117
        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.isolate") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto *brush1 = document->world()->createBrush(builder.createCuboid(box, "texture").value());
            document->addNode(brush1, document->parentForNodes());

            auto *brush2 = document->world()->createBrush(builder.createCuboid(box.translate(vm::vec3(1, 1, 1)), "texture").value());
            document->addNode(brush2, document->parentForNodes());

            document->selectAllNodes();

            Model::EntityNode* brushEntity = document->createBrushEntity(m_brushEntityDef);

            document->deselectAll();

            // Check initial state
            REQUIRE(1 == document->currentLayer()->childCount());
            REQUIRE(brushEntity == dynamic_cast<Model::EntityNode*>(document->currentLayer()->children().at(0)));
            REQUIRE(2 == brushEntity->childCount());
            REQUIRE(brush1 == dynamic_cast<Model::BrushNode*>(brushEntity->children().at(0)));
            REQUIRE(brush2 == dynamic_cast<Model::BrushNode*>(brushEntity->children().at(1)));

            CHECK(!brushEntity->selected());
            CHECK(!brush1->selected());
            CHECK(!brush2->selected());
            CHECK(!brushEntity->hidden());
            CHECK(!brush1->hidden());
            CHECK(!brush2->hidden());

            // Select just brush1
            document->select(brush1);
            CHECK(!brushEntity->selected());
            CHECK(brush1->selected());
            CHECK(!brush2->selected());

            // Isolate brush1
            document->isolate();

            CHECK(!brushEntity->hidden());
            CHECK(!brush1->hidden());
            CHECK(brush2->hidden());
        }

        TEST_CASE_METHOD(MapDocumentTest, "IssueGenerator.emptyAttribute") {
            Model::EntityNode* entityNode = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            
            document->deselectAll();
            document->select(entityNode);
            document->setAttribute("", "");
            REQUIRE(entityNode->entity().hasAttribute(""));

            auto issueGenerators = std::vector<Model::IssueGenerator*>{
                new Model::EmptyAttributeNameIssueGenerator(),
                new Model::EmptyAttributeValueIssueGenerator()
            };

            class AcceptAllIssues {
            public:
                bool operator()(const Model::Issue*) const {
                    return true;
                }
            };

            auto issues = std::vector<Model::Issue*>{};
            document->world()->accept(kdl::overload(
                [&](auto&& thisLambda, Model::WorldNode* w)  { issues = kdl::vec_concat(std::move(issues), w->issues(issueGenerators)); w->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::LayerNode* l)  { issues = kdl::vec_concat(std::move(issues), l->issues(issueGenerators)); l->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::GroupNode* g)  { issues = kdl::vec_concat(std::move(issues), g->issues(issueGenerators)); g->visitChildren(thisLambda); },
                [&](auto&& thisLambda, Model::EntityNode* e) { issues = kdl::vec_concat(std::move(issues), e->issues(issueGenerators)); e->visitChildren(thisLambda); },
                [&](Model::BrushNode* b)                     { issues = kdl::vec_concat(std::move(issues), b->issues(issueGenerators)); }
            ));

            REQUIRE(2 == issues.size());

            Model::Issue* issue0 = issues.at(0);
            Model::Issue* issue1 = issues.at(1);

            // Should be one EmptyAttributeNameIssue and one EmptyAttributeValueIssue
            CHECK(((issue0->type() == issueGenerators[0]->type() && issue1->type() == issueGenerators[1]->type())
                || (issue0->type() == issueGenerators[1]->type() && issue1->type() == issueGenerators[0]->type())));
            
            std::vector<Model::IssueQuickFix*> fixes = document->world()->quickFixes(issue0->type());
            REQUIRE(1 == fixes.size());

            Model::IssueQuickFix* quickFix = fixes.at(0);
            quickFix->apply(document.get(), std::vector<Model::Issue*>{issue0});

            // The fix should have deleted the attribute
            CHECK(!entityNode->entity().hasAttribute(""));

            kdl::vec_clear_and_delete(issueGenerators);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.defaultLayerSortIndexImmutable", "[LayerTest]") {
            Model::LayerNode* defaultLayer = document->world()->defaultLayer();

            defaultLayer->setSortIndex(555);
            CHECK(defaultLayer->sortIndex() == Model::LayerNode::defaultLayerSortIndex());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.renameLayer", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::LayerNode* layer = document->world()->createLayer("test1");
            document->addNode(layer, document->world());
            CHECK(layer->name() == "test1");

            document->renameLayer(layer, "test2");
            CHECK(layer->name() == "test2");

            document->undoCommand();
            CHECK(layer->name() == "test1");
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.duplicateObjectGoesIntoSourceLayer", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::LayerNode* layer1 = document->world()->createLayer("test1");
            Model::LayerNode* layer2 = document->world()->createLayer("test2");
            document->addNode(layer1, document->world());
            document->addNode(layer2, document->world());

            document->setCurrentLayer(layer1);
            Model::EntityNode* entity = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity->parent() == layer1);
            CHECK(layer1->childCount() == 1);

            document->setCurrentLayer(layer2);
            document->select(entity);
            document->duplicateObjects(); // the duplicate should stay in layer1

            REQUIRE(document->selectedNodes().entityCount() == 1);
            Model::EntityNode* entityClone = document->selectedNodes().entities().at(0);
            CHECK(entityClone->parent() == layer1);
            CHECK(layer1->childCount() == 2);
            CHECK(document->currentLayer() == layer2);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.newGroupGoesIntoSourceLayer", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::LayerNode* layer1 = document->world()->createLayer("test1");
            Model::LayerNode* layer2 = document->world()->createLayer("test2");
            document->addNode(layer1, document->world());
            document->addNode(layer2, document->world());

            document->setCurrentLayer(layer1);
            Model::EntityNode* entity = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity->parent() == layer1);
            CHECK(layer1->childCount() == 1);

            document->setCurrentLayer(layer2);
            document->select(entity);
            Model::GroupNode* newGroup = document->groupSelection("Group in Layer 1"); // the new group should stay in layer1

            CHECK(entity->parent() == newGroup);
            CHECK(Model::findContainingLayer(entity) == layer1);
            CHECK(Model::findContainingLayer(newGroup) == layer1);
            CHECK(document->currentLayer() == layer2);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.newObjectsInHiddenLayerAreVisible", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::LayerNode* layer1 = document->world()->createLayer("test1");
            Model::LayerNode* layer2 = document->world()->createLayer("test2");
            document->addNode(layer1, document->world());
            document->addNode(layer2, document->world());

            document->setCurrentLayer(layer1);

            // Create an entity in layer1
            Model::EntityNode* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity1->parent() == layer1);
            CHECK(layer1->childCount() == 1u);

            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(entity1->visible());

            // Hide layer1. If any nodes in the layer were Visibility_Shown they would be reset to Visibility_Inherited
            document->hideLayers({layer1}); 

            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity1->visible());

            // Create another entity in layer1. It will be visible, while entity1 will still be hidden.
            Model::EntityNode* entity2 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity2->parent() == layer1);
            CHECK(layer1->childCount() == 2u);

            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity1->visible());
            CHECK(entity2->visibilityState() == Model::VisibilityState::Visibility_Shown);
            CHECK(entity2->visible());

            // Change to layer2. This hides all objects in layer1
            document->setCurrentLayer(layer2);

            CHECK(document->currentLayer() == layer2);
            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity1->visible());
            CHECK(entity2->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity2->visible());

            // Undo (Switch current layer back to layer1)
            document->undoCommand();

            CHECK(document->currentLayer() == layer1);
            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity1->visible());
            CHECK(entity2->visibilityState() == Model::VisibilityState::Visibility_Shown);
            CHECK(entity2->visible());

            // Undo (entity2 creation)
            document->undoCommand();

            CHECK(layer1->childCount() == 1u);
            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity1->visible());

            // Undo (hiding layer1)
            document->undoCommand();

            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(entity1->visible());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.duplicatedObjectInHiddenLayerIsVisible", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::LayerNode* layer1 = document->world()->createLayer("test1");
            document->addNode(layer1, document->world());

            document->setCurrentLayer(layer1);
            document->hideLayers({layer1});

            // Create entity1 and brush1 in the hidden layer1
            Model::EntityNode* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            Model::BrushNode* brush1 = createBrushNode();
            document->addNode(brush1, document->parentForNodes());

            CHECK(entity1->parent() == layer1);
            CHECK(brush1->parent() == layer1);
            CHECK(layer1->childCount() == 2u);

            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Shown);
            CHECK(brush1->visibilityState() == Model::VisibilityState::Visibility_Shown);
            CHECK(entity1->visible());
            CHECK(brush1->visible());

            document->select({entity1, brush1});

            // Duplicate entity1 and brush1
            CHECK(document->duplicateObjects());
            REQUIRE(document->selectedNodes().entityCount() == 1u);
            REQUIRE(document->selectedNodes().brushCount() == 1u);
            Model::EntityNode* entity2 = document->selectedNodes().entities().front();
            Model::BrushNode* brush2 =  document->selectedNodes().brushes().front();

            CHECK(entity2 != entity1);
            CHECK(brush2 != brush1);

            CHECK(entity2->visibilityState() == Model::VisibilityState::Visibility_Shown);
            CHECK(entity2->visible());

            CHECK(brush2->visibilityState() == Model::VisibilityState::Visibility_Shown);
            CHECK(brush2->visible());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.newObjectsInLockedLayerAreUnlocked", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            auto* layer1 = document->world()->createLayer("test1");
            auto* layer2 = document->world()->createLayer("test2");
            document->addNode(layer1, document->world());
            document->addNode(layer2, document->world());

            document->setCurrentLayer(layer1);

            // Create an entity in layer1
            auto* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity1->parent() == layer1);
            CHECK(layer1->childCount() == 1u);

            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(!entity1->locked());

            // Lock layer1
            document->lock({layer1}); 

            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity1->locked());

            // Create another entity in layer1. It will be unlocked, while entity1 will still be locked (inherited).
            auto* entity2 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity2->parent() == layer1);
            CHECK(layer1->childCount() == 2u);

            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity1->locked());
            CHECK(entity2->lockState() == Model::LockState::Lock_Unlocked);
            CHECK(!entity2->locked());

            // Change to layer2. This causes the Lock_Unlocked objects in layer1 to be degraded to Lock_Inherited
            // (i.e. everything in layer1 becomes locked)
            document->setCurrentLayer(layer2);

            CHECK(document->currentLayer() == layer2);
            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity1->locked());
            CHECK(entity2->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity2->locked());

            // Undo (Switch current layer back to layer1)
            document->undoCommand();

            CHECK(document->currentLayer() == layer1);
            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity1->locked());
            CHECK(entity2->lockState() == Model::LockState::Lock_Unlocked);
            CHECK(!entity2->locked());

            // Undo entity2 creation
            document->undoCommand();

            CHECK(layer1->childCount() == 1u);
            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity1->locked());

            // Undo locking layer1
            document->undoCommand();

            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(!entity1->locked());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.moveLayer", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            auto* layer0 = document->world()->createLayer("layer0");
            auto* layer1 = document->world()->createLayer("layer1");
            auto* layer2 = document->world()->createLayer("laeyr2");

            document->addNode(layer0, document->world());
            document->addNode(layer1, document->world());
            document->addNode(layer2, document->world());

            layer0->setSortIndex(0);
            layer1->setSortIndex(1);
            layer2->setSortIndex(2);

            SECTION("check canMoveLayer") {
                // defaultLayer() can never be moved
                CHECK(!document->canMoveLayer(document->world()->defaultLayer(), 1));
                CHECK( document->canMoveLayer(layer0,  0));
                CHECK(!document->canMoveLayer(layer0, -1));
                CHECK( document->canMoveLayer(layer0,  1));
                CHECK( document->canMoveLayer(layer0,  2));
                CHECK(!document->canMoveLayer(layer0,  3));
            }

            SECTION("moveLayer by 0 has no effect") {
                document->moveLayer(layer0, 0);
                CHECK(layer0->sortIndex() == 0);
            }
            SECTION("moveLayer by invalid negative amount is clamped") {
                document->moveLayer(layer0, -1000);
                CHECK(layer0->sortIndex() == 0);
            }
            SECTION("moveLayer by 1") {
                document->moveLayer(layer0, 1);
                CHECK(layer1->sortIndex() == 0);
                CHECK(layer0->sortIndex() == 1);
                CHECK(layer2->sortIndex() == 2);
            }
            SECTION("moveLayer by 2") {
                document->moveLayer(layer0, 2);
                CHECK(layer1->sortIndex() == 0);
                CHECK(layer2->sortIndex() == 1);
                CHECK(layer0->sortIndex() == 2);
            }
            SECTION("moveLayer by invalid positive amount is clamped") {
                document->moveLayer(layer0, 1000);
                CHECK(layer1->sortIndex() == 0);
                CHECK(layer2->sortIndex() == 1);
                CHECK(layer0->sortIndex() == 2);
            }
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.setCurrentLayerCollation", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            auto* defaultLayer = document->world()->defaultLayer();
            auto* layer1 = document->world()->createLayer("test1");
            auto* layer2 = document->world()->createLayer("test2");
            document->addNode(layer1, document->world());
            document->addNode(layer2, document->world());
            CHECK(document->currentLayer() == defaultLayer);

            document->setCurrentLayer(layer1);
            document->setCurrentLayer(layer2);
            CHECK(document->currentLayer() == layer2);

            // No collation currently because of the transactions in setCurrentLayer()
            document->undoCommand();
            CHECK(document->currentLayer() == layer1);
            document->undoCommand();
            CHECK(document->currentLayer() == defaultLayer);

            document->redoCommand();
            CHECK(document->currentLayer() == layer1);
            document->redoCommand();
            CHECK(document->currentLayer() == layer2);
        }
    }
}
