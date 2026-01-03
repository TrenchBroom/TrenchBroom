/*
 Copyright (C) 2026 Kristian Duske

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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/PerspectiveCamera.h"
#include "ui/CameraTool3D.h"
#include "ui/CatchConfig.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"

#include "vm/approx.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

namespace tb::ui
{

TEST_CASE("CameraTool3D")
{
  using namespace Catch::Matchers;

  auto camera = gl::PerspectiveCamera{};
  auto cameraTool = CameraTool3D{camera};
  auto inputState = InputState{};

  SECTION("Left drag is ignored")
  {
    inputState.mouseDown(MouseButtons::Left);
    inputState.mouseMove(10, 0, 10, 0);

    CHECK(cameraTool.acceptMouseDrag(inputState) == nullptr);
  }

  SECTION("Horizontal middle drag pans left/right")
  {
    REQUIRE(camera.direction() == vm::vec3f{1, 0, 0});
    REQUIRE(camera.position() == vm::vec3f{0, 0, 0});

    inputState.mouseDown(MouseButtons::Middle);
    inputState.mouseMove(100, 0, 100, 0);

    auto tracker = cameraTool.acceptMouseDrag(inputState);
    REQUIRE(tracker != nullptr);

    tracker->update(inputState);
    CHECK(camera.direction() == vm::vec3f{1, 0, 0});
    CHECK(camera.position() == vm::vec3f{0, -50, 0});
  }

  SECTION("Vertical middle drag pans up/down")
  {
    REQUIRE(camera.direction() == vm::vec3f{1, 0, 0});
    REQUIRE(camera.position() == vm::vec3f{0, 0, 0});

    inputState.mouseDown(MouseButtons::Middle);
    inputState.mouseMove(0, 100, 0, 100);

    auto tracker = cameraTool.acceptMouseDrag(inputState);
    REQUIRE(tracker != nullptr);

    tracker->update(inputState);
    CHECK(camera.direction() == vm::vec3f{1, 0, 0});
    CHECK(camera.position() == vm::vec3f{0, 0, 50});
  }

  SECTION("Scrolling pans forward / backward")
  {
    REQUIRE(camera.direction() == vm::vec3f{1, 0, 0});
    REQUIRE(camera.position() == vm::vec3f{0, 0, 0});

    inputState.scroll(ScrollSource::Mouse, 0.0f, 5.0f);
    cameraTool.mouseScroll(inputState);

    CHECK(camera.direction() == vm::vec3f{1, 0, 0});
    CHECK(camera.position() == vm::vec3f{30, 0, 0});
  }

  SECTION("Horizontal right drag adjust yaw angle")
  {
    REQUIRE(camera.direction() == vm::vec3f{1, 0, 0});
    REQUIRE(camera.position() == vm::vec3f{0, 0, 0});

    inputState.mouseDown(MouseButtons::Right);
    inputState.mouseMove(100, 0, 100, 0);

    auto tracker = cameraTool.acceptMouseDrag(inputState);
    REQUIRE(tracker != nullptr);

    tracker->update(inputState);
    CHECK(camera.direction() == vm::approx{vm::vec3f{0.54f, -0.84f, 0.0f}, 0.01f});
    CHECK(camera.position() == vm::vec3f{0, 0, 0});
  }

  SECTION("Vertical right drag adjust pitch angle")
  {
    REQUIRE(camera.direction() == vm::vec3f{1, 0, 0});
    REQUIRE(camera.position() == vm::vec3f{0, 0, 0});

    inputState.mouseDown(MouseButtons::Right);
    inputState.mouseMove(0, 100, 0, 100);

    auto tracker = cameraTool.acceptMouseDrag(inputState);
    REQUIRE(tracker != nullptr);

    tracker->update(inputState);
    CHECK(camera.direction() == vm::approx{vm::vec3f{0.54f, 0.0f, -0.84f}, 0.01f});
    CHECK(camera.position() == vm::vec3f{0, 0, 0});
  }

  SECTION("Scrolling during right drag adjusts speed")
  {
    REQUIRE(camera.direction() == vm::vec3f{1, 0, 0});
    REQUIRE(camera.position() == vm::vec3f{0, 0, 0});
    REQUIRE(pref(Preferences::CameraFlyMoveSpeed) == 0.5f);

    inputState.mouseDown(MouseButtons::Right);
    inputState.mouseMove(0, 0, 0, 0);

    auto tracker = cameraTool.acceptMouseDrag(inputState);
    REQUIRE(tracker != nullptr);

    tracker->update(inputState);
    REQUIRE(camera.direction() == vm::vec3f{1, 0, 0});
    REQUIRE(camera.position() == vm::vec3f{0, 0, 0});

    inputState.scroll(ScrollSource::Mouse, 0.0f, 5.0f);

    tracker->mouseScroll(inputState);
    CHECK(camera.direction() == vm::vec3f{1, 0, 0});
    CHECK(camera.position() == vm::vec3f{0, 0, 0});
    CHECK(pref(Preferences::CameraFlyMoveSpeed) == 0.625f);
  }

  SECTION("Right drag while holding alt orbits")
  {
    REQUIRE(camera.direction() == vm::vec3f{1, 0, 0});
    REQUIRE(camera.position() == vm::vec3f{0, 0, 0});

    inputState.setPickRequest({vm::ray3d{{0, 0, 0}, {1, 0, 0}}, camera});
    inputState.setModifierKeys(ModifierKeys::Alt);
    inputState.mouseDown(MouseButtons::Right);
    inputState.mouseMove(100, 0, 100, 0);

    auto tracker = cameraTool.acceptMouseDrag(inputState);
    REQUIRE(tracker != nullptr);

    tracker->update(inputState);
    CHECK(camera.direction() == vm::approx{vm::vec3f{0.54f, -0.84f, 0.0f}, 0.01f});
    CHECK(camera.position() == vm::approx{vm::vec3f{117.68f, 215.41f, 0.0f}, 0.01f});
  }
}

} // namespace tb::ui
