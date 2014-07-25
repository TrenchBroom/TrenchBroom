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

#include "TextureFont.h"

#include "Renderer/FontTexture.h"

namespace TrenchBroom {
    namespace Renderer {
        TextureFont::TextureFont(FontTexture* texture, const FontGlyph::List& glyphs, const size_t lineHeight, const unsigned char firstChar, const unsigned char charCount) :
        m_texture(texture),
        m_glyphs(glyphs),
        m_lineHeight(lineHeight),
        m_firstChar(firstChar),
        m_charCount(charCount) {}
        
        TextureFont::~TextureFont() {
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
                
                if (c < m_firstChar || c >= m_firstChar + m_charCount)
                    c = ' '; // space
                
                const FontGlyph& glyph = m_glyphs[static_cast<size_t>(c - m_firstChar)];
                if (c != ' ')
                    glyph.appendVertices(result, x, y, m_texture->size(), clockwise);
                
                x += glyph.advance();
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
                
                if (c < m_firstChar || c >= m_firstChar + m_charCount)
                    c = 32; // space
                
                const FontGlyph& glyph = m_glyphs[static_cast<size_t>(c - m_firstChar)];
                x += glyph.advance();
            }
            
            result[0] = std::max(result[0], static_cast<float>(x));
            result[1] = static_cast<float>(y + static_cast<int>(m_lineHeight));
            return result;
        }
        
        void TextureFont::activate() {
            m_texture->activate();
        }
        
        void TextureFont::deactivate() {
            m_texture->deactivate();
        }
    }
}
