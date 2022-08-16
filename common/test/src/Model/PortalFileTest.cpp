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

#include "Model/PortalFile.h"
#include "IO/DiskIO.h"
#include "IO/Path.h"

#include <vecmath/polygon.h>

#include <memory>

#include "Catch2.h"

namespace TrenchBroom {
namespace Model {
TEST_CASE("PortalFileTest.parseInvalidPRT1", "[PortalFileTest]") {
  const auto path = IO::Path("fixture/test/Model/PortalFile/portaltest_prt1_invalid.prt");

  CHECK_THROWS([&]() {
    const Model::PortalFile p = Model::PortalFile(path);
  }());
}

static const std::vector<vm::polygon3f> ExpectedPortals{
  {{-96, -32, 80}, {-96, 160, 80}, {0, 160, 80}, {0, -32, 80}},
  {{208, -64, 80}, {64, -64, 80}, {64, 160, 80}, {208, 160, 80}},
  {{64, 80, 48},
   {64, 80, 16},
   {64, 64, 0},
   {64, 32, 0},
   {64, 16, 16},
   {64, 16, 48},
   {64, 32, 64},
   {64, 64, 64}},
  {{0, 80, 48},
   {0, 80, 16},
   {0, 64, 0},
   {0, 32, 0},
   {0, 16, 16},
   {0, 16, 48},
   {0, 32, 64},
   {0, 64, 64}},
  {{-64, -32, 0}, {-32, -32, 0}, {-48, -32, 64}}};

TEST_CASE("PortalFileTest.parsePRT1", "[PortalFileTest]") {
  const auto path = IO::Path{"fixture/test/Model/PortalFile/portaltest_prt1.prt"};
  const auto portalFile = Model::PortalFile{path};
  CHECK(portalFile.portals() == ExpectedPortals);
}

TEST_CASE("PortalFileTest.parsePRT1Q3", "[PortalFileTest]") {
  const auto path = IO::Path{"fixture/test/Model/PortalFile/portaltest_prt1q3.prt"};
  const auto portalFile = Model::PortalFile{path};
  CHECK(portalFile.portals() == ExpectedPortals);
}

TEST_CASE("PortalFileTest.parsePRT1AM", "[PortalFileTest]") {
  const auto path = IO::Path{"fixture/test/Model/PortalFile/portaltest_prt1am.prt"};
  const auto portalFile = Model::PortalFile{path};
  CHECK(portalFile.portals() == ExpectedPortals);
}

TEST_CASE("PortalFileTest.parsePRT2", "[PortalFileTest]") {
  const auto path = IO::Path{"fixture/test/Model/PortalFile/portaltest_prt2.prt"};
  const auto portalFile = Model::PortalFile{path};
  CHECK(portalFile.portals() == ExpectedPortals);
}
} // namespace Model
} // namespace TrenchBroom
