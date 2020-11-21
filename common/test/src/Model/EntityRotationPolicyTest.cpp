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

#include "FloatType.h"
#include "Model/EntityRotationPolicy.h"

#include <vecmath/approx.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "Catch2.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("EntityRotationPolicy.getYawPitchRoll") {
            const auto roll  = vm::to_radians(12.0);
            const auto pitch = vm::to_radians(13.0);
            const auto yaw   = vm::to_radians(14.0);

            const auto rotMat = vm::rotation_matrix(roll, pitch, yaw);
            const auto yawPitchRoll = EntityRotationPolicy::getYawPitchRoll(vm::mat4x4::identity(), rotMat);

            CHECK(yawPitchRoll == vm::approx(vm::vec3(14, 13, 12)));
        }

        TEST_CASE("EntityRotationPolicy.getYawPitchRoll_uniformScale") {
            const auto roll = vm::to_radians(12.0);
            const auto pitch = vm::to_radians(13.0);
            const auto yaw = vm::to_radians(14.0);

            const auto scaleMat = vm::scaling_matrix(vm::vec3(2.0, 2.0, 2.0));
            const auto rotMat = vm::rotation_matrix(roll, pitch, yaw);

            const auto yawPitchRoll = EntityRotationPolicy::getYawPitchRoll(scaleMat, rotMat);
            CHECK(yawPitchRoll == vm::approx(vm::vec3(14, 13, 12)));
        }

        TEST_CASE("EntityRotationPolicy.getYawPitchRoll_nonUniformScale") {
            const auto roll = vm::to_radians(0.0);
            const auto pitch = vm::to_radians(45.0);
            const auto yaw = vm::to_radians(0.0);

            const auto scaleMat = vm::scaling_matrix(vm::vec3(2.0, 1.0, 1.0));
            const auto rotMat = vm::rotation_matrix(roll, pitch, yaw);

            const auto yawPitchRoll = EntityRotationPolicy::getYawPitchRoll(scaleMat, rotMat);
            const auto expectedPitch = vm::to_degrees(std::atan(0.5)); // ~= 26.57 degrees

            CHECK(yawPitchRoll == vm::approx(vm::vec3(0.0, expectedPitch, 0.0)));
        }

        TEST_CASE("EntityRotationPolicy.getYawPitchRoll_flip") {
            const auto roll = vm::to_radians(10.0);
            const auto pitch = vm::to_radians(45.0);
            const auto yaw = vm::to_radians(0.0);

            const auto scaleMat = vm::scaling_matrix(vm::vec3(-1.0, 1.0, 1.0));
            const auto rotMat = vm::rotation_matrix(roll, pitch, yaw);

            const auto yawPitchRoll = EntityRotationPolicy::getYawPitchRoll(scaleMat, rotMat);

            CHECK(yawPitchRoll == vm::approx(vm::vec3(180, 45, -10)));
        }
    }
}