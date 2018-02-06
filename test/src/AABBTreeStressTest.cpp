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

#include <gtest/gtest.h>

#include "AABBTree.h"
#include "BBox.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"
#include "IO/TestParserStatus.h"
#include "IO/WorldReader.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        using AABB = AABBTree<double, 3, Node*>;
        using BOX = AABB::Box;

        class TreeBuilder : public NodeVisitor {
        private:
            AABB& m_tree;
        public:
            TreeBuilder(AABB& tree) : m_tree(tree) {}
        private:
            void doVisit(World* world) override {}
            void doVisit(Layer* layer) override {}
            void doVisit(Group* group) override {}
            void doVisit(Entity* entity) override {
                m_tree.insert(entity->bounds(), entity);
            }
            void doVisit(Brush* brush) override {
                m_tree.insert(brush->bounds(), brush);
            }
        };

        TEST(AABBTreeTest, parseMapTest) {
            const auto mapPath = IO::Disk::getCurrentWorkingDir() + IO::Path("data/IO/Map/rtz_q1.map");
            auto file = IO::Disk::openFile(mapPath);

            IO::TestParserStatus status;
            IO::WorldReader reader(file->begin(), file->end(), nullptr);

            BBox3 worldBounds(8192);
            auto* world = reader.read(Model::MapFormat::Standard, worldBounds, status);

            AABB tree;
            TreeBuilder builder(tree);
            world->acceptAndRecurse(builder);

            delete world;
        }
    }
}

