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


#ifndef FREEIMAGETEXTUREREADER_H
#define FREEIMAGETEXTUREREADER_H

#include "IO/TextureReader.h"

namespace TrenchBroom {
    namespace IO {
        class Path;

        class FreeImageTextureReader : public TextureReader {
        public:
            FreeImageTextureReader(const NameStrategy& nameStrategy);
        private:
            Assets::Texture* doReadTexture(const char* const begin, const char* const end, const Path& path) const override;
        };
    }
}

#endif // FREEIMAGETEXTUREREADER_H
