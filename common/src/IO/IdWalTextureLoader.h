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

#ifndef IdWalTextureLoader_h
#define IdWalTextureLoader_h

#include "IO/WalTextureLoader.h"
#include "Assets/AssetTypes.h"

namespace TrenchBroom {
    namespace IO {
        class FileSystem;
        class Path;
        
        class IdWalTextureLoader : public WalTextureLoader {
        public:
            IdWalTextureLoader(const FileSystem& fs, const Assets::Palette& palette);
        private:
            Assets::Texture* doReadTexture(const IO::Path& path, MappedFile::Ptr file, const Assets::Palette& palette) const;
        };
    }
}

#endif /* IdWalTextureLoader_h */
