/*
 Copyright (C) 2021 Kristian Duske
 Copyright (C) 2021 Eric Wasylishen

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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/BrushBuilder.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/PatchNode.h"
#include "Model/Polyhedron.h"
#include "Model/WorldNode.h"

#include <vecmath/approx.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <kdl/result.h>
#include <kdl/vector_utils.h>
#include <kdl/zip_iterator.h>

#include <vector>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
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

        static void checkTransformation(const Model::Node& node, const Model::Node& original, const vm::mat4x4d& transformation) {
            CHECK(node.physicalBounds() == original.physicalBounds().transform(transformation));

            REQUIRE(node.childCount() == original.childCount());
            for (const auto [nodeChild, originalChild] : kdl::make_zip_range(node.children(), original.children())) {
                checkTransformation(*nodeChild, *originalChild, transformation);
            }
        }

        TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.flip") {
            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0.0, 0.0, 0.0), vm::vec3(30.0, 31.0, 31.0)), "texture").value());
            Model::BrushNode* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(30.0, 0.0, 0.0), vm::vec3(31.0, 31.0, 31.0)), "texture").value());

            checkBrushIntegral(brushNode1);
            checkBrushIntegral(brushNode2);

            addNode(*document, document->parentForNodes(), brushNode1);
            addNode(*document, document->parentForNodes(), brushNode2);

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

        TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.transformObjects") {
            using CreateNode = std::function<Model::Node*(const MapDocumentTest& test)>;
            const auto createNode = GENERATE_COPY(
                CreateNode{[](const auto& test) -> Model::Node* {
                    auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
                    auto* brushNode = test.createBrushNode();
                    auto* patchNode = test.createPatchNode();
                    auto* entityNode = new Model::EntityNode{Model::Entity{}};
                    groupNode->addChildren({brushNode, patchNode, entityNode});
                    return groupNode;
                }},
                CreateNode{[](const auto&) -> Model::Node* {
                    return new Model::EntityNode{Model::Entity{}};
                }},
                CreateNode{[](const auto& test) -> Model::Node* {
                    auto* entityNode = new Model::EntityNode{Model::Entity{}};
                    auto* brushNode = test.createBrushNode();
                    auto* patchNode = test.createPatchNode();
                    entityNode->addChildren({brushNode, patchNode});
                    return entityNode;
                }},
                CreateNode{[](const auto& test) -> Model::Node* {
                    return test.createBrushNode();
                }},
                CreateNode{[](const auto& test) -> Model::Node* {
                    return test.createPatchNode();
                }}
            );

            GIVEN("A node to transform") {
                auto* node = createNode(*this);
                CAPTURE(node->name());

                document->addNodes({{document->parentForNodes(), {node}}});
                
                const auto originalNode = std::unique_ptr<Model::Node>{node->cloneRecursively(document->worldBounds())};
                const auto transformation = vm::translation_matrix(vm::vec3d{1, 2, 3});

                WHEN("The node is transformed") {
                    document->select(node);
                    document->transformObjects("Transform Nodes", transformation);

                    THEN("The transformation was applied to the node and its children") {
                        checkTransformation(*node, *originalNode.get(), transformation);
                    }

                    AND_WHEN("The transformation is undone") {
                        document->undoCommand();

                        THEN("The node is back in its original state") {
                            checkTransformation(*node, *originalNode.get(), vm::mat4x4d::identity());
                        }
                    }
                }
            }
        }

        TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.rotate") {
            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0.0, 0.0, 0.0), vm::vec3(30.0, 31.0, 31.0)), "texture").value());
            Model::BrushNode* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(30.0, 0.0, 0.0), vm::vec3(31.0, 31.0, 31.0)), "texture").value());

            checkBrushIntegral(brushNode1);
            checkBrushIntegral(brushNode2);

            addNode(*document, document->parentForNodes(), brushNode1);
            addNode(*document, document->parentForNodes(), brushNode2);

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

        TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.shearCube") {
            const vm::bbox3 initialBBox(vm::vec3(100,100,100), vm::vec3(200,200,200));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCuboid(initialBBox, "texture").value());

            addNode(*document, document->parentForNodes(), brushNode);
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

        TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.shearPillar") {
            const vm::bbox3 initialBBox(vm::vec3(0,0,0), vm::vec3(100,100,400));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCuboid(initialBBox, "texture").value());

            addNode(*document, document->parentForNodes(), brushNode);
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

        TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.scaleObjects") {
            const vm::bbox3 initialBBox(vm::vec3(-100,-100,-100), vm::vec3(100,100,100));
            const vm::bbox3 doubleBBox(2.0 * initialBBox.min, 2.0 * initialBBox.max);
            const vm::bbox3 invalidBBox(vm::vec3(0,-100,-100), vm::vec3(0,100,100));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCuboid(initialBBox, "texture").value());
            const Model::Brush& brush = brushNode->brush();

            addNode(*document, document->parentForNodes(), brushNode);
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

        TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.scaleObjectsInGroup") {
            const vm::bbox3 initialBBox(vm::vec3(-100, -100, -100), vm::vec3(100, 100, 100));
            const vm::bbox3 doubleBBox(2.0 * initialBBox.min, 2.0 * initialBBox.max);
            const vm::bbox3 invalidBBox(vm::vec3(0, -100, -100), vm::vec3(0, 100, 100));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCuboid(initialBBox, "texture").value());

            addNode(*document, document->parentForNodes(), brushNode);
            document->select(std::vector<Model::Node*>{ brushNode });
            [[maybe_unused]] Model::GroupNode* group = document->groupSelection("my group");

            // attempting an invalid scale has no effect
            CHECK_FALSE(document->scaleObjects(initialBBox, invalidBBox));
            CHECK(brushNode->logicalBounds().size() == vm::vec3(200, 200, 200));

            CHECK(document->scaleObjects(initialBBox, doubleBBox));
            CHECK(brushNode->logicalBounds().size() == vm::vec3(400, 400, 400));
        }

        TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.scaleObjectsWithCenter") {
            const vm::bbox3 initialBBox(vm::vec3(0,0,0), vm::vec3(100,100,400));
            const vm::bbox3 expectedBBox(vm::vec3(-50,0,0), vm::vec3(150,100,400));

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCuboid(initialBBox, "texture").value());

            addNode(*document, document->parentForNodes(), brushNode);
            document->select(std::vector<Model::Node*>{brushNode});

            const vm::vec3 boundsCenter = initialBBox.center();
            CHECK(document->scaleObjects(boundsCenter, vm::vec3(2.0, 1.0, 1.0)));
            CHECK(brushNode->logicalBounds() == expectedBBox);
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/3784
        TEST_CASE_METHOD(MapDocumentTest, "TransformNodesTest.translateLinkedGroup") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(box, "texture").value());
            addNode(*document, document->parentForNodes(), brushNode1);
            document->select(brushNode1);

            auto* group = document->groupSelection("testGroup");
            document->select(group);

            auto* linkedGroup = document->createLinkedDuplicate();
            document->deselectAll();
            document->select(linkedGroup);
            REQUIRE_THAT(document->selectedNodes().nodes(), Catch::UnorderedEquals(std::vector<Model::Node*>{linkedGroup}));

            auto* linkedBrushNode = dynamic_cast<Model::BrushNode*>(linkedGroup->children().at(0));
            REQUIRE(linkedBrushNode != nullptr);

            setPref(Preferences::TextureLock, false);

            const auto delta = vm::vec3(0.125, 0, 0);
            REQUIRE(document->translateObjects(delta));

            auto getTexCoords = [](Model::BrushNode* brushNode, const vm::vec3& normal) -> std::vector<vm::vec2f> {
                const Model::BrushFace& face = brushNode->brush().face(*brushNode->brush().findFace(normal));
                return kdl::vec_transform(face.vertexPositions(), [&](auto x) { return face.textureCoords(x); });
            };

            // Brushes in linked groups should have texture lock forced on
            CHECK(UVListsEqual(getTexCoords(brushNode1, vm::vec3::pos_z()),
                               getTexCoords(linkedBrushNode, vm::vec3::pos_z())));

            PreferenceManager::instance().resetToDefault(Preferences::TextureLock);
        }
    }
}
