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

#include "Color.h"
#include "IO/Reader.h"

#include <cassert>
#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class BufferedReader;
        class FileSystem;
        class Path;
    }

    namespace Assets {
        struct PaletteData;
        class TextureBuffer;

        enum class PaletteTransparency {
            Opaque, Index255Transparent
        };

        class Palette {
        private:
            std::shared_ptr<PaletteData> m_data;
        public:
            Palette();
            /**
             * @throws AssetException if data is not 768 bytes
             */
            Palette(const std::vector<unsigned char>& data);

            /**
             * @throws AssetException if the palette can't be loaded
             */
            static Palette loadFile(const IO::FileSystem& fs, const IO::Path& path);
            static Palette loadLmp(IO::Reader& reader);
            static Palette loadPcx(IO::Reader& reader);
            static Palette loadBmp(IO::Reader& reader);
            static Palette fromRaw(IO::Reader& reader);

            bool initialized() const;

            /**
             * Reads `pixelCount` bytes from `reader` where each byte is a palette index,
             * and writes `pixelCount` * 4 bytes to `rgbaImage` using the palette to convert
             * the image to RGBA.
             *
             * Must not be called if `initialized()` is false.
             *
             * @param reader the reader to read from; the position will be advanced
             * @param pixelCount number of pixels (bytes) to read
             * @param rgbaImage the destination buffer, size must be exactly `pixelCount` * 4 bytes
             * @param transparency controls whether or not the palette contains a transparent index
             * @param averageColor output parameter for the average color of the generated pixel buffer
             * @return true if the given index buffer did contain a transparent index, unless the transparency parameter
             *     indicates that the image is opaque
             *
             * @throws ReaderException if reader doesn't have pixelCount bytes available
             */
            bool indexedToRgba(IO::BufferedReader& reader, size_t pixelCount, TextureBuffer& rgbaImage, const PaletteTransparency transparency, Color& averageColor) const;
        };
    }
}

