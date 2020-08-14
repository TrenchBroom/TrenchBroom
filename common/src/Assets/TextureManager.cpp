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
#include "Logger.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "IO/TextureLoader.h"

#include <kdl/map_utils.h>
#include <kdl/string_format.h>
#include <kdl/vector_utils.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

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
                if (left->usageCount() == right->usageCount()) {
                    return left->name() < right->name();
                } else {
                    return left->usageCount() > right->usageCount();
                }
            }
        };

        TextureManager::TextureManager(int magFilter, int minFilter, Logger& logger) :
        m_logger(logger),
        m_minFilter(minFilter),
        m_magFilter(magFilter),
        m_resetTextureMode(false) {}

        TextureManager::~TextureManager() {
            clear();
        }

        void TextureManager::setTextureCollections(const std::vector<IO::Path>& paths, IO::TextureLoader& loader) {
            auto collections = collectionMap();
            m_collections.clear();
            clear();

            for (const auto& path : paths) {
                const auto it = collections.find(path);
                if (it == std::end(collections) || !it->second->loaded()) {
                    try {
                        auto collection = loader.loadTextureCollection(path);
                        m_logger.info() << "Loaded texture collection '" << path << "'";
                        collection->usageCountDidChange.addObserver(usageCountDidChange);
                        addTextureCollection(collection.release());
                    } catch (const Exception& e) {
                        addTextureCollection(new Assets::TextureCollection(path));
                        if (it == std::end(collections)) {
                            m_logger.error() << "Could not load texture collection '" << path << "': " << e.what();
                        }
                    }
                } else {
                    addTextureCollection(it->second);
                }
                if (it != std::end(collections)) {
                    collections.erase(it);
                }
            }

            updateTextures();
            kdl::vec_append(m_toRemove, kdl::map_values(collections));
        }

        void TextureManager::setTextureCollections(const std::vector<TextureCollection*>& collections) {
            clear();
            for (auto* collection : collections) {
                collection->usageCountDidChange.addObserver(usageCountDidChange);
                addTextureCollection(collection);
            }
            updateTextures();
        }

        TextureManager::TextureCollectionMap TextureManager::collectionMap() const {
            auto result = TextureCollectionMap();
            for (auto* collection : m_collections) {
                result.insert(std::make_pair(collection->path(), collection));
            }
            return result;
        }

        void TextureManager::addTextureCollection(Assets::TextureCollection* collection) {
            m_collections.push_back(collection);
            if (collection->loaded() && !collection->prepared()) {
                m_toPrepare.push_back(collection);
            }

            m_logger.debug() << "Added texture collection " << collection->path();
        }

        void TextureManager::clear() {
            kdl::vec_clear_and_delete(m_collections);
            kdl::vec_clear_and_delete(m_toRemove);

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
            kdl::vec_clear_and_delete(m_toRemove);
        }

        const Texture* TextureManager::texture(const std::string& name) const {
            auto it = m_texturesByName.find(kdl::str_to_lower(name));
            if (it == std::end(m_texturesByName)) {
                return nullptr;
            } else {
                return it->second;
            }
        }

        Texture* TextureManager::texture(const std::string& name) {
            return const_cast<Texture*>(const_cast<const TextureManager*>(this)->texture(name));
        }

        const std::vector<const Texture*>& TextureManager::textures() const {
            return m_textures;
        }

        const std::vector<TextureCollection*>& TextureManager::collections() const {
            return m_collections;
        }

        const std::vector<std::string> TextureManager::collectionNames() const {
            std::vector<std::string> result;
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

            for (auto* collection : m_collections) {
                for (auto& texture : collection->textures()) {
                    const auto key = kdl::str_to_lower(texture.name());
                    texture.setOverridden(false);

                    auto mIt = m_texturesByName.find(key);
                    if (mIt != std::end(m_texturesByName)) {
                        mIt->second->setOverridden(true);
                        mIt->second = &texture;
                    } else {
                        m_texturesByName.insert(std::make_pair(key, &texture));
                    }
                }
            }

            m_textures = kdl::vec_transform(kdl::map_values(m_texturesByName), [](auto* t) { return const_cast<const Texture*>(t); });
        }
    }
}
