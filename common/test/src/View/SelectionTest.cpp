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

#include "Exceptions.h"
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/NodeCollection.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

#include <kdl/result.h>

#include "Catch2.h"

#include "TestUtils.h"

namespace TrenchBroom {
    namespace View {
        TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.allSelectedEntityNodes") {
            GIVEN("A document with multiple entity nodes in various configurations") {
                auto* topLevelEntityNode = new Model::EntityNode{Model::Entity{}};
                
                auto* emptyGroupNode = new Model::GroupNode{Model::Group{"empty"}};
                auto* groupNodeWithEntity = new Model::GroupNode{Model::Group{"group"}};
                auto* groupedEntityNode = new Model::EntityNode{Model::Entity{}};
                groupNodeWithEntity->addChild(groupedEntityNode);

                auto* topLevelBrushNode = createBrushNode();
                auto* topLevelPatchNode = createPatchNode();

                auto* topLevelBrushEntityNode = new Model::EntityNode{Model::Entity{}};
                auto* brushEntityBrushNode = createBrushNode();
                auto* brushEntityPatchNode = createPatchNode();
                topLevelBrushEntityNode->addChildren({brushEntityBrushNode, brushEntityPatchNode});

                document->addNodes({{document->parentForNodes(), {
                    topLevelEntityNode, 
                    topLevelBrushEntityNode, 
                    topLevelBrushNode,
                    topLevelPatchNode,
                    emptyGroupNode,
                    groupNodeWithEntity}}});

                document->deselectAll();

                WHEN("Nothing is selected") {
                    THEN("The world node is returned") {
                        CHECK_THAT(document->allSelectedEntityNodes(), Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
                            document->world()
                        }));
                    }
                }

