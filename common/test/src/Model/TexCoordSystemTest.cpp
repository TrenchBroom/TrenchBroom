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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "FloatType.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/ParallelTexCoordSystem.h"

#include <vecmath/vec.h>

#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace Model {
        // Disable a clang warning when using ASSERT_DEATH
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

        TEST_CASE("TexCoordSystemTest.testSnapshotTypeSafety", "[TexCoordSystemTest]") {
            BrushFaceAttributes attribs("");

            ParaxialTexCoordSystem paraxial(vm::vec3::pos_z(), attribs);
            ASSERT_EQ(nullptr, paraxial.takeSnapshot());

            ParallelTexCoordSystem parallel(vm::vec3::pos_y(), vm::vec3::pos_x());
            auto parallelSnapshot = parallel.takeSnapshot();
            ASSERT_NE(nullptr, parallelSnapshot);

#if 0 // not supported with Catch2
            ASSERT_DEATH(parallelSnapshot->restore(paraxial), "");
#endif
            parallelSnapshot->restore(parallel);
        }

#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }
}
