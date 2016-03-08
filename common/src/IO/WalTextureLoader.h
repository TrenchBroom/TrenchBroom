/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_WalTextureLoader
#define TrenchBroom_WalTextureLoader

#include "IO/TextureLoader.h"
#include "Assets/AssetTypes.h"

namespace TrenchBroom {
    namespace IO {
        class FileSystem;
        class Path;
        
        class WalTextureLoader : public TextureLoader {
        private:
            const FileSystem& m_fs;
            const Assets::Palette& m_palette;
        public:
            WalTextureLoader(const FileSystem& fs, const Assets::Palette& palette);
        private:
            Assets::TextureCollection* doLoadTextureCollection(const Assets::TextureCollectionSpec& spec) const;
            Assets::Texture* readTexture(const Path& path) const;
        };
    }
}

#endif /* defined(TrenchBroom_WalTextureLoader) */
