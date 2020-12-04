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

#pragma once

#include "Assets/Palette.h"
#include "Assets/TextureBuffer.h"
#include "IO/TextureReader.h"

namespace TrenchBroom {
    class Color;
    class Logger;

    namespace IO {
        class File;
        class FileSystem;
        class Path;
        class Reader;

        class WalTextureReader : public TextureReader {
        private:
            mutable Assets::Palette m_palette;
        public:
            WalTextureReader(const NameStrategy& nameStrategy, const FileSystem& fs, Logger& logger, const Assets::Palette& palette = Assets::Palette());
        private:
            Assets::Texture doReadTexture(std::shared_ptr<File> file) const override;
            Assets::Texture readQ2Wal(BufferedReader& reader, const Path& path) const;
            Assets::Texture readDkWal(BufferedReader& reader, const Path& path) const;
            size_t readMipOffsets(size_t maxMipLevels, size_t offsets[], size_t width, size_t height, Reader& reader) const;
            static bool readMips(const Assets::Palette& palette, size_t mipLevels, const size_t offsets[], size_t width, size_t height, BufferedReader& reader, Assets::TextureBufferList& buffers, Color& averageColor, Assets::PaletteTransparency transparency);
        };
    }
}


