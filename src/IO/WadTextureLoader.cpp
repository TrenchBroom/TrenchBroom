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

#include "WadTextureLoader.h"

#include "IO/Wad.h"
#include "Model/Texture.h"
#include "Model/TextureCollection.h"

namespace TrenchBroom {
    namespace IO {
        Model::TextureCollection::Ptr WadTextureLoader::doLoadTextureCollection(const Path& path) {
            Wad wad(path);
            const WadEntryList mipEntries = wad.entriesWithType(WadEntryType::WEMip);

            Model::TextureList textures;
            textures.reserve(mipEntries.size());
            
            WadEntryList::const_iterator it, end;
            for (it = mipEntries.begin(), end = mipEntries.end(); it != end; ++it) {
                const WadEntry& entry = *it;
                const MipSize mipSize = wad.mipSize(entry);
                textures.push_back(Model::Texture::newTexture(entry.name(), mipSize.width, mipSize.height));
            }
            
            return Model::TextureCollection::newTextureCollection(path.asString(), textures);
        }
    }
}
