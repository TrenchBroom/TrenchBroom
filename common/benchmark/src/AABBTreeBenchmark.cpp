/*
 Copyright (C) 2019 Kristian Duske

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

#include <catch2/catch.hpp>

#include "../../test/src/GTestCompat.h"

#include "BenchmarkUtils.h"

#include "AABBTree.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/Reader.h"
#include "IO/TestParserStatus.h"
#include "IO/WorldReader.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/EntityNode.h"
#include "Model/NodeVisitor.h"
#include "Model/WorldNode.h"

#include <vecmath/bbox.h>

namespace TrenchBroom {
    using AABB = AABBTree<double, 3, Model::Node*>;
    using BOX = AABB::Box;

    class TreeBuilder : public Model::NodeVisitor {
    private:
        AABB& m_tree;
    public:
        explicit TreeBuilder(AABB& tree) : m_tree(tree) {}
    private:
        void doVisit(Model::WorldNode*) override {}
        void doVisit(Model::LayerNode*) override {}
        void doVisit(Model::GroupNode*) override {}
        void doVisit(Model::EntityNode* entity) override {
            doInsert(entity);
        }
        void doVisit(Model::BrushNode* brush) override {
            doInsert(brush);
        }

        void doInsert(Model::Node* node) {
            m_tree.insert(node->physicalBounds(), node);
        }
    };

    TEST_CASE("AABBTreeBenchmark.benchBuildTree", "[AABBTreeBenchmark]") {
        const auto mapPath = IO::Disk::getCurrentWorkingDir() + IO::Path("fixture/benchmark/AABBTree/ne_ruins.map");
        const auto file = IO::Disk::openFile(mapPath);
        auto fileReader = file->reader().buffer();

        IO::TestParserStatus status;
        IO::WorldReader worldReader(std::begin(fileReader), std::end(fileReader));

        const vm::bbox3 worldBounds(8192.0);
        auto world = worldReader.read(Model::MapFormat::Standard, worldBounds, status);

        std::vector<AABB> trees(100);
        timeLambda([&world, &trees]() {
            for (auto& tree : trees) {
                TreeBuilder builder(tree);
                world->acceptAndRecurse(builder);
            }
        }, "Add objects to AABB tree");
    }
}
