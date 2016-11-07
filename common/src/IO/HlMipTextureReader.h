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

#include "IO/TextureReader.h"
#include "Assets/Palette.h"
#include "IO/IdMipTextureReader.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
        class CharArrayReader;
        
        class HlMipTextureReader : public IdMipTextureReader {
        private:
        public:
            HlMipTextureReader(const NameStrategy& nameStrategy);
        protected:
            virtual const Assets::Palette getPalette(CharArrayReader &reader, const size_t offset[], const size_t width, const size_t height) const;
        };
    }
}

#endif /* HlMipTextureReader_h */
