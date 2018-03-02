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

#include "WalTextureReader.h"

#include "Color.h"
#include "StringUtils.h"
#include "Assets/Texture.h"
#include "IO/CharArrayReader.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace IO {
        namespace WalLayout {
            const size_t TextureNameLength = 32;
        }
        
        WalTextureReader::WalTextureReader(const NameStrategy& nameStrategy, const Assets::Palette& palette) :
        TextureReader(nameStrategy),
        m_palette(palette) {}
        
        Assets::Texture* WalTextureReader::doReadTexture(const char* const begin, const char* const end, const Path& path) const {
            CharArrayReader reader(begin, end);
            const char version = reader.readChar<char>();
            reader.seekFromBegin(0);

            if (version == 3) {
                return readDkWal(reader, path);
            } else {
                return readQ2Wal(reader, path);
            }
        }

        Assets::Texture* WalTextureReader::readQ2Wal(CharArrayReader& reader, const Path& path) const {
            static const size_t MipLevels = 4;
            static Color tempColor, averageColor;
            static Assets::TextureBuffer::List buffers(MipLevels);
            static size_t offsets[MipLevels];

            assert(m_palette.initialized());

            const String name = reader.readString(WalLayout::TextureNameLength);
            const size_t width = reader.readSize<uint32_t>();
            const size_t height = reader.readSize<uint32_t>();

            Assets::setMipBufferSize(buffers, width, height);

            for (size_t i = 0; i < MipLevels; ++i)
                offsets[i] = reader.readSize<uint32_t>();

            for (size_t i = 0; i < MipLevels; ++i) {
                reader.seekFromBegin(offsets[i]);
                const size_t size = mipSize(width, height, i);
                const char* data = reader.cur<char>();

                m_palette.indexedToRgb(data, size, buffers[i], tempColor);
                if (i == 0)
                    averageColor = tempColor;
            }

            return new Assets::Texture(textureName(name, path), width, height, averageColor, buffers);
        }

        Assets::Texture* WalTextureReader::readDkWal(CharArrayReader& reader, const Path& path) const {
            static const size_t MipLevels = 9;
            static Color tempColor, averageColor;
            static Assets::TextureBuffer::List buffers(MipLevels);
            static size_t offsets[MipLevels];

            const char version = reader.readChar<char>();
            ensure(version == 3, "Unknown WAL texture version");

            const auto name = reader.readString(WalLayout::TextureNameLength);
            reader.seekForward(3); // garbage

            const auto width = reader.readSize<uint32_t>();
            const auto height = reader.readSize<uint32_t>();

            Assets::setMipBufferSize(buffers, width, height);

            for (size_t i = 0; i < MipLevels; ++i)
                offsets[i] = reader.readSize<uint32_t>();

            reader.seekForward(32 + 2 * sizeof(uint32_t)); // animation name, flags, contents
            assert(reader.canRead(3 * 256));

            if (!m_palette.initialized()) {
                m_palette = Assets::Palette::fromRaw(3 * 256, reader.cur<unsigned char>());
            }

            size_t curWidth = width;
            size_t curHeight = height;
            for (size_t i = 0; i < MipLevels; ++i) {
                const auto offset = offsets[i];
                reader.seekFromBegin(offset);
                const auto size = curWidth * curHeight;
                const auto* data = reader.cur<char>();

                m_palette.indexedToRgb(data, size, buffers[i], tempColor);
                if (i == 0)
                    averageColor = tempColor;

                // stop loading once we have loaded a mip of width or height 1
                if (curWidth == 1 || curHeight == 1) {
                    break;
                } else {
                    curWidth  /= 2;
                    curHeight /= 2;
                }
            }

            return new Assets::Texture(textureName(name, path), width, height, averageColor, buffers);
        }
    }
}
