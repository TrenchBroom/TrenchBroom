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
#include "mdl/UVUtils.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{
namespace
{

auto invertHorizontalAxis(
  const mdl::UvAxis horizontalUvAxis,
  const vm::vec3f& uAxis,
  const vm::vec3f& vAxis,
  const vm::vec3f& rightAxis)
{
  switch (horizontalUvAxis)
  {
  case mdl::UvAxis::u:
    return vm::dot(uAxis, rightAxis) >= 0.0f;
  case mdl::UvAxis::v:
    return vm::dot(vAxis, rightAxis) >= 0.0f;
    switchDefault();
  }
}

auto invertVerticalAxis(
  const mdl::UvAxis horizontalUvAxis,
  const vm::vec3f& uAxis,
  const vm::vec3f& vAxis,
  const vm::vec3f& upAxis)
{
  switch (horizontalUvAxis)
  {
  case mdl::UvAxis::u:
    return vm::dot(vAxis, upAxis) >= 0.0f;
  case mdl::UvAxis::v:
    return vm::dot(uAxis, upAxis) >= 0.0f;
    switchDefault();
  }
}

auto getDirectionAxes(
  const vm::vec3f& uAxis,
  const vm::vec3f& vAxis,
  const vm::vec3f& upAxis,
  const vm::vec3f& rightAxis)
{
  // Which UV axis corresponds is horizontal?
  const auto [horizontalUvAxis, verticalUvAxis] =
    vm::abs(vm::dot(uAxis, rightAxis)) >= vm::abs(vm::dot(vAxis, rightAxis))
      ? std::tuple{mdl::UvAxis::u, mdl::UvAxis::v}
      : std::tuple{mdl::UvAxis::v, mdl::UvAxis::u};

  const auto invertH = invertHorizontalAxis(horizontalUvAxis, uAxis, vAxis, rightAxis);
  const auto invertV = invertVerticalAxis(horizontalUvAxis, uAxis, vAxis, upAxis);

  return std::tuple{horizontalUvAxis, verticalUvAxis, invertH, invertV};
}

std::tuple<mdl::UvAxis, mdl::UvSign> convertJustifyDirection(
  const BrushFace& brushFace, const UvJustifyDirection uvJustifyDirection)
{
  const auto [cameraUp, cameraRight] = computeCameraAxesForFaceNormal(brushFace.normal());

  const auto uAxis = vm::vec3f{brushFace.uAxis()};
  const auto vAxis = vm::vec3f{brushFace.vAxis()};

  const auto [horizontalUvAxis, verticalUvAxis, invertH, invertV] =
    getDirectionAxes(uAxis, vAxis, vm::vec3f{cameraUp}, vm::vec3f{cameraRight});

  switch (uvJustifyDirection)
  {
  case UvJustifyDirection::Left:
    return std::tuple{horizontalUvAxis, invertH ? mdl::UvSign::minus : mdl::UvSign::plus};
  case UvJustifyDirection::Right:
    return std::tuple{horizontalUvAxis, invertH ? mdl::UvSign::plus : mdl::UvSign::minus};
  case UvJustifyDirection::Up:
    return std::tuple{verticalUvAxis, invertV ? mdl::UvSign::minus : mdl::UvSign::plus};
  case UvJustifyDirection::Down:
    return std::tuple{verticalUvAxis, invertV ? mdl::UvSign::plus : mdl::UvSign::minus};
    switchDefault();
  }
}

std::tuple<mdl::UvAxis, mdl::UvSign> convertFitDirection(
  const BrushFace& brushFace, const UvFitDirection fitDirection)
{
  const auto [cameraUp, cameraRight] = computeCameraAxesForFaceNormal(brushFace.normal());

  const auto uAxis = vm::vec3f{brushFace.uAxis()};
  const auto vAxis = vm::vec3f{brushFace.vAxis()};

  const auto [horizontalUvAxis, verticalUvAxis, invertH, invertV] =
    getDirectionAxes(uAxis, vAxis, vm::vec3f{cameraUp}, vm::vec3f{cameraRight});

  switch (fitDirection)
  {
  case UvFitDirection::Horizontal:
    return std::tuple{horizontalUvAxis, invertH ? mdl::UvSign::minus : mdl::UvSign::plus};
  case UvFitDirection::Vertical:
    return std::tuple{verticalUvAxis, invertV ? mdl::UvSign::minus : mdl::UvSign::plus};
    switchDefault();
  }
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

} // namespace

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
    face.translateUV(vm::vec3d{cameraUp}, vm::vec3d{cameraRight}, delta);
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
  auto transaction = Transaction{map, "Align Texture"};

