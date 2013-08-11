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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "WalTextureLoader.h"

#include "Assets/FaceTexture.h"
#include "Assets/FaceTextureCollection.h"
#include "IO/IOUtils.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        WalTextureLoader::WalTextureLoader(const Assets::Palette& palette) :
        m_palette(palette) {}

        Assets::FaceTextureCollection* WalTextureLoader::doLoadTextureCollection(const Path& path) {
            FileSystem fs;
            Path::List texturePaths = fs.directoryContents(path, FileSystem::FSFiles, "wal");
            std::sort(texturePaths.begin(), texturePaths.end());
            
            Assets::FaceTextureList textures;
            textures.reserve(texturePaths.size());
            
            Path::List::const_iterator it, end;
            for (it = texturePaths.begin(), end = texturePaths.end(); it != end; ++it) {
                const Path& texturePath = path + *it;
                MappedFile::Ptr file = fs.mapFile(texturePath, std::ios::in);
                Assets::FaceTexture* texture = readTexture(file);
                textures.push_back(texture);
            }
            
            return new Assets::FaceTextureCollection(path, textures);
        }
        
        Assets::FaceTexture* WalTextureLoader::readTexture(MappedFile::Ptr file) {
            const char* cursor = file->begin();
            char textureName[33];
            textureName[32] = 0;
            readBytes(cursor, textureName, 32);
            
            const size_t width = readSize<uint32_t>(cursor);
            const size_t height = readSize<uint32_t>(cursor);
            
            return new Assets::FaceTexture(textureName, width, height);
        }

        void WalTextureLoader::doUploadTextureCollection(Assets::FaceTextureCollection* collection) {
        }
    }
}
