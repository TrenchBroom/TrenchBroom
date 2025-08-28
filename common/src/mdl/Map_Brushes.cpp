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

#include "Logger.h"
#include "Map.h"
#include "mdl/ApplyAndSwap.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "mdl/Game.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{

bool Map::createBrush(const std::vector<vm::vec3d>& points)
{
  const auto builder = BrushBuilder{
    world()->mapFormat(), worldBounds(), game()->config().faceAttribsConfig.defaults};

  return builder.createBrush(points, currentMaterialName())
         | kdl::and_then([&](auto b) -> Result<void> {
             auto* brushNode = new BrushNode{std::move(b)};

             auto transaction = Transaction{*this, "Create Brush"};
             deselectAll();
             if (addNodes(*this, {{parentForNodes(*this), {brushNode}}}).empty())
             {
               transaction.cancel();
               return Error{"Could not add brush to document"};
             }
             selectNodes({brushNode});
             if (!transaction.commit())
             {
               return Error{"Could not add brush to document"};
             }

             return kdl::void_success;
           })
         | kdl::if_error(
           [&](auto e) { logger().error() << "Could not create brush: " << e.msg; })
         | kdl::is_success();
}

bool Map::setFaceAttributes(const BrushFaceAttributes& attributes)
{
  ChangeBrushFaceAttributesRequest request;
  request.setAll(attributes);
  return setFaceAttributes(request);
}

bool Map::setFaceAttributesExceptContentFlags(const BrushFaceAttributes& attributes)
{
  ChangeBrushFaceAttributesRequest request;
  request.setAllExceptContentFlags(attributes);
  return setFaceAttributes(request);
}

bool Map::setFaceAttributes(const ChangeBrushFaceAttributesRequest& request)
{
  return applyAndSwap(
    *this, request.name(), selection().allBrushFaces(), [&](BrushFace& brushFace) {
      request.evaluate(brushFace);
      return true;
    });
}

bool Map::copyUVFromFace(
  const UVCoordSystemSnapshot& coordSystemSnapshot,
  const BrushFaceAttributes& attribs,
  const vm::plane3d& sourceFacePlane,
  const WrapStyle wrapStyle)
{
  return applyAndSwap(
    *this, "Copy UV Alignment", selection().brushFaces, [&](BrushFace& face) {
      face.copyUVCoordSystemFromFace(
        coordSystemSnapshot, attribs, sourceFacePlane, wrapStyle);
      return true;
    });
}

bool Map::translateUV(
  const vm::vec3f& cameraUp, const vm::vec3f& cameraRight, const vm::vec2f& delta)
{
  return applyAndSwap(*this, "Move UV", selection().brushFaces, [&](BrushFace& face) {
    face.moveUV(vm::vec3d(cameraUp), vm::vec3d(cameraRight), delta);
    return true;
  });
}

bool Map::rotateUV(const float angle)
{
  return applyAndSwap(*this, "Rotate UV", selection().brushFaces, [&](BrushFace& face) {
    face.rotateUV(angle);
    return true;
  });
}

bool Map::shearUV(const vm::vec2f& factors)
{
  return applyAndSwap(*this, "Shear UV", selection().brushFaces, [&](BrushFace& face) {
    face.shearUV(factors);
    return true;
  });
}

bool Map::flipUV(
  const vm::vec3f& cameraUp,
  const vm::vec3f& cameraRight,
  const vm::direction cameraRelativeFlipDirection)
{
  const bool isHFlip =
    (cameraRelativeFlipDirection == vm::direction::left
     || cameraRelativeFlipDirection == vm::direction::right);
  return applyAndSwap(
    *this,
    isHFlip ? "Flip UV Horizontally" : "Flip UV Vertically",
    selection().brushFaces,
    [&](BrushFace& face) {
      face.flipUV(
        vm::vec3d(cameraUp), vm::vec3d(cameraRight), cameraRelativeFlipDirection);
      return true;
    });
}

} // namespace tb::mdl
