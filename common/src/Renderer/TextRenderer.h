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

#ifndef __TrenchBroom__TextRenderer__
#define __TrenchBroom__TextRenderer__

#include "VecMath.h"
#include "Color.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/Renderable.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    class AttrString;
    
    namespace Renderer {
        class RenderContext;
        class TextAnchor;
        
        class TextRenderer : public Renderable {
        private:
            static const size_t RectCornerSegments;
            
            struct CachedString {
                Vec2f::List vertices;
                Vec2f size;

                CachedString(Vec2f::List& i_vertices, const Vec2f& i_size);
            };
            
            typedef std::map<AttrString, CachedString> StringCache;
            
            struct Entry {

                StringCache::iterator string;
                Vec3f offset;
                Color textColor;
                Color backgroundColor;

                Entry(StringCache::iterator i_string, const Vec3f& i_offset, const Color& i_textColor, const Color& i_backgroundColor);
            };
            
            typedef std::vector<Entry> EntryList;
            
            typedef VertexSpecs::P3T2C4::Vertex TextVertex;
            typedef VertexSpecs::P3C4::Vertex RectVertex;
            
            FontDescriptor m_fontDescriptor;
            Vec2f m_inset;
            
            StringCache m_cache;
            EntryList m_entries;
            size_t m_textVertexCount;
            size_t m_rectVertexCount;
            
            VertexArray m_textArray;
            VertexArray m_rectArray;
        public:
            TextRenderer(const FontDescriptor& fontDescriptor, const Vec2f& inset = Vec2f(4.0f, 4.0f));
            
            void renderString(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position);
            void renderStringOnTop(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position);
        private:
            void renderString(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position, bool onTop);
            
            bool isVisible(RenderContext& renderContext, const AttrString& string, const TextAnchor& position) const;
            
            Vec2f stringSize(RenderContext& renderContext, const AttrString& string) const;
            StringCache::iterator findOrCreateCachedString(RenderContext& renderContext, const AttrString& string);
        private:
            void doPrepare(Vbo& vbo);
            void addEntry(const Entry& entry, TextVertex::List textVertices, RectVertex::List rectVertices);
            
            void doRender(RenderContext& renderContext);

            void clear();
        };
    }
}

#endif /* defined(__TrenchBroom__TextRenderer__) */
