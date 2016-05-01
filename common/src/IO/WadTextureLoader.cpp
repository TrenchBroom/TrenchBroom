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
#include "Assets/Palette.h"
#include "Renderer/GL.h"
#include "IO/CharArrayReader.h"
#include "IO/PaletteLoader.h"
#include "IO/WadFileSystem.h"
#include "Assets/Palette.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureCollectionSpec.h"

#include <iterator>

namespace TrenchBroom {
    namespace IO {
        namespace MipLayout {
            static const size_t WidthOffset = 16;
        }
        
        WadTextureLoader::WadTextureLoader(const PaletteLoader* paletteLoader) :
        m_paletteLoader(paletteLoader) {
            assert(m_paletteLoader != NULL);
        }
        
        Assets::TextureCollection* WadTextureLoader::doLoadTextureCollection(const Assets::TextureCollectionSpec& spec) const {
            const WadFileSystem fs(spec.path());
            const IO::Path::List files = fs.findItems(Path(""));
            const size_t textureCount = files.size();
            
            Assets::TextureList textures;
            textures.reserve(textureCount);
            
            try {
                for (size_t i = 0; i < textureCount; ++i) {
                    const IO::Path& path = files[i];
                    const MappedFile::Ptr file = fs.openFile(path);
                    textures.push_back(loadMipTexture(path.asString(), file, m_paletteLoader));
                }
                return new Assets::TextureCollection(spec.name(), textures);
            } catch (...) {
                VectorUtils::clearAndDelete(textures);
                throw;
            }
        }

        Assets::Texture* WadTextureLoader::loadMipTexture(const String& name, MappedFile::Ptr file, const PaletteLoader* paletteLoader) {
            static const size_t MipLevels = 4;
            
            static Color tempColor, averageColor;
            static Assets::TextureBuffer::List buffers(MipLevels);
            static size_t offset[MipLevels];

            Assets::Palette::Ptr palette = paletteLoader->loadPalette(file);

            CharArrayReader reader(file->begin(), file->end());
            reader.seekFromBegin(MipLayout::WidthOffset);
            const size_t width = reader.readSize<int32_t>();
            const size_t height = reader.readSize<int32_t>();
            for (size_t i = 0; i < MipLevels; ++i)
                offset[i] = reader.readSize<int32_t>();
            
            Assets::setMipBufferSize(buffers, width, height);
            
            for (size_t i = 0; i < MipLevels; ++i) {
                const char* data = file->begin() + offset[i];
                const size_t size = mipSize(width, height, i);
                
                palette->indexedToRgb(data, size, buffers[i], tempColor);
                if (i == 0)
                    averageColor = tempColor;
                
            }
            
            return new Assets::Texture(name, width, height, averageColor, buffers);
        }

        size_t WadTextureLoader::mipFileSize(const size_t width, const size_t height, const size_t mipLevels) {
            size_t result = 0;
            for (size_t i = 0; i < mipLevels; ++i)
                result += mipSize(width, height, i);
            return result;
        }
    }
}
