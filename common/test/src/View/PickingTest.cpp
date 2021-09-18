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

#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"
#include "Model/WorldNode.h"
#include "Model/PickResult.h"
#include "View/SelectionTool.h"

#include <vecmath/approx.h>
#include <vecmath/ray.h>
#include <vecmath/ray_io.h>

#include <kdl/result.h>

#include <vector>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        TEST_CASE_METHOD(MapDocumentTest, "PickingTest.pickSingleBrush") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            addNode(*document, document->parentForNodes(), brushNode1);

            Model::PickResult pickResult;
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.all();
            CHECK(hits.size() == 1u);

            const auto& brush1 = brushNode1->brush();
            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::neg_x()), pickResult);
            CHECK(pickResult.all().empty());
        }

        TEST_CASE_METHOD(MapDocumentTest, "PickingTest.pickSingleEntity") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            Model::EntityNode* ent1 = new Model::EntityNode{Model::Entity{}};
            addNode(*document, document->parentForNodes(), ent1);

            const auto origin = ent1->entity().origin();
            const auto bounds = ent1->logicalBounds();

            const auto rayOrigin = origin + vm::vec3(-32.0, bounds.size().y() / 2.0, bounds.size().z() / 2.0);

            Model::PickResult pickResult;
            document->pick(vm::ray3(rayOrigin, vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.all();
            CHECK(hits.size() == 1u);

            CHECK(hits.front().target<Model::EntityNode*>() == ent1);
            CHECK(hits.front().distance() == vm::approx(32.0 - bounds.size().x() / 2.0));

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::neg_x()), pickResult);
            CHECK(pickResult.all().empty());
        }

        TEST_CASE_METHOD(MapDocumentTest, "PickingTest.pickSimpleGroup") {
            using namespace Model::HitFilters;

            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            addNode(*document, document->parentForNodes(), brushNode1);

            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture").value());
            addNode(*document, document->parentForNodes(), brushNode2);

            document->selectAllNodes();
            auto* group = document->groupSelection("test");

            Model::PickResult pickResult;
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            // picking a grouped object when the containing group is closed should return the object,
            // which is converted to the group when hitsToNodesWithGroupPicking() is used.
            auto hits = pickResult.all(type(Model::BrushNode::BrushHitType));
            CHECK(hits.size() == 1u);

            const auto& brush1 = brushNode1->brush();
            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));

            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ group }));

            // hitting both objects in the group should return the group only once
            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(32, 32, -32), vm::vec3::pos_z()), pickResult);

            hits = pickResult.all(type(Model::BrushNode::BrushHitType));
            CHECK(hits.size() == 2u);

            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ group }));

            // hitting the group bounds doesn't count as a hit
            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 96), vm::vec3::pos_x()), pickResult);

            hits = pickResult.all(type(Model::BrushNode::BrushHitType));
            CHECK(hits.empty());

            // hitting a grouped object when the containing group is open should return the object only
            document->openGroup(group);

            pickResult.clear();
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            hits = pickResult.all(type(Model::BrushNode::BrushHitType));
            CHECK(hits.size() == 1u);

            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));

            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ brushNode1 }));
        }

        TEST_CASE_METHOD(MapDocumentTest, "PickingTest.pickNestedGroup") {
            using namespace Model::HitFilters;

            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            addNode(*document, document->parentForNodes(), brushNode1);

            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture").value());
            addNode(*document, document->parentForNodes(), brushNode2);

            document->selectAllNodes();
            auto* innerGroup = document->groupSelection("inner");

            document->deselectAll();
            auto* brushNode3 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 256)), "texture").value());
            addNode(*document, document->parentForNodes(), brushNode3);

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

            auto hits = pickResult.all(type(Model::BrushNode::BrushHitType));
            CHECK(hits.size() == 1u);

            const auto& brush3 = brushNode3->brush();
            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush3.face(*brush3.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));

            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ brushNode3 }));

            // hitting the brush in the inner group should return the inner group when hitsToNodesWithGroupPicking() is used
            pickResult.clear();
            document->pick(lowRay, pickResult);

            hits = pickResult.all(type(Model::BrushNode::BrushHitType));
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

            hits = pickResult.all(type(Model::BrushNode::BrushHitType));
            CHECK(hits.size() == 1u);

            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush3.face(*brush3.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));
            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ brushNode3 }));

            // pick a brush in the inner group
            pickResult.clear();
            document->pick(lowRay, pickResult);

            hits = pickResult.all(type(Model::BrushNode::BrushHitType));
            CHECK(hits.size() == 1u);

            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));
            CHECK_THAT(hitsToNodesWithGroupPicking(hits), Catch::Equals(std::vector<Model::Node*>{ brushNode1 }));
        }

        TEST_CASE_METHOD(MapDocumentTest, "PickingTest.pickBrushEntity") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            addNode(*document, document->parentForNodes(), brushNode1);

            auto* brushNode2 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)).translate(vm::vec3(0, 0, 128)), "texture").value());
            addNode(*document, document->parentForNodes(), brushNode2);

            document->selectAllNodes();

            document->createBrushEntity(m_brushEntityDef);
            document->deselectAll();

            Model::PickResult pickResult;

            // picking entity brushes should only return the brushes and not the entity
            document->pick(vm::ray3(vm::vec3(-32, 0, 0), vm::vec3::pos_x()), pickResult);

            auto hits = pickResult.all();
            CHECK(hits.size() == 1u);

            const auto& brush1 = brushNode1->brush();
            CHECK(Model::hitToFaceHandle(hits.front())->face() == brush1.face(*brush1.findFace(vm::vec3::neg_x())));
            CHECK(hits.front().distance() == vm::approx(32.0));
        }
    }
}
