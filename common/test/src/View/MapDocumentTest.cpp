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
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Assets/EntityDefinition.h"
#include "IO/WorldReader.h"
#include "Model/BezierPatch.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EmptyPropertyKeyIssueGenerator.h"
#include "Model/EmptyPropertyValueIssueGenerator.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
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
#include "Model/PatchNode.h"
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
#include <kdl/zip_iterator.h>

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
        MapDocumentTest(Model::MapFormat::Standard) {}

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

        Model::BrushNode* MapDocumentTest::createBrushNode(const std::string& textureName, const std::function<void(Model::Brush&)>& brushFunc) const {
            const Model::WorldNode* world = document->world();
            Model::BrushBuilder builder(world->mapFormat(), document->worldBounds(), document->game()->defaultFaceAttribs());
            Model::Brush brush = builder.createCube(32.0, textureName).value();
            brushFunc(brush);
            return new Model::BrushNode(std::move(brush));
        }

        Model::PatchNode* MapDocumentTest::createPatchNode(const std::string& textureName) const {
            return new Model::PatchNode{Model::BezierPatch{3, 3, {
                {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
                {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
                {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, textureName}};
        }

        ValveMapDocumentTest::ValveMapDocumentTest() :
        MapDocumentTest(Model::MapFormat::Valve) {}

        Quake3MapDocumentTest::Quake3MapDocumentTest() :
        MapDocumentTest{Model::MapFormat::Quake3} {}

        static void setLayerSortIndex(Model::LayerNode& layerNode, int sortIndex) {
            auto layer = layerNode.layer();
            layer.setSortIndex(sortIndex);
            layerNode.setLayer(layer);
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.throwExceptionDuringCommand") {
            CHECK_THROWS_AS(document->throwExceptionDuringCommand(), CommandProcessorException);
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/2776
        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.pasteAndTranslateGroup") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(box, "texture").value());
            addNode(*document, document->parentForNodes(), brushNode1);
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

        // https://github.com/TrenchBroom/TrenchBroom/issues/3784
        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.translateLinkedGroup") {
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

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.isolate") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            GIVEN("An unrelated top level node") {
                auto* nodeToHide = new Model::EntityNode{Model::Entity{}};
                document->addNodes({{document->parentForNodes(), {nodeToHide}}});

                REQUIRE(!nodeToHide->hidden());

                AND_GIVEN("Another top level node that should be isolated") {
                    using CreateNode = std::function<Model::Node*(const MapDocumentTest&)>;
                    const auto createNode = GENERATE_COPY(
                        CreateNode{[](const auto& test) {
                            auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
                            groupNode->addChild(test.createBrushNode());
                            return groupNode;
                        }},
                        CreateNode{[](const auto&) { return new Model::EntityNode{Model::Entity{}}; }},
                        CreateNode{[](const auto& test) { return test.createBrushNode(); }},
                        CreateNode{[](const auto& test) { return test.createPatchNode(); }}
                    );

                    auto* nodeToIsolate = createNode(*this);
                    document->addNodes({{document->parentForNodes(), {nodeToIsolate}}});

                    REQUIRE(!nodeToIsolate->hidden());

                    WHEN("The node is isolated") {
                        document->select(nodeToIsolate);

                        const auto selectedNodes = document->selectedNodes().nodes();
                        document->isolate();

                        THEN("The node is isolated and selected") {
                            CHECK_FALSE(nodeToIsolate->hidden());
                            CHECK(nodeToHide->hidden());
                            CHECK(nodeToIsolate->selected());
                        }

                        AND_WHEN("The operation is undone") {
                            document->undoCommand();

                            THEN("All nodes are visible again and selection is restored") {
                                CHECK_FALSE(nodeToIsolate->hidden());
                                CHECK_FALSE(nodeToHide->hidden());

                                CHECK_THAT(document->selectedNodes().nodes(), Catch::Matchers::UnorderedEquals(selectedNodes));
                            }
                        }
                    }
                }

                AND_GIVEN("A top level brush entity") {
                    auto* childNode1 = createBrushNode();
                    auto* childNode2 = createPatchNode();

                    auto* entityNode = new Model::EntityNode{Model::Entity{}};
                    entityNode->addChildren({childNode1, childNode2});

                    document->addNodes({{document->parentForNodes(), {entityNode}}});

                    // Check initial state
                    REQUIRE_FALSE(nodeToHide->hidden());
                    REQUIRE_FALSE(entityNode->hidden());
                    REQUIRE_FALSE(childNode1->hidden());
                    REQUIRE_FALSE(childNode2->hidden());

                    WHEN("Any child node is isolated") {
                        const auto [selectChild1, selectChild2] = GENERATE(
                            std::make_tuple(true, true),
                            std::make_tuple(true, false),
                            std::make_tuple(false, true)
                        );

                        if (selectChild1) {
                            document->select(childNode1);
                        }
                        if (selectChild2) {
                            document->select(childNode2);
                        }
                        REQUIRE_FALSE(entityNode->selected());

                        const auto selectedNodes = document->selectedNodes().nodes();
                        document->isolate();

                        // https://github.com/TrenchBroom/TrenchBroom/issues/3117
                        THEN("The containining entity node is visible") {
                            CHECK(!entityNode->hidden());

                            AND_THEN("The top level node is hidden") {
                                CHECK(nodeToHide->hidden());
                            }

                            AND_THEN("Any selected child node is visible and selected") {
                                CHECK(childNode1->hidden() != selectChild1);
                                CHECK(childNode2->hidden() != selectChild2);
                                CHECK(childNode1->selected() == selectChild1);
                                CHECK(childNode2->selected() == selectChild2);
                            }
                        }

                        AND_WHEN("The operation is undone") {
                            document->undoCommand();

                            THEN("All nodes are visible and selection is restored") {
                                CHECK_FALSE(nodeToHide->hidden());
                                CHECK_FALSE(entityNode->hidden());
                                CHECK_FALSE(childNode1->hidden());
                                CHECK_FALSE(childNode2->hidden());

                                CHECK_THAT(document->selectedNodes().nodes(), Catch::Matchers::UnorderedEquals(selectedNodes));
                            }
                        }
                    }
                }
            }
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
                [&](Model::BrushNode* b)                     { issues = kdl::vec_concat(std::move(issues), b->issues(issueGenerators)); },
                [&](Model::PatchNode* p)                     { issues = kdl::vec_concat(std::move(issues), p->issues(issueGenerators)); }
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

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.updateSpawnflagOnBrushEntity") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* brushNode = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            addNode(*document, document->parentForNodes(), brushNode);

            document->selectAllNodes();

            Model::EntityNode* brushEntNode = document->createBrushEntity(m_brushEntityDef);
            REQUIRE_THAT(document->selectedNodes().nodes(), Catch::UnorderedEquals(std::vector<Model::Node*>{brushNode}));

            REQUIRE(!brushEntNode->entity().hasProperty("spawnflags"));
            CHECK(document->updateSpawnflag("spawnflags", 1, true));

            REQUIRE(brushEntNode->entity().hasProperty("spawnflags"));
            CHECK(*brushEntNode->entity().property("spawnflags") == "2");
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
            addNode(*document, document->world(), layerNode);
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
            addNode(*document, document->world(), layerNode1);
            addNode(*document, document->world(), layerNode2);

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
            addNode(*document, document->world(), layerNode1);
            addNode(*document, document->world(), layerNode2);

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
            addNode(*document, document->world(), layerNode1);
            addNode(*document, document->world(), layerNode2);

            document->setCurrentLayer(layerNode1);

            // Create an entity in layer1
            Model::EntityNode* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity1->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 1u);

            CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
            CHECK(entity1->visible());

            // Hide layer1. If any nodes in the layer were Visibility_Shown they would be reset to Visibility_Inherited
            document->hideLayers({ layerNode1});

            CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
            CHECK(!entity1->visible());

            // Create another entity in layer1. It will be visible, while entity1 will still be hidden.
            Model::EntityNode* entity2 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity2->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 2u);

            CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
            CHECK(!entity1->visible());
            CHECK(entity2->visibilityState() == Model::VisibilityState::Shown);
            CHECK(entity2->visible());

            // Change to layer2. This hides all objects in layer1
            document->setCurrentLayer(layerNode2);

            CHECK(document->currentLayer() == layerNode2);
            CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
            CHECK(!entity1->visible());
            CHECK(entity2->visibilityState() == Model::VisibilityState::Inherited);
            CHECK(!entity2->visible());

            // Undo (Switch current layer back to layer1)
            document->undoCommand();

            CHECK(document->currentLayer() == layerNode1);
            CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
            CHECK(!entity1->visible());
            CHECK(entity2->visibilityState() == Model::VisibilityState::Shown);
            CHECK(entity2->visible());

            // Undo (entity2 creation)
            document->undoCommand();

            CHECK(layerNode1->childCount() == 1u);
            CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
            CHECK(!entity1->visible());

            // Undo (hiding layer1)
            document->undoCommand();

            CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
            CHECK(entity1->visible());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.duplicatedObjectInHiddenLayerIsVisible", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::LayerNode* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
            addNode(*document, document->world(), layerNode1);

            document->setCurrentLayer(layerNode1);
            document->hideLayers({ layerNode1});

            // Create entity1 and brush1 in the hidden layer1
            Model::EntityNode* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            Model::BrushNode* brush1 = createBrushNode();
            addNode(*document, document->parentForNodes(), brush1);

            CHECK(entity1->parent() == layerNode1);
            CHECK(brush1->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 2u);

            CHECK(entity1->visibilityState() == Model::VisibilityState::Shown);
            CHECK(brush1->visibilityState() == Model::VisibilityState::Shown);
            CHECK(entity1->visible());
            CHECK(brush1->visible());

            document->select({entity1, brush1});

            // Duplicate entity1 and brush1
            document->duplicateObjects();
            REQUIRE(document->selectedNodes().entityCount() == 1u);
            REQUIRE(document->selectedNodes().brushCount() == 1u);
            Model::EntityNode* entity2 = document->selectedNodes().entities().front();
            Model::BrushNode* brush2 =  document->selectedNodes().brushes().front();

            CHECK(entity2 != entity1);
            CHECK(brush2 != brush1);

            CHECK(entity2->visibilityState() == Model::VisibilityState::Shown);
            CHECK(entity2->visible());

            CHECK(brush2->visibilityState() == Model::VisibilityState::Shown);
            CHECK(brush2->visible());
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.newObjectsInLockedLayerAreUnlocked", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            auto* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
            auto* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
            addNode(*document, document->world(), layerNode1);
            addNode(*document, document->world(), layerNode2);

            document->setCurrentLayer(layerNode1);

            // Create an entity in layer1
            auto* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity1->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 1u);

            CHECK(entity1->lockState() == Model::LockState::Inherited);
            CHECK(!entity1->locked());

            // Lock layer1
            document->lock({ layerNode1});

            CHECK(entity1->lockState() == Model::LockState::Inherited);
            CHECK(entity1->locked());

            // Create another entity in layer1. It will be unlocked, while entity1 will still be locked (inherited).
            auto* entity2 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
            CHECK(entity2->parent() == layerNode1);
            CHECK(layerNode1->childCount() == 2u);

            CHECK(entity1->lockState() == Model::LockState::Inherited);
            CHECK(entity1->locked());
            CHECK(entity2->lockState() == Model::LockState::Unlocked);
            CHECK(!entity2->locked());

            // Change to layer2. This causes the Lock_Unlocked objects in layer1 to be degraded to Lock_Inherited
            // (i.e. everything in layer1 becomes locked)
            document->setCurrentLayer(layerNode2);

            CHECK(document->currentLayer() == layerNode2);
            CHECK(entity1->lockState() == Model::LockState::Inherited);
            CHECK(entity1->locked());
            CHECK(entity2->lockState() == Model::LockState::Inherited);
            CHECK(entity2->locked());

            // Undo (Switch current layer back to layer1)
            document->undoCommand();

            CHECK(document->currentLayer() == layerNode1);
            CHECK(entity1->lockState() == Model::LockState::Inherited);
            CHECK(entity1->locked());
            CHECK(entity2->lockState() == Model::LockState::Unlocked);
            CHECK(!entity2->locked());

            // Undo entity2 creation
            document->undoCommand();

            CHECK(layerNode1->childCount() == 1u);
            CHECK(entity1->lockState() == Model::LockState::Inherited);
            CHECK(entity1->locked());

            // Undo locking layer1
            document->undoCommand();

            CHECK(entity1->lockState() == Model::LockState::Inherited);
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

            addNode(*document, document->world(), layerNode0);
            addNode(*document, document->world(), layerNode1);
            addNode(*document, document->world(), layerNode2);

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

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.moveSelectionToLayer", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            auto* customLayer = new Model::LayerNode(Model::Layer("layer"));
            addNode(*document, document->world(), customLayer);

            auto* defaultLayer = document->world()->defaultLayer();

            GIVEN("A top level node") {
                using CreateNode = std::function<Model::Node*(const MapDocumentTest&)>;
                const auto createNode = GENERATE_COPY(
                    CreateNode{[](const auto& test) {
                        auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
                        groupNode->addChild(test.createBrushNode());
                        return groupNode;
                    }},
                    CreateNode{[](const auto&) { return new Model::EntityNode{Model::Entity{}}; }},
                    CreateNode{[](const auto& test) { return test.createBrushNode(); }},
                    CreateNode{[](const auto& test) { return test.createPatchNode(); }}
                );

                auto* node = createNode(*this);
                document->addNodes({{document->parentForNodes(), {node}}});
                
                REQUIRE(Model::findContainingLayer(node) == defaultLayer);
                
                WHEN("The node is moved to another layer") {
                    document->select(node);
                    document->moveSelectionToLayer(customLayer);

                    THEN("The group node is in the target layer") {
                        CHECK(Model::findContainingLayer(node) == customLayer);
                        
                        AND_THEN("The node is selected") {
                            CHECK(document->selectedNodes().nodes() == std::vector<Model::Node*>{node});
                        }
                    }

                    AND_WHEN("The operation is undone") {
                        document->undoCommand();

                        THEN("The node is back in the original layer") {
                            CHECK(Model::findContainingLayer(node) == defaultLayer);

                            AND_THEN("The node is selected") {
                                CHECK(document->selectedNodes().nodes() == std::vector<Model::Node*>{node});
                            }
                        }
                    }
                }
            }

            GIVEN("A brush entity node") {
                auto* entityNode = new Model::EntityNode{Model::Entity{}};
                auto* childNode1 = createBrushNode();
                auto* childNode2 = createPatchNode();

                entityNode->addChildren({childNode1, childNode2});
                document->addNodes({{document->parentForNodes(), {entityNode}}});

                REQUIRE(Model::findContainingLayer(entityNode) == defaultLayer);

                WHEN("Any child node is selected and moved to another layer") {
                    const auto [selectChild1, selectChild2] = GENERATE(
                        std::make_tuple(true, true),
                        std::make_tuple(true, false),
                        std::make_tuple(false, true)
                    );

                    if (selectChild1) {
                        document->select(childNode1);
                    }
                    if (selectChild2) {
                        document->select(childNode2);
                    }

                    const auto selectedNodes = document->selectedNodes().nodes();
                    document->moveSelectionToLayer(customLayer);

                    THEN("The brush entity node is moved to the target layer") {
                        CHECK(Model::findContainingLayer(entityNode) == customLayer);
                        CHECK(childNode1->parent() == entityNode);
                        CHECK(childNode2->parent() == entityNode);
                        
                        AND_THEN("The child nodes are selected") {
                            CHECK(document->selectedNodes().nodes() == entityNode->children());
                        }
                    }

                    AND_WHEN("The operation is undone") {
                        document->undoCommand();

                        THEN("The brush entity node is back in the original layer") {
                            CHECK(Model::findContainingLayer(entityNode) == defaultLayer);
                            CHECK(childNode1->parent() == entityNode);
                            CHECK(childNode2->parent() == entityNode);

                            AND_THEN("The originally selected nodes are selected") {
                                CHECK_THAT(document->selectedNodes().nodes(), Catch::Matchers::UnorderedEquals(selectedNodes));
                            }
                        }
                    }
                }
            }
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.setCurrentLayerCollation", "[LayerTest]") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            auto* defaultLayerNode = document->world()->defaultLayer();
            auto* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
            auto* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
            addNode(*document, document->world(), layerNode1);
            addNode(*document, document->world(), layerNode2);
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

        TEST_CASE("MapDocumentTest.detectValveFormatMap", "[MapDocumentTest]") {
            auto [document, game, gameConfig] = View::loadMapDocument(IO::Path("fixture/test/View/MapDocumentTest/valveFormatMapWithoutFormatTag.map"),
                                                                      "Quake", Model::MapFormat::Unknown);
            CHECK(document->world()->mapFormat() == Model::MapFormat::Valve);
            CHECK(document->world()->defaultLayer()->childCount() == 1);
        }

        TEST_CASE("MapDocumentTest.detectStandardFormatMap", "[MapDocumentTest]") {
            auto [document, game, gameConfig] = View::loadMapDocument(IO::Path("fixture/test/View/MapDocumentTest/standardFormatMapWithoutFormatTag.map"),
                                                                      "Quake", Model::MapFormat::Unknown);
            CHECK(document->world()->mapFormat() == Model::MapFormat::Standard);
            CHECK(document->world()->defaultLayer()->childCount() == 1);
        }

        TEST_CASE("MapDocumentTest.detectEmptyMap", "[MapDocumentTest]") {
            auto [document, game, gameConfig] = View::loadMapDocument(IO::Path("fixture/test/View/MapDocumentTest/emptyMapWithoutFormatTag.map"),
                                                                      "Quake", Model::MapFormat::Unknown);
            // an empty map detects as Valve because Valve is listed first in the Quake game config
            CHECK(document->world()->mapFormat() == Model::MapFormat::Valve);
            CHECK(document->world()->defaultLayer()->childCount() == 0);
        }

        TEST_CASE("MapDocumentTest.mixedFormats", "[MapDocumentTest]") {
            // map has both Standard and Valve brushes
            CHECK_THROWS_AS(View::loadMapDocument(IO::Path("fixture/test/View/MapDocumentTest/mixedFormats.map"),
                                                  "Quake", Model::MapFormat::Unknown), IO::WorldReaderException);
        }

        void checkTransformation(const Model::Node& node, const Model::Node& original, const vm::mat4x4d& transformation) {
            CHECK(node.physicalBounds() == original.physicalBounds().transform(transformation));

            REQUIRE(node.childCount() == original.childCount());
            for (const auto [nodeChild, originalChild] : kdl::make_zip_range(node.children(), original.children())) {
                checkTransformation(*nodeChild, *originalChild, transformation);
            }
        }

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.transformObjects") {
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
    }
}
