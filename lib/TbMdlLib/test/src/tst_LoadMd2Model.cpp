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

#include "TestEnvironment.h"
#include "base/Logger.h"
#include "fs/DiskIO.h"
#include "fs/VirtualFileSystem.h"
#include "mdl/CatchConfig.h"
#include "mdl/LoadMd2Model.h"
#include "mdl/Palette.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("loadMd2Model")
{
  auto logger = NullLogger{};
  auto fs = fs::VirtualFileSystem{};
  const auto palette = Palette{nullptr};

  SECTION("valid MD2 model")
  {
    const auto path = getFixtureRoot() / "test/mdl/LoadMd2Model/cube.md2";
    const auto file = fs::Disk::openFile(path) | kdl::value();

    auto reader = file->reader().buffer();
    loadMd2Model("cube", reader, palette, fs, logger)
      | kdl::transform([](const auto& modelData) {
          CHECK(modelData.surfaceCount() == 1u);
          CHECK(modelData.frameCount() == 1u);

          const auto* frame = modelData.frame(0u);
          REQUIRE(frame != nullptr);
          CHECK(frame->name() == "frame1");

          const auto& surfaces = modelData.surfaces();
          const auto& surface = surfaces.front();
          CHECK(surface.skinCount() == 0u);
          CHECK(surface.frameCount() == 1u);
        })
      | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
  }

  SECTION("invalid MD2 file")
  {
    const auto path = getFixtureRoot() / "test/mdl/LoadMd2Model/invalid.md2";
    const auto file = fs::Disk::openFile(path) | kdl::value();

    auto reader = file->reader().buffer();
    CHECK(
      loadMd2Model("cube", reader, palette, fs, logger)
      == Result<mdl::EntityModelData>{Error{"Unknown MD2 model ident: 305419896"}});
  }
}

} // namespace tb::mdl
