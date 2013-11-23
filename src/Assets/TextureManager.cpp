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
            VectorUtils::clearAndDelete(m_builtinCollections);
            VectorUtils::clearAndDelete(m_externalCollections);
            MapUtils::clearAndDelete(m_toRemove);
        }

        void TextureManager::setBuiltinTextureCollections(const IO::Path::List& paths) {
            clearBuiltinTextureCollections();
            doAddTextureCollections(paths, m_builtinCollections, m_builtinCollectionsByPath);
            updateTextures();
        }

        void TextureManager::addExternalTextureCollection(const IO::Path& path) {
            assert(m_game != NULL);
            doAddTextureCollection(path, m_externalCollections, m_externalCollectionsByPath);
            updateTextures();
        }
        
        void TextureManager::addExternalTextureCollections(const IO::Path::List& paths) {
            assert(m_game != NULL);
            if (paths.empty())
                return;
            
            doAddTextureCollections(paths, m_externalCollections, m_externalCollectionsByPath);
            updateTextures();
        }

        void TextureManager::removeExternalTextureCollection(const IO::Path& path) {
            doRemoveTextureCollection(path, m_externalCollections, m_externalCollectionsByPath, m_toRemove);
            updateTextures();
        }
        
        void TextureManager::removeExternalTextureCollections(const IO::Path::List& paths) {
            assert(m_game != NULL);
            if (paths.empty())
                return;
            
            doRemoveTextureCollections(paths, m_externalCollections, m_externalCollectionsByPath, m_toRemove);
            updateTextures();
        }

        void TextureManager::moveExternalTextureCollectionUp(const IO::Path& path) {
            assert(m_game != NULL);
            TextureCollectionMap::iterator it = m_externalCollectionsByPath.find(path);
            if (it == m_externalCollectionsByPath.end())
                throw AssetException("Could not find external texture collection: '" + path.asString() + "'");
            
            TextureCollection* collection = it->second;
            const size_t index = VectorUtils::indexOf(m_externalCollections, collection);
            if (index == 0)
                throw AssetException("Could not move texture collection");
            
            std::swap(m_externalCollections[index-1], m_externalCollections[index]);
            updateTextures();
        }
    
        void TextureManager::moveExternalTextureCollectionDown(const IO::Path& path) {
            assert(m_game != NULL);
            TextureCollectionMap::iterator it = m_externalCollectionsByPath.find(path);
            if (it == m_externalCollectionsByPath.end())
                throw AssetException("Could not find external texture collection: '" + path.asString() + "'");
            
            TextureCollection* collection = it->second;
            const size_t index = VectorUtils::indexOf(m_externalCollections, collection);
            if (index == m_externalCollections.size() - 1)
                throw AssetException("Could not move texture collection");
            
            std::swap(m_externalCollections[index+1], m_externalCollections[index]);
            updateTextures();
        }

        void TextureManager::reset(Model::GamePtr game) {
            clearBuiltinTextureCollections();
            clearExternalTextureCollections();
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
            return m_allCollections;
        }

        void TextureManager::doAddTextureCollections(const IO::Path::List& paths, TextureCollectionList& collections, TextureCollectionMap& collectionsByPath) {

            TextureCollectionList newCollections;
            TextureCollectionMap newCollectionsByPath;

            try {
                IO::Path::List::const_iterator it, end;
                for (it = paths.begin(), end = paths.end(); it != end; ++it) {
                    const IO::Path& path = *it;
                    doAddTextureCollection(path, newCollections, newCollectionsByPath);
                }
            } catch (...) {
                VectorUtils::clearAndDelete(newCollections);
                throw;
            }
            
            collections.insert(collections.end(), newCollections.begin(), newCollections.end());
            collectionsByPath.insert(newCollectionsByPath.begin(), newCollectionsByPath.end());
        }

        void TextureManager::doAddTextureCollection(const IO::Path& path, TextureCollectionList& collections, TextureCollectionMap& collectionsByPath) {
            if (collectionsByPath.find(path) == collectionsByPath.end()) {
                TextureCollection* collection = m_game->loadTextureCollection(path);
                collections.push_back(collection);
                collectionsByPath.insert(std::make_pair(path, collection));
            }
        }

        void TextureManager::doRemoveTextureCollections(const IO::Path::List& paths, TextureCollectionList& collections, TextureCollectionMap& collectionsByPath, TextureCollectionMap& toRemove) {
            TextureCollectionList newCollections = collections;
            TextureCollectionMap newCollectionsByPath = collectionsByPath;
            TextureCollectionMap newToRemove = toRemove;
            
            IO::Path::List::const_iterator it, end;
            for (it = paths.begin(), end = paths.end(); it != end; ++it) {
                const IO::Path& path = *it;
                doRemoveTextureCollection(path, newCollections, newCollectionsByPath, newToRemove);
            }
            
            collections = newCollections;
            collectionsByPath = newCollectionsByPath;
            toRemove = newToRemove;
        }
        
        void TextureManager::doRemoveTextureCollection(const IO::Path& path, TextureCollectionList& collections, TextureCollectionMap& collectionsByPath, TextureCollectionMap& toRemove) {
            TextureCollectionMap::iterator it = collectionsByPath.find(path);
            if (it == collectionsByPath.end())
                throw AssetException("Could not find external texture collection: '" + path.asString() + "'");
            
            TextureCollection* collection = it->second;
            VectorUtils::remove(collections, collection);
            
            collectionsByPath.erase(it);
            toRemove.insert(TextureCollectionMapEntry(path, collection));
        }

        void TextureManager::clearBuiltinTextureCollections() {
            m_toRemove.insert(m_builtinCollectionsByPath.begin(), m_builtinCollectionsByPath.end());
            m_builtinCollections.clear();
            m_builtinCollectionsByPath.clear();
        }
        
        void TextureManager::clearExternalTextureCollections() {
            m_toRemove.insert(m_externalCollectionsByPath.begin(), m_externalCollectionsByPath.end());
            m_externalCollections.clear();
            m_externalCollectionsByPath.clear();
        }

        void TextureManager::updateTextures() {
            m_allCollections = VectorUtils::concatenate(m_builtinCollections, m_externalCollections);
            m_texturesByName.clear();
            m_sortedGroups[Name].clear();
            m_sortedGroups[Usage].clear();
            
            TextureCollectionList::iterator cIt, cEnd;
            for (cIt = m_allCollections.begin(), cEnd = m_allCollections.end(); cIt != cEnd; ++cIt) {
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
            for (cIt = m_allCollections.begin(), cEnd = m_allCollections.end(); cIt != cEnd; ++cIt) {
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
