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

#include "IO/IdPakFileSystem.h"
#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/FileMatcher.h"

#include <algorithm>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("IdPakFileSystemTest.directoryExists")
{
  const Path pakPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Pak/pak3.pak");

  const IdPakFileSystem fs(pakPath);
  CHECK_THROWS_AS(fs.directoryExists(Path("/asdf")), FileSystemException);
  CHECK_THROWS_AS(fs.directoryExists(Path("/gfx")), FileSystemException);

  CHECK(fs.directoryExists(Path("gfx")));
  CHECK(fs.directoryExists(Path("GFX")));
  CHECK_FALSE(fs.directoryExists(Path("gfx/palette.lmp")));
}

TEST_CASE("IdPakFileSystemTest.fileExists")
{
  const Path pakPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Pak/pak3.pak");

  const IdPakFileSystem fs(pakPath);
  CHECK_THROWS_AS(fs.fileExists(Path("/asdf.blah")), FileSystemException);
  CHECK_THROWS_AS(fs.fileExists(Path("/gfx/palette.lmp")), FileSystemException);

  CHECK(fs.fileExists(Path("gfx/palette.lmp")));
  CHECK(fs.fileExists(Path("GFX/Palette.LMP")));
}

TEST_CASE("IdPakFileSystemTest.findItems")
{
  const Path pakPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Pak/pak1.pak");

  const IdPakFileSystem fs(pakPath);
  CHECK_THROWS_AS(fs.findItems(Path("/")), FileSystemException);
  CHECK_THROWS_AS(fs.findItems(Path("/pics/")), FileSystemException);
  CHECK_THROWS_AS(fs.findItems(Path("pics/tag1.pcx")), FileSystemException);

  CHECK_THAT(
    fs.findItems(Path("")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("pics"), Path("textures"), Path("amnet.cfg"), Path("bear.cfg")}));

  CHECK_THAT(
    fs.findItems(Path(""), FileExtensionMatcher("cfg")),
    Catch::UnorderedEquals(std::vector<Path>{Path("amnet.cfg"), Path("bear.cfg")}));

  CHECK_THAT(
    fs.findItems(Path("pics"), FileExtensionMatcher("cfg")),
    Catch::UnorderedEquals(std::vector<Path>{}));

  CHECK_THAT(
    fs.findItems(Path("pics")),
    Catch::UnorderedEquals(
      std::vector<Path>{Path("pics/tag1.pcx"), Path("pics/tag2.pcx")}));
}

TEST_CASE("IdPakFileSystemTest.findItemsRecursively")
{
  const Path pakPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Pak/pak1.pak");

  const IdPakFileSystem fs(pakPath);
  CHECK_THROWS_AS(fs.findItemsRecursively(Path("/")), FileSystemException);
  CHECK_THROWS_AS(fs.findItemsRecursively(Path("/pics/")), FileSystemException);
  CHECK_THROWS_AS(fs.findItemsRecursively(Path("pics/tag1.pcx")), FileSystemException);

  CHECK_THAT(
    fs.findItemsRecursively(Path("")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("pics"),
      Path("pics/tag1.pcx"),
      Path("pics/tag2.pcx"),
      Path("textures/e1u1"),
      Path("textures/e1u1/box1_3.wal"),
      Path("textures/e1u1/brlava.wal"),
      Path("textures/e1u2"),
      Path("textures/e1u2/angle1_1.wal"),
      Path("textures/e1u2/angle1_2.wal"),
      Path("textures/e1u2/basic1_7.wal"),
      Path("textures/e1u3"),
      Path("textures/e1u3/stairs1_3.wal"),
      Path("textures/e1u3/stflr1_5.wal"),
      Path("textures"),
      Path("amnet.cfg"),
      Path("bear.cfg"),
    }));

  CHECK_THAT(
    fs.findItemsRecursively(Path(""), FileExtensionMatcher("wal")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("textures/e1u1/box1_3.wal"),
      Path("textures/e1u1/brlava.wal"),
      Path("textures/e1u2/angle1_1.wal"),
      Path("textures/e1u2/angle1_2.wal"),
      Path("textures/e1u2/basic1_7.wal"),
      Path("textures/e1u3/stairs1_3.wal"),
      Path("textures/e1u3/stflr1_5.wal"),
    }));

  CHECK_THAT(
    fs.findItemsRecursively(Path("textures"), FileExtensionMatcher("WAL")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("textures/e1u1/box1_3.wal"),
      Path("textures/e1u1/brlava.wal"),
      Path("textures/e1u2/angle1_1.wal"),
      Path("textures/e1u2/angle1_2.wal"),
      Path("textures/e1u2/basic1_7.wal"),
      Path("textures/e1u3/stairs1_3.wal"),
      Path("textures/e1u3/stflr1_5.wal"),
    }));
}

TEST_CASE("IdPakFileSystemTest.openFile")
{
  const Path pakPath =
    Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Pak/pak1.pak");

  const IdPakFileSystem fs(pakPath);
  CHECK_THROWS_AS(fs.openFile(Path("")), FileSystemException);
  CHECK_THROWS_AS(fs.openFile(Path("/amnet.cfg")), FileSystemException);
  CHECK_THROWS_AS(fs.openFile(Path("/textures")), FileSystemException);

  CHECK(fs.openFile(Path("amnet.cfg")) != nullptr);
}
} // namespace IO
} // namespace TrenchBroom
