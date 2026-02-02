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

#include "mdl/Map_Brushes.h"

#include "Logger.h"
#include "mdl/ApplyAndSwap.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Transaction.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{

bool createBrush(Map& map, const std::vector<vm::vec3d>& points)
{
  const auto builder = BrushBuilder{
    map.worldNode().mapFormat(),
    map.worldBounds(),
    map.gameInfo().gameConfig.faceAttribsConfig.defaults};

  return builder.createBrush(points, map.currentMaterialName())
         | kdl::and_then([&](auto b) -> Result<void> {
             auto* brushNode = new BrushNode{std::move(b)};

             auto transaction = Transaction{map, "Create Brush"};
             deselectAll(map);
             if (addNodes(map, {{parentForNodes(map), {brushNode}}}).empty())
             {
               transaction.cancel();
               return Error{"Could not add brush to document"};
             }
             selectNodes(map, {brushNode});
             if (!transaction.commit())
             {
               return Error{"Could not add brush to document"};
             }

             return kdl::void_success;
           })
         | kdl::if_error(
           [&](auto e) { map.logger().error() << "Could not create brush: " << e.msg; })
         | kdl::is_success();
}

bool setBrushFaceAttributes(Map& map, const UpdateBrushFaceAttributes& update)
{
  return applyAndSwap(
    map, "Change Face Attributes", map.selection().allBrushFaces(), [&](auto& brushFace) {
      evaluate(update, brushFace);
      return true;
    });
}

bool copyUV(
  Map& map,
  const UVCoordSystemSnapshot& coordSystemSnapshot,
  const BrushFaceAttributes& attribs,
  const vm::plane3d& sourceFacePlane,
  const WrapStyle wrapStyle)
{
  return applyAndSwap(
    map, "Copy UV Alignment", map.selection().brushFaces, [&](auto& face) {
      face.copyUVCoordSystemFromFace(
        coordSystemSnapshot, attribs, sourceFacePlane, wrapStyle);
      return true;
    });
}

bool translateUV(
  Map& map,
  const vm::vec3f& cameraUp,
  const vm::vec3f& cameraRight,
  const vm::vec2f& delta)
{
  return applyAndSwap(map, "Translate UV", map.selection().brushFaces, [&](auto& face) {
    face.moveUV(vm::vec3d{cameraUp}, vm::vec3d{cameraRight}, delta);
    return true;
  });
}

bool rotateUV(Map& map, const float angle)
{
  return applyAndSwap(map, "Rotate UV", map.selection().brushFaces, [&](auto& face) {
    face.rotateUV(angle);
    return true;
  });
}

bool shearUV(Map& map, const vm::vec2f& factors)
{
  return applyAndSwap(map, "Shear UV", map.selection().brushFaces, [&](auto& face) {
    face.shearUV(factors);
    return true;
  });
}

bool flipUV(
  Map& map,
  const vm::vec3f& cameraUp,
  const vm::vec3f& cameraRight,
  const vm::direction cameraRelativeFlipDirection)
{
  const bool isHFlip =
    (cameraRelativeFlipDirection == vm::direction::left
     || cameraRelativeFlipDirection == vm::direction::right);
  return applyAndSwap(
    map,
    isHFlip ? "Flip UV Horizontally" : "Flip UV Vertically",
    map.selection().brushFaces,
    [&](auto& face) {
      face.flipUV(
        vm::vec3d{cameraUp}, vm::vec3d{cameraRight}, cameraRelativeFlipDirection);
      return true;
    });
}

void alignUV(Map& map, const UvPolicy uvPolicy)
{
  auto& selection = map.selection();
  contract_assert(selection.brushFaces.size() == 1);

  const auto& brushFace = selection.brushFaces.front().face();
  setBrushFaceAttributes(map, mdl::align(brushFace, uvPolicy));
}

void justifyUV(
  Map& map, const UvAxis uvAxis, const UvSign uvSign, const UvPolicy uvPolicy)
{
  auto& selection = map.selection();
  contract_assert(selection.brushFaces.size() == 1);

  const auto& brushFace = selection.brushFaces.front().face();
  setBrushFaceAttributes(map, mdl::justify(brushFace, uvAxis, uvSign, uvPolicy));
}

template <typename F>
void withInvariantUvCoords(
  Map& map,
  const BrushFaceHandle faceHandle,
  const std::optional<vm::vec3d>& vertex,
  const F& f)
{
  if (vertex)
  {
    const auto previousUvCoords = vm::vec2f{
      faceHandle.face().toUVCoordSystemMatrix(
        faceHandle.face().attributes().offset(), faceHandle.face().attributes().scale())
      * *vertex};

    f(faceHandle.face());

    const auto newUvCoords = vm::vec2f{
      faceHandle.face().toUVCoordSystemMatrix(
        faceHandle.face().attributes().offset(), faceHandle.face().attributes().scale())
      * *vertex};
    const auto delta = previousUvCoords - newUvCoords;

    setBrushFaceAttributes(
      map,
      {
        .xOffset = mdl::AddValue{delta.x()},
        .yOffset = mdl::AddValue{delta.y()},
      });
  }
  else
  {
    f(faceHandle.face());
  }
}

void fitUV(Map& map, const UvAxis uvAxis, const UvSign uvSign, const UvPolicy uvPolicy)
{
  auto& selection = map.selection();
  contract_assert(selection.brushFaces.size() == 1);

  const auto faceHandle = selection.brushFaces.front();

  const auto invariantVertex = anchorVertex(faceHandle.face(), uvAxis, uvSign);
  withInvariantUvCoords(map, faceHandle, invariantVertex, [&](const auto& brushFace) {
    setBrushFaceAttributes(map, mdl::fit(brushFace, uvAxis, uvPolicy));
  });
}

} // namespace tb::mdl
