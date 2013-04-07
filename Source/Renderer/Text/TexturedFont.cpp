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

#include "TexturedFont.h"

#include "Renderer/Text/TextureBitmap.h"

#include <cassert>
#include <cmath>

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            TexturedFont::TexturedFont(FT_Face face, const unsigned char minChar, const unsigned char maxChar) :
            m_minChar(minChar),
            m_maxChar(maxChar),
            m_lineHeight(0),
            m_textureId(0),
            m_textureLength(0),
            m_bitmap(NULL) {
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

                m_bitmap = new TextureBitmap(static_cast<size_t>(m_textureLength), static_cast<size_t>(m_textureLength));

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

                    m_bitmap->drawGlyph(x, y, maxAscend, glyph);

                    const int cx = x;
                    const int cy = y;
                    const int cw = cellSize;
                    const int ch = cellSize;
                    const int ca = static_cast<int>(glyph->advance.x >> 6);

                    m_chars.push_back(Char(cx, cy, cw, ch, ca));
                    x += cellSize + Border;
                }
            }

            TexturedFont::~TexturedFont() {
                if (m_textureId > 0) {
                    glDeleteTextures(1, &m_textureId);
                    m_textureId = 0;
                }

                delete m_bitmap;
                m_bitmap = NULL;
            }

            Vec2f::List TexturedFont::quads(const String& string, bool clockwise, const Vec2f& offset) {
                Vec2f::List result;

                int x = static_cast<int>(Math::round(offset.x));
                int y = static_cast<int>(Math::round(offset.y));
                for (size_t i = 0; i < string.length(); i++) {
                    char c = string[i];
                    if (c < m_minChar || c > m_maxChar)
                        c = 32; // space

                    const Char& glyph = m_chars[static_cast<size_t>(c - m_minChar)];
                    glyph.append(result, x, y, m_textureLength, clockwise);

                    x += glyph.a;
                }

                return result;
            }

            Vec2f TexturedFont::measure(const String& string) {
                Vec2f result;
                result.y = static_cast<float>(m_lineHeight);

                for (size_t i = 0; i < string.length(); i++) {
                    char c = string[i];
                    if (c < m_minChar || c > m_maxChar)
                        c = 32; // space

                    const Char& glyph = m_chars[static_cast<size_t>(c - m_minChar)];
                    result.x += glyph.a;
                }

                return result;
            }

            void TexturedFont::activate() {
                if (m_textureId == 0) {
                    assert(m_bitmap != NULL);
                    glGenTextures(1, &m_textureId);
                    glBindTexture(GL_TEXTURE_2D, m_textureId);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, static_cast<GLsizei>(m_textureLength), static_cast<GLsizei>(m_textureLength), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_bitmap->buffer());
                    delete m_bitmap;
                    m_bitmap = NULL;
                }

                assert(m_textureId > 0);
                glBindTexture(GL_TEXTURE_2D, m_textureId);
            }

            void TexturedFont::deactivate() {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }
}
