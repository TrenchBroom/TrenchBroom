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
#include "mdl/LoadMdxModel.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("loadMdxModel")
{
  auto logger = NullLogger{};
  auto fs = fs::VirtualFileSystem{};

  SECTION("valid MDX model")
  {
    const auto path = getFixtureRoot() / "test/mdl/LoadMdxModel/cube.mdx";
    const auto file = fs::Disk::openFile(path) | kdl::value();

    auto reader = file->reader().buffer();
    loadMdxModel("cube", reader, fs, logger) | kdl::transform([](const auto& modelData) {
      CHECK(modelData.surfaceCount() == 1u);
      CHECK(modelData.frameCount() == 1u);

      const auto* frame = modelData.frame(0u);
      REQUIRE(frame != nullptr);
      CHECK(frame->name() == "frame1");

      const auto& surfaces = modelData.surfaces();
      const auto& surface = surfaces.front();
      CHECK(surface.skinCount() == 0u);
      CHECK(surface.frameCount() == 1u);
    }) | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
  }

  SECTION("invalid MDX file")
  {
    const auto path = getFixtureRoot() / "test/mdl/LoadMdxModel/invalid.mdx";
    const auto file = fs::Disk::openFile(path) | kdl::value();

    auto reader = file->reader().buffer();
    CHECK(
      loadMdxModel("cube", reader, fs, logger)
      == Result<mdl::EntityModelData>{Error{"Unknown MDX model ident: 305419896"}});
  }
}

} // namespace tb::mdl
