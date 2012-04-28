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

#ifndef TrenchBroom_TextRenderer_h
#define TrenchBroom_TextRenderer_h

#include <map>
#include "Utilities/VecMath.h"
#include "Renderer/FontManager.h"
#include "Utilities/SharedPointer.h"

using namespace std;

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        class FontManager;
        class FontDescriptor;
        class StringRenderer;
        
        class TextRenderer {
        public:
            class Anchor {
            public:
                virtual const Vec3f& position() = 0;
            };
            typedef tr1::shared_ptr<Anchor> AnchorPtr;
        private:
            typedef pair<StringRendererPtr, AnchorPtr> TextEntry;
            typedef map<int, TextEntry> TextMap;
            
            float m_fadeDistance;
            FontManager& m_fontManager;
            TextMap m_entries;
            
            void addString(int key, StringRendererPtr stringRenderer, AnchorPtr anchor);
        public:
            TextRenderer(FontManager& fontManager, float fadeDistance);
            ~TextRenderer();
            
            void addString(int key, const string& str, const FontDescriptor& descriptor, AnchorPtr anchor);
            void removeString(int key);
            void transferString(int key, TextRenderer& destination);
            void clear();
            void setFadeDistance(float fadeDistance);
            
            void render(RenderContext& context, const Vec4f& color);
        };
    }
}

#endif
