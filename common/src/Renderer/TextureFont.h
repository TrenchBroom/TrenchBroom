/*
 Copyright (C) 2010-2017 Kristian Duske

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

#pragma once

#include "Macros.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class AttrString;
        class FontGlyph;
        class FontTexture;

        class TextureFont {
        private:
            std::unique_ptr<FontTexture> m_texture;
            std::vector<FontGlyph> m_glyphs;
            int m_lineHeight;

            unsigned char m_firstChar;
            unsigned char m_charCount;
        public:
            TextureFont(std::unique_ptr<FontTexture> texture, const std::vector<FontGlyph>& glyphs, int lineHeight, unsigned char firstChar, unsigned char charCount);
            ~TextureFont();

            deleteCopyAndMove(TextureFont)

            std::vector<vm::vec2f> quads(const AttrString& string, bool clockwise, const vm::vec2f& offset = vm::vec2f::zero()) const;
            vm::vec2f measure(const AttrString& string) const;

            std::vector<vm::vec2f> quads(const std::string& string, bool clockwise, const vm::vec2f& offset = vm::vec2f::zero()) const;
            vm::vec2f measure(const std::string& string) const;

            void activate();
            void deactivate();
        };
    }
}



