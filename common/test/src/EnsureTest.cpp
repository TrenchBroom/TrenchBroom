/*
 Copyright (C) 2016 Eric Wasylishen

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

#include "Ensure.h"
#include "Macros.h"

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace Ensure {
        TEST_CASE("EnsureTest.successfulEnsure", "[EnsureTest]") {
            EXPECT_NO_THROW([](){
                ensure(true, "this shouldn't fail");
            }());
        }

        // Disable a clang warning when using ASSERT_DEATH
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

        TEST_CASE("EnsureTest.failingEnsure", "[EnsureTest]") {
            // FIXME: not with catch2
            //ASSERT_DEATH(ensure(false, "this should fail"), "");
        }

#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }
}
