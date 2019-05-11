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

#ifndef MipTextureReader_h
#define MipTextureReader_h

#include "IO/TextureReader.h"
#include "Assets/Palette.h"

namespace TrenchBroom {
    namespace IO {
        class File;
        class Path;

        namespace MipLayout {
            const size_t TextureNameLength = 16;
        }

        class MipTextureReader : public TextureReader {
        protected:
            explicit MipTextureReader(const NameStrategy& nameStrategy);
        public:
            ~MipTextureReader() override;
        public:
            static size_t mipFileSize(size_t width, size_t height, size_t mipLevels);
        protected:
            Assets::Texture* doReadTexture(std::shared_ptr<File> file) const override;
            virtual Assets::Palette doGetPalette(Reader& reader, const size_t offset[], size_t width, size_t height) const = 0;
        };
    }
}

#endif /* MipTextureReader_h */
