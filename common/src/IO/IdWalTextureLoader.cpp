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

#include "IdWalTextureLoader.h"

#include "CollectionUtils.h"
#include "Color.h"
#include "ByteBuffer.h"
#include "Exceptions.h"
#include "StringUtils.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureCollectionSpec.h"
#include "IO/CharArrayReader.h"
#include "IO/DiskFileSystem.h"
#include "IO/FileMatcher.h"
#include "IO/IOUtils.h"
#include "IO/PaletteLoader.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        IdWalTextureLoader::IdWalTextureLoader(const FileSystem& fs, const PaletteLoader* paletteLoader) :
        WalTextureLoader(fs, paletteLoader) {}
        
        Assets::Texture* IdWalTextureLoader::doReadTexture(const IO::Path& path, MappedFile::Ptr file, const PaletteLoader* paletteLoader) const {
            static const size_t MipLevels = 4;
            static Color tempColor, averageColor;
            static Assets::TextureBuffer::List buffers(MipLevels);
            static size_t offset[MipLevels];

            const Assets::Palette::Ptr palette = paletteLoader->loadPalette(file);

            CharArrayReader reader(file->begin(), file->end());
            reader.seekFromBegin(32);
            
            const size_t width = reader.readSize<uint32_t>();
            const size_t height = reader.readSize<uint32_t>();
            const String textureName = path.suffix(2).deleteExtension().asString('/');
            
            Assets::setMipBufferSize(buffers, width, height);

            reader.seekFromBegin(32 + 2 * sizeof(uint32_t));
            for (size_t i = 0; i < MipLevels; ++i)
                offset[i] = reader.readSize<int32_t>();
            
            for (size_t i = 0; i < MipLevels; ++i) {
                reader.seekFromBegin(offset[i]);
                const size_t size = mipSize(width, height, i);
                const char* data = file->begin() + offset[i];

                palette->indexedToRgb(data, size, buffers[i], tempColor);
                if (i == 0)
                    averageColor = tempColor;
            }
            
            return new Assets::Texture(textureName, width, height, averageColor, buffers);
        }
    }
}
