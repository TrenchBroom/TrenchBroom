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

#include "Renderer/Camera.h"
#include "Renderer/PerspectiveCamera.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace Renderer
{
TEST_CASE("CameraTest.testInvalidUp")
{
  PerspectiveCamera c;
  c.setDirection(vm::vec3f(0, 0, 1), vm::vec3f(0, 0, 1));

  CHECK_FALSE(vm::is_nan(c.direction()));
  CHECK_FALSE(vm::is_nan(c.right()));
  CHECK_FALSE(vm::is_nan(c.up()));
}

TEST_CASE("CameraTest.testOrbitDown")
{
  PerspectiveCamera c;
  c.setDirection(vm::vec3f(1, 0, 0), vm::vec3f(0, 0, 1));

  c.orbit(vm::vec3f::zero(), 0.0f, vm::constants<float>::pi());

  CHECK_FALSE(vm::is_nan(c.direction()));
  CHECK_FALSE(vm::is_nan(c.right()));
  CHECK_FALSE(vm::is_nan(c.up()));
}

TEST_CASE("CameraTest.testOrbitWhileInverted")
{
  PerspectiveCamera c;
  c.setDirection(vm::vec3f(1, 0, 0), vm::vec3f(0, 0, -1));

  c.orbit(vm::vec3f::zero(), vm::constants<float>::pi(), 0.0f);

  CHECK_FALSE(vm::is_nan(c.direction()));
  CHECK_FALSE(vm::is_nan(c.right()));
  CHECK_FALSE(vm::is_nan(c.up()));
}

TEST_CASE("CameraTest.testYawWhenPitchedDown")
{
  PerspectiveCamera c;
  c.setDirection(vm::vec3f::neg_z(), vm::vec3f::pos_x());

  c.rotate(0.1f, 0.0f);

  CHECK_FALSE(vm::is_nan(c.direction()));
  CHECK_FALSE(vm::is_nan(c.right()));
  CHECK_FALSE(vm::is_nan(c.up()));
}
} // namespace Renderer
} // namespace TrenchBroom