  for (const auto& faceHandle : map.selection().brushFaces)
  {
    setBrushFaceAttributes(map, mdl::align(faceHandle.face(), uvPolicy));
  }

  transaction.commit();
}

void justifyUV(
  Map& map, const UvJustifyDirection uvJustifyDirection, const UvPolicy uvPolicy)
{
  auto transaction = Transaction{map, "Justify Texture"};

  for (const auto& faceHandle : map.selection().brushFaces)
  {
    const auto [uvAxis, uvSign] =
      convertJustifyDirection(faceHandle.face(), uvJustifyDirection);

    setBrushFaceAttributes(
      map, mdl::justify(faceHandle.face(), uvAxis, uvSign, uvPolicy));
  }

  transaction.commit();
}

void fitUV(Map& map, const UvFitDirection uvFitDirection, const UvPolicy uvPolicy)
{
  auto transaction = Transaction{map, "Fit Texture"};

  for (const auto& faceHandle : map.selection().brushFaces)
  {
    const auto [uvAxis, uvSign] = convertFitDirection(faceHandle.face(), uvFitDirection);

    const auto invariantVertex = anchorVertex(faceHandle.face(), uvAxis, uvSign);
    withInvariantUvCoords(map, faceHandle, invariantVertex, [&](const auto& brushFace) {
      setBrushFaceAttributes(map, mdl::fit(brushFace, uvAxis, uvPolicy));
    });
  }

  transaction.commit();
}

void autoFitUV(Map& map, const UvPolicy uvPolicy)
{
  auto transaction = Transaction{map, "Auto fit texture"};

  for (const auto& faceHandle : map.selection().brushFaces)
  {
    if (!isAligned(faceHandle.face()))
    {
      setBrushFaceAttributes(map, mdl::align(faceHandle.face(), UvPolicy::best));
    }
    else if (
      isJustified(faceHandle.face(), UvAxis::u, UvSign::plus)
      && isFitted(faceHandle.face(), UvAxis::u)
      && isJustified(faceHandle.face(), UvAxis::v, UvSign::plus)
      && isFitted(faceHandle.face(), UvAxis::v))
    {
      setBrushFaceAttributes(map, mdl::align(faceHandle.face(), uvPolicy));
    }

    setBrushFaceAttributes(
      map, mdl::justify(faceHandle.face(), UvAxis::u, UvSign::plus, UvPolicy::best));
    setBrushFaceAttributes(
      map, mdl::justify(faceHandle.face(), UvAxis::v, UvSign::plus, UvPolicy::best));

    const auto invariantUVertex =
      anchorVertex(faceHandle.face(), UvAxis::u, UvSign::plus);
    withInvariantUvCoords(map, faceHandle, invariantUVertex, [&](const auto& brushFace) {
      setBrushFaceAttributes(map, mdl::fit(brushFace, UvAxis::u, UvPolicy::best));
    });

    const auto invariantVVertex =
      anchorVertex(faceHandle.face(), UvAxis::v, UvSign::plus);
    withInvariantUvCoords(map, faceHandle, invariantVVertex, [&](const auto& brushFace) {
      setBrushFaceAttributes(map, mdl::fit(brushFace, UvAxis::v, UvPolicy::best));
    });
  }

  transaction.commit();
}

} // namespace tb::mdl
