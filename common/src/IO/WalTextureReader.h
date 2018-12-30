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

#ifndef IdWalTextureReader_h
#define IdWalTextureReader_h

#include "Color.h"
#include "IO/TextureReader.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace IO {
        class CharArrayReader;
        class Path;
        
        class WalTextureReader : public TextureReader {
        private:
            mutable Assets::Palette m_palette;
        public:
            WalTextureReader(const NameStrategy& nameStrategy, const Assets::Palette& palette = Assets::Palette());
        private:
            Assets::Texture* doReadTexture(MappedFile::Ptr file) const override;
            Assets::Texture* readQ2Wal(CharArrayReader& reader, const Path& path) const;
            Assets::Texture* readDkWal(CharArrayReader& reader, const Path& path) const;
            size_t readMipOffsets(size_t maxMipLevels, size_t offsets[], size_t width, size_t height, CharArrayReader& reader) const;
            static bool readMips(const Assets::Palette& palette, size_t mipLevels, const size_t offsets[], size_t width, size_t height, CharArrayReader& reader, Assets::TextureBuffer::List& buffers, Color& averageColor, Assets::PaletteTransparency transparency);
        };
    }
}

#endif /* IdWalTextureReader_h */
