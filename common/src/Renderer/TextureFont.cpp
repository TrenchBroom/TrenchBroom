/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
        
        class MeasureString : public AttrString::LineFunc {
        private:
            TextureFont& m_font;
            Vec2f m_size;
        public:
            MeasureString(TextureFont& font) :
            m_font(font) {}
            
            const Vec2f& size() const {
                return m_size;
            }
        private:
            void justifyLeft(const String& str) {
                measure(str);
            }
            
            void justifyRight(const String& str) {
                measure(str);
            }
            
            void center(const String& str) {
                measure(str);
            }
            
            void measure(const String& str) {
                const Vec2f size = m_font.measure(str);
                m_size[0] = std::max(m_size[0], size[0]);
                m_size[1] += size[1];
            }
        };
        
        class MeasureLines : public AttrString::LineFunc {
        private:
            TextureFont& m_font;
            Vec2f::List m_sizes;
        public:
            MeasureLines(TextureFont& font) :
            m_font(font) {}
            
            const Vec2f::List& sizes() const {
                return m_sizes;
            }
        private:
            void justifyLeft(const String& str) {
                measure(str);
            }
            
            void justifyRight(const String& str) {
                measure(str);
            }
            
            void center(const String& str) {
                measure(str);
            }
            
            void measure(const String& str) {
                m_sizes.push_back(m_font.measure(str));
            }
        };
        
        class MakeQuads : public AttrString::LineFunc {
        private:
            TextureFont& m_font;

            bool m_clockwise;
            Vec2f m_offset;
            
            const Vec2f::List& m_sizes;
            Vec2f m_maxSize;
            
            size_t m_index;
            float m_y;
            Vec2f::List m_vertices;
        public:
            MakeQuads(TextureFont& font, const bool clockwise, const Vec2f& offset, const Vec2f::List& sizes) :
            m_font(font),
            m_clockwise(clockwise),
            m_offset(offset),
            m_sizes(sizes),
            m_index(0),
            m_y(0.0f) {
                for (size_t i = 0; i < m_sizes.size(); ++i) {
                    m_maxSize = m_maxSize.max(m_sizes[i]);
                    m_y += m_sizes[i].y();
                }
                m_y -= m_sizes.back().y();
            }
            
            const Vec2f::List& vertices() const {
                return m_vertices;
            }
        private:
            void justifyLeft(const String& str) {
                makeQuads(str, 0.0f);
            }
            
            void justifyRight(const String& str) {
                const float w = m_sizes[m_index].x();
                makeQuads(str, m_maxSize.x() - w);
            }
            
            void center(const String& str) {
                const float w = m_sizes[m_index].x();
                makeQuads(str, (m_maxSize.x() - w) / 2.0f);
            }
            
            void makeQuads(const String& str, const float x) {
                const Vec2f offset = m_offset + Vec2f(x, m_y);
                VectorUtils::append(m_vertices, m_font.quads(str, m_clockwise, offset));
                
                m_y -= m_sizes[m_index].y();
                m_index++;
            }
        };
        
        Vec2f::List TextureFont::quads(const AttrString& string, const bool clockwise, const Vec2f& offset) {
            MeasureLines measureLines(*this);
            string.lines(measureLines);
            const Vec2f::List& sizes = measureLines.sizes();
            
            MakeQuads makeQuads(*this, clockwise, offset, sizes);
            string.lines(makeQuads);
            return makeQuads.vertices();
        }

        Vec2f TextureFont::measure(const AttrString& string) {
            MeasureString measureString(*this);
            string.lines(measureString);
            return measureString.size();
        }

        Vec2f::List TextureFont::quads(const String& string, const bool clockwise, const Vec2f& offset) {
            Vec2f::List result;
            result.reserve(string.length() * 4 * 2);
            
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
