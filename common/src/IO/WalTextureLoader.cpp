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

#include "WalTextureLoader.h"

#include "CollectionUtils.h"
#include "Color.h"
#include "ByteBuffer.h"
#include "Exceptions.h"
#include "StringUtils.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureCollectionSpec.h"
#include "IO/DiskFileSystem.h"
#include "IO/FileMatcher.h"
#include "IO/IOUtils.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        WalTextureLoader::WalTextureLoader(const FileSystem& fs, const Assets::Palette& palette) :
        m_fs(fs),
        m_palette(palette) {}
        
        WalTextureLoader::~WalTextureLoader() {}
        
        Assets::TextureCollection* WalTextureLoader::doLoadTextureCollection(const Assets::TextureCollectionSpec& spec) const {
            Path::List texturePaths = m_fs.findItems(spec.path(), FileExtensionMatcher("wal"));
            std::sort(texturePaths.begin(), texturePaths.end());
            
            Assets::TextureList textures;
            textures.reserve(texturePaths.size());
            
            try {
                Path::List::const_iterator it, end;
                for (it = texturePaths.begin(), end = texturePaths.end(); it != end; ++it) {
                    const Path& texturePath = *it;
                    Assets::Texture* texture = doReadTexture(texturePath, m_fs.openFile(texturePath), m_palette);
                    textures.push_back(texture);
                }
                
                return new Assets::TextureCollection(spec.name(), textures);
            } catch (...) {
                VectorUtils::clearAndDelete(textures);
                throw;
            }
        }
    }
}
