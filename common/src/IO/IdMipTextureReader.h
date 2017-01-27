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

#ifndef IdMipTextureReader_h
#define IdMipTextureReader_h

#include "IO/TextureReader.h"
#include "Assets/Palette.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class IdMipTextureReader : public TextureReader {
        private:
            const Assets::Palette m_palette;
        public:
            IdMipTextureReader(const NameStrategy& nameStrategy, const Assets::Palette& palette);
            static size_t mipFileSize(size_t width, size_t height, size_t mipLevels);
        private:
            Assets::Texture* doReadTexture(const char* const begin, const char* const end, const Path& path) const;
        };
    }
}

#endif /* IdMipTextureReader_h */
