/*
 Copyright (C) 2010 Kristian Duske

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

#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/LayerNode.h"
#include "mdl/MapFormat.h"
#include "mdl/Tag.h"
#include "mdl/WorldNode.h"

#include "kdl/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("TaggingTest.testTagBrush")
{
  const auto worldBounds = vm::bbox3d{4096.0};
  auto worldNode = WorldNode{{}, {}, MapFormat::Standard};

  auto builder = BrushBuilder{MapFormat::Standard, worldBounds};
  auto* brushNode = new BrushNode{
    builder.createCube(64.0, "left", "right", "front", "back", "top", "bottom")
    | kdl::value()};

  worldNode.defaultLayer()->addChild(brushNode);

  auto tag1 = Tag{"tag1", {}};
  auto tag2 = Tag{"tag2", {}};

  tag1.setIndex(0);
  tag2.setIndex(1);

  CHECK_FALSE(brushNode->hasTag(tag1));
  CHECK_FALSE(brushNode->hasTag(tag2));

  CHECK(brushNode->addTag(tag1));
  CHECK_FALSE(brushNode->addTag(tag1));

  CHECK(brushNode->hasTag(tag1));
  CHECK_FALSE(brushNode->hasTag(tag2));

  CHECK(brushNode->removeTag(tag1));
  CHECK_FALSE(brushNode->removeTag(tag1));

  CHECK_FALSE(brushNode->hasTag(tag1));
  CHECK_FALSE(brushNode->hasTag(tag2));
}

} // namespace tb::mdl