                WHEN("A top level brush node is selected") {
                    document->select(topLevelBrushNode);

                    THEN("The world node is returned") {
                        CHECK_THAT(document->allSelectedEntityNodes(), Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
                            document->world()
                        }));
                    }
                }

                WHEN("A top level patch node is selected") {
                    document->select(topLevelPatchNode);

                    THEN("The world node is returned") {
                        CHECK_THAT(document->allSelectedEntityNodes(), Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
                            document->world()
                        }));
                    }
                }

                WHEN("An empty group node is selected") {
                    document->select(emptyGroupNode);

                    THEN("An empty vector is returned") {
                        CHECK_THAT(document->allSelectedEntityNodes(), Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
                        }));
                    }
                }

                WHEN("A group node containing an entity node is selected") {
                    document->select(groupNodeWithEntity);

                    THEN("The grouped entity node is returned") {
                        CHECK_THAT(document->allSelectedEntityNodes(), Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
                            groupedEntityNode
                        }));
                    }

                    AND_WHEN("A top level entity node is selected") {
                        document->select(topLevelEntityNode);

                        THEN("The top level entity node and the grouped entity node are returned") {
                            CHECK_THAT(document->allSelectedEntityNodes(), Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
                                groupedEntityNode,
                                topLevelEntityNode
                            }));
                        }
                    }
                }

                WHEN("An empty top level entity node is selected") {
                    document->select(topLevelEntityNode);

                    THEN("That entity node is returned") {
                        CHECK_THAT(document->allSelectedEntityNodes(), Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
                            topLevelEntityNode
                        }));
                    }
                }

                WHEN("A node in a brush entity node is selected") {
                    const auto selectBrushNode = [](auto* brushNode, auto* patchNode) -> std::tuple<Model::Node*, Model::Node*> { return {brushNode, patchNode}; };
                    const auto selectPatchNode = [](auto* brushNode, auto* patchNode) -> std::tuple<Model::Node*, Model::Node*> { return {patchNode, brushNode}; };
                    const auto selectNodes = GENERATE_COPY(selectBrushNode, selectPatchNode);
                    
                    const auto [nodeToSelect, otherNode] = selectNodes(brushEntityBrushNode, brushEntityPatchNode);

                    CAPTURE(nodeToSelect->name(), otherNode->name());

                    document->select(nodeToSelect);

                    THEN("The containing entity node is returned") {
                        CHECK_THAT(document->allSelectedEntityNodes(), Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
                            topLevelBrushEntityNode
                        }));
                    }

                    AND_WHEN("Another node in the same entity node is selected") {
                        document->select(otherNode);

                        THEN("The containing entity node is returned only once") {
                            CHECK_THAT(document->allSelectedEntityNodes(), Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
                                topLevelBrushEntityNode
                            }));
                        }
                    }

                    AND_WHEN("A top level entity node is selected") {
                        document->select(topLevelEntityNode);

                        THEN("The top level entity node and the brush entity node are returned") {
                            CHECK_THAT(document->allSelectedEntityNodes(), Catch::Matchers::UnorderedEquals(std::vector<Model::EntityNodeBase*>{
                                topLevelBrushEntityNode,
                                topLevelEntityNode
                            }));
                        }
                    }
                }
            }
        }

        TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectTouchingWithGroup") {
            document->selectAllNodes();
            document->deleteObjects();
            assert(document->selectedNodes().nodeCount() == 0);

            Model::LayerNode* layer = new Model::LayerNode(Model::Layer("Layer 1"));
            addNode(*document, document->world(), layer);

            Model::GroupNode* group = new Model::GroupNode(Model::Group("Unnamed"));
            addNode(*document, layer, group);

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const vm::bbox3 brushBounds(vm::vec3(-32.0, -32.0, -32.0),
                                    vm::vec3(+32.0, +32.0, +32.0));

            Model::BrushNode* brush = new Model::BrushNode(builder.createCuboid(brushBounds, "texture").value());
            addNode(*document, group, brush);

            const vm::bbox3 selectionBounds(vm::vec3(-16.0, -16.0, -48.0),
                                        vm::vec3(+16.0, +16.0, +48.0));

            Model::BrushNode* selectionBrush = new Model::BrushNode(builder.createCuboid(selectionBounds, "texture").value());
            addNode(*document, layer, selectionBrush);

            document->select(selectionBrush);
            document->selectTouching(true);

            CHECK(document->selectedNodes().nodeCount() == 1u);
        }

        TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.selectInsideWithGroup") {
            document->selectAllNodes();
            document->deleteObjects();
            assert(document->selectedNodes().nodeCount() == 0);

            Model::LayerNode* layer = new Model::LayerNode(Model::Layer("Layer 1"));
            addNode(*document, document->world(), layer);

            Model::GroupNode* group = new Model::GroupNode(Model::Group("Unnamed"));
            addNode(*document, layer, group);

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const vm::bbox3 brushBounds(vm::vec3(-32.0, -32.0, -32.0),
                                    vm::vec3(+32.0, +32.0, +32.0));

            Model::BrushNode* brush = new Model::BrushNode(builder.createCuboid(brushBounds, "texture").value());
            addNode(*document, group, brush);

            const vm::bbox3 selectionBounds(vm::vec3(-48.0, -48.0, -48.0),
                                        vm::vec3(+48.0, +48.0, +48.0));

            Model::BrushNode* selectionBrush = new Model::BrushNode(builder.createCuboid(selectionBounds, "texture").value());
            addNode(*document, layer, selectionBrush);

            document->select(selectionBrush);
            document->selectInside(true);

            CHECK(document->selectedNodes().nodeCount() == 1u);
        }
        
        TEST_CASE_METHOD(MapDocumentTest, "SelectionTest.updateLastSelectionBounds") {
            auto* entityNode = new Model::EntityNode({
                {"classname", "point_entity"}
            });
            addNode(*document, document->parentForNodes(), entityNode);
            REQUIRE(!entityNode->logicalBounds().is_empty());
            
            document->selectAllNodes();
            
            auto bounds = document->selectionBounds();
            document->deselectAll();
            CHECK(document->lastSelectionBounds() == bounds);
            
            document->deselectAll();
            CHECK(document->lastSelectionBounds() == bounds);
            
            auto* brushNode = createBrushNode();
            addNode(*document, document->parentForNodes(), brushNode);
            
            document->select(brushNode);
            CHECK(document->lastSelectionBounds() == bounds);
            
            bounds = brushNode->logicalBounds();
            
            document->deselectAll();
            CHECK(document->lastSelectionBounds() == bounds);
        }

        TEST_CASE_METHOD(MapDocumentTest, "SelectionCommandTest.faceSelectionUndoAfterTranslationUndo") {
            Model::BrushNode* brushNode = createBrushNode();
            CHECK(brushNode->logicalBounds().center() == vm::vec3::zero());

            addNode(*document, document->parentForNodes(), brushNode);

            const auto topFaceIndex = brushNode->brush().findFace(vm::vec3::pos_z());
            REQUIRE(topFaceIndex);

            // select the top face
            document->select({ brushNode, *topFaceIndex });
            CHECK_THAT(document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{{ brushNode, *topFaceIndex }}));

            // deselect it
            document->deselect({ brushNode, *topFaceIndex });
            CHECK_THAT(document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{}));

            // select the brush
            document->select(brushNode);
            CHECK_THAT(document->selectedNodes().brushes(), Catch::Equals(std::vector<Model::BrushNode*>{ brushNode }));

            // translate the brush
            document->translateObjects(vm::vec3(10.0, 0.0, 0.0));
            CHECK(brushNode->logicalBounds().center() == vm::vec3(10.0, 0.0, 0.0));

            // Start undoing changes

            document->undoCommand();
            CHECK(brushNode->logicalBounds().center() == vm::vec3::zero());
            CHECK_THAT(document->selectedNodes().brushes(), Catch::Equals(std::vector<Model::BrushNode*>{ brushNode }));
            CHECK_THAT(document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{}));

            document->undoCommand();
            CHECK_THAT(document->selectedNodes().brushes(), Catch::Equals(std::vector<Model::BrushNode*>{}));
            CHECK_THAT(document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{}));

            document->undoCommand();
            CHECK_THAT(document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{{ brushNode, *topFaceIndex }}));
        }
    }
}
