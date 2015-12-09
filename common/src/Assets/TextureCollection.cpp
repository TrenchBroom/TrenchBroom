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

#include "TextureCollection.h"

#include "CollectionUtils.h"
#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace Assets {
        TextureCollection::TextureCollection(const String& name) :
        m_loaded(false),
        m_name(name) {}

        TextureCollection::TextureCollection(const String& name, const TextureList& textures) :
        m_loaded(true),
        m_name(name),
        m_textures(textures.size()) {
            for (size_t i = 0; i < textures.size(); ++i) {
                Texture* texture = textures[i];
                texture->setCollection(this);
                m_textures[i] = texture;
            }
        }

        TextureCollection::~TextureCollection() {
            VectorUtils::clearAndDelete(m_textures);
            if (!m_textureIds.empty()) {
                glAssert(glDeleteTextures(static_cast<GLsizei>(m_textureIds.size()),
                                          static_cast<GLuint*>(&m_textureIds.front())));
                m_textureIds.clear();
            }
        }

        bool TextureCollection::loaded() const {
            return m_loaded;
        }

        const String& TextureCollection::name() const {
            return m_name;
        }
        
        const TextureList& TextureCollection::textures() const {
            return m_textures;
        }

        void TextureCollection::prepare(const int minFilter, const int magFilter) {
            assert(m_textureIds.empty());
            
            const size_t textureCount = m_textures.size();
            m_textureIds.resize(textureCount);
            glAssert(glGenTextures(static_cast<GLsizei>(textureCount),
                                   static_cast<GLuint*>(&m_textureIds.front())));

            for (size_t i = 0; i < textureCount; ++i) {
                Texture* texture = m_textures[i];
                texture->prepare(m_textureIds[i], minFilter, magFilter);
            }
        }

        void TextureCollection::setTextureMode(const int minFilter, const int magFilter) {
            for (size_t i = 0; i < m_textures.size(); ++i) {
                Texture* texture = m_textures[i];
                texture->setMode(minFilter, magFilter);
            }
        }
    }
}
