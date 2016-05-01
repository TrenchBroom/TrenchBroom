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

#include "Assets/TextureCollectionSpec.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureCollectionSpec.h"
#include "IO/DiskFileSystem.h"
#include "IO/PaletteLoader.h"
#include "IO/Path.h"
#include "IO/WadTextureLoader.h"

namespace TrenchBroom {
    namespace IO {
        inline void assertTexture(const String& name, const size_t width, const size_t height, const Assets::TextureList& textures) {
            for (size_t i = 0; i < textures.size(); ++i) {
                const Assets::Texture* texture = textures[i];
                if (texture->name() == name) {
                    ASSERT_EQ(width, texture->width());
                    ASSERT_EQ(height, texture->height());
                    return;
                }
            }
            ASSERT_TRUE(false);
        }
        
        TEST(WadTextureLoaderTest, testLoadWad) {
            DiskFileSystem fs(IO::Disk::getCurrentWorkingDir());
            FilePaletteLoader paletteLoader(fs, Path("data/palette.lmp"));

            WadTextureLoader loader(&paletteLoader);
            
            const Path wadPath = Disk::getCurrentWorkingDir() + Path("data/IO/Wad/cr8_czg.wad");
            const Assets::TextureCollectionSpec spec("cr8_czg.wad", wadPath);
            Assets::TextureCollection* collection = loader.loadTextureCollection(spec);
            ASSERT_TRUE(collection->loaded());
            
            const Assets::TextureList& textures = collection->textures();
            ASSERT_EQ(21u, textures.size());
            assertTexture("cr8_czg_1",          64,  64, textures);
            assertTexture("cr8_czg_2",          64,  64, textures);
            assertTexture("cr8_czg_3",          64, 128, textures);
            assertTexture("cr8_czg_4",          64, 128, textures);
            assertTexture("cr8_czg_5",          64, 128, textures);
            assertTexture("speedM_1",          128, 128, textures);
            assertTexture("cap4can-o-jam",      64,  64, textures);
            assertTexture("can-o-jam",          64,  64, textures);
            assertTexture("eat_me",             64,  64, textures);
            assertTexture("coffin1",           128, 128, textures);
            assertTexture("coffin2",           128, 128, textures);
            assertTexture("czg_fronthole",     128, 128, textures);
            assertTexture("czg_backhole",      128, 128, textures);
            assertTexture("u_get_this",         64,  64, textures);
            assertTexture("for_sux-m-ass",      64,  64, textures);
            assertTexture("dex_5",             128, 128, textures);
            assertTexture("polished_turd",      64,  64, textures);
            assertTexture("crackpipes",        128, 128, textures);
            assertTexture("bongs2",            128, 128, textures);
            assertTexture("blowjob_machine",   128, 128, textures);
            assertTexture("lasthopeofhuman",   128, 128, textures);
            
            delete collection;
        }
    }
}
