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

#include "IO/Wad.h"

namespace TrenchBroom {
    namespace Model {
        TextureCollection::TextureCollection(const String& name, const IO::Wad& wad, const Palette& palette) : m_name(name) {
            IO::Mip::List mips = wad.loadMips();
            while (!mips.empty()) {
                IO::Mip* mip = mips.back();
                Texture* texture = new Texture(*mip, palette);
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

        void TextureManager::reloadTextures() {
            m_texturesCaseSensitive.clear();
            m_texturesCaseInsensitive.clear();
            m_texturesByName.clear();
            m_texturesByUsage.clear();
            
            for (unsigned int i = 0; i < m_collections.size(); i++) {
                TextureCollection* collection = m_collections[i];
                const TextureList textures = collection->textures();
                for (unsigned int j = 0; j < textures.size(); j++) {
                    Texture* texture = textures[j];
                    m_texturesCaseSensitive.insert(std::pair<String, Texture*>(texture->name(), texture));
                    m_texturesCaseInsensitive.insert(std::pair<String, Texture*>(Utility::toLower(texture->name()), texture));
                    m_texturesByName.push_back(texture);
                    m_texturesByUsage.push_back(texture);
                }
            }

            for (unsigned int i = 0; i < m_collections.size(); i++) {
                TextureCollection* collection = m_collections[i];
                const TextureList textures = collection->textures();
                for (unsigned int j = 0; j < textures.size(); j++) {
                    Texture* texture = textures[j];
                    texture->setOverridden(m_texturesCaseSensitive.find(texture->name())->second != texture);
                }
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
        
        void TextureManager::removeCollection(size_t index) {
            assert(index < m_collections.size());
            delete m_collections[index];
            m_collections.erase(m_collections.begin() + index);
            reloadTextures();
        }
        
        void TextureManager::removeCollection(const std::string& name) {
            size_t index = m_collections.size();
            size_t i = 0;
            TextureCollectionList::iterator it, end;
            for (it = m_collections.begin(), end = m_collections.end(); it != end; ++it) {
                if (name == (*it)->name())
                    index = i;
                i++;
            }
            
            if (index < m_collections.size())
                removeCollection(index);
        }
        
        void TextureManager::clear() {
            m_texturesCaseSensitive.clear();
            m_texturesCaseInsensitive.clear();
            while (!m_collections.empty()) delete m_collections.back(), m_collections.pop_back();
        }
    }
}