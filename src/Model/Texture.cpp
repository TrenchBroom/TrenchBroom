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

#include "Texture.h"

namespace TrenchBroom {
    namespace Model {
        Texture::Texture(const String& name, const size_t width, const size_t height) :
        m_textureId(0),
        m_name(name),
        m_width(width),
        m_height(height),
        m_usageCount(0),
        m_overridden(false) {}

        Texture::~Texture() {
            if (m_textureId != 0) {
                glDeleteTextures(1, &m_textureId);
                m_textureId = 0;
            }
        }

        GLuint Texture::textureId() const {
            return m_textureId;
        }
        
        void Texture::setTextureId(const GLuint textureId) {
            assert(m_textureId == 0);
            m_textureId = textureId;
        }

        const Color& Texture::averageColor() const {
            return m_averageColor;
        }
        
        void Texture::setAverageColor(const Color& averageColor) {
            m_averageColor = averageColor;
        }

        const String& Texture::name() const {
            return m_name;
        }
        
        size_t Texture::width() const {
            return m_width;
        }
        
        size_t Texture::height() const {
            return m_height;
        }
        
        size_t Texture::usageCount() const {
            return m_usageCount;
        }
        
        void Texture::incUsageCount() {
            ++m_usageCount;
        }
        
        void Texture::decUsageCount() {
            assert(m_usageCount > 0);
            --m_usageCount;
        }
        
        bool Texture::isOverridden() const {
            return m_overridden;
        }
        
        void Texture::setOverridden(const bool overridden) {
            m_overridden = overridden;
        }

        void Texture::activate() {
            glBindTexture(GL_TEXTURE_2D, m_textureId);
        }
        
        void Texture::deactivate() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}
