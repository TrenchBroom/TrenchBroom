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

#include "FaceTexture.h"

namespace TrenchBroom {
    namespace Assets {
        FaceTexture::FaceTexture(const String& name, const size_t width, const size_t height) :
        m_textureId(0),
        m_name(name),
        m_width(width),
        m_height(height),
        m_usageCount(0),
        m_overridden(false) {}

        FaceTexture::~FaceTexture() {
            if (m_textureId != 0) {
                glDeleteTextures(1, &m_textureId);
                m_textureId = 0;
            }
        }

        GLuint FaceTexture::textureId() const {
            return m_textureId;
        }
        
        void FaceTexture::setTextureId(const GLuint textureId) {
            assert(m_textureId == 0);
            m_textureId = textureId;
        }

        const Color& FaceTexture::averageColor() const {
            return m_averageColor;
        }
        
        void FaceTexture::setAverageColor(const Color& averageColor) {
            m_averageColor = averageColor;
        }

        const String& FaceTexture::name() const {
            return m_name;
        }
        
        size_t FaceTexture::width() const {
            return m_width;
        }
        
        size_t FaceTexture::height() const {
            return m_height;
        }
        
        size_t FaceTexture::usageCount() const {
            return m_usageCount;
        }
        
        void FaceTexture::incUsageCount() {
            ++m_usageCount;
        }
        
        void FaceTexture::decUsageCount() {
            assert(m_usageCount > 0);
            --m_usageCount;
        }
        
        bool FaceTexture::isOverridden() const {
            return m_overridden;
        }
        
        void FaceTexture::setOverridden(const bool overridden) {
            m_overridden = overridden;
        }

        void FaceTexture::activate() const {
            glBindTexture(GL_TEXTURE_2D, m_textureId);
        }
        
        void FaceTexture::deactivate() const {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}
