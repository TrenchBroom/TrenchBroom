/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "render/SceneLighting.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace tb::render
{

TEST_CASE("SceneLighting")
{
  SECTION("daylight largely equalizes player vision")
  {
    const auto human = sceneLightingProfile(PlayerVision::Human, 12.0f);
    const auto infravision = sceneLightingProfile(PlayerVision::Infravision, 12.0f);
    const auto ultravision = sceneLightingProfile(PlayerVision::Ultravision, 12.0f);

    CHECK(human.ambientColor.x() == Catch::Approx(infravision.ambientColor.x()));
    CHECK(human.ambientColor.y() == Catch::Approx(infravision.ambientColor.y()));
    CHECK(human.ambientColor.z() == Catch::Approx(infravision.ambientColor.z()));
    CHECK(human.ambientColor.x() == Catch::Approx(ultravision.ambientColor.x()));
    CHECK(human.ambientColor.y() == Catch::Approx(ultravision.ambientColor.y()));
    CHECK(human.ambientColor.z() == Catch::Approx(ultravision.ambientColor.z()));
    CHECK(human.visionTint == vm::vec3f{1.0f, 1.0f, 1.0f});
    CHECK(human.sunDirection.z() > 0.99f);
    CHECK(human.skyColor.z() > human.skyColor.x());
  }

  SECTION("night vision profiles remain visually distinct")
  {
    const auto human = sceneLightingProfile(PlayerVision::Human, 0.0f);
    const auto infravision = sceneLightingProfile(PlayerVision::Infravision, 0.0f);
    const auto ultravision = sceneLightingProfile(PlayerVision::Ultravision, 0.0f);

    CHECK(infravision.ambientColor.x() > human.ambientColor.x());
    CHECK(infravision.visionTint.x() > infravision.visionTint.z());
    CHECK(ultravision.ambientColor.z() > ultravision.ambientColor.x());
    CHECK(ultravision.visionTint.z() > ultravision.visionTint.x());
    CHECK(human.sunColor == vm::vec3f{});
    CHECK(human.skyColor.x() < 0.03f);
  }

  SECTION("time wraps across midnight")
  {
    const auto midnight = sceneLightingProfile(PlayerVision::Human, 0.0f);
    const auto nextMidnight = sceneLightingProfile(PlayerVision::Human, 24.0f);
    CHECK(midnight.ambientColor == nextMidnight.ambientColor);
    CHECK(midnight.sunColor == nextMidnight.sunColor);
    CHECK(midnight.skyColor == nextMidnight.skyColor);
  }
}

} // namespace tb::render
