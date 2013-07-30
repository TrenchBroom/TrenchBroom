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
#include <gmock/gmock.h>

#include "IO/Path.h"
#include "IO/WadTextureLoader.h"
#include "Assets/Palette.h"
#include "Assets/FaceTexture.h"
#include "Assets/FaceTextureCollection.h"

namespace TrenchBroom {
    namespace IO {
        inline void assertTexture(const String& name, const size_t width, const size_t height, Assets::FaceTexture* texture) {
            ASSERT_EQ(name, texture->name());
            ASSERT_EQ(width, texture->width());
            ASSERT_EQ(height, texture->height());
        }
        
        TEST(WadTextureLoaderTest, testLoadWad) {
            using namespace testing;
            InSequence forceInSequenceMockCalls;
            std::srand(static_cast<unsigned int>(std::time(NULL)));
            
            GLMock = new CGLMock();
            Mock::AllowLeak(GLMock);

            typedef std::vector<GLuint> TextureIdList;
            TextureIdList mockIds;
            for (size_t i = 0; i < 21; ++i)
                mockIds.push_back(i);
            
            EXPECT_CALL(*GLMock, Enable(GL_TEXTURE_2D));
            EXPECT_CALL(*GLMock, GenTextures(21,_)).WillOnce(SetArrayArgument<1>(mockIds.begin(), mockIds.end()));
            
            for (size_t i = 0; i < 21; ++i) {
                EXPECT_CALL(*GLMock, BindTexture(GL_TEXTURE_2D, mockIds[i]));
                EXPECT_CALL(*GLMock, TexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
                EXPECT_CALL(*GLMock, TexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST));
                EXPECT_CALL(*GLMock, TexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
                EXPECT_CALL(*GLMock, TexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
                for (size_t j = 0; j < 4; j++)
                    EXPECT_CALL(*GLMock, TexImage2D(GL_TEXTURE_2D, j, GL_RGBA, _, _, 0, GL_RGB, GL_UNSIGNED_BYTE, _));
                EXPECT_CALL(*GLMock, BindTexture(GL_TEXTURE_2D, 0));
            }
            
            const Assets::Palette palette(Path("data/palette.lmp"));
            WadTextureLoader loader(palette);
            
            const Path wadPath("data/IO/Wad/cr8_czg.wad");
            Assets::FaceTextureCollection* collection = loader.loadTextureCollection(wadPath);
            
            const Assets::FaceTextureList& textures = collection->textures();
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
        }
    }
}
