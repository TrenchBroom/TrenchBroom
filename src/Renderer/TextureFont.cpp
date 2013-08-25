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

#include "TextureFont.h"

#include "Renderer/FontTexture.h"

namespace TrenchBroom {
    namespace Renderer {
        TextureFont::Char::Char(const int i_x, const int i_y, const int i_w, const int i_h, const int i_a) :
        x(i_x),
        y(i_y),
        w(i_w),
        h(i_h),
        a(i_a) {}
        
        void TextureFont::Char::append(Vec2f::List& vertices, const int xOffset, const int yOffset, const int textureLength, const bool clockwise) const {
            if (clockwise) {
                vertices.push_back(Vec2f(static_cast<float>(xOffset), static_cast<float>(yOffset)));
                vertices.push_back(Vec2f(static_cast<float>(x), static_cast<float>(y + h)) / static_cast<float>(textureLength));
                vertices.push_back(Vec2f(static_cast<float>(xOffset), static_cast<float>(yOffset + h)));
                vertices.push_back(Vec2f(static_cast<float>(x), static_cast<float>(y)) / static_cast<float>(textureLength));
                vertices.push_back(Vec2f(static_cast<float>(xOffset + w), static_cast<float>(yOffset + h)));
                vertices.push_back(Vec2f(static_cast<float>(x + w), static_cast<float>(y)) / static_cast<float>(textureLength));
                vertices.push_back(Vec2f(static_cast<float>(xOffset + w), static_cast<float>(yOffset)));
                vertices.push_back(Vec2f(static_cast<float>(x + w), static_cast<float>(y + h)) / static_cast<float>(textureLength));
            } else {
                vertices.push_back(Vec2f(static_cast<float>(xOffset), static_cast<float>(yOffset)));
                vertices.push_back(Vec2f(static_cast<float>(x), static_cast<float>(y + h)) / static_cast<float>(textureLength));
                vertices.push_back(Vec2f(static_cast<float>(xOffset + w), static_cast<float>(yOffset)));
                vertices.push_back(Vec2f(static_cast<float>(x + w), static_cast<float>(y + h)) / static_cast<float>(textureLength));
                vertices.push_back(Vec2f(static_cast<float>(xOffset + w), static_cast<float>(yOffset + h)));
                vertices.push_back(Vec2f(static_cast<float>(x + w), static_cast<float>(y)) / static_cast<float>(textureLength));
                vertices.push_back(Vec2f(static_cast<float>(xOffset), static_cast<float>(yOffset + h)));
                vertices.push_back(Vec2f(static_cast<float>(x), static_cast<float>(y)) / static_cast<float>(textureLength));
            }
        }
        
        float TextureFont::Char::sMin(const size_t textureWidth) const {
            return static_cast<float>(x) / static_cast<float>(textureWidth);
        }
        
        float TextureFont::Char::sMax(const size_t textureWidth) const {
            return static_cast<float>(x + w) / static_cast<float>(textureWidth);
        }
        
        float TextureFont::Char::tMin(const size_t textureHeight) const {
            return static_cast<float>(y) / static_cast<float>(textureHeight);
        }
        
        float TextureFont::Char::tMax(const size_t textureHeight) const {
            return static_cast<float>(y + h) / static_cast<float>(textureHeight);
        }
        
        TextureFont::TextureFont(FT_Face face, const unsigned char minChar, const unsigned char maxChar) :
        m_minChar(minChar),
        m_maxChar(maxChar),
        m_lineHeight(0),
        m_textureId(0),
        m_textureLength(0),
        m_texture(NULL) {
            FT_GlyphSlot glyph = face->glyph;
            
            int maxWidth = 0;
            int maxAscend = 0;
            int maxDescend = 0;
            
            for (unsigned char c = m_minChar; c <= m_maxChar; c++) {
                FT_Error error = FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER);
                if (error != 0)
                    continue;
                
                maxWidth = std::max(maxWidth, glyph->bitmap_left + glyph->bitmap.width);
                maxAscend = std::max(maxAscend, glyph->bitmap_top);
                maxDescend = std::max(maxDescend, glyph->bitmap.rows - glyph->bitmap_top);
                m_lineHeight = std::max(m_lineHeight, static_cast<int>(glyph->metrics.height >> 6));
            }
            
