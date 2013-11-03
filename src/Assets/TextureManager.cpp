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

#include "TextureManager.h"

#include "Exceptions.h"
#include "CollectionUtils.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Model/Game.h"

namespace TrenchBroom {
    namespace Assets {
        class CompareByName {
        public:
            CompareByName() {}
            bool operator() (const Texture* left, const Texture* right) const {
                return left->name() < right->name();
            }
        };
        
        class CompareByUsage {
        public:
            bool operator() (const Texture* left, const Texture* right) const {
                if (left->usageCount() == right->usageCount())
                    return left->name() < right->name();
                return left->usageCount() > right->usageCount();
            }
        };

        TextureManager::~TextureManager() {
            VectorUtils::clearAndDelete(m_collections);
        }

        void TextureManager::addTextureCollection(const IO::Path& path) {
            assert(m_game != NULL);
            doAddTextureCollection(path, m_collections, m_collectionsByPath, m_toRemove);
            updateTextures();
        }
        
        void TextureManager::addTextureCollections(const IO::Path::List& paths) {
            assert(m_game != NULL);

            if (paths.empty())
                return;
            
            TextureCollectionList collections;
            TextureCollectionMap collectionsByPath;
            TextureCollectionMap toRemove = m_toRemove;
            
            try {
                IO::Path::List::const_iterator it, end;
                for (it = paths.begin(), end = paths.end(); it != end; ++it) {
                    const IO::Path& path = *it;
                    doAddTextureCollection(path, collections, collectionsByPath, toRemove);
                }
            } catch (...) {
                VectorUtils::clearAndDelete(collections);
                throw;
            }

            m_collections.insert(m_collections.end(), collections.begin(), collections.end());
            m_collectionsByPath.insert(collectionsByPath.begin(), collectionsByPath.end());
            m_toRemove = toRemove;
            
            updateTextures();
        }

        void TextureManager::removeTextureCollection(const IO::Path& path) {
            TextureCollectionMap::iterator it = m_collectionsByPath.find(path);
            if (it == m_collectionsByPath.end())
                throw AssetException("Could not find texture collection: '" + path.asString() + "'");
            
            TextureCollection* collection = it->second;
            VectorUtils::remove(m_collections, collection);

            m_collectionsByPath.erase(it);
            m_toRemove.insert(TextureCollectionMapEntry(path, collection));
            updateTextures();
        }
        
        void TextureManager::reset(Model::GamePtr game) {
            m_toRemove.insert(m_collectionsByPath.begin(), m_collectionsByPath.end());
            m_collections.clear();
            m_collectionsByPath.clear();
            m_game = game;
            updateTextures();
        }
        
        void TextureManager::commitChanges() {
            MapUtils::clearAndDelete(m_toRemove);
        }

        Texture* TextureManager::texture(const String& name) const {
            TextureMap::const_iterator it = m_texturesByName.find(name);
            if (it == m_texturesByName.end())
                return NULL;
            return it->second;
        }

        const TextureList& TextureManager::textures(const SortOrder sortOrder) const {
            return m_sortedTextures[sortOrder];
        }

        const TextureManager::GroupList& TextureManager::groups(const SortOrder sortOrder) const {
            return m_sortedGroups[sortOrder];
        }

        const TextureCollectionList& TextureManager::collections() const {
            return m_collections;
        }

        void TextureManager::doAddTextureCollection(const IO::Path& path, TextureCollectionList& collections, TextureCollectionMap& collectionsByPath, TextureCollectionMap& toRemove) {
            if (collectionsByPath.count(path) > 0)
                return;
            
            TextureCollectionMap::iterator it = toRemove.find(path);
            if (it != toRemove.end()) {
                TextureCollection* collection = it->second;
                toRemove.erase(it);
                doAddTextureCollection(path, collection, collections, collectionsByPath);
            } else {
                TextureCollection* collection = m_game->loadTextureCollection(path);
                doAddTextureCollection(path, collection, collections, collectionsByPath);
            }
        }

        void TextureManager::doAddTextureCollection(const IO::Path& path, TextureCollection* collection, TextureCollectionList& collections, TextureCollectionMap& collectionsByPath) {
            collectionsByPath.insert(TextureCollectionMapEntry(path, collection));
            collections.push_back(collection);
        }

        void TextureManager::updateTextures() {
            m_texturesByName.clear();
            m_sortedGroups[Name].clear();
            m_sortedGroups[Usage].clear();
            
            TextureCollectionList::iterator cIt, cEnd;
            for (cIt = m_collections.begin(), cEnd = m_collections.end(); cIt != cEnd; ++cIt) {
                TextureCollection* collection = *cIt;
                const TextureList textures = collection->textures();
                
                TextureList::const_iterator tIt, tEnd;
                for (tIt = textures.begin(), tEnd = textures.end(); tIt != tEnd; ++tIt) {
                    Texture* texture = *tIt;
                    texture->setOverridden(false);
                    
                    TextureMap::iterator mIt = m_texturesByName.find(texture->name());
                    if (mIt != m_texturesByName.end()) {
                        mIt->second->setOverridden(true);
                        mIt->second = texture;
                    } else {
                        m_texturesByName.insert(std::make_pair(texture->name(), texture));
                    }
                }

                const Group group = std::make_pair(collection, textures);
                m_sortedGroups[Name].push_back(group);
                m_sortedGroups[Usage].push_back(group);
                std::sort(m_sortedGroups[Name].back().second.begin(),
                          m_sortedGroups[Name].back().second.end(),
                          CompareByName());
                std::sort(m_sortedGroups[Usage].back().second.begin(),
                          m_sortedGroups[Usage].back().second.end(),
                          CompareByUsage());
            }
            
            m_sortedTextures[Name] = m_sortedTextures[Usage] = textureList();
            std::sort(m_sortedTextures[Name].begin(), m_sortedTextures[Name].end(), CompareByName());
            std::sort(m_sortedTextures[Usage].begin(), m_sortedTextures[Usage].end(), CompareByUsage());
        }

        TextureList TextureManager::textureList() const {
            TextureList result;
            TextureCollectionList::const_iterator cIt, cEnd;
            for (cIt = m_collections.begin(), cEnd = m_collections.end(); cIt != cEnd; ++cIt) {
                const TextureCollection* collection = *cIt;
                const TextureList textures = collection->textures();
                
                TextureList::const_iterator tIt, tEnd;
                for (tIt = textures.begin(), tEnd = textures.end(); tIt != tEnd; ++tIt) {
                    Texture* texture = *tIt;
                    result.push_back(texture);
                }
            }
            
            return result;
        }
    }
}
