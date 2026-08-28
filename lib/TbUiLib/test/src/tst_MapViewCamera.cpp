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

#include <QApplication>

#include "gl/Camera.h"
#include "gl/GlManager.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/MapFormat.h"
#include "ui/AppControllerFixture.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapViewBase.h"
#include "ui/MapWindow.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("MapViewBase camera automation")
{
  auto appControllerFixture = AppControllerFixture{};
  auto& appController = appControllerFixture.appController();
  auto document = MapDocument::createDocument(
                    appController.environmentConfig(),
                    mdl::QuakeGameInfo,
                    mdl::MapFormat::Valve,
                    vm::bbox3d{8192.0},
                    appController.taskManager(),
                    appController.glManager().resourceManager())
                  | kdl::value();
  auto window = MapWindow{appController, std::move(document)};
  auto* view = window.currentMapViewBase();

  REQUIRE(view != nullptr);
  CHECK_FALSE(view->hasFocus());

  SECTION("setCameraState changes only the view camera")
  {
    CHECK(view->setCameraState(
      {16.0f, 32.0f, 48.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));
    CHECK(view->camera().position() == vm::vec3f{16.0f, 32.0f, 48.0f});
    CHECK(view->camera().direction() == vm::vec3f{0.0f, 1.0f, 0.0f});
    CHECK(view->camera().up() == vm::vec3f{0.0f, 0.0f, 1.0f});
    CHECK_FALSE(view->hasFocus());
  }

  SECTION("invalid camera directions are rejected")
  {
    CHECK_FALSE(view->setCameraState(
      {16.0f, 32.0f, 48.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}));
  }

  SECTION("framing does not focus the view")
  {
    CHECK_FALSE(view->frameSelection());
    view->frameBounds(vm::bbox3d{{-32.0, -32.0, -32.0}, {32.0, 32.0, 32.0}});
    CHECK_FALSE(view->hasFocus());
  }
}

} // namespace tb::ui
