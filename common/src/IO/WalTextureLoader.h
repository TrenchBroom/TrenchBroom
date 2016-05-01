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

#ifndef TrenchBroom_WalTextureLoader
#define TrenchBroom_WalTextureLoader

#include "IO/TextureLoader.h"

#include "Assets/AssetTypes.h"
#include "IO/MappedFile.h"

namespace TrenchBroom {
    namespace IO {
        class FileSystem;
        class PaletteLoader;
        class Path;
        
        class WalTextureLoader : public TextureLoader {
        private:
            const FileSystem& m_fs;
            const PaletteLoader* m_paletteLoader;
        protected:
            WalTextureLoader(const FileSystem& fs, const PaletteLoader* paletteLoader);
        public:
            virtual ~WalTextureLoader();
        private:
            Assets::TextureCollection* doLoadTextureCollection(const Assets::TextureCollectionSpec& spec) const;
            virtual Assets::Texture* doReadTexture(const IO::Path& path, MappedFile::Ptr file, const PaletteLoader* paletteLoader) const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_WalTextureLoader) */
