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

#include "IO/DiskFileSystem.h"
#include "IO/ResourceUtils.h"
#include "Logger.h"
#include "asset/Material.h"

#include <filesystem>
#include <memory>

#include "Catch2.h"

namespace tb::IO
{

TEST_CASE("ResourceUtilsTest.loadDefaultMaterial")
{
  auto fs = std::make_shared<DiskFileSystem>(
    std::filesystem::current_path() / "fixture/test/IO/ResourceUtils/assets");
  NullLogger logger;

  auto material = loadDefaultMaterial(*fs, "some_name", logger);
  CHECK(material.name() == "some_name");
}

} // namespace tb::IO
