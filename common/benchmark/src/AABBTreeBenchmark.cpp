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

#include "AABBTree.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/Reader.h"
#include "IO/TestParserStatus.h"
#include "IO/WorldReader.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/EntityProperties.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>

#include <vecmath/bbox.h>

#include "../../test/src/Catch2.h"
#include "BenchmarkUtils.h"

namespace TrenchBroom
{
using AABB = AABBTree<double, 3, Model::Node*>;
using BOX = AABB::Box;

TEST_CASE("AABBTreeBenchmark.benchBuildTree", "[AABBTreeBenchmark]")
{
  const auto mapPath = IO::Disk::getCurrentWorkingDir()
                       + IO::Path("fixture/benchmark/AABBTree/ne_ruins.map");
  const auto file = IO::Disk::openFile(mapPath);
  auto fileReader = file->reader().buffer();

  IO::TestParserStatus status;
  IO::WorldReader worldReader(fileReader.stringView(), Model::MapFormat::Standard, {});

  const vm::bbox3 worldBounds(8192.0);
  auto world = worldReader.read(worldBounds, status);

  std::vector<AABB> trees(100);
  timeLambda(
    [&world, &trees]() {
      for (auto& tree : trees)
      {
        world->accept(kdl::overload(
          [](auto&& thisLambda, Model::WorldNode* world_) {
            world_->visitChildren(thisLambda);
          },
          [](auto&& thisLambda, Model::LayerNode* layer) {
            layer->visitChildren(thisLambda);
          },
          [](auto&& thisLambda, Model::GroupNode* group) {
            group->visitChildren(thisLambda);
          },
          [&](auto&& thisLambda, Model::EntityNode* entity) {
            entity->visitChildren(thisLambda);
            tree.insert(entity->physicalBounds(), entity);
          },
          [&](Model::BrushNode* brush) { tree.insert(brush->physicalBounds(), brush); },
          [&](Model::PatchNode* patch) { tree.insert(patch->physicalBounds(), patch); }));
      }
    },
    "Add objects to AABB tree");
}
} // namespace TrenchBroom
