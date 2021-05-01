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

        TEST_CASE_METHOD(MapDocumentTest, "MapDocumentTest.throwExceptionDuringCommand") {
            CHECK_THROWS_AS(document->throwExceptionDuringCommand(), CommandProcessorException);
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
    }
}
