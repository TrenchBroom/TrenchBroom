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

#include "TextureCollection.h"

#include "Ensure.h"

#include <kdl/vector_utils.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        TextureCollection::TextureCollection() :
        m_loaded(false),
        m_usageCount(0) {}

        TextureCollection::TextureCollection(std::vector<Texture> textures) :
        m_loaded(false),
        m_textures(std::move(textures)),
        m_usageCount(0) {
            for (auto& texture : m_textures) {
                texture.setCollection(this);
            }
        }

        TextureCollection::TextureCollection(const IO::Path& path) :
        m_loaded(false),
        m_path(path),
        m_usageCount(0) {}

        TextureCollection::TextureCollection(const IO::Path& path, std::vector<Texture> textures) :
        m_loaded(true),
        m_path(path),
        m_textures(std::move(textures)),
        m_usageCount(0) {
            for (auto& texture : m_textures) {
                texture.setCollection(this);
            }
        }

        TextureCollection::~TextureCollection() {
            if (!m_textureIds.empty()) {
                glAssert(glDeleteTextures(static_cast<GLsizei>(m_textureIds.size()),
                                          static_cast<GLuint*>(&m_textureIds.front())));
                m_textureIds.clear();
            }
        }

        bool TextureCollection::loaded() const {
            return m_loaded;
        }

        const IO::Path& TextureCollection::path() const {
            return m_path;
        }

        std::string TextureCollection::name() const {
            if (m_path.isEmpty())
                return "";
            return m_path.lastComponent().asString();
        }

        size_t TextureCollection::textureCount() const {
            return m_textures.size();
        }

        const std::vector<Texture>& TextureCollection::textures() const {
            return m_textures;
        }

        std::vector<Texture>& TextureCollection::textures() {
            return m_textures;
        }

        const Texture* TextureCollection::textureByIndex(const size_t index) const {
            if (index >= m_textures.size()) {
                return nullptr;
            } else {
                return &(m_textures[index]);
            }
        }

        Texture* TextureCollection::textureByIndex(const size_t index) {
            return const_cast<Texture*>(const_cast<const TextureCollection*>(this)->textureByIndex(index));
        }

        const Texture* TextureCollection::textureByName(const std::string& name) const {
            for (const auto& texture : m_textures) {
                if (texture.name() == name) {
                    return &texture;
                }
            }
            return nullptr;
        }

        Texture* TextureCollection::textureByName(const std::string& name) {
            return const_cast<Texture*>(const_cast<const TextureCollection*>(this)->textureByName(name));
        }

        size_t TextureCollection::usageCount() const {
            return m_usageCount;
        }

        bool TextureCollection::prepared() const {
            return !m_textureIds.empty();
        }

        void TextureCollection::prepare(const int minFilter, const int magFilter) {
            assert(!prepared());

            m_textureIds.resize(textureCount());
            glAssert(glGenTextures(static_cast<GLsizei>(textureCount()),
                                   static_cast<GLuint*>(&m_textureIds.front())));

            for (size_t i = 0; i < textureCount(); ++i) {
                Texture& texture = m_textures[i];
                texture.prepare(m_textureIds[i], minFilter, magFilter);
            }
        }

        void TextureCollection::setTextureMode(const int minFilter, const int magFilter) {
            for (auto& texture : m_textures) {
                texture.setMode(minFilter, magFilter);
            }
        }

        void TextureCollection::incUsageCount() {
            ++m_usageCount;
        }

        void TextureCollection::decUsageCount() {
            assert(m_usageCount > 0);
            --m_usageCount;
        }
    }
}
