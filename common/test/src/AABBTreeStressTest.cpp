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

#include "AABBTree.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/TestParserStatus.h"
#include "IO/WorldReader.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>

#include <iostream>

#include "Catch2.h"

namespace TrenchBroom {
    namespace Model {
        using AABB = AABBTree<double, 3, Node*>;
        using BOX = AABB::Box;

        static auto makeTreeBuilder(AABB& tree, vm::bbox3::builder& totalBounds) {
            const auto insert = [&](auto* node) {
                if (!tree.empty()) {
                    const auto oldBounds = tree.bounds();

                    tree.insert(node->physicalBounds(), node);
                    totalBounds.add(node->physicalBounds());

                    if (!tree.bounds().contains(oldBounds)) {
                        UNSCOPED_INFO("Node at line " << node->lineNumber() << " decreased tree bounds: " << oldBounds << " -> " << tree.bounds());
                        REQUIRE(tree.bounds().contains(oldBounds));
                    }
                } else {
                    tree.insert(node->physicalBounds(), node);
                    totalBounds.add(node->physicalBounds());
                }

                if (!tree.contains(node)) {
                    tree.print(std::cout);
                    UNSCOPED_INFO("Node " << node << " with bounds " << node->physicalBounds() << " at line " << node->lineNumber() << " not found in tree after insertion");
                    REQUIRE(tree.contains(node));
                }

                UNSCOPED_INFO("Node at line " << node->lineNumber() << " mangled tree bounds");
                REQUIRE(totalBounds.bounds() == tree.bounds());
            };

            return kdl::overload(
                [] (auto&& thisLambda, WorldNode* world)   { world->visitChildren(thisLambda); },
                [] (auto&& thisLambda, LayerNode* layer)   { layer->visitChildren(thisLambda); },
                [] (auto&& thisLambda, GroupNode* group)   { group->visitChildren(thisLambda); },
                [=](auto&& thisLambda, EntityNode* entity) { // capture insert helper lambda by value!
                    insert(entity);
                    entity->visitChildren(thisLambda);
                },
                [=](BrushNode* brush) {
                    insert(brush);
                }
            );
        }

        TEST_CASE("AABBTreeStressTest.parseMapTest", "[AABBTreeStressTest]") {
            const auto mapPath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/test/IO/Map/rtz_q1.map");
            const auto file = IO::Disk::openFile(mapPath);
            auto fileReader = file->reader().buffer();

            IO::TestParserStatus status;
            IO::WorldReader worldReader(fileReader.stringView(), Model::MapFormat::Standard);

            const auto worldBounds = vm::bbox3(8192.0);
            auto world = worldReader.read(worldBounds, status);

            AABB tree;
            vm::bbox3::builder totalBounds;
            world->accept(makeTreeBuilder(tree, totalBounds));
        }
    }
}

