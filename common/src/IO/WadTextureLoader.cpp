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

#include "WadTextureLoader.h"

#include "ByteBuffer.h"
#include "CollectionUtils.h"
#include "Color.h"
#include "Exceptions.h"
#include "Renderer/GL.h"
#include "IO/Wad.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureCollectionSpec.h"

#include <iterator>

namespace TrenchBroom {
    namespace IO {
        WadTextureLoader::WadTextureLoader(const Assets::Palette& palette) :
        m_palette(palette) {}
        
        Assets::TextureCollection* WadTextureLoader::doLoadTextureCollection(const Assets::TextureCollectionSpec& spec) const {
            Wad wad(spec.path());
            const WadEntryList mipEntries = wad.entriesWithType(WadEntryType::WEMip);
            const size_t textureCount = mipEntries.size();
            
            Assets::TextureList textures;
            textures.reserve(textureCount);
            
            try {
                for (size_t i = 0; i < textureCount; ++i) {
                    const WadEntry& entry = mipEntries[i];
                    Assets::Texture* texture = loadTexture(wad, entry);
                    textures.push_back(texture);
                }
                return new Assets::TextureCollection(spec.name(), textures);
            } catch (...) {
                VectorUtils::clearAndDelete(textures);
                throw;
            }
        }

        Assets::Texture* WadTextureLoader::loadTexture(const Wad& wad, const WadEntry& entry) const {
            static Color tempColor, averageColor;
            static Assets::TextureBuffer::List buffers(4);

            const MipSize mipSize = wad.mipSize(entry);
            Assets::setMipBufferSize(buffers, mipSize.width, mipSize.height);
            
            for (size_t j = 0; j < 4; ++j) {
                const MipData mipData = wad.mipData(entry, j);
                const size_t size = static_cast<size_t>(std::distance(mipData.begin, mipData.end));
                m_palette.indexedToRgb(mipData.begin, size, buffers[j], tempColor);
                if (j == 0)
                    averageColor = tempColor;
            }
            
            return new Assets::Texture(entry.name(), mipSize.width, mipSize.height, averageColor, buffers);
        }
    }
}
