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

#include "mdl/CatchConfig.h"
#include "mdl/ParallelUvCoordSystem.h"
#include "mdl/ParaxialUvCoordSystem.h"
#include "mdl/UvAttributes.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

// Disable a clang warning when using ASSERT_DEATH
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

TEST_CASE("UvCoordSystemTest.testSnapshotTypeSafety")
{
  UvAttributes uvAttributes;

  ParaxialUvCoordSystem paraxial(vm::vec3d{0, 0, 1}, uvAttributes);
  CHECK(paraxial.takeSnapshot() == nullptr);

  ParallelUvCoordSystem parallel(vm::vec3d{0, 1, 0}, vm::vec3d{1, 0, 0});
  auto parallelSnapshot = parallel.takeSnapshot();
  CHECK(parallelSnapshot != nullptr);

#if 0 // not supported with Catch2
            ASSERT_DEATH(parallelSnapshot->restore(paraxial), "");
#endif
  parallelSnapshot->restore(parallel);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

} // namespace tb::mdl
