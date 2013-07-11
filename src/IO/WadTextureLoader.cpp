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

#include "Color.h"
#include "Exceptions.h"
#include "GL/GL.h"
#include "IO/Wad.h"
#include "Model/Texture.h"
#include "Model/TextureCollection.h"

namespace TrenchBroom {
    namespace IO {
        WadTextureLoader::WadTextureLoader(const Model::Palette& palette) :
        m_palette(palette) {}

        Model::TextureCollection::Ptr WadTextureLoader::doLoadTextureCollection(const Path& path) {
            Wad wad(path);
            const WadEntryList mipEntries = wad.entriesWithType(WadEntryType::WEMip);
            const size_t textureCount = mipEntries.size();

            Model::Texture::List textures;
            textures.reserve(textureCount);

            for (size_t i = 0; i < textureCount; ++i) {
                const WadEntry& entry = mipEntries[i];
                const MipSize mipSize = wad.mipSize(entry);
                textures.push_back(Model::Texture::newTexture(entry.name(), mipSize.width, mipSize.height));
            }
            
            return Model::TextureCollection::newTextureCollection(path, textures);
        }

        void WadTextureLoader::doUploadTextureCollection(Model::TextureCollection::Ptr collection) {
            Model::Palette::TextureBuffer buffer;
            buffer.resize(InitialBufferSize);
            Color averageColor;
            
            const IO::Path& path = collection->path();
            Wad wad(path);

            const WadEntryList mipEntries = wad.entriesWithType(WadEntryType::WEMip);
            const Model::Texture::List& textures = collection->textures();
            
            if (mipEntries.size() != textures.size())
                throw WadException("Found different number of textures in " + path.asString() + " while uploading mip data");
            
            const size_t textureCount = mipEntries.size();
            
            glEnable(GL_TEXTURE_2D);
            
            typedef std::vector<GLuint> TextureIdList;
            TextureIdList textureIds;
            textureIds.resize(textureCount);
            glGenTextures(textureCount, &textureIds[0]);

            for (size_t i = 0; i < textureCount; ++i) {
                const WadEntry& entry = mipEntries[i];
                const Model::Texture::Ptr texture = textures[i];
                assert(entry.name() == texture->name());
                
                const GLuint textureId = textureIds[i];
                glBindTexture(GL_TEXTURE_2D, textureId);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                
                for (size_t i = 1; i <= 4; ++i) {
                    const MipData mipData = wad.mipData(entry, i);
                    m_palette.indexedToRgb(mipData.begin, texture->width() * texture->height(), buffer, averageColor);
                    if (i == 1)
                        texture->setAverageColor(averageColor);
                    glTexImage2D(GL_TEXTURE_2D, i - 1, GL_RGBA,
                                 static_cast<GLsizei>(texture->width()),
                                 static_cast<GLsizei>(texture->height()),
                                 0, GL_RGB, GL_UNSIGNED_BYTE, &buffer[0]);
                }
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }
}
