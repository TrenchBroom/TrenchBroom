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

#include "StringUtils.h"
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
            static const size_t MaxMipLevels = 4;
            static Color tempColor, averageColor;
            static Assets::TextureBuffer::List buffers(MaxMipLevels);
            static size_t offsets[MaxMipLevels];

            assert(m_palette.initialized());

            const String name = reader.readString(WalLayout::TextureNameLength);
            const size_t width = reader.readSize<uint32_t>();
            const size_t height = reader.readSize<uint32_t>();

            const auto mipLevels = readMipOffsets(MaxMipLevels, offsets, width, height, reader);
            Assets::setMipBufferSize(buffers, mipLevels, width, height, GL_RGBA);
            readMips(m_palette, mipLevels, offsets, width, height, reader, buffers, averageColor);

            return new Assets::Texture(textureName(name, path), width, height, averageColor, buffers, GL_RGBA, Assets::TextureType::Opaque);
        }

        Assets::Texture* WalTextureReader::readDkWal(CharArrayReader& reader, const Path& path) const {
            static const size_t MaxMipLevels = 9;
            static Color tempColor, averageColor;
            static Assets::TextureBuffer::List buffers(MaxMipLevels);
            static size_t offsets[MaxMipLevels];

            const char version = reader.readChar<char>();
            ensure(version == 3, "Unknown WAL texture version");

            const auto name = reader.readString(WalLayout::TextureNameLength);
            reader.seekForward(3); // garbage

            const auto width = reader.readSize<uint32_t>();
            const auto height = reader.readSize<uint32_t>();

            const auto mipLevels = readMipOffsets(MaxMipLevels, offsets, width, height, reader);
            Assets::setMipBufferSize(buffers, mipLevels, width, height, GL_RGBA);

            reader.seekForward(32 + 2 * sizeof(uint32_t)); // animation name, flags, contents
            assert(reader.canRead(3 * 256));

            const auto palette = Assets::Palette::fromRaw(3 * 256, reader.cur<unsigned char>());
            readMips(palette, mipLevels, offsets, width, height, reader, buffers, averageColor);

            return new Assets::Texture(textureName(name, path), width, height, averageColor, buffers, GL_RGBA, Assets::TextureType::Opaque);
        }

        size_t WalTextureReader::readMipOffsets(const size_t maxMipLevels, size_t offsets[], const size_t width, const size_t height, CharArrayReader& reader) const {
            size_t mipLevels = 0;
            for (size_t i = 0; i < maxMipLevels; ++i) {
                offsets[i] = reader.readSize<uint32_t>();
                ++mipLevels;
                if (width / (1 << i) == 1 || height / (1 << i) == 1) {
                    break;
                }
            }

            // make sure the reader position is correct afterwards
            reader.seekForward((maxMipLevels - mipLevels) * sizeof(uint32_t));

            return mipLevels;
        }

        void WalTextureReader::readMips(const Assets::Palette& palette, const size_t mipLevels, const size_t offsets[], const size_t width, const size_t height, CharArrayReader& reader, Assets::TextureBuffer::List& buffers, Color& averageColor) {
            static Color tempColor;

            for (size_t i = 0; i < mipLevels; ++i) {
                const auto offset = offsets[i];
                reader.seekFromBegin(offset);
                const auto curWidth = width / (1 << i);
                const auto curHeight = height / (1 << i);
                const auto size = curWidth * curHeight;
                const auto* data = reader.cur<char>();

                // FIXME: Confirm this is actually happening because of bad data and not a bug.
                // FIXME: Corrupt or missing mips should be deleted, rather than uploaded with garbage.
                if (!reader.canRead(size)) {
                    std::cerr << "WalTextureReader::readMips: buffer overrun\n";
                    return;
                }

                palette.indexedToRgba(data, size, buffers[i], tempColor);
                if (i == 0) {
                    averageColor = tempColor;
                }
            }
        }
    }
}
