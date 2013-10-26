/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__WalTextureLoader__
#define __TrenchBroom__WalTextureLoader__

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
            Assets::FaceTextureCollection* doLoadTextureCollection(const Path& path);
            Assets::FaceTexture* readTexture(const Path& path);
            void doUploadTextureCollection(Assets::FaceTextureCollection* collection);
            static String translateTextureName(const String& textureName);
        };
    }
}

#endif /* defined(__TrenchBroom__WalTextureLoader__) */
