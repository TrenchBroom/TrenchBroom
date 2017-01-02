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

#include "IdMipTextureReader.h"

#include "Color.h"
#include "StringUtils.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "IO/CharArrayReader.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace IO {
        IdMipTextureReader::IdMipTextureReader(const NameStrategy& nameStrategy, const Assets::Palette& palette) :
        MipTextureReader(nameStrategy),
        m_palette(palette) {}
        
        Assets::Palette IdMipTextureReader::doGetPalette(CharArrayReader& reader, const size_t offset[], const size_t width, const size_t height) const {
            return m_palette;
        }
    }
}
