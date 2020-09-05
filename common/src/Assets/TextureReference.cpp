/*
Copyright (C) 2020 Kristian Duske

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

#include "TextureReference.h"

#include "Assets/Texture.h"

namespace TrenchBroom {
    namespace Assets {
        TextureReference::TextureReference(Assets::Texture* texture) :
        m_texture(texture) {
            if (m_texture != nullptr) {
                m_texture->incUsageCount();
            }
        }
        
        TextureReference::TextureReference(const TextureReference& other) :
        TextureReference(other.m_texture) {}
        
        TextureReference::TextureReference(TextureReference&& other) :
        m_texture(other.m_texture) {
            other.m_texture = nullptr;
        }
        
        TextureReference& TextureReference::operator=(TextureReference other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        void swap(TextureReference& lhs, TextureReference& rhs) {
            using std::swap;
            swap(lhs.m_texture, rhs.m_texture);
        }

        TextureReference::~TextureReference() {
            if (m_texture != nullptr) {
                m_texture->decUsageCount();
            }
        }
        
        const Assets::Texture* TextureReference::texture() const {
            return m_texture;
        }
    }
}
