/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "IO/Wad.h"

namespace TrenchBroom {
    namespace IO {
        TEST(WadTest, loadEntries) {
            const Path wadPath("data/IO/Wad/cr8_czg.wad");
            Wad wad(wadPath);
            
            const WadEntryList& entries = wad.allEntries();
            ASSERT_EQ(21u, entries.size());
            ASSERT_EQ(String("cr8_czg_1"), entries[ 0].name());
            ASSERT_EQ(String("cr8_czg_2"), entries[ 1].name());
            ASSERT_EQ(String("cr8_czg_3"), entries[ 2].name());
            ASSERT_EQ(String("cr8_czg_4"), entries[ 3].name());
            ASSERT_EQ(String("cr8_czg_5"), entries[ 4].name());
            ASSERT_EQ(String("speedM_1"), entries[ 5].name());
            ASSERT_EQ(String("cap4can-o-jam"), entries[ 6].name());
            ASSERT_EQ(String("can-o-jam"), entries[ 7].name());
            ASSERT_EQ(String("eat_me"), entries[ 8].name());
            ASSERT_EQ(String("coffin1"), entries[ 9].name());
            ASSERT_EQ(String("coffin2"), entries[10].name());
            ASSERT_EQ(String("czg_fronthole"), entries[11].name());
            ASSERT_EQ(String("czg_backhole"), entries[12].name());
            ASSERT_EQ(String("u_get_this"), entries[13].name());
            ASSERT_EQ(String("for_sux-m-ass"), entries[14].name());
            ASSERT_EQ(String("dex_5"), entries[15].name());
            ASSERT_EQ(String("polished_turd"), entries[16].name());
            ASSERT_EQ(String("crackpipes"), entries[17].name());
            ASSERT_EQ(String("bongs2"), entries[18].name());
            ASSERT_EQ(String("blowjob_machine"), entries[19].name());
            ASSERT_EQ(String("lasthopeofhuman"), entries[20].name());
        }
        
        TEST(WadTest, getMipSize) {
            const Path wadPath("data/IO/Wad/cr8_czg.wad");
            Wad wad(wadPath);
            
            const WadEntryList& entries = wad.allEntries();
            ASSERT_EQ(MipSize( 64,  64), wad.mipSize(entries[0]));
            ASSERT_EQ(MipSize( 64,  64), wad.mipSize(entries[1]));
            ASSERT_EQ(MipSize( 64, 128), wad.mipSize(entries[2]));
            ASSERT_EQ(MipSize( 64, 128), wad.mipSize(entries[3]));
            ASSERT_EQ(MipSize( 64, 128), wad.mipSize(entries[4]));
        }
    }
}
