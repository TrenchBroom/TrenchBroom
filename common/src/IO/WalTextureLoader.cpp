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

#include "WalTextureLoader.h"

#include "CollectionUtils.h"
#include "Color.h"
#include "ByteBuffer.h"
#include "Exceptions.h"
#include "StringUtils.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureCollectionSpec.h"
#include "IO/DiskFileSystem.h"
#include "IO/IOUtils.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        WalTextureLoader::WalTextureLoader(const FileSystem& fs, const Assets::Palette& palette) :
        m_fs(fs),
        m_palette(palette) {}
        
        Assets::TextureCollection* WalTextureLoader::doLoadTextureCollection(const Assets::TextureCollectionSpec& spec) const {
            Path::List texturePaths = m_fs.findItems(spec.path(), FileSystem::ExtensionMatcher("wal"));
            std::sort(texturePaths.begin(), texturePaths.end());
            
            Assets::TextureList textures;
            textures.reserve(texturePaths.size());
            
            try {
                Path::List::const_iterator it, end;
                for (it = texturePaths.begin(), end = texturePaths.end(); it != end; ++it) {
                    const Path& texturePath = *it;
                    Assets::Texture* texture = readTexture(texturePath);
                    textures.push_back(texture);
                }
                
                return new Assets::TextureCollection(spec.name(), textures);
            } catch (...) {
                VectorUtils::clearAndDelete(textures);
                throw;
            }
        }
        
        Assets::Texture* WalTextureLoader::readTexture(const IO::Path& path) const {
            MappedFile::Ptr file = m_fs.openFile(path);
            const char* cursor = file->begin();
            
            advance<char[32]>(cursor);
            const size_t width = readSize<uint32_t>(cursor);
            const size_t height = readSize<uint32_t>(cursor);
            const String textureName = path.suffix(2).deleteExtension().asString('/');
            
            Color tempColor, averageColor;
            Assets::TextureBuffer::List buffers(4);
            Assets::setMipBufferSize(buffers, width, height);
            
            const char* offsetCursor = file->begin() + 32 + 2*sizeof(uint32_t);
            for (size_t i = 0; i < 4; ++i) {
                const size_t divisor = 1 << i;
                const size_t offset = IO::readSize<int32_t>(offsetCursor);
                const char* mipCursor = file->begin() + offset;
                const size_t pixelCount = (width * height) / (divisor * divisor);
                
                m_palette.indexedToRgb(mipCursor, pixelCount, buffers[i], tempColor);
                if (i == 0)
                    averageColor = tempColor;
            }
            
            return new Assets::Texture(textureName, width, height, averageColor, buffers);
        }
    }
}
