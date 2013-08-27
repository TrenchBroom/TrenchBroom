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

#include "WalTextureLoader.h"

#include "Color.h"
#include "ByteBuffer.h"
#include "Exceptions.h"
#include "StringUtils.h"
#include "Assets/FaceTexture.h"
#include "Assets/FaceTextureCollection.h"
#include "Assets/Palette.h"
#include "IO/FileSystem.h"
#include "IO/IOUtils.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        WalTextureLoader::WalTextureLoader(const Assets::Palette& palette) :
        m_palette(palette) {}

        Assets::FaceTextureCollection* WalTextureLoader::doLoadTextureCollection(const Path& path) {
            FileSystem fs;
            Path::List texturePaths = fs.directoryContents(path, FileSystem::FSFiles, "wal");
            std::sort(texturePaths.begin(), texturePaths.end());
            
            Assets::FaceTextureList textures;
            textures.reserve(texturePaths.size());
            
            Path::List::const_iterator it, end;
            for (it = texturePaths.begin(), end = texturePaths.end(); it != end; ++it) {
                const Path& texturePath = path + *it;
                Assets::FaceTexture* texture = readTexture(texturePath);
                textures.push_back(texture);
            }
            
            return new Assets::FaceTextureCollection(path, textures);
        }
        
        Assets::FaceTexture* WalTextureLoader::readTexture(const IO::Path& path) {
            FileSystem fs;
            MappedFile::Ptr file = fs.mapFile(path, std::ios::in);
            const char* cursor = file->begin();
            
            /*
            char textureNameC[33];
            textureNameC[32] = 0;
            readBytes(cursor, textureNameC, 32);
             */
            
            advance<char[32]>(cursor);
            const size_t width = readSize<uint32_t>(cursor);
            const size_t height = readSize<uint32_t>(cursor);
            
            const String textureName = path.suffix(2).deleteExtension().asString('/');
            return new Assets::FaceTexture(textureName, width, height);
        }

        void WalTextureLoader::doUploadTextureCollection(Assets::FaceTextureCollection* collection) {
            Buffer<unsigned char> buffer(3 * 512 * 512);
            FileSystem fs;
            Color averageColor;
            
            const IO::Path& collectionPath = collection->path();
            const Assets::FaceTextureList& textures = collection->textures();
            const size_t textureCount = textures.size();
            
            typedef std::vector<GLuint> TextureIdList;
            TextureIdList textureIds;
            textureIds.resize(textureCount);
            glEnable(GL_TEXTURE_2D);
            glGenTextures(textureCount, &textureIds[0]);
            
            for (size_t i = 0; i < textureCount; ++i) {
                Assets::FaceTexture* texture = textures[i];
                const String translatedTextureName = translateTextureName(texture->name());
                const Path& texturePath = collectionPath + IO::Path(translatedTextureName + ".wal").lastComponent();
                
                if (!fs.exists(texturePath)) {
                    glDeleteTextures(textureCount, &textureIds[0]);
                    throw ResourceNotFoundException("Cannot find wal texture: " + texture->name() + ".wal");
                }
                
                if (texture->width() * texture->height() > buffer.size())
                    buffer = Buffer<unsigned char>(texture->width() * texture->height());
                
                const GLuint textureId = textureIds[i];
                texture->setTextureId(textureId);
                
                glBindTexture(GL_TEXTURE_2D, textureId);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER , GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                
                MappedFile::Ptr file = fs.mapFile(texturePath, std::ios::in);
                const char* offsetCursor = file->begin() + 32 + 2*sizeof(uint32_t);

                for (size_t j = 0; j < 4; ++j) {
                    const size_t divisor = 1 << j;
                    const size_t offset = IO::readSize<int32_t>(offsetCursor);
                    const char* mipCursor = file->begin() + offset;
                    const size_t pixelCount = texture->width() * texture->height() / (divisor * divisor);
                    
                    m_palette.indexedToRgb(mipCursor, pixelCount, buffer, averageColor);
                    if (j == 0)
                        texture->setAverageColor(averageColor);
                    
                    glTexImage2D(GL_TEXTURE_2D, j, GL_RGBA,
                                 static_cast<GLsizei>(texture->width() / divisor),
                                 static_cast<GLsizei>(texture->height() / divisor),
                                 0, GL_RGB, GL_UNSIGNED_BYTE, &buffer[0]);
                }
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }

        String WalTextureLoader::translateTextureName(const String& textureName) {
            return StringUtils::replaceChars(textureName, "*", "#");
        }
    }
}
