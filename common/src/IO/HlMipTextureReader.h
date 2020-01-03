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

#ifndef HlMipTextureReader_h
#define HlMipTextureReader_h

#include "IO/MipTextureReader.h"

namespace TrenchBroom {
    namespace IO {
        class Reader;

        class HlMipTextureReader : public MipTextureReader {
        public:
            HlMipTextureReader(const NameStrategy& nameStrategy);
        protected:
            Assets::Palette doGetPalette(Reader& reader, const size_t offset[], size_t width, size_t height) const override;
        };
    }
}

#endif /* HlMipTextureReader_h */
