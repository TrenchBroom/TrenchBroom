/*
 Copyright (C) 2020 Kristian Duske

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

#include "TestUtils.h"

#include "Model/Group.h"

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>

#include <kdl/result.h>

#include <memory>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom
{
namespace Model
{
TEST_CASE("GroupTest.transform")
{
  auto group = Group{"name"};
  REQUIRE(group.transformation() == vm::mat4x4());

  group.transform(vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)));
  CHECK(group.transformation() == vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)));

  group.transform(vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0)));
  CHECK(
    group.transformation()
    == vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0))
         * vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)));
}
} // namespace Model
} // namespace TrenchBroom
