/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "IdMipTextureReader.h"

#include "Color.h"
#include "StringUtils.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "IO/CharArrayReader.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace IO {
        namespace MipLayout {
            static const size_t TextureNameLength = 16;
        }

        IdMipTextureReader::IdMipTextureReader(const NameStrategy& nameStrategy, const Assets::Palette& palette) :
        TextureReader(nameStrategy),
        m_palette(palette) {}

        size_t IdMipTextureReader::mipFileSize(const size_t width, const size_t height, const size_t mipLevels) {
            size_t result = 0;
            for (size_t i = 0; i < mipLevels; ++i)
                result += mipSize(width, height, i);
            return result;
        }

        Assets::Texture* IdMipTextureReader::doReadTexture(const char* const begin, const char* const end, const Path& path) const {
            static const size_t MipLevels = 4;
            
            static Color tempColor, averageColor;
            static Assets::TextureBuffer::List buffers(MipLevels);
            static size_t offset[MipLevels];
            
            CharArrayReader reader(begin, end);
            const String name = reader.readString(MipLayout::TextureNameLength);
            const size_t width = reader.readSize<int32_t>();
            const size_t height = reader.readSize<int32_t>();
            for (size_t i = 0; i < MipLevels; ++i)
                offset[i] = reader.readSize<int32_t>();
            
            Assets::setMipBufferSize(buffers, width, height);
            
            for (size_t i = 0; i < MipLevels; ++i) {
                const char* data = begin + offset[i];
                const size_t size = mipSize(width, height, i);
                
                m_palette.indexedToRgb(data, size, buffers[i], tempColor);
                if (i == 0)
                    averageColor = tempColor;
                
            }

            return new Assets::Texture(textureName(name, path), width, height, averageColor, buffers);
        }
    }
}
