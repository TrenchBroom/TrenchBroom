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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Assets/TextureCollectionSpec.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "IO/WadTextureLoader.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureCollectionSpec.h"

namespace TrenchBroom {
    namespace IO {
        inline void assertTexture(const String& name, const size_t width, const size_t height, Assets::Texture* texture) {
            ASSERT_EQ(name, texture->name());
            ASSERT_EQ(width, texture->width());
            ASSERT_EQ(height, texture->height());
        }
        
        TEST(WadTextureLoaderTest, testLoadWad) {
            const Assets::Palette palette(Disk::getCurrentWorkingDir() + Path("data/palette.lmp"));
            WadTextureLoader loader(palette);
            
            const Path wadPath = Disk::getCurrentWorkingDir() + Path("data/IO/Wad/cr8_czg.wad");
            const Assets::TextureCollectionSpec spec("cr8_czg.wad", wadPath);
            Assets::TextureCollection* collection = loader.loadTextureCollection(spec);
            ASSERT_TRUE(collection->loaded());
            
            const Assets::TextureList& textures = collection->textures();
            ASSERT_EQ(21u, textures.size());
            assertTexture("cr8_czg_1",          64,  64, textures[ 0]);
            assertTexture("cr8_czg_2",          64,  64, textures[ 1]);
            assertTexture("cr8_czg_3",          64, 128, textures[ 2]);
            assertTexture("cr8_czg_4",          64, 128, textures[ 3]);
            assertTexture("cr8_czg_5",          64, 128, textures[ 4]);
            assertTexture("speedM_1",          128, 128, textures[ 5]);
            assertTexture("cap4can-o-jam",      64,  64, textures[ 6]);
            assertTexture("can-o-jam",          64,  64, textures[ 7]);
            assertTexture("eat_me",             64,  64, textures[ 8]);
            assertTexture("coffin1",           128, 128, textures[ 9]);
            assertTexture("coffin2",           128, 128, textures[10]);
            assertTexture("czg_fronthole",     128, 128, textures[11]);
            assertTexture("czg_backhole",      128, 128, textures[12]);
            assertTexture("u_get_this",         64,  64, textures[13]);
            assertTexture("for_sux-m-ass",      64,  64, textures[14]);
            assertTexture("dex_5",             128, 128, textures[15]);
            assertTexture("polished_turd",      64,  64, textures[16]);
            assertTexture("crackpipes",        128, 128, textures[17]);
            assertTexture("bongs2",            128, 128, textures[18]);
            assertTexture("blowjob_machine",   128, 128, textures[19]);
            assertTexture("lasthopeofhuman",   128, 128, textures[20]);
            
            delete collection;
        }
    }
}
