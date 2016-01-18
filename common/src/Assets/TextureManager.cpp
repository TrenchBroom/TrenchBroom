/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Assets/TextureCollectionSpec.h"
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
        m_loader(NULL),
        m_minFilter(minFilter),
        m_magFilter(magFilter),
        m_resetTextureMode(false) {}
        
        TextureManager::~TextureManager() {
            clear();
        }
        
        void TextureManager::setBuiltinTextureCollections(const IO::Path::List& paths) {
            clearBuiltinTextureCollections();
            
            TextureCollectionList newCollections;
            TextureCollectionMap newCollectionsByName;
            
            try {
                IO::Path::List::const_iterator it, end;
                for (it = paths.begin(), end = paths.end(); it != end; ++it) {
                    const IO::Path& path = *it;
                    const TextureCollectionSpec spec(path.suffix(2).asString(), path);
                    addTextureCollection(spec, newCollections, newCollectionsByName);
                }
                
                using std::swap;
                std::swap(m_builtinCollections, newCollections);
                std::swap(m_builtinCollectionsByName, newCollectionsByName);
                
                updateTextures();
            } catch (...) {
                updateTextures();
                VectorUtils::deleteAll(newCollections);
                throw;
            }
        }
        
        void TextureManager::addExternalTextureCollection(const TextureCollectionSpec& spec) {
            try {
                addTextureCollection(spec, m_externalCollections, m_externalCollectionsByName);
                updateTextures();
            } catch (...) {
                TextureCollection* dummy = new TextureCollection(spec.name());
                m_externalCollections.push_back(dummy);
                m_externalCollectionsByName[spec.name()] = dummy;
                updateTextures();
                
                throw;
            }
        }
        
        void TextureManager::removeExternalTextureCollection(const String& name) {
            removeTextureCollection(name, m_externalCollections, m_externalCollectionsByName);
            updateTextures();
        }
        
        void TextureManager::moveExternalTextureCollectionUp(const String& name) {
            TextureCollectionMap::iterator it = m_externalCollectionsByName.find(name);
            if (it == m_externalCollectionsByName.end())
                throw AssetException("Unknown external texture collection: '" + name + "'");
            
            TextureCollection* collection = it->second;
            const size_t index = VectorUtils::indexOf(m_externalCollections, collection);
            if (index == 0)
                throw AssetException("Could not move texture collection");
            
            using std::swap;
            swap(m_externalCollections[index-1], m_externalCollections[index]);
            updateTextures();
        }
        
        void TextureManager::moveExternalTextureCollectionDown(const String& name) {
            TextureCollectionMap::iterator it = m_externalCollectionsByName.find(name);
            if (it == m_externalCollectionsByName.end())
                throw AssetException("Unknown external texture collection: '" + name + "'");
            
            TextureCollection* collection = it->second;
            const size_t index = VectorUtils::indexOf(m_externalCollections, collection);
            if (index == m_externalCollections.size() - 1)
                throw AssetException("Could not move texture collection");
            
            using std::swap;
            swap(m_externalCollections[index+1], m_externalCollections[index]);
            updateTextures();
        }
        
        void TextureManager::clear() {
            VectorUtils::clearAndDelete(m_builtinCollections);
            VectorUtils::clearAndDelete(m_externalCollections);
            MapUtils::clearAndDelete(m_toRemove);
            
            m_toPrepare.clear();
            m_builtinCollectionsByName.clear();
            m_externalCollectionsByName.clear();
            m_allCollections.clear();
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
        
        void TextureManager::setLoader(const IO::TextureLoader* loader) {
            clear();
            m_loader = loader;
            updateTextures();
        }

        void TextureManager::commitChanges() {
            resetTextureMode();
            prepare();
            MapUtils::clearAndDelete(m_toRemove);
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
            return m_allCollections;
        }
        
        const StringList TextureManager::externalCollectionNames() const {
            StringList names;
            names.reserve(m_externalCollections.size());
            
            TextureCollectionList::const_iterator it, end;
            for (it = m_externalCollections.begin(), end = m_externalCollections.end(); it != end; ++it) {
                const TextureCollection* collection = *it;
                names.push_back(collection->name());
            }
            
            return names;
        }
        
        void TextureManager::addTextureCollection(const TextureCollectionSpec& spec, TextureCollectionList& collections, TextureCollectionMap& collectionsByName) {
            
            const String& name = spec.name();
            if (collectionsByName.find(spec.name()) == collectionsByName.end()) {
                TextureCollection* collection = loadTextureCollection(spec);
                collections.push_back(collection);
                collectionsByName.insert(std::make_pair(name, collection));
                
                m_toPrepare.insert(TextureCollectionMapEntry(name, collection));
                m_toRemove.erase(name);
                
                if (m_logger != NULL)
                    m_logger->debug("Added texture collection %s", name.c_str());
            }
        }
        
        void TextureManager::removeTextureCollection(const String& name, TextureCollectionList& collections, TextureCollectionMap& collectionsByName) {
            TextureCollectionMap::iterator it = collectionsByName.find(name);
            if (it == collectionsByName.end())
                throw AssetException("Unknown external texture collection: '" + name + "'");
            
            TextureCollection* collection = it->second;
            VectorUtils::erase(collections, collection);
            
            collectionsByName.erase(it);
            m_toPrepare.erase(name);
            m_toRemove.insert(TextureCollectionMapEntry(name, collection));
            
            if (m_logger != NULL)
                m_logger->debug("Removed texture collection '%s'", name.c_str());
        }
        
        TextureCollection* TextureManager::loadTextureCollection(const TextureCollectionSpec& spec) const {
            assert(m_loader != NULL);
            return m_loader->loadTextureCollection(spec);
        }

        void TextureManager::resetTextureMode() {
            if (m_resetTextureMode) {
                TextureCollectionList::const_iterator it, end;
                for (it = m_allCollections.begin(), end = m_allCollections.end(); it != end; ++it) {
                    TextureCollection* collection = *it;
                    collection->setTextureMode(m_minFilter, m_magFilter);
                }
                m_resetTextureMode = false;
            }
        }
        
        void TextureManager::prepare() {
            TextureCollectionMap::const_iterator it, end;
            for (it = m_toPrepare.begin(), end = m_toPrepare.end(); it != end; ++it) {
                TextureCollection* collection = it->second;
                collection->prepare(m_minFilter, m_magFilter);
            }
            m_toPrepare.clear();
        }
        
        void TextureManager::clearBuiltinTextureCollections() {
            m_toRemove.insert(m_builtinCollectionsByName.begin(), m_builtinCollectionsByName.end());
            m_builtinCollections.clear();
            m_builtinCollectionsByName.clear();
            
            if (m_logger != NULL)
                m_logger->debug("Cleared builtin texture collections");
        }
        
        void TextureManager::clearExternalTextureCollections() {
            m_toRemove.insert(m_externalCollectionsByName.begin(), m_externalCollectionsByName.end());
            m_externalCollections.clear();
            m_externalCollectionsByName.clear();
            
            if (m_logger != NULL)
                m_logger->debug("Cleared builtin texture collections");
        }
        
        void TextureManager::updateTextures() {
            m_allCollections = VectorUtils::concatenate(m_builtinCollections, m_externalCollections);
            m_texturesByName.clear();
            m_sortedGroups[SortOrder_Name].clear();
            m_sortedGroups[SortOrder_Usage].clear();
            
            TextureCollectionList::iterator cIt, cEnd;
            for (cIt = m_allCollections.begin(), cEnd = m_allCollections.end(); cIt != cEnd; ++cIt) {
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
