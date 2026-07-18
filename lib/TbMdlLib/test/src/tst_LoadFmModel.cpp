/*
 Copyright (C) 2026 MaxED

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
#include "fs/DiskFileSystem.h"
#include "mdl/LoadFmModel.h"

#include <fs/VirtualFileSystem.h>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("loadFmModel")
{
  auto logger = NullLogger{};
  auto fs = fs::VirtualFileSystem{};
  fs.mount(
    "", std::make_unique<fs::DiskFileSystem>(getFixtureRoot() / "test/mdl/LoadFmModel"));

  SECTION("valid FM model")
  {
    const auto fmPath = "models/objects/halberd/tris.fm";
    const auto fmFile = fs.openFile(fmPath) | kdl::value();

    const auto reader = fmFile->reader().buffer();
    loadFmModel("halberd", reader, fs, logger)
      | kdl::transform([](const auto& modelData) {
          CHECK(modelData.surfaceCount() == 1u);
          CHECK(modelData.frameCount() == 1u);

          const auto& surfaces = modelData.surfaces();
          const auto& surface = surfaces.front();
          CHECK(surface.skinCount() == 1u);
          CHECK(surface.frameCount() == 1u);
        })
      | kdl::transform_error([](const auto& e) { FAIL(e); });
  }
}

} // namespace tb::mdl
