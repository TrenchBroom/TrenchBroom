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

#include "TextureManager.h"

#include "Exceptions.h"
#include "CollectionUtils.h"
#include "Logger.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "IO/TextureLoader.h"

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
        
        TextureManager::TextureManager(Logger* logger, int minFilter, int magFilter) :
        m_logger(logger),
        m_minFilter(minFilter),
        m_magFilter(magFilter),
        m_resetTextureMode(false) {}
        
        TextureManager::~TextureManager() {
            clear();
        }
        
        void TextureManager::setTextureCollections(const IO::Path::List& paths, IO::TextureLoader& loader) {
            TextureCollectionMap collections = collectionMap();
            m_collections.clear();
            clear();
            
            IO::Path::List::const_iterator pathIt, pathEnd;
            for (pathIt = paths.begin(), pathEnd = paths.end(); pathIt != pathEnd; ++pathIt) {
                const IO::Path& path = *pathIt;
                const TextureCollectionMap::iterator colIt = collections.find(path);
                if (colIt == collections.end() || !colIt->second->loaded()) {
                    try {
                        addTextureCollection(loader.loadTextureCollection(path));
                        m_logger->info("Loaded texture collection '" + path.asString() + "'");
                    } catch (const Exception& e) {
                        addTextureCollection(new Assets::TextureCollection(path));
                        if (colIt == collections.end())
                            m_logger->error("Could not load texture collection '" + path.asString() + "': " + e.what());
                    }
                } else {
                    addTextureCollection(colIt->second);
                }
                if (colIt != collections.end())
                    collections.erase(colIt);
            }
            
            VectorUtils::append(m_toRemove, collections);
        }

        TextureManager::TextureCollectionMap TextureManager::collectionMap() const {
            TextureCollectionMap result;
            TextureCollectionList::const_iterator it, end;
            for (it = m_collections.begin(), end = m_collections.end(); it != end; ++it) {
                Assets::TextureCollection* collection = *it;
                result.insert(std::make_pair(collection->path(), collection));
            }
            return result;
        }

        void TextureManager::addTextureCollection(Assets::TextureCollection* collection) {
            m_collections.push_back(collection);
            if (collection->loaded())
                m_toPrepare.push_back(collection);
            
            if (m_logger != NULL)
                m_logger->debug("Added texture collection %s", collection->path().asString().c_str());
        }

        void TextureManager::clear() {
            VectorUtils::clearAndDelete(m_collections);
            VectorUtils::clearAndDelete(m_toRemove);
            
            m_toPrepare.clear();
            m_texturesByName.clear();
            
            for (size_t i = 0; i < 2; ++i) {
                m_sortedTextures[i].clear();
                m_sortedGroups[i].clear();
            }
            
            if (m_logger != NULL)
                m_logger->debug("Cleared texture collections");
        }
        
        void TextureManager::setTextureMode(const int minFilter, const int magFilter) {
            m_minFilter = minFilter;
            m_magFilter = magFilter;
            m_resetTextureMode = true;
        }

        void TextureManager::commitChanges() {
            resetTextureMode();
            prepare();
            VectorUtils::clearAndDelete(m_toRemove);
        }
        
        Texture* TextureManager::texture(const String& name) const {
            TextureMap::const_iterator it = m_texturesByName.find(StringUtils::toLower(name));
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
        
        const StringList TextureManager::collectionNames() const {
            StringList names;
            names.reserve(m_collections.size());
            
            TextureCollectionList::const_iterator it, end;
            for (it = m_collections.begin(), end = m_collections.end(); it != end; ++it) {
                const TextureCollection* collection = *it;
                names.push_back(collection->name());
            }
            
            return names;
        }
        
        void TextureManager::resetTextureMode() {
            if (m_resetTextureMode) {
                TextureCollectionList::const_iterator it, end;
                for (it = m_collections.begin(), end = m_collections.end(); it != end; ++it) {
                    TextureCollection* collection = *it;
                    collection->setTextureMode(m_minFilter, m_magFilter);
                }
                m_resetTextureMode = false;
            }
        }
        
        void TextureManager::prepare() {
            TextureCollectionList::const_iterator it, end;
            for (it = m_toPrepare.begin(), end = m_toPrepare.end(); it != end; ++it) {
                TextureCollection* collection = *it;
                collection->prepare(m_minFilter, m_magFilter);
            }
            m_toPrepare.clear();
        }
        
        void TextureManager::updateTextures() {
            m_texturesByName.clear();
            m_sortedGroups[SortOrder_Name].clear();
            m_sortedGroups[SortOrder_Usage].clear();
            
            TextureCollectionList::iterator cIt, cEnd;
            for (cIt = m_collections.begin(), cEnd = m_collections.end(); cIt != cEnd; ++cIt) {
                TextureCollection* collection = *cIt;
                const TextureList textures = collection->textures();
                
                TextureList::const_iterator tIt, tEnd;
                for (tIt = textures.begin(), tEnd = textures.end(); tIt != tEnd; ++tIt) {
                    Texture* texture = *tIt;
                    const String key = StringUtils::toLower(texture->name());
                    texture->setOverridden(false);
                    
                    TextureMap::iterator mIt = m_texturesByName.find(key);
                    if (mIt != m_texturesByName.end()) {
                        mIt->second->setOverridden(true);
                        mIt->second = texture;
                    } else {
                        m_texturesByName.insert(std::make_pair(key, texture));
                    }
                }
                
                const Group group = std::make_pair(collection, textures);
                m_sortedGroups[SortOrder_Name].push_back(group);
                m_sortedGroups[SortOrder_Usage].push_back(group);
                std::sort(m_sortedGroups[SortOrder_Name].back().second.begin(),
                          m_sortedGroups[SortOrder_Name].back().second.end(),
                          CompareByName());
                std::sort(m_sortedGroups[SortOrder_Usage].back().second.begin(),
                          m_sortedGroups[SortOrder_Usage].back().second.end(),
                          CompareByUsage());
            }
            
            m_sortedTextures[SortOrder_Name] = m_sortedTextures[SortOrder_Usage] = textureList();
            std::sort(m_sortedTextures[SortOrder_Name].begin(), m_sortedTextures[SortOrder_Name].end(), CompareByName());
            std::sort(m_sortedTextures[SortOrder_Usage].begin(), m_sortedTextures[SortOrder_Usage].end(), CompareByUsage());
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
