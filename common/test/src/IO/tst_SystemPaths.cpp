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


#include "IO/SystemPaths.h"

#include "Catch2.h"


namespace TrenchBroom
{
namespace IO
{
TEST_CASE("Portable flag begins false")
{
  CHECK(SystemPaths::isPortable() == false);
}

TEST_CASE("Portable flag is changed by setPortable")
{
  CHECK(SystemPaths::isPortable() == false);
  SystemPaths::setPortable();
  CHECK(SystemPaths::isPortable() == true);
  // cleanup
  SystemPaths::setPortable(false);
}

TEST_CASE("userDataDirectory is changed by setPortable")
{
  auto initialDataDir = SystemPaths::userDataDirectory().string();
  SystemPaths::setPortable();
  CHECK(SystemPaths::userDataDirectory() != initialDataDir);
  // cleanup
  SystemPaths::setPortable(false);
}
} // namespace IO
} // namespace TrenchBroom