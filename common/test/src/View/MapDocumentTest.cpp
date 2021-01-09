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
#include "Model/EmptyPropertyKeyIssueGenerator.h"
#include "Model/EmptyPropertyValueIssueGenerator.h"
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

#include <vecmath/approx.h>
#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/ray.h>
#include <vecmath/ray_io.h>
#include <vecmath/scalar.h>

#include "Catch2.h"
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
            Model::BrushBuilder builder(world->mapFormat(), document->worldBounds(), document->game()->defaultFaceAttribs());
            Model::Brush brush = builder.createCube(32.0, textureName).value();
            brushFunc(brush);
            return new Model::BrushNode(std::move(brush));
        }

        static void checkPlanePointsIntegral(const Model::BrushNode* brushNode) {
            for (const Model::BrushFace& face : brushNode->brush().faces()) {
                for (size_t i=0; i<3; i++) {
                    vm::vec3 point = face.points()[i];
                    CHECK(pointExactlyIntegral(point));
                }
            }
        }

        static void checkVerticesIntegral(const Model::BrushNode* brushNode) {
            const Model::Brush& brush = brushNode->brush();
            for (const Model::BrushVertex* vertex : brush.vertices()) {
                CHECK(pointExactlyIntegral(vertex->position()));
            }
        }

        static void checkBoundsIntegral(const Model::BrushNode* brush) {
            CHECK(pointExactlyIntegral(brush->logicalBounds().min));
            CHECK(pointExactlyIntegral(brush->logicalBounds().max));
        }

        static void checkBrushIntegral(const Model::BrushNode* brush) {
            checkPlanePointsIntegral(brush);
            checkVerticesIntegral(brush);
            checkBoundsIntegral(brush);
        }

        static void setLayerSortIndex(Model::LayerNode& layerNode, int sortIndex) {
            auto layer = layerNode.layer();
            layer.setSortIndex(sortIndex);
            layerNode.setLayer(layer);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.flip") {
            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0.0, 0.0, 0.0), vm::vec3(30.0, 31.0, 31.0)), "texture").value());
            Model::BrushNode* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(30.0, 0.0, 0.0), vm::vec3(31.0, 31.0, 31.0)), "texture").value());

            checkBrushIntegral(brushNode1);
            checkBrushIntegral(brushNode2);

            document->addNodes({{document->parentForNodes(), {brushNode1}}});
            document->addNodes({{document->parentForNodes(), {brushNode2}}});

            std::vector<Model::Node*> brushes;
            brushes.push_back(brushNode1);
            brushes.push_back(brushNode2);
            document->select(brushes);

            const vm::vec3 boundsCenter = document->selectionBounds().center();
            CHECK(boundsCenter == vm::approx(vm::vec3(15.5, 15.5, 15.5)));

            document->flipObjects(boundsCenter, vm::axis::x);

            checkBrushIntegral(brushNode1);
            checkBrushIntegral(brushNode2);

            CHECK(brushNode1->logicalBounds() == vm::bbox3(vm::vec3(1.0, 0.0, 0.0), vm::vec3(31.0, 31.0, 31.0)));
            CHECK(brushNode2->logicalBounds() == vm::bbox3(vm::vec3(0.0, 0.0, 0.0), vm::vec3(1.0, 31.0, 31.0)));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.rotate") {
            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0.0, 0.0, 0.0), vm::vec3(30.0, 31.0, 31.0)), "texture").value());
            Model::BrushNode* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(30.0, 0.0, 0.0), vm::vec3(31.0, 31.0, 31.0)), "texture").value());

            checkBrushIntegral(brushNode1);
            checkBrushIntegral(brushNode2);

            document->addNodes({{document->parentForNodes(), {brushNode1}}});
            document->addNodes({{document->parentForNodes(), {brushNode2}}});

            std::vector<Model::Node*> brushes;
            brushes.push_back(brushNode1);
            brushes.push_back(brushNode2);
            document->select(brushes);

            vm::vec3 boundsCenter = document->selectionBounds().center();
            CHECK(boundsCenter == vm::vec3(15.5, 15.5, 15.5));

            // 90 degrees CCW about the Z axis through the center of the selection
            document->rotateObjects(boundsCenter, vm::vec3::pos_z(), vm::to_radians(90.0));

            checkBrushIntegral(brushNode1);
            checkBrushIntegral(brushNode2);

            const vm::bbox3 brush1ExpectedBounds(vm::vec3(0.0, 0.0, 0.0), vm::vec3(31.0, 30.0, 31.0));
            const vm::bbox3 brush2ExpectedBounds(vm::vec3(0.0, 30.0, 0.0), vm::vec3(31.0, 31.0, 31.0));

            // these should be exactly integral
            CHECK(brushNode1->logicalBounds() == brush1ExpectedBounds);
            CHECK(brushNode2->logicalBounds() == brush2ExpectedBounds);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.shearCube") {
            const vm::bbox3 initialBBox(vm::vec3(100,100,100), vm::vec3(200,200,200));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCuboid(initialBBox, "texture").value());

            document->addNodes({{document->parentForNodes(), {brushNode}}});
            document->select(std::vector<Model::Node*>{brushNode});

            CHECK_THAT(brushNode->brush().vertexPositions(), Catch::UnorderedEquals(std::vector<vm::vec3>{
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
            }));

            // Shear the -Y face by (50, 0, 0). That means the verts with Y=100 will get sheared.
            CHECK(document->shearObjects(initialBBox, vm::vec3::neg_y(), vm::vec3(50,0,0)));

            CHECK_THAT(brushNode->brush().vertexPositions(), Catch::UnorderedEquals(std::vector<vm::vec3>{
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
            }));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.shearPillar") {
            const vm::bbox3 initialBBox(vm::vec3(0,0,0), vm::vec3(100,100,400));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCuboid(initialBBox, "texture").value());

            document->addNodes({{document->parentForNodes(), {brushNode}}});
            document->select(std::vector<Model::Node*>{brushNode});

            CHECK_THAT(brushNode->brush().vertexPositions(), Catch::UnorderedEquals(std::vector<vm::vec3>{
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
            }));

            // Shear the +Z face by (50, 0, 0). That means the verts with Z=400 will get sheared.
            CHECK(document->shearObjects(initialBBox, vm::vec3::pos_z(), vm::vec3(50,0,0)));

            CHECK_THAT(brushNode->brush().vertexPositions(), Catch::UnorderedEquals(std::vector<vm::vec3>{
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
            }));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.scaleObjects") {
            const vm::bbox3 initialBBox(vm::vec3(-100,-100,-100), vm::vec3(100,100,100));
            const vm::bbox3 doubleBBox(2.0 * initialBBox.min, 2.0 * initialBBox.max);
            const vm::bbox3 invalidBBox(vm::vec3(0,-100,-100), vm::vec3(0,100,100));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCuboid(initialBBox, "texture").value());
            const Model::Brush& brush = brushNode->brush();

            document->addNodes({{document->parentForNodes(), {brushNode}}});
            document->select(std::vector<Model::Node*>{brushNode});

            CHECK(brushNode->logicalBounds().size() == vm::vec3(200,200,200));
            CHECK(brush.face(*brush.findFace(vm::vec3::pos_z())).boundary() == vm::plane3(100.0, vm::vec3::pos_z()));

            // attempting an invalid scale has no effect
            CHECK_FALSE(document->scaleObjects(initialBBox, invalidBBox));
            CHECK(brushNode->logicalBounds().size() == vm::vec3(200,200,200));
            CHECK(brush.face(*brush.findFace(vm::vec3::pos_z())).boundary() == vm::plane3(100.0, vm::vec3::pos_z()));

            CHECK(document->scaleObjects(initialBBox, doubleBBox));
            CHECK(brushNode->logicalBounds().size() == vm::vec3(400,400,400));
            CHECK(brush.face(*brush.findFace(vm::vec3::pos_z())).boundary() == vm::plane3(200.0, vm::vec3::pos_z()));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.scaleObjectsInGroup") {
            const vm::bbox3 initialBBox(vm::vec3(-100, -100, -100), vm::vec3(100, 100, 100));
            const vm::bbox3 doubleBBox(2.0 * initialBBox.min, 2.0 * initialBBox.max);
            const vm::bbox3 invalidBBox(vm::vec3(0, -100, -100), vm::vec3(0, 100, 100));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCuboid(initialBBox, "texture").value());

            document->addNodes({{document->parentForNodes(), {brushNode}}});
            document->select(std::vector<Model::Node*>{ brushNode });
            [[maybe_unused]] Model::GroupNode* group = document->groupSelection("my group");

            // attempting an invalid scale has no effect
            CHECK_FALSE(document->scaleObjects(initialBBox, invalidBBox));
            CHECK(brushNode->logicalBounds().size() == vm::vec3(200, 200, 200));

            CHECK(document->scaleObjects(initialBBox, doubleBBox));
            CHECK(brushNode->logicalBounds().size() == vm::vec3(400, 400, 400));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.scaleObjectsWithCenter") {
            const vm::bbox3 initialBBox(vm::vec3(0,0,0), vm::vec3(100,100,400));
            const vm::bbox3 expectedBBox(vm::vec3(-50,0,0), vm::vec3(150,100,400));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCuboid(initialBBox, "texture").value());

            document->addNodes({{document->parentForNodes(), {brushNode}}});
            document->select(std::vector<Model::Node*>{brushNode});

            const vm::vec3 boundsCenter = initialBBox.center();
            CHECK(document->scaleObjects(boundsCenter, vm::vec3(2.0, 1.0, 1.0)));
            CHECK(brushNode->logicalBounds() == expectedBBox);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.csgConvexMergeBrushes") {
            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* entity = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entity}}});

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture").value());
            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNodes({{entity, {brushNode1}}});
            document->addNodes({{document->parentForNodes(), {brushNode2}}});
            CHECK(entity->children().size() == 1u);

            document->select(std::vector<Model::Node*> { brushNode1, brushNode2 });
            CHECK(document->csgConvexMerge());
            CHECK(entity->children().size() == 1u); // added to the parent of the first brush

            auto* brush3 = entity->children().front();
            CHECK(brush3->logicalBounds() == vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.csgConvexMergeFaces") {
            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* entity = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entity}}});

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture").value());
            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNodes({{entity, {brushNode1}}});
            document->addNodes({{document->parentForNodes(), {brushNode2}}});
            CHECK(entity->children().size() == 1u);

            const auto faceIndex = 0u;
            const auto& face1 = brushNode1->brush().face(faceIndex);
            const auto& face2 = brushNode2->brush().face(faceIndex);

            document->select({
                { brushNode1, faceIndex },
                { brushNode2, faceIndex }
            });
            CHECK(document->csgConvexMerge());
            CHECK(entity->children().size() == 2u); // added to the parent of the first brush, original brush is not deleted

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

            CHECK(brush3->logicalBounds() == bounds);
        }

        ValveMapDocumentTest::ValveMapDocumentTest() :
        MapDocumentTest(Model::MapFormat::Valve) {}

        TEST_CASE_METHOD(ValveMapDocumentTest, "ValveMapDocumentTest.csgConvexMergeTexturing") {
            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entity}}});

            Model::ParallelTexCoordSystem texAlignment(vm::vec3(1, 0, 0), vm::vec3(0, 1, 0));
            auto texAlignmentSnapshot = texAlignment.takeSnapshot();

            Model::Brush brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 64, 64)), "texture").value();
            brush1.face(*brush1.findFace(vm::vec3::pos_z())).restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);

            Model::Brush brush2 = builder.createCuboid(vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 64, 64)), "texture").value();
            brush2.face(*brush2.findFace(vm::vec3::pos_z())).restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);

            Model::BrushNode* brushNode1 = new Model::BrushNode(std::move(brush1));
            Model::BrushNode* brushNode2 = new Model::BrushNode(std::move(brush2));
            
            document->addNodes({{entity, {brushNode1}}});
            document->addNodes({{entity, {brushNode2}}});
            CHECK(entity->children().size() == 2u);

            document->select(std::vector<Model::Node*> { brushNode1, brushNode2 });
            CHECK(document->csgConvexMerge());
            CHECK(entity->children().size() == 1u);

            Model::BrushNode* brushNode3 = static_cast<Model::BrushNode*>(entity->children()[0]);
            const Model::Brush& brush3 = brushNode3->brush();
            
            const Model::BrushFace& top = brush3.face(*brush3.findFace(vm::vec3::pos_z()));
            CHECK(top.textureXAxis() == vm::vec3(1, 0, 0));
            CHECK(top.textureYAxis() == vm::vec3(0, 1, 0));
        }

        TEST_CASE_METHOD(ValveMapDocumentTest, "ValveMapDocumentTest.csgSubtractTexturing") {
            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entity}}});

            Model::ParallelTexCoordSystem texAlignment(vm::vec3(1, 0, 0), vm::vec3(0, 1, 0));
            auto texAlignmentSnapshot = texAlignment.takeSnapshot();

            Model::Brush brush1 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value();
            Model::Brush brush2 = builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 32)), "texture").value();
            brush2.face(*brush2.findFace(vm::vec3::pos_z())).restoreTexCoordSystemSnapshot(*texAlignmentSnapshot);

            Model::BrushNode* brushNode1 = new Model::BrushNode(std::move(brush1));
            Model::BrushNode* brushNode2 = new Model::BrushNode(std::move(brush2));
            
            document->addNodes({{entity, {brushNode1}}});
            document->addNodes({{entity, {brushNode2}}});
            CHECK(entity->children().size() == 2u);

            // we want to compute brush1 - brush2
            document->select(std::vector<Model::Node*> { brushNode2 });
            CHECK(document->csgSubtract());
            CHECK(entity->children().size() == 1u);

            Model::BrushNode* brushNode3 = static_cast<Model::BrushNode*>(entity->children()[0]);
            const Model::Brush& brush3 = brushNode3->brush();

            CHECK(brushNode3->logicalBounds() == vm::bbox3(vm::vec3(0, 0, 32), vm::vec3(64, 64, 64)));

            // the texture alignment from the top of brush2 should have transferred
            // to the bottom face of brush3
            const Model::BrushFace& top = brush3.face(*brush3.findFace(vm::vec3::neg_z()));
            CHECK(top.textureXAxis() == vm::vec3(1, 0, 0));
            CHECK(top.textureYAxis() == vm::vec3(0, 1, 0));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.csgSubtractMultipleBrushes") {
            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* entity = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entity}}});

            Model::BrushNode* minuend = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            Model::BrushNode* subtrahend1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(32, 32, 64)), "texture").value());
            Model::BrushNode* subtrahend2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(32, 32, 0), vm::vec3(64, 64, 64)), "texture").value());

            document->addNodes({{entity, {minuend, subtrahend1, subtrahend2}}});
            CHECK(entity->children().size() == 3u);

            // we want to compute minuend - {subtrahend1, subtrahend2}
            document->select(std::vector<Model::Node*>{subtrahend1, subtrahend2});
            CHECK(document->csgSubtract());
            CHECK(entity->children().size() == 2u);

            auto* remainder1 = dynamic_cast<Model::BrushNode*>(entity->children()[0]);
            auto* remainder2 = dynamic_cast<Model::BrushNode*>(entity->children()[1]);
            CHECK(remainder1 != nullptr);
            CHECK(remainder2 != nullptr);

            const auto expectedBBox1 = vm::bbox3(vm::vec3(0, 32, 0), vm::vec3(32, 64, 64));
            const auto expectedBBox2 = vm::bbox3(vm::vec3(32, 0, 0), vm::vec3(64, 32, 64));

            if (remainder1->logicalBounds() != expectedBBox1) {
                std::swap(remainder1, remainder2);
            }

            CHECK(remainder1->logicalBounds() == expectedBBox1);
            CHECK(remainder2->logicalBounds() == expectedBBox2);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.csgSubtractAndUndoRestoresSelection") {
            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* entity = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entity}}});

            Model::BrushNode* subtrahend1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNodes({{entity, {subtrahend1}}});

            document->select(std::vector<Model::Node*>{subtrahend1});
            CHECK(document->csgSubtract());
            CHECK(entity->children().size() == 0u);
            CHECK(document->selectedNodes().empty());

            // check that the selection is restored after undo
            document->undoCommand();

            CHECK(document->selectedNodes().hasOnlyBrushes());
            CHECK_THAT(document->selectedNodes().brushes(), Catch::Equals(std::vector<Model::BrushNode*>{ subtrahend1 }));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.newWithGroupOpen") {
            Model::EntityNode* entity = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entity}}});
            document->select(entity);
            Model::GroupNode* group = document->groupSelection("my group");
            document->openGroup(group);

            CHECK(document->currentGroup() == group);

            document->newDocument(Model::MapFormat::Valve, MapDocument::DefaultWorldBounds, document->game());

            CHECK(document->currentGroup() == nullptr);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.ungroupInnerGroup") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2050
            Model::EntityNode* outerEnt1 = new Model::EntityNode();
            Model::EntityNode* outerEnt2 = new Model::EntityNode();
            Model::EntityNode* innerEnt1 = new Model::EntityNode();
            Model::EntityNode* innerEnt2 = new Model::EntityNode();

            document->addNodes({{document->parentForNodes(), {innerEnt1}}});
            document->addNodes({{document->parentForNodes(), {innerEnt2}}});
            document->select(std::vector<Model::Node*> {innerEnt1, innerEnt2});

            Model::GroupNode* inner = document->groupSelection("Inner");

            document->deselectAll();
            document->addNodes({{document->parentForNodes(), {outerEnt1}}});
            document->addNodes({{document->parentForNodes(), {outerEnt2}}});
            document->select(std::vector<Model::Node*> {inner, outerEnt1, outerEnt2});

            Model::GroupNode* outer = document->groupSelection("Outer");
            document->deselectAll();

            // check our assumptions
            CHECK(outer->childCount() == 3u);
            CHECK(inner->childCount() == 2u);

            CHECK(outer->parent() == document->currentLayer());

            CHECK(outerEnt1->parent() == outer);
            CHECK(outerEnt2->parent() == outer);
            CHECK(inner->parent() == outer);

            CHECK(innerEnt1->parent() == inner);
            CHECK(innerEnt2->parent() == inner);

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

            CHECK(innerEnt1->parent() == outer);
            CHECK(innerEnt2->parent() == outer);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.ungroupLeavesPointEntitySelected") {
            Model::EntityNode* ent1 = new Model::EntityNode();

            document->addNodes({{document->parentForNodes(), {ent1}}});
            document->select(std::vector<Model::Node*> {ent1});

            Model::GroupNode* group = document->groupSelection("Group");
            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> {group}));

            document->ungroupSelection();
            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> {ent1}));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.ungroupLeavesBrushEntitySelected") {
            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            Model::EntityNode* ent1 = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {ent1}}});

            Model::BrushNode* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNodes({{ent1, {brushNode1}}});
            document->select(std::vector<Model::Node*>{ent1});
            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> { brushNode1}));
            CHECK_FALSE(ent1->selected());
            CHECK(brushNode1->selected());

            Model::GroupNode* group = document->groupSelection("Group");
            CHECK_THAT(group->children(), Catch::Equals(std::vector<Model::Node*> {ent1}));
            CHECK_THAT(ent1->children(), Catch::Equals(std::vector<Model::Node*> { brushNode1}));
            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> {group}));
            CHECK(document->selectedNodes().brushesRecursively() == std::vector<Model::BrushNode*>{ brushNode1});
            CHECK(document->selectedNodes().hasBrushesRecursively());
            CHECK(!document->selectedNodes().hasBrushes());

            document->ungroupSelection();
            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> { brushNode1}));
            CHECK_FALSE(ent1->selected());
            CHECK(brushNode1->selected());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.mergeGroups") {
            document->selectAllNodes();
            document->deleteObjects();

            Model::EntityNode* ent1 = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {ent1}}});
            document->deselectAll();
            document->select(std::vector<Model::Node*> {ent1});
            Model::GroupNode* group1 = document->groupSelection("group1");

            Model::EntityNode* ent2 = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {ent2}}});
            document->deselectAll();
            document->select(std::vector<Model::Node*> {ent2});
            Model::GroupNode* group2 = document->groupSelection("group2");

            CHECK_THAT(document->currentLayer()->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{ group1, group2 }));

            document->select(std::vector<Model::Node*> {group1, group2});
            document->mergeSelectedGroupsWithGroup(group2);

            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> {group2}));
            CHECK_THAT(document->currentLayer()->children(), Catch::Equals(std::vector<Model::Node*> {group2}));

            CHECK_THAT(group1->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{}));
            CHECK_THAT(group2->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{ ent1, ent2 }));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pickSingleBrush") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode1}}});

            Model::PickResult pickResult;
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.query().all();
            CHECK(hits.size() == 1u);

            const auto& brush1 = brushNode1->brush();
            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::neg_x()), pickResult);
            CHECK(pickResult.query().all().empty());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pickSingleEntity") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::EntityNode* ent1 = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {ent1}}});

            const auto origin = ent1->entity().origin();
            const auto bounds = ent1->logicalBounds();

            const auto rayOrigin = origin + vm::vec3(-32.0, bounds.size().y() / 2.0, bounds.size().z() / 2.0);

            Model::PickResult pickResult;
            document->pick(vm::ray3(rayOrigin, vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.query().all();
            CHECK(hits.size() == 1u);

            CHECK(hits.front().target<Model::EntityNode*>() == ent1);
            CHECK(hits.front().distance() == vm::approx(32.0 - bounds.size().x() / 2.0));

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::neg_x()), pickResult);
            CHECK(pickResult.query().all().empty());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pickSimpleGroup") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode1}}});

            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode2}}});

            document->selectAllNodes();
            auto* group = document->groupSelection("test");

            Model::PickResult pickResult;
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            // picking a grouped object when the containing group is closed should return the object,
            // which is converted to the group when hitsToNodesWithGroupPicking() is used.
            auto hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            CHECK(hits.size() == 1u);

            const auto& brush1 = brushNode1->brush();
            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));

            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ group }));

            // hitting both objects in the group should return the group only once
            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(32, 32, -32), vm::vec3::pos_z()), pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            CHECK(hits.size() == 2u);

            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ group }));

            // hitting the group bounds doesn't count as a hit
            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 96), vm::vec3::pos_x()), pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            CHECK(hits.empty());

            // hitting a grouped object when the containing group is open should return the object only
            document->openGroup(group);

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            CHECK(hits.size() == 1u);

            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));

            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ brushNode1 }));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pickNestedGroup") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode1}}});

            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode2}}});

            document->selectAllNodes();
            auto* innerGroup = document->groupSelection("inner");

            document->deselectAll();
            auto* brushNode3 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 256)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode3}}});

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
            CHECK(hits.size() == 1u);

            const auto& brush3 = brushNode3->brush();
            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush3.face(*brush3.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));

            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ brushNode3 }));

            // hitting the brush in the inner group should return the inner group when hitsToNodesWithGroupPicking() is used
            pickResult.clear();
            document->pick(lowRay, pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            CHECK(hits.size() == 1u);

            const auto& brush1 = brushNode1->brush();
            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));
            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ innerGroup }));

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

            CHECK(innerGroup->opened());
            CHECK_FALSE(outerGroup->opened());
            CHECK(outerGroup->hasOpenedDescendant());

            // pick a brush in the outer group
            pickResult.clear();
            document->pick(highRay, pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            CHECK(hits.size() == 1u);

            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush3.face(*brush3.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));
            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ brushNode3 }));

            // pick a brush in the inner group
            pickResult.clear();
            document->pick(lowRay, pickResult);

            hits = pickResult.query().type(Model::BrushNode::BrushHitType).all();
            CHECK(hits.size() == 1u);

            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));
            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ brushNode1 }));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pickBrushEntity") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode1}}});

            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode2}}});

            document->selectAllNodes();

            document->createBrushEntity(m_brushEntityDef);
            document->deselectAll();

            Model::PickResult pickResult;

            // picking entity brushes should only return the brushes and not the entity
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.query().all();
            CHECK(hits.size() == 1u);

            const auto& brush1 = brushNode1->brush();
            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.throwExceptionDuringCommand") {
            CHECK_THROWS_AS(document->throwExceptionDuringCommand(), CommandProcessorException);
        }

        static void transform(Model::BrushNode* brushNode, const vm::bbox3& worldBounds, const vm::mat4x4& transform) {
            auto brush = brushNode->brush();
            REQUIRE(brush.transform(worldBounds, transform, false).is_success());
            brushNode->setBrush(std::move(brush));
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.selectTouching") {
            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode1 = new Model::BrushNode(builder.createCube(64.0, "none").value());
            Model::BrushNode* brushNode2 = new Model::BrushNode(builder.createCube(64.0, "none").value());
            Model::BrushNode* brushNode3 = new Model::BrushNode(builder.createCube(64.0, "none").value());

            transform(brushNode2, document->worldBounds(), vm::translation_matrix(vm::vec3(10.0, 0.0, 0.0)));
            transform(brushNode3, document->worldBounds(), vm::translation_matrix(vm::vec3(100.0, 0.0, 0.0)));

            document->addNodes({{document->parentForNodes(), {brushNode1}}});
            document->addNodes({{document->parentForNodes(), {brushNode2}}});
            document->addNodes({{document->parentForNodes(), {brushNode3}}});

            REQUIRE(brushNode1->intersects(brushNode2));
            REQUIRE(brushNode2->intersects(brushNode1));

            REQUIRE(!brushNode1->intersects(brushNode3));
            REQUIRE(!brushNode3->intersects(brushNode1));

            document->select(brushNode1);
            document->selectTouching(false);

            using Catch::Matchers::UnorderedEquals;
            CHECK_THAT(document->selectedNodes().brushes(), UnorderedEquals(std::vector<Model::BrushNode*>{ brushNode2}));
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/2476
        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.selectTouching_2476") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(box, "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode1}}});

            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(box.translate(vm::vec3(1, 1, 1)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode2}}});

            document->selectAllNodes();

            CHECK_THAT(document->selectedNodes().brushes(), Catch::Equals(std::vector<Model::BrushNode* >{ brushNode1, brushNode2}));
            CHECK_THAT(document->currentLayer()->children(), Catch::Equals(std::vector<Model::Node* >{ brushNode1, brushNode2}));

            document->selectTouching(true);

            // only this next line was failing
            CHECK_THAT(document->selectedNodes().brushes(), Catch::Equals(std::vector<Model::BrushNode* >{}));
            CHECK_THAT(document->currentLayer()->children(), Catch::Equals(std::vector<Model::Node* >{}));

            // brush1 and brush2 are deleted
            CHECK(brushNode1->parent() == nullptr);
            CHECK(brushNode2->parent() == nullptr);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.selectTall") {
            using Catch::Matchers::UnorderedEquals;

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode1 = new Model::BrushNode(builder.createCube(64.0, "none").value());
            Model::BrushNode* brushNode2 = new Model::BrushNode(builder.createCube(64.0, "none").value());
            Model::BrushNode* brushNode3 = new Model::BrushNode(builder.createCube(64.0, "none").value());

            transform(brushNode2, document->worldBounds(), vm::translation_matrix(vm::vec3(0.0, 0.0, -500.0)));
            transform(brushNode3, document->worldBounds(), vm::translation_matrix(vm::vec3(100.0, 0.0, 0.0)));

            document->addNodes({{document->parentForNodes(), {brushNode1}}});
            document->addNodes({{document->parentForNodes(), {brushNode2}}});
            document->addNodes({{document->parentForNodes(), {brushNode3}}});

            REQUIRE(!brushNode1->intersects(brushNode2));
            REQUIRE(!brushNode1->intersects(brushNode3));

            document->select(brushNode1);

            SECTION("z camera") {
                document->selectTall(vm::axis::z);

                CHECK_THAT(document->selectedNodes().brushes(), UnorderedEquals(std::vector<Model::BrushNode*>{ brushNode2}));
            }
            SECTION("x camera") {
                document->selectTall(vm::axis::x);

                CHECK_THAT(document->selectedNodes().brushes(), UnorderedEquals(std::vector<Model::BrushNode*>{ brushNode3}));
            }
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.selectInverse") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(box, "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode1}}});

            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(box.translate(vm::vec3(1, 1, 1)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode2}}});

            auto* brushNode3 = new Model::BrushNode(builder.createCuboid(box.translate(vm::vec3(2, 2, 2)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode3}}});

            document->select(std::vector<Model::Node *>{ brushNode1, brushNode2});
            Model::EntityNode* brushEnt = document->createBrushEntity(m_brushEntityDef);

            document->deselectAll();

            // worldspawn {
            //   brushEnt { brush1, brush2 },
            //   brush3
            // }

            document->select(brushNode1);
            CHECK( brushNode1->selected());
            CHECK(!brushNode2->selected());
            CHECK(!brushNode3->selected());
            CHECK(!brushEnt->selected());

            document->selectInverse();

            CHECK_THAT(document->selectedNodes().brushes(), Catch::UnorderedEquals(std::vector<Model::BrushNode *>{ brushNode2, brushNode3}));
            CHECK(!brushNode1->selected());
            CHECK( brushNode2->selected());
            CHECK( brushNode3->selected());
            CHECK(!brushEnt->selected());
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/2776
        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pasteAndTranslateGroup") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(box, "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode1}}});
            document->select(brushNode1);

            const auto groupName = std::string("testGroup");

            auto* group = document->groupSelection(groupName);
            CHECK(group != nullptr);
            document->select(group);

            const std::string copied = document->serializeSelectedNodes();

            const auto delta = vm::vec3(16, 16, 16);
            CHECK(document->paste(copied) == PasteType::Node);
            CHECK(document->selectedNodes().groupCount() == 1u);
            CHECK(document->selectedNodes().groups().at(0)->name() == groupName);
            CHECK(document->translateObjects(delta));
            CHECK(document->selectionBounds() == box.translate(delta));
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/3117
        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.isolate") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(box, "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode1}}});

            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(box.translate(vm::vec3(1, 1, 1)), "texture").value());
            document->addNodes({{document->parentForNodes(), {brushNode2}}});

            document->selectAllNodes();

            Model::EntityNode* brushEntity = document->createBrushEntity(m_brushEntityDef);

            document->deselectAll();

            // Check initial state
            REQUIRE(1 == document->currentLayer()->childCount());
            REQUIRE(brushEntity == dynamic_cast<Model::EntityNode*>(document->currentLayer()->children().at(0)));
            REQUIRE(2 == brushEntity->childCount());
            REQUIRE(brushNode1 == dynamic_cast<Model::BrushNode*>(brushEntity->children().at(0)));
            REQUIRE(brushNode2 == dynamic_cast<Model::BrushNode*>(brushEntity->children().at(1)));

            CHECK(!brushEntity->selected());
            CHECK(!brushNode1->selected());
            CHECK(!brushNode2->selected());
            CHECK(!brushEntity->hidden());
            CHECK(!brushNode1->hidden());
            CHECK(!brushNode2->hidden());

            // Select just brush1
            document->select(brushNode1);
            CHECK(!brushEntity->selected());
            CHECK(brushNode1->selected());
            CHECK(!brushNode2->selected());

            // Isolate brush1
            document->isolate();

            CHECK(!brushEntity->hidden());
            CHECK(!brushNode1->hidden());
            CHECK(brushNode2->hidden());
        }

        TEST_CASE_METHOD(MapDocumentTest, "IssueGenerator.emptyProperty") {
            Model::EntityNode* entityNode = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            
            document->deselectAll();
            document->select(entityNode);
            document->setProperty("", "");
            REQUIRE(entityNode->entity().hasProperty(""));

            auto issueGenerators = std::vector<Model::IssueGenerator*>{
                new Model::EmptyPropertyKeyIssueGenerator(),
                new Model::EmptyPropertyValueIssueGenerator()
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

            // Should be one EmptyPropertyNameIssue and one EmptyPropertyValueIssue
            CHECK(((issue0->type() == issueGenerators[0]->type() && issue1->type() == issueGenerators[1]->type())
                || (issue0->type() == issueGenerators[1]->type() && issue1->type() == issueGenerators[0]->type())));
            
            std::vector<Model::IssueQuickFix*> fixes = document->world()->quickFixes(issue0->type());
            REQUIRE(1 == fixes.size());

            Model::IssueQuickFix* quickFix = fixes.at(0);
            quickFix->apply(document.get(), std::vector<Model::Issue*>{issue0});

            // The fix should have deleted the property
            CHECK(!entityNode->entity().hasProperty(""));

            kdl::vec_clear_and_delete(issueGenerators);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.defaultLayerSortIndexImmutable", "[LayerTest]") {
            Model::LayerNode* defaultLayerNode = document->world()->defaultLayer();
            setLayerSortIndex(*defaultLayerNode, 555);

            CHECK(defaultLayerNode->layer().sortIndex() == Model::Layer::defaultLayerSortIndex());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.renameLayer", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::LayerNode* layerNode = new Model::LayerNode(Model::Layer("test1"));
            document->addNodes({{document->world(), {layerNode}}});
            CHECK(layerNode->name() == "test1");

            document->renameLayer(layerNode, "test2");
            CHECK(layerNode->name() == "test2");

            document->undoCommand();
            CHECK(layerNode->name() == "test1");
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.duplicateObjectGoesIntoSourceLayer", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::LayerNode* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
            Model::LayerNode* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
            document->addNodes({{document->world(), {layerNode1}}});
            document->addNodes({{document->world(), {layerNode2}}});

            document->setCurrentLayer(layerNode1);
            Model::EntityNode* entity = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 1);

            document->setCurrentLayer(layerNode2);
            document->select(entity);
            document->duplicateObjects(); // the duplicate should stay in layer1

            REQUIRE(document->selectedNodes().entityCount() == 1);
            Model::EntityNode* entityClone = document->selectedNodes().entities().at(0);
            CHECK(entityClone->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 2);
            CHECK(document->currentLayer() == layerNode2);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.newGroupGoesIntoSourceLayer", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::LayerNode* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
            Model::LayerNode* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
            document->addNodes({{document->world(), {layerNode1}}});
            document->addNodes({{document->world(), {layerNode2}}});

            document->setCurrentLayer(layerNode1);
            Model::EntityNode* entity = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 1);

            document->setCurrentLayer(layerNode2);
            document->select(entity);
            Model::GroupNode* newGroup = document->groupSelection("Group in Layer 1"); // the new group should stay in layer1

            CHECK(entity->parent() == newGroup);
            CHECK(Model::findContainingLayer(entity) == layerNode1);
            CHECK(Model::findContainingLayer(newGroup) == layerNode1);
            CHECK(document->currentLayer() == layerNode2);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.newObjectsInHiddenLayerAreVisible", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::LayerNode* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
            Model::LayerNode* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
            document->addNodes({{document->world(), {layerNode1}}});
            document->addNodes({{document->world(), {layerNode2}}});

            document->setCurrentLayer(layerNode1);

            // Create an entity in layer1
            Model::EntityNode* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity1->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 1u);

            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(entity1->visible());

            // Hide layer1. If any nodes in the layer were Visibility_Shown they would be reset to Visibility_Inherited
            document->hideLayers({ layerNode1});

            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity1->visible());

            // Create another entity in layer1. It will be visible, while entity1 will still be hidden.
            Model::EntityNode* entity2 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity2->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 2u);

            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity1->visible());
            CHECK(entity2->visibilityState() == Model::VisibilityState::Visibility_Shown);
            CHECK(entity2->visible());

            // Change to layer2. This hides all objects in layer1
            document->setCurrentLayer(layerNode2);

            CHECK(document->currentLayer() == layerNode2);
            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity1->visible());
            CHECK(entity2->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity2->visible());

            // Undo (Switch current layer back to layer1)
            document->undoCommand();

            CHECK(document->currentLayer() == layerNode1);
            CHECK(entity1->visibilityState() == Model::VisibilityState::Visibility_Inherited);
            CHECK(!entity1->visible());
            CHECK(entity2->visibilityState() == Model::VisibilityState::Visibility_Shown);
            CHECK(entity2->visible());

            // Undo (entity2 creation)
            document->undoCommand();

            CHECK(layerNode1->childCount() == 1u);
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

            Model::LayerNode* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
            document->addNodes({{document->world(), {layerNode1}}});

            document->setCurrentLayer(layerNode1);
            document->hideLayers({ layerNode1});

            // Create entity1 and brush1 in the hidden layer1
            Model::EntityNode* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            Model::BrushNode* brush1 = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brush1}}});

            CHECK(entity1->parent() == layerNode1);
            CHECK(brush1->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 2u);

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

            auto* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
            auto* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
            document->addNodes({{document->world(), {layerNode1}}});
            document->addNodes({{document->world(), {layerNode2}}});

            document->setCurrentLayer(layerNode1);

            // Create an entity in layer1
            auto* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity1->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 1u);

            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(!entity1->locked());

            // Lock layer1
            document->lock({ layerNode1});

            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity1->locked());

            // Create another entity in layer1. It will be unlocked, while entity1 will still be locked (inherited).
            auto* entity2 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity2->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 2u);

            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity1->locked());
            CHECK(entity2->lockState() == Model::LockState::Lock_Unlocked);
            CHECK(!entity2->locked());

            // Change to layer2. This causes the Lock_Unlocked objects in layer1 to be degraded to Lock_Inherited
            // (i.e. everything in layer1 becomes locked)
            document->setCurrentLayer(layerNode2);

            CHECK(document->currentLayer() == layerNode2);
            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity1->locked());
            CHECK(entity2->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity2->locked());

            // Undo (Switch current layer back to layer1)
            document->undoCommand();

            CHECK(document->currentLayer() == layerNode1);
            CHECK(entity1->lockState() == Model::LockState::Lock_Inherited);
            CHECK(entity1->locked());
            CHECK(entity2->lockState() == Model::LockState::Lock_Unlocked);
            CHECK(!entity2->locked());

            // Undo entity2 creation
            document->undoCommand();

            CHECK(layerNode1->childCount() == 1u);
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

            auto* layerNode0 = new Model::LayerNode(Model::Layer("layer0"));
            auto* layerNode1 = new Model::LayerNode(Model::Layer("layer1"));
            auto* layerNode2 = new Model::LayerNode(Model::Layer("layer2"));

            setLayerSortIndex(*layerNode0, 0);
            setLayerSortIndex(*layerNode1, 1);
            setLayerSortIndex(*layerNode2, 2);

            document->addNodes({{document->world(), {layerNode0}}});
            document->addNodes({{document->world(), {layerNode1}}});
            document->addNodes({{document->world(), {layerNode2}}});

            SECTION("check canMoveLayer") {
                // defaultLayer() can never be moved
                CHECK(!document->canMoveLayer(document->world()->defaultLayer(), 1));
                CHECK( document->canMoveLayer(layerNode0,  0));
                CHECK(!document->canMoveLayer(layerNode0, -1));
                CHECK( document->canMoveLayer(layerNode0,  1));
                CHECK( document->canMoveLayer(layerNode0,  2));
                CHECK(!document->canMoveLayer(layerNode0,  3));
            }

            SECTION("moveLayer by 0 has no effect") {
                document->moveLayer(layerNode0, 0);
                CHECK(layerNode0->layer().sortIndex() == 0);
            }
            SECTION("moveLayer by invalid negative amount is clamped") {
                document->moveLayer(layerNode0, -1000);
                CHECK(layerNode0->layer().sortIndex() == 0);
            }
            SECTION("moveLayer by 1") {
                document->moveLayer(layerNode0, 1);
                CHECK(layerNode1->layer().sortIndex() == 0);
                CHECK(layerNode0->layer().sortIndex() == 1);
                CHECK(layerNode2->layer().sortIndex() == 2);
            }
            SECTION("moveLayer by 2") {
                document->moveLayer(layerNode0, 2);
                CHECK(layerNode1->layer().sortIndex() == 0);
                CHECK(layerNode2->layer().sortIndex() == 1);
                CHECK(layerNode0->layer().sortIndex() == 2);
            }
            SECTION("moveLayer by invalid positive amount is clamped") {
                document->moveLayer(layerNode0, 1000);
                CHECK(layerNode1->layer().sortIndex() == 0);
                CHECK(layerNode2->layer().sortIndex() == 1);
                CHECK(layerNode0->layer().sortIndex() == 2);
            }
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.setCurrentLayerCollation", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            auto* defaultLayerNode = document->world()->defaultLayer();
            auto* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
            auto* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
            document->addNodes({{document->world(), {layerNode1}}});
            document->addNodes({{document->world(), {layerNode2}}});
            CHECK(document->currentLayer() == defaultLayerNode);

            document->setCurrentLayer(layerNode1);
            document->setCurrentLayer(layerNode2);
            CHECK(document->currentLayer() == layerNode2);

            // No collation currently because of the transactions in setCurrentLayer()
            document->undoCommand();
            CHECK(document->currentLayer() == layerNode1);
            document->undoCommand();
            CHECK(document->currentLayer() == defaultLayerNode);

            document->redoCommand();
            CHECK(document->currentLayer() == layerNode1);
            document->redoCommand();
            CHECK(document->currentLayer() == layerNode2);
        }
    }
}
