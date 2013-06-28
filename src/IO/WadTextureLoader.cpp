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

#include "WadTextureLoader.h"

#include "GL/GL.h"
#include "IO/Wad.h"
#include "Model/Texture.h"
#include "Model/TextureCollection.h"

namespace TrenchBroom {
    namespace IO {
        Model::TextureCollection::Ptr WadTextureLoader::doLoadTextureCollection(const Path& path) {
            Wad wad(path);
            const WadEntryList mipEntries = wad.entriesWithType(WadEntryType::WEMip);

            Model::TextureList textures;
            textures.reserve(mipEntries.size());
            
            glEnable(GL_TEXTURE_2D);

            WadEntryList::const_iterator it, end;
            for (it = mipEntries.begin(), end = mipEntries.end(); it != end; ++it) {
                const WadEntry& entry = *it;
                const MipSize mipSize = wad.mipSize(entry);
                GLuint textureId = 0;
                
                glGenTextures(1, &textureId);
                glBindTexture(GL_TEXTURE_2D, textureId);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                for (size_t i = 1; i <= 4; ++i) {
                    const MipData mipData = wad.mipData(entry, i);
                    glTexImage2D(GL_TEXTURE_2D, i - 1, GL_RGBA,
                                 static_cast<GLsizei>(mipSize.width),
                                 static_cast<GLsizei>(mipSize.height),
                                 0, GL_RGB, GL_UNSIGNED_BYTE, mipData.begin);
                }
                glBindTexture(GL_TEXTURE_2D, 0);
                
                textures.push_back(Model::Texture::newTexture(textureId, entry.name(), mipSize.width, mipSize.height));
            }
            
            return Model::TextureCollection::newTextureCollection(path.asString(), textures);
        }
    }
}