            const int cellSize = std::max(maxWidth, maxAscend + maxDescend);
            const int cellCount = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(m_maxChar - m_minChar + 1))));
            const int minTextureLength = Border + cellCount * (cellSize + Border);
            m_textureLength = 1;
            while (m_textureLength < minTextureLength)
                m_textureLength = m_textureLength << 1;
            
            m_texture = new FontTexture(static_cast<size_t>(m_textureLength), static_cast<size_t>(m_textureLength));
            
            int x = Border;
            int y = Border;
            for (unsigned char c = m_minChar; c <= m_maxChar; c++) {
                FT_Error error = FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER);
                if (error != 0) {
                    m_chars.push_back(Char(0, 0, 0, 0, 0));
                    continue;
                }
                
                if (x + cellSize + Border > m_textureLength) {
                    x = Border;
                    y += cellSize + Border;
                }
                
                m_texture->drawGlyph(x, y, maxAscend, glyph);
                
                const int cx = x;
                const int cy = y;
                const int cw = cellSize;
                const int ch = cellSize;
                const int ca = static_cast<int>(glyph->advance.x >> 6);
                
                m_chars.push_back(Char(cx, cy, cw, ch, ca));
                x += cellSize + Border;
            }
        }
        
        TextureFont::~TextureFont() {
            if (m_textureId > 0) {
                glDeleteTextures(1, &m_textureId);
                m_textureId = 0;
            }
            
            delete m_texture;
            m_texture = NULL;
        }
        
        Vec2f::List TextureFont::quads(const String& string, const bool clockwise, const Vec2f& offset) {
            Vec2f::List result;
            
            int x = static_cast<int>(Math::round(offset.x()));
            int y = static_cast<int>(Math::round(offset.y()));
            for (size_t i = 0; i < string.length(); i++) {
                char c = string[i];
                if (c == '\n') {
                    x = 0;
                    y += m_lineHeight;
                    continue;
                }
                
                if (c < m_minChar || c > m_maxChar)
                    c = ' '; // space
                
                const Char& glyph = m_chars[static_cast<size_t>(c - m_minChar)];
                if (c != ' ')
                    glyph.append(result, x, y, m_textureLength, clockwise);
                
                x += glyph.a;
            }
            
            return result;
        }
        
        Vec2f TextureFont::measure(const String& string) {
            Vec2f result;
            
            int x = 0;
            int y = 0;
            for (size_t i = 0; i < string.length(); i++) {
                char c = string[i];
                if (c == '\n') {
                    result[0] = std::max(result[0], static_cast<float>(x));
                    x = 0;
                    y += m_lineHeight;
                    continue;
                }
                
                if (c < m_minChar || c > m_maxChar)
                    c = 32; // space
                
                const Char& glyph = m_chars[static_cast<size_t>(c - m_minChar)];
                x += glyph.a;
            }
            
            result[0] = std::max(result[0], static_cast<float>(x));
            result[1] = static_cast<float>(y + m_lineHeight);
            return result;
        }
        
        void TextureFont::activate() {
            if (m_textureId == 0) {
                assert(m_texture != NULL);
                glGenTextures(1, &m_textureId);
                glBindTexture(GL_TEXTURE_2D, m_textureId);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, static_cast<GLsizei>(m_textureLength), static_cast<GLsizei>(m_textureLength), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_texture->buffer());
                delete m_texture;
                m_texture = NULL;
            }
            
            assert(m_textureId > 0);
            glBindTexture(GL_TEXTURE_2D, m_textureId);
        }
        
        void TextureFont::deactivate() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}
