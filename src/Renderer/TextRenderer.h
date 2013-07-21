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

#ifndef TrenchBroom_TextRenderer_h
#define TrenchBroom_TextRenderer_h

#include "VecMath.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/TextureFont.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        namespace Alignment {
            typedef unsigned int Type;
            static const Type Top       = 1 << 0;
            static const Type Bottom    = 1 << 1;
            static const Type Left      = 1 << 2;
            static const Type Right     = 1 << 3;
            static const Type Center    = 1 << 4;
        }
        
        class TextAnchor {
        public:
            typedef std::tr1::shared_ptr<TextAnchor> Ptr;
        protected:
            virtual const Vec3f basePosition() const = 0;
            virtual const Alignment::Type alignment() const = 0;
            
            inline const Vec2f alignmentFactors() const {
                const Alignment::Type a = alignment();
                Vec2f factors;
                if ((a & Alignment::Left))
                    factors[0] = +0.5f;
                else if ((a & Alignment::Right))
                    factors[0] = -0.5f;
                if ((a & Alignment::Top))
                    factors[1] = -0.5f;
                else if ((a & Alignment::Bottom))
                    factors[1] = +0.5f;
                return factors;
            }
        public:
            virtual ~TextAnchor() {}
            
            inline const Vec3f offset(const Camera& camera, const Vec2f& size) const {
                const Vec2f halfSize = size / 2.0f;
                const Vec2f factors = alignmentFactors();
                Vec3f offset = camera.project(basePosition());
                for (size_t i = 0; i < 2; i++) {
                    offset[i] += factors[i] * size[i];
                    offset[i] -= halfSize[i];
                    offset[i] = Math<float>::round(offset[i]);
                }
                return offset;
            }
            
            inline const Vec3f position() const {
                return basePosition();
            }
        };
        
        class SimpleTextAnchor : public TextAnchor {
        private:
            Vec3f m_position;
            Alignment::Type m_alignment;
        protected:
            inline const Vec3f basePosition() const {
                return m_position;
            }
            
            inline const Alignment::Type alignment() const {
                return m_alignment;
            }
        public:
            SimpleTextAnchor(const Vec3f& position, const Alignment::Type alignment) :
            m_position(position),
            m_alignment(alignment) {}
        };
        
        template <typename Key>
        class DefaultKeyComparator {
        public:
            inline bool operator()(const Key& lhs, const Key& rhs) const {
                return lhs < rhs;
            }
        };
                
        template <typename Key, typename Comparator = DefaultKeyComparator<Key> >
        class TextRenderer {
        public:
            class TextRendererFilter {
            public:
                TextRendererFilter() {}
                virtual ~TextRendererFilter() {}
                virtual bool stringVisible(RenderContext& context, const Key& key) const = 0;
            };
            
            class SimpleTextRendererFilter : public TextRendererFilter {
            public:
                inline bool stringVisible(RenderContext& context, const Key& key) const {
                    return true;
                }
            };
            
        protected:
            class TextEntry {
            private:
                Vec2f::List m_vertices;
                Vec2f m_size;
                TextAnchor::Ptr m_textAnchor;
            public:
                TextEntry(const Vec2f::List& vertices, const Vec2f& size, TextAnchor::Ptr textAnchor) :
                m_vertices(vertices),
                m_size(size),
                m_textAnchor(textAnchor) {}
                
                inline const Vec2f::List& vertices() const {
                    return m_vertices;
                }
                
                inline void update(const Vec2f::List& vertices, const Vec2f& size) {
                    m_vertices = vertices;
                    m_size = size;
                }
                
                inline const Vec2f& size() const {
                    return m_size;
                }
                
                inline const TextAnchor& textAnchor() const {
                    return *m_textAnchor.get();
                }
            };
            
            typedef std::map<Key, TextEntry, Comparator> TextMap;
            typedef std::pair<Key, TextEntry> TextMapItem;
            typedef std::vector<TextEntry> EntryList;
            
            TextureFont& m_font;
            float m_fadeDistance;
            float m_hInset;
            float m_vInset;
            
            TextMap m_entries;
            Vbo m_vbo;
            
            inline void addString(Key key, const Vec2f::List& vertices, const Vec2f& size, TextAnchor::Ptr anchor) {
                removeString(key);
                m_entries.insert(TextMapItem(key, TextEntry(vertices, size, anchor)));
            }
            
            EntryList visibleEntries(RenderContext& context, const TextRendererFilter& filter) {
                float cutoff = (m_fadeDistance + 100) * (m_fadeDistance + 100);
                EntryList result;
                
                typename TextMap::iterator it, end;
                for (it = m_entries.begin(), end = m_entries.end(); it != end; ++it) {
                    Key key = it->first;
                    if (filter.stringVisible(context, key)) {
                        const TextEntry& entry = it->second;
                        const TextAnchor& anchor = entry.textAnchor();
                        const Vec3f position = anchor.position();
                        
                        const float dist2 = context.camera().squaredDistanceTo(position);
                        if (dist2 <= cutoff)
                            result.push_back(entry);
                    }
                }
                
                return result;
            }
        public:
            TextRenderer(TextureFont& font) :
            m_font(font),
            m_fadeDistance(100.0f),
            m_hInset(4.0f),
            m_vInset(4.0f),
            m_vbo(GL_ARRAY_BUFFER, 0xFFFF) {}
            
            ~TextRenderer() {
                clear();
            }
            
            inline void addString(Key key, const String& string, TextAnchor::Ptr anchor) {
                const Vec2f::List vertices = m_font.quads(string, true);
                const Vec2f size = m_font.measure(string);
                addString(key, vertices, size, anchor);
            }
            
            inline void removeString(Key key)  {
                typename TextMap::iterator it = m_entries.find(key);
                if (it != m_entries.end()) {
                    m_entries.erase(it);
                }
            }
            
            inline void updateString(Key key, const String& string) {
                typename TextMap::iterator it = m_entries.find(key);
                if (it != m_entries.end()) {
                    TextEntry& entry = it->second;
                    entry.setVertices(m_font.quads(string, true));
                }
            }
            
            inline void transferString(Key key, TextRenderer& destination)  {
                typename TextMap::iterator it = m_entries.find(key);
                if (it != m_entries.end()) {
                    TextEntry& entry = it->second;
                    destination.addString(key, entry.vertices(), entry.textAnchor());
                    m_entries.erase(it);
                }
            }
            
            inline bool empty() const {
                return m_entries.empty();
            }
            
            inline void clear()  {
                m_entries.clear();
            }
            
            inline void setFadeDistance(float fadeDistance)  {
                m_fadeDistance = fadeDistance;
            }
            
            void render(RenderContext& context, const TextRendererFilter& filter, const ShaderConfig& textProgram, const Color& textColor, const ShaderConfig& backgroundProgram, const Color& backgroundColor) {
                if (m_entries.empty())
                    return;
                
                EntryList entries = visibleEntries(context, filter);
                if (entries.empty())
                    return;

                typedef VertexSpecs::P3T2::Vertex FontVertex;
                FontVertex::List fontVertices;
                
                typedef VertexSpecs::P3::Vertex RectVertex;
                RectVertex::List rectVertices;
                rectVertices.reserve( 3 * 16 * entries.size());
                
                Vec2f::List tempRect;
                tempRect.reserve(3 * 16);
                
                typename EntryList::const_iterator it, end;
                for (it = entries.begin(), end = entries.end(); it != end; ++it) {
                    const TextEntry& entry = *it;
                    const Vec2f& size = entry.size().rounded();
                    const TextAnchor& anchor = entry.textAnchor();
                    const Vec3f offset = anchor.offset(context.camera(), size);
                    
                    const Vec2f::List& textVertices = entry.vertices();
                    for (size_t j = 0; j < textVertices.size() / 2; j++) {
                        const Vec2f& position = textVertices[2 * j];
                        const Vec2f& texCoords = textVertices[2 * j + 1];
                        fontVertices.push_back(FontVertex(position, texCoords));
                    }
                    
                    roundedRect(size.x() + 2.0f * m_hInset, size.y() + 2.0f * m_vInset, 3.0f, 3, tempRect);
                    for (size_t j = 0; j < tempRect.size(); j++) {
                        const Vec2f& vertex = tempRect[j];
                        const Vec3f position = Vec3f(vertex.x() + offset.x() + size.x() / 2.0f,
                                                     vertex.y() + offset.y() + size.y() / 2.0f,
                                                     -offset.z());
                        rectVertices.push_back(RectVertex(position));
                    }
                }
                
                const Camera::Viewport& viewport = context.camera().viewport();
                
                const Mat4x4f projection = orthoMatrix(0.0f, 1.0f,
                                                       static_cast<float>(viewport.x),
                                                       static_cast<float>(viewport.height),
                                                       static_cast<float>(viewport.width),
                                                       static_cast<float>(viewport.y));
                const Mat4x4f view = viewMatrix(Vec3f::NegZ, Vec3f::PosY);
                
                ReplaceTransformation ortho(context.transformation(), projection, view);
                VertexArray fontArray(m_vbo, GL_QUADS, fontVertices);
                VertexArray rectArray(m_vbo, GL_TRIANGLES, rectVertices);

                SetVboState vboState(m_vbo);
                vboState.mapped();
                fontArray.prepare();
                rectArray.prepare();
                vboState.active();
                
                glDepthMask(GL_FALSE);
                
                ActiveShader backgroundShader(context.shaderManager(), backgroundProgram);
                backgroundShader.set("Color", backgroundColor);
                rectArray.render();
                
                ActiveShader textShader(context.shaderManager(), textProgram);
                textShader.set("Color", textColor);
                textShader.set("Texture", 0);
                m_font.activate();
                fontArray.render();
                m_font.deactivate();
                
                glDepthMask(GL_TRUE);
            }
        };
    }
}

#endif
