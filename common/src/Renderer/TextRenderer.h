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

#ifndef TrenchBroom_TextRenderer
#define TrenchBroom_TextRenderer

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
        
        class TextRenderer : public DirectRenderable {
        private:
            static const float DefaultMaxViewDistance;
            static const float DefaultMinZoomFactor;
            static const Vec2f DefaultInset;
            static const size_t RectCornerSegments;
            static const float RectCornerRadius;
            
            struct Entry {
                Vec2f::List vertices;
                Vec2f size;
                Vec3f offset;
                Color textColor;
                Color backgroundColor;

                Entry(Vec2f::List& i_vertices, const Vec2f& i_size, const Vec3f& i_offset, const Color& i_textColor, const Color& i_backgroundColor);
            };
            
            typedef std::vector<Entry> EntryList;
            
            struct EntryCollection {
                EntryList entries;
                size_t textVertexCount;
                size_t rectVertexCount;
                
                VertexArray textArray;
                VertexArray rectArray;

                EntryCollection();
            };
            
            typedef VertexSpecs::P3T2C4::Vertex TextVertex;
            typedef VertexSpecs::P3C4::Vertex RectVertex;
            
            FontDescriptor m_fontDescriptor;
            float m_maxViewDistance;
            float m_minZoomFactor;
            Vec2f m_inset;
            
            EntryCollection m_entries;
            EntryCollection m_entriesOnTop;
        public:
            TextRenderer(const FontDescriptor& fontDescriptor, float maxViewDistance = DefaultMaxViewDistance, float minZoomFactor = DefaultMinZoomFactor, const Vec2f& inset = DefaultInset);
            
            void renderString(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position);
            void renderStringOnTop(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position);
        private:
            void renderString(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position, bool onTop);
            
            bool isVisible(RenderContext& renderContext, const AttrString& string, const TextAnchor& position, float distance, bool onTop) const;
            float computeAlphaFactor(const RenderContext& renderContext, float distance, bool onTop) const;
            void addEntry(EntryCollection& collection, const Entry& entry);
            
            Vec2f stringSize(RenderContext& renderContext, const AttrString& string) const;
        private:
            void doPrepareVertices(Vbo& vertexVbo);
            void prepare(EntryCollection& collection, bool onTop, Vbo& vbo);
            
            void addEntry(const Entry& entry, bool onTop, TextVertex::List& textVertices, RectVertex::List& rectVertices);
            
            void doRender(RenderContext& renderContext);
            void render(EntryCollection& collection, RenderContext& renderContext);

            void clear();
        };
    }
}

#endif /* defined(TrenchBroom_TextRenderer) */
