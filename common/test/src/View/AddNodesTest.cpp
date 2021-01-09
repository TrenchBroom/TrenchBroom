/*
 Copyright (C) 2020 Kristian Duske

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

#include "Model/BrushNode.h"
#include "Model/GroupNode.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        class AddNodesTest : public MapDocumentTest {};

        TEST_CASE_METHOD(AddNodesTest, "AddNodesTest.connectAddedSingletonGroups") {
            auto* group = new Model::GroupNode{Model::Group{"group"}};
            
            document->addNodes({{document->parentForNodes(), {group}}});
            CHECK(group->connectedToLinkSet());

            document->undoCommand();
            CHECK_FALSE(group->connectedToLinkSet());
        }

        TEST_CASE_METHOD(AddNodesTest, "AddNodesTest.recursivelyConnectAddedSingletonGroups") {
            auto* outer = new Model::GroupNode{Model::Group{"outer"}};
            auto* inner = new Model::GroupNode{Model::Group{"inner"}};
            outer->addChild(inner);
            
            document->addNodes({{document->parentForNodes(), {outer}}});
            CHECK(outer->connectedToLinkSet());
            CHECK(inner->connectedToLinkSet());

            document->undoCommand();
            CHECK_FALSE(outer->connectedToLinkSet());
            CHECK_FALSE(inner->connectedToLinkSet());
        }

        TEST_CASE_METHOD(AddNodesTest, "AddNodesTest.updateLinkedGroups") {
            auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
            auto* linkedGroupNode = static_cast<Model::GroupNode*>(groupNode->cloneRecursively(document->worldBounds()));
            groupNode->addToLinkSet(*linkedGroupNode);

            document->addNodes({{document->parentForNodes(), {groupNode, linkedGroupNode}}});
            document->deselectAll();
            document->select(linkedGroupNode);

            document->translateObjects(vm::vec3(32.0, 0.0, 0.0));

            document->deselectAll();

            auto* brushNode = createBrushNode();
            document->addNodes({{groupNode, {brushNode}}});

            REQUIRE(groupNode->childCount() == 1u);
            CHECK(linkedGroupNode->childCount() == 1u);
            
            auto* linkedBrushNode = dynamic_cast<Model::BrushNode*>(linkedGroupNode->children().front());
            CHECK(linkedBrushNode != nullptr);

            CHECK(linkedBrushNode->physicalBounds() == brushNode->physicalBounds().transform(linkedGroupNode->group().transformation()));

            document->undoCommand();
            REQUIRE(groupNode->childCount() == 0u);
            CHECK(linkedGroupNode->childCount() == 0u);

            document->redoCommand();
            REQUIRE(groupNode->childCount() == 1u);
            CHECK(linkedGroupNode->childCount() == 1u);
        }

        TEST_CASE_METHOD(AddNodesTest, "AddNodesTest.updateLinkedGroupsFails") {
            auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
            auto* linkedGroupNode = static_cast<Model::GroupNode*>(groupNode->cloneRecursively(document->worldBounds()));
            groupNode->addToLinkSet(*linkedGroupNode);

            document->addNodes({{document->parentForNodes(), {groupNode, linkedGroupNode}}});
            document->deselectAll();
            document->select(linkedGroupNode);

            // adding a brush to the linked group node will fail because it will go out of world bounds
            document->translateObjects(document->worldBounds().max);

            document->deselectAll();

            auto* brushNode = createBrushNode();
            CHECK(document->addNodes({{groupNode, {brushNode}}}).empty());

            CHECK(groupNode->childCount() == 0u);
            CHECK(linkedGroupNode->childCount() == 0u);
        }
    }
}
