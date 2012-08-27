/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "IO/Wad.h"
#include "Model/Palette.h"

namespace TrenchBroom {
    namespace Model {
        const String Texture::Empty = "__TB__empty";

        void Texture::init(const String& name, unsigned int width, unsigned int height) {
            static IdType currentId = 1;
            m_uniqueId = currentId++;
            m_name = name;
            m_width = width;
            m_height = height;
            m_dummy = false;
            m_usageCount = 0;
            m_textureBuffer = NULL;
        }
        
        void Texture::init(const String& name, const unsigned char* indexedImage, unsigned int width, unsigned int height, const Palette& palette) {
            init(name, width, height);
            
            if (indexedImage != NULL) {
                unsigned int pixelCount = width * height;
                m_textureBuffer = new unsigned char[pixelCount * 3];
                palette.indexedToRgb(indexedImage, m_textureBuffer, pixelCount);
                
                m_averageColor.x = m_averageColor.y = m_averageColor.z = 0.0f;
                m_averageColor.w = 1.0f;
                for (unsigned int i = 0; i < pixelCount; i++) {
                    m_averageColor.x += (m_textureBuffer[i * 3 + 0] / 255.0f);
                    m_averageColor.y += (m_textureBuffer[i * 3 + 1] / 255.0f);
                    m_averageColor.z += (m_textureBuffer[i * 3 + 2] / 255.0f);
                }
                
                m_averageColor.x /= pixelCount;
                m_averageColor.y /= pixelCount;
                m_averageColor.z /= pixelCount;
            }
        }

        Texture::Texture(const String& name, const unsigned char* rgbImage, unsigned int width, unsigned int height) {
            init(name, width, height);
            if (rgbImage != NULL) {
                int pixelCount = width * height;
                m_textureBuffer = new unsigned char[pixelCount * 3];
                memcpy(m_textureBuffer, rgbImage, pixelCount * 3);
            }
        }
        
        Texture::Texture(const String& name, const unsigned char* indexImage, unsigned int width, unsigned int height, const Palette& palette) {
            init(name, indexImage, width, height, palette);
        }
        
        Texture::Texture(const IO::Mip& mip, const Palette& palette) {
            init(mip.name(), mip.mip0(), mip.width(), mip.height(), palette);
        }
        
        /*
        Texture::Texture(const String& name, const AliasSkin& skin, unsigned int skinIndex, const Palette& palette) {
            init(name, skin.pictures[skinIndex], skin.width, skin.height, palette);
        }
        
        Texture::Texture(const String& name, const BspTexture& texture, const Palette& palette) {
            init(name, texture.image, texture.width, texture.height, palette);
        }
        */
        
        Texture::Texture(const String& name) {
            init(name, 1, 1);
            m_textureBuffer = new unsigned char[4];
            for (int i = 0; i < 4; i++)
                m_textureBuffer[i] = 0;
            m_dummy = true;
        }
        
        Texture::~Texture() {
            if (m_textureId > 0)
                glDeleteTextures(1, &m_textureId);
            if (m_textureBuffer != NULL)
                delete[] m_textureBuffer;
        }

        void Texture::activate() {
            if (m_textureId == 0) {
                if (m_textureBuffer != NULL) {
                    glGenTextures(1, &m_textureId);
                    glBindTexture(GL_TEXTURE_2D, m_textureId);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_textureBuffer);
                    delete[] m_textureBuffer;
                    m_textureBuffer = NULL;
                }
            }
            
            glBindTexture(GL_TEXTURE_2D, m_textureId);
        }
        
        void Texture::deactivate() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}