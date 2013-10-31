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

#include "Texture.h"
#include "Assets/TextureCollection.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        void setMipBufferSize(Assets::TextureBuffer::List& buffers, const size_t width, const size_t height) {
            for (size_t i = 0; i < buffers.size(); ++i) {
                const size_t div = 1 << i;
                const size_t size = 3 * (width * height) / (div * div);
                buffers[i] = Assets::TextureBuffer(size);
            }
        }

        Texture::Texture(const String& name, const size_t width, const size_t height, const Color& averageColor, const TextureBuffer& buffer) :
        m_collection(NULL),
        m_name(name),
        m_width(width),
        m_height(height),
        m_averageColor(averageColor),
        m_usageCount(0),
        m_overridden(false),
        m_textureId(0) {
            assert(m_width > 0);
            assert(m_height > 0);
            assert(buffer.size() >= m_width * m_height * 4);
            m_buffers.push_back(buffer);
        }
        
        Texture::Texture(const String& name, const size_t width, const size_t height, const Color& averageColor, const TextureBuffer::List& buffers) :
        m_collection(NULL),
        m_name(name),
        m_width(width),
        m_height(height),
        m_averageColor(averageColor),
        m_usageCount(0),
        m_overridden(false),
        m_textureId(0),
        m_buffers(buffers) {
            assert(m_width > 0);
            assert(m_height > 0);
            for (size_t i = 0; i < m_buffers.size(); ++i) {
                const size_t div = 1 << i;
                assert(m_buffers[i].size() >= (m_width * m_height) / (div * div));
            }
        }
        
        Texture::~Texture() {
            if (m_collection == NULL && m_textureId != 0)
                glDeleteTextures(1, &m_textureId);
            m_textureId = 0;
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
        
        const Color& Texture::averageColor() const {
            return m_averageColor;
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
        
        bool Texture::overridden() const {
            return m_overridden;
        }
        
        void Texture::setOverridden(const bool overridden) {
            m_overridden = overridden;
        }
        
        void Texture::activate() {
            if (!isPrepared())
                prepare();
            
            assert(m_textureId != 0);
            glBindTexture(GL_TEXTURE_2D, m_textureId);
        }
        
        void Texture::deactivate() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        void Texture::setCollection(TextureCollection* collection) {
            m_collection = collection;
        }

        bool Texture::isPrepared() const {
            return m_textureId != 0;
        }

        void Texture::prepare() {
            assert(!isPrepared());
            
            if (m_collection != NULL)
                m_collection->prepare();
            else
                doUploadTextureBuffer(createTexture());
        }
        
        GLuint Texture::createTexture() {
            GLuint textureId = 0;
            glGenTextures(1, &textureId);
            return textureId;
        }

        void Texture::doUploadTextureBuffer(const GLuint textureId) {
            assert(!m_buffers.empty());
            
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(m_buffers.size() - 1));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            
            size_t mipWidth = m_width;
            size_t mipHeight = m_height;
            size_t offset = 0;
            for (size_t j = 0; j < m_buffers.size(); ++j) {
                const GLvoid* data = static_cast<GLvoid*>(m_buffers[j].ptr());
                glTexImage2D(GL_TEXTURE_2D, j, GL_RGBA,
                             static_cast<GLsizei>(mipWidth),
                             static_cast<GLsizei>(mipHeight),
                             0, GL_RGB, GL_UNSIGNED_BYTE, data);
                offset += mipWidth * mipHeight;
                mipWidth /= 2;
                mipHeight /= 2;
            }
            
            m_buffers.clear();
            m_textureId = textureId;
        }
    }
}
