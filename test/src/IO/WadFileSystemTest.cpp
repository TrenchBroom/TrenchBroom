/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include <gtest/gtest.h>

#include "CollectionUtils.h"
#include "IO/DiskFileSystem.h"
#include "IO/WadFileSystem.h"

namespace TrenchBroom {
    namespace IO {
        TEST(WadFileSystemTest, loadEntries) {
            const Path wadPath = Disk::getCurrentWorkingDir() + Path("data/IO/Wad/cr8_czg.wad");
            WadFileSystem fs(wadPath);
            const IO::Path::List files = fs.findItems(IO::Path(""));
            
            ASSERT_EQ(21u, files.size());
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("blowjob_machine")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("bongs2")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("can-o-jam")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("cap4can-o-jam")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("coffin1")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("coffin2")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("cr8_czg_1")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("cr8_czg_2")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("cr8_czg_3")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("cr8_czg_4")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("cr8_czg_5")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("crackpipes")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("czg_backhole")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("czg_fronthole")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("dex_5")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("eat_me")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("for_sux-m-ass")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("lasthopeofhuman")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("polished_turd")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("speedM_1")));
            ASSERT_TRUE(VectorUtils::contains(files, IO::Path("u_get_this")));
        }
    }
}
