/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "TextureManager.h"

#include "Renderer/Palette.h"

namespace TrenchBroom {
    namespace Model {
        TextureCollectionLoader::TextureCollectionLoader(const String& path) :
        m_wad(path) {}
        
        unsigned char* TextureCollectionLoader::load(const Texture& texture, const Renderer::Palette& palette, Color& averageColor) {
            IO::Mip* mip = m_wad.loadMip(texture.name(), 1);
            if (mip == NULL)
                return NULL;
            
            size_t pixelCount = texture.width() * texture.height();
            unsigned char* rgbImage = new unsigned char[pixelCount * 3];
            palette.indexedToRgb(mip->mip0(), rgbImage, pixelCount, averageColor);
            delete mip;
            
            return rgbImage;
        }

        TextureCollection::TextureCollection(const String& name, const String& path) :
        m_name(name),
        m_path(path) {
            IO::Wad wad(m_path);
            IO::Mip::List mips = wad.loadMips(0);
            while (!mips.empty()) {
                IO::Mip* mip = mips.back();
                Texture* texture = new Texture(*this, mip->name(), mip->width(), mip->height());
                m_textures.push_back(texture);
                delete mip;
                mips.pop_back();
            }

            m_texturesByName = m_textures;
            m_texturesByUsage = m_textures;
            std::sort(m_texturesByName.begin(), m_texturesByName.end(), CompareTexturesByName());
        }
        
        TextureCollection::~TextureCollection() {
            while (!m_textures.empty()) delete m_textures.back(), m_textures.pop_back();
            m_texturesByName.clear();
            m_texturesByUsage.clear();
        }

        TextureCollection::LoaderPtr TextureCollection::loader() const {
            return LoaderPtr(new TextureCollectionLoader(m_path));
        }

        void TextureManager::reloadTextures() {
            m_collectionMap.clear();
            m_texturesCaseSensitive.clear();
            m_texturesCaseInsensitive.clear();
            m_texturesByName.clear();
            m_texturesByUsage.clear();
            
            typedef std::pair<TextureMap::iterator, bool> InsertResult;
            
            for (unsigned int i = 0; i < m_collections.size(); i++) {
                TextureCollection* collection = m_collections[i];
                const TextureList textures = collection->textures();
                for (unsigned int j = 0; j < textures.size(); j++) {
                    Texture* texture = textures[j];
                    m_collectionMap[texture] = collection;
                    
                    InsertResult result = m_texturesCaseSensitive.insert(TextureMapEntry(texture->name(), texture));
                    if (!result.second) { // texture with this name already existed
                        result.first->second->setOverridden(true);
                        result.first->second = texture;
                    }
                    m_texturesCaseInsensitive[Utility::toLower(texture->name())] = texture;
                    texture->setOverridden(false);
                }
            }

            TextureMap::iterator it, end;
            for (it = m_texturesCaseSensitive.begin(), end = m_texturesCaseSensitive.end(); it != end; ++it) {
                Texture* texture = it->second;
                m_texturesByName.push_back(texture);
                m_texturesByUsage.push_back(texture);
            }
            
            std::sort(m_texturesByName.begin(), m_texturesByName.end(), CompareTexturesByName());
        }
        
        TextureManager::~TextureManager() {
            clear();
        }

        void TextureManager::addCollection(TextureCollection* collection, size_t index) {
            assert(index <= m_collections.size());
            m_collections.insert(m_collections.begin() + index, collection);
            reloadTextures();
        }
        
        TextureCollection* TextureManager::removeCollection(size_t index) {
            assert(index < m_collections.size());
            TextureCollection* collection = m_collections[index];
            m_collections.erase(m_collections.begin() + index);
            reloadTextures();
            return collection;
        }
        
        size_t TextureManager::indexOfTextureCollection(const String& name) {
            size_t index = m_collections.size();
            size_t i = 0;
            TextureCollectionList::iterator it, end;
            for (it = m_collections.begin(), end = m_collections.end(); it != end; ++it) {
                if (name == (*it)->name())
                    index = i;
                i++;
            }
            return index;
        }
        
        void TextureManager::clear() {
            m_texturesCaseSensitive.clear();
            m_texturesCaseInsensitive.clear();
            while (!m_collections.empty()) delete m_collections.back(), m_collections.pop_back();
        }
    }
}