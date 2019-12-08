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

#ifndef WalTextureReader_h
#define WalTextureReader_h

#include "Assets/Palette.h"
#include "Assets/Texture.h" // get rid of this include by sanitizing TextureBuffer::List (make forward declaration)
#include "IO/TextureReader.h"

class Color;

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace IO {
        class Reader;
        class Path;

        class WalTextureReader : public TextureReader {
        private:
            mutable Assets::Palette m_palette;
        public:
            WalTextureReader(const NameStrategy& nameStrategy, const Assets::Palette& palette = Assets::Palette());
        private:
            Assets::Texture* doReadTexture(std::shared_ptr<File> file) const override;
            Assets::Texture* readQ2Wal(Reader& reader, const Path& path) const;
            Assets::Texture* readDkWal(Reader& reader, const Path& path) const;
            size_t readMipOffsets(size_t maxMipLevels, size_t offsets[], size_t width, size_t height, Reader& reader) const;
            static bool readMips(const Assets::Palette& palette, size_t mipLevels, const size_t offsets[], size_t width, size_t height, Reader& reader, Assets::TextureBuffer::List& buffers, Color& averageColor, Assets::PaletteTransparency transparency);
        };
    }
}

#endif /* WalTextureReader_h */
