/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include <algorithm>
#include <iterator>

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
            
            for (const IO::Path& path : paths) {
                const auto it = collections.find(path);
                if (it == std::end(collections) || !it->second->loaded()) {
                    try {
                        Assets::TextureCollection* collection = loader.loadTextureCollection(path);
                        m_logger->info("Loaded texture collection '" + path.asString() + "'");
                        addTextureCollection(collection);
                        collection->usageCountDidChange.addObserver(usageCountDidChange);
                    } catch (const Exception& e) {
                        addTextureCollection(new Assets::TextureCollection(path));
                        if (it == std::end(collections))
                            m_logger->error("Could not load texture collection '" + path.asString() + "': " + e.what());
                    }
                } else {
                    addTextureCollection(it->second);
                }
                if (it != std::end(collections))
                    collections.erase(it);
            }
            
            updateTextures();
            VectorUtils::append(m_toRemove, collections);
        }

        TextureManager::TextureCollectionMap TextureManager::collectionMap() const {
            TextureCollectionMap result;
            for (Assets::TextureCollection* collection : m_collections)
                result.insert(std::make_pair(collection->path(), collection));
            return result;
        }

        void TextureManager::addTextureCollection(Assets::TextureCollection* collection) {
            m_collections.push_back(collection);
            if (collection->loaded() && !collection->prepared())
                m_toPrepare.push_back(collection);
            
            if (m_logger != NULL)
                m_logger->debug("Added texture collection %s", collection->path().asString().c_str());
        }

        void TextureManager::clear() {
            VectorUtils::clearAndDelete(m_collections);
            VectorUtils::clearAndDelete(m_toRemove);
            
            m_toPrepare.clear();
            m_texturesByName.clear();
            m_textures.clear();
            
            // Remove logging because it might fail when the document is already destroyed.
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
            if (it == std::end(m_texturesByName))
                return NULL;
            return it->second;
        }
        
        const TextureList& TextureManager::textures() const {
            return m_textures;
        }
        
        const TextureCollectionList& TextureManager::collections() const {
            return m_collections;
        }
        
        const StringList TextureManager::collectionNames() const {
            StringList result;
            result.reserve(m_collections.size());
            std::transform(std::begin(m_collections), std::end(m_collections), std::back_inserter(result),
                           [](auto collection) { return collection->name(); });
            return result;
        }
        
        void TextureManager::resetTextureMode() {
            if (m_resetTextureMode) {
                std::for_each(std::begin(m_collections), std::end(m_collections),
                              [this](auto collection) { collection->setTextureMode(m_minFilter, m_magFilter); });
                m_resetTextureMode = false;
            }
        }
        
        void TextureManager::prepare() {
            std::for_each(std::begin(m_toPrepare), std::end(m_toPrepare),
                          [this](auto collection) { collection->prepare(m_minFilter, m_magFilter); });
            m_toPrepare.clear();
        }
        
        void TextureManager::updateTextures() {
            m_texturesByName.clear();
            m_textures.clear();
            
            for (TextureCollection* collection : m_collections) {
                for (Texture* texture : collection->textures()) {
                    const String key = StringUtils::toLower(texture->name());
                    texture->setOverridden(false);
                    
                    TextureMap::iterator mIt = m_texturesByName.find(key);
                    if (mIt != std::end(m_texturesByName)) {
                        mIt->second->setOverridden(true);
                        mIt->second = texture;
                    } else {
                        m_texturesByName.insert(std::make_pair(key, texture));
                    }
                }
            }

            m_textures = MapUtils::valueList(m_texturesByName);
        }
    }
}
