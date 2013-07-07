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

#include "TextureManager.h"

namespace TrenchBroom {
    namespace Model {
        void TextureManager::addTextureCollection(const IO::Path& path) {
            assert(m_game != NULL);
            
            if (m_collectionsByPath.count(path) > 0)
                return;

            TextureCollectionMap::iterator it = m_toRemove.find(path);
            if (it != m_toRemove.end()) {
                TextureCollection::Ptr collection = it->second;
                m_toRemove.erase(it);
                doAddTextureCollection(collection);
            } else {
                TextureCollection::Ptr collection = m_game->loadTextureCollection(path);
                doAddTextureCollection(collection);
            }
        }
        
        void TextureManager::removeTextureCollection(const size_t index) {
            assert(index < m_collections.size());
            
            TextureCollection::List::iterator it = m_collections.begin() + index;
            TextureCollection::Ptr collection = *it;
            
            m_collections.erase(it);
            m_collectionsByPath.erase(collection->path());
            m_toUpload.erase(collection->path());
            m_toRemove.insert(TextureCollectionMapEntry(collection->path(), collection));
        }
        
        void TextureManager::reset(Game::Ptr game) {
            m_toRemove.insert(m_collectionsByPath.begin(), m_collectionsByPath.end());
            m_collections.clear();
            m_collectionsByPath.clear();
            m_toUpload.clear();
            m_game = game;
        }
        
        void TextureManager::commitChanges() {
            assert(m_game != NULL);
            
            TextureCollectionMap::iterator it, end;
            for (it = m_toUpload.begin(), end = m_toUpload.end(); it != end; ++it) {
                TextureCollection::Ptr collection = it->second;
                m_game->uploadTextureCollection(collection);
            }

            m_toUpload.clear();
            m_toRemove.clear();
        }

        void TextureManager::doAddTextureCollection(TextureCollection::Ptr collection) {
            m_toUpload.insert(TextureCollectionMapEntry(collection->path(), collection));
            m_collectionsByPath.insert(TextureCollectionMapEntry(collection->path(), collection));
            m_collections.push_back(collection);
        }
    }
}
