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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/FileMatcher.h"
#include "IO/IdPakFileSystem.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("IdPakFileSystemTest.directoryExists", "[IdPakFileSystemTest]") {
            const Path pakPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Pak/pak3.pak");

            const IdPakFileSystem fs(pakPath);
            ASSERT_THROW(fs.directoryExists(Path("/asdf")), FileSystemException);
            ASSERT_THROW(fs.directoryExists(Path("/gfx")), FileSystemException);

            ASSERT_TRUE(fs.directoryExists(Path("gfx")));
            ASSERT_TRUE(fs.directoryExists(Path("GFX")));
            ASSERT_FALSE(fs.directoryExists(Path("gfx/palette.lmp")));
        }

        TEST_CASE("IdPakFileSystemTest.fileExists", "[IdPakFileSystemTest]") {
            const Path pakPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Pak/pak3.pak");

            const IdPakFileSystem fs(pakPath);
            ASSERT_THROW(fs.fileExists(Path("/asdf.blah")), FileSystemException);
            ASSERT_THROW(fs.fileExists(Path("/gfx/palette.lmp")), FileSystemException);

            ASSERT_TRUE(fs.fileExists(Path("gfx/palette.lmp")));
            ASSERT_TRUE(fs.fileExists(Path("GFX/Palette.LMP")));
        }

        TEST_CASE("IdPakFileSystemTest.findItems", "[IdPakFileSystemTest]") {
            const Path pakPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Pak/pak1.pak");

            const IdPakFileSystem fs(pakPath);
            ASSERT_THROW(fs.findItems(Path("/")), FileSystemException);
            ASSERT_THROW(fs.findItems(Path("/pics/")), FileSystemException);
            ASSERT_THROW(fs.findItems(Path("pics/tag1.pcx")), FileSystemException);

            std::vector<Path> items = fs.findItems(Path(""));
            ASSERT_EQ(4u, items.size());
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("pics")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("amnet.cfg")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("bear.cfg")) != std::end(items));

            items = fs.findItems(Path(""), FileExtensionMatcher("cfg"));
            ASSERT_EQ(2u, items.size());
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("amnet.cfg")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("bear.cfg")) != std::end(items));

            items = fs.findItems(Path("pics"), FileExtensionMatcher("cfg"));
            ASSERT_TRUE(items.empty());

            items = fs.findItems(Path("pics"));
            ASSERT_EQ(2u, items.size());
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("pics/tag1.pcx")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("pics/tag2.pcx")) != std::end(items));
        }

        TEST_CASE("IdPakFileSystemTest.findItemsRecursively", "[IdPakFileSystemTest]") {
            const Path pakPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Pak/pak1.pak");

            const IdPakFileSystem fs(pakPath);
            ASSERT_THROW(fs.findItemsRecursively(Path("/")), FileSystemException);
            ASSERT_THROW(fs.findItemsRecursively(Path("/pics/")), FileSystemException);
            ASSERT_THROW(fs.findItemsRecursively(Path("pics/tag1.pcx")), FileSystemException);

            std::vector<Path> items = fs.findItemsRecursively(Path(""));
            ASSERT_EQ(16u, items.size());
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("pics")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("pics/tag1.pcx")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("pics/tag2.pcx")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u1")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u1/box1_3.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u1/brlava.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u2")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u2/angle1_1.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u2/angle1_2.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u2/basic1_7.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u3")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u3/stairs1_3.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u3/stflr1_5.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("amnet.cfg")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("bear.cfg")) != std::end(items));

            items = fs.findItemsRecursively(Path(""), FileExtensionMatcher("wal"));
            ASSERT_EQ(7u, items.size());
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u1/box1_3.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u1/brlava.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u2/angle1_1.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u2/angle1_2.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u2/basic1_7.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u3/stairs1_3.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u3/stflr1_5.wal")) != std::end(items));

            items = fs.findItemsRecursively(Path("textures"), FileExtensionMatcher("WAL"));
            ASSERT_EQ(7u, items.size());
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u1/box1_3.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u1/brlava.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u2/angle1_1.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u2/angle1_2.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u2/basic1_7.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u3/stairs1_3.wal")) != std::end(items));
            ASSERT_TRUE(std::find(std::begin(items), std::end(items), Path("textures/e1u3/stflr1_5.wal")) != std::end(items));
        }

        TEST_CASE("IdPakFileSystemTest.openFile", "[IdPakFileSystemTest]") {
            const Path pakPath = Disk::getCurrentWorkingDir() + Path("fixture/test/IO/Pak/pak1.pak");

            const IdPakFileSystem fs(pakPath);
            ASSERT_THROW(fs.openFile(Path("")), FileSystemException);
            ASSERT_THROW(fs.openFile(Path("/amnet.cfg")), FileSystemException);
            ASSERT_THROW(fs.openFile(Path("/textures")), FileSystemException);

            ASSERT_TRUE(fs.openFile(Path("amnet.cfg")) != nullptr);
        }
    }
}
