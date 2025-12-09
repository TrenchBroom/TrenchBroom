/*
 Copyright (C) 2025 Kristian Duske

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

#include "TestFactory.h"

#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Game.h"
#include "mdl/GameInfo.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{

BrushNode* createBrushNode(
  const Map& map,
  const std::string& materialName,
  const std::function<void(Brush&)>& brushFunc)
{
  const auto& worldNode = map.worldNode();
  auto builder = BrushBuilder{
    worldNode.mapFormat(),
    map.worldBounds(),
    map.game().config().faceAttribsConfig.defaults};

  auto brush = builder.createCube(32.0, materialName) | kdl::value();
  brushFunc(brush);
  return new BrushNode(std::move(brush));
}

PatchNode* createPatchNode(const std::string& materialName)
{
  // clang-format off
  return new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, materialName}};
  // clang-format on
}

} // namespace tb::mdl
