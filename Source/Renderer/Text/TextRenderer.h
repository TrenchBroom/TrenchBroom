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

#include "Renderer/ApplyMatrix.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Text/TexturedFont.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <cassert>
#include <map>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        namespace Text {
            
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
                virtual ~TextAnchor() {}
                
                virtual const Vec3f position() const = 0;
                virtual const Alignment::Type alignment() const = 0;
            };
            
            class SimpleTextAnchor : public TextAnchor {
            private:
                Vec3f m_position;
                Alignment::Type m_alignment;
            public:
                SimpleTextAnchor(const Vec3f& position, const Alignment::Type alignment) :
                m_position(position),
                m_alignment(alignment) {}
                
                inline const Vec3f position() const {
                    return m_position;
                }
                
                inline const Alignment::Type alignment() const {
                    return m_alignment;
                }
            };
            
            template <typename Key>
            class DefaultKeyComparator {
            public:
                inline bool operator()(const Key& lhs, const Key& rhs) const {
                    return lhs < rhs;
                }
            };
            
            template <typename Key, typename Anchor, typename Comparator = DefaultKeyComparator<Key> >
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
                    Anchor m_textAnchor;
                public:
                    TextEntry(const Vec2f::List& vertices, const Vec2f& size, const Anchor& textAnchor) :
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
                    
                    inline const Anchor& textAnchor() const {
                        return m_textAnchor;
                    }
                };
                
                typedef std::map<Key, TextEntry, Comparator> TextMap;
                typedef std::pair<Key, TextEntry> TextMapItem;
                typedef std::vector<TextEntry> EntryList;
                
                TexturedFont& m_font;
                float m_fadeDistance;
                float m_hInset;
                float m_vInset;

                TextMap m_entries;
                Vbo* m_vbo;
                
                inline void addString(Key key, const Vec2f::List& vertices, const Vec2f& size, const Anchor& anchor) {
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
                            const Anchor& anchor = entry.textAnchor();
                            const Vec3f position = anchor.position();
                            
                            float dist2 = context.camera().squaredDistanceTo(position);
                            if (dist2 <= cutoff)
                                result.push_back(entry);
                        }
                    }
                    
                    return result;
                }
                
                void renderBackground(const EntryList& entries, RenderContext& context, ShaderProgram& shaderProgram, const Color& color) {
                    /*
                    if (shaderProgram.activate()) {
                        Mat4f billboardMatrix = context.camera().billboardMatrix();
                        
                        unsigned int vertexCount = static_cast<unsigned int>(3 * 16 * entries.size()); // 16 triangles (for a rounded rect with 3 triangles per corner: 3 * 4 + 4 = 16)
                        VertexArray vertexArray(*m_vbo, GL_TRIANGLES, vertexCount,
                                                Attribute::position3f(),
                                                Attribute::color4f());
                        Vec2f::List vertices;
                        vertices.reserve(vertexCount);
                        
                        SetVboState(*m_vbo, Vbo::VboActive);
                        {
                            SetVboState(*m_vbo, Vbo::VboMapped);
                            for (unsigned int i = 0; i < entries.size(); i++) {
                                const EntryWithDistance& entryWithDistance = entries[i];
                                TextEntry& entry = entryWithDistance.entry();
                                float dist = entryWithDistance.distance();
                                float factor = dist / 300.0f;
                                
                                StringRendererPtr stringRenderer = entry.stringRenderer();
                                Anchor& anchor = entry.textAnchor();
                                const Vec3f position = anchor->position();
                                
                                Mat4f matrix;
                                matrix.translate(position);
                                matrix *= billboardMatrix;
                                matrix.scale(Vec3f(factor, factor, 0.0f));
                                
                                Vec3f alignment = anchor->alignmentFactors();
                                alignment.x *= (stringRenderer->width() + 2.0f * m_hInset);
                                alignment.y *= (stringRenderer->height() + 2.0f * m_vInset);
                                matrix.translate(alignment);
                                
                                roundedRect(stringRenderer->width() + 2.0f * m_hInset, stringRenderer->height() + 2.0f * m_vInset, 3.0f, 3, vertices);
                                for (unsigned int j = 0; j < vertices.size(); j++) {
                                    Vec3f vertex = Vec3f(vertices[j].x, vertices[j].y, 0.0f);
                                    vertexArray.addAttribute(matrix * vertex);
                                    vertexArray.addAttribute(color);
                                }
                                vertices.clear();
                            }
                        }
                        
                        vertexArray.render();
                        shaderProgram.deactivate();
                    }
                     */
                }
                
                void renderText(const EntryList& entries, RenderContext& context, ShaderProgram& shaderProgram, const Color& color)  {
                    if (shaderProgram.activate()) {
                        size_t vertexCount = 0;
                        for (size_t i = 0; i < entries.size(); i++) {
                            const TextEntry& entry = entries[i];
                            vertexCount += entry.vertices().size() / 2;
                        }
                        
                        VertexArray vertexArray(*m_vbo, GL_QUADS, static_cast<unsigned int>(vertexCount),
                                                Attribute::position3f(),
                                                Attribute::texCoord02f());

                        SetVboState mapVbo(*m_vbo, Vbo::VboMapped);
                        for (size_t i = 0; i < entries.size(); i++) {
                            const TextEntry& entry = entries[i];
                            const Vec2f& size = entry.size().rounded();
                            const Anchor& anchor = entry.textAnchor();
                            Vec3f offset = context.camera().project(anchor.position());
                            offset.x = Math::round(offset.x - size.x / 2.0f);
                            offset.y = Math::round(offset.y - size.y / 2.0f);
                            
                            
                            const Vec2f::List& vertices = entry.vertices();
                            for (size_t j = 0; j < vertices.size() / 2; j++) {
                                const Vec2f& vertex = vertices[2 * j];
                                const Vec2f& texCoords = vertices[2 * j + 1];
                                
                                vertexArray.addAttribute(Vec3f(vertex.x + offset.x, vertex.y + offset.y, -offset.z));
                                vertexArray.addAttribute(texCoords);
                            }
                        }
                        
                        const Camera::Viewport& viewport = context.camera().viewport();
                        
                        Mat4f projection;
                        projection.setOrtho(0.0f, 1.0f,
                                            static_cast<float>(viewport.x),
                                            static_cast<float>(viewport.height),
                                            static_cast<float>(viewport.width),
                                            static_cast<float>(viewport.y));
                        
                        Mat4f view;
                        view.setView(Vec3f::NegZ, Vec3f::PosY);
                        view.translate(Vec3f::Null);
                        
                        ApplyMatrix orthoMatrix(context.transformation(), projection * view, true);

                        glDepthMask(GL_FALSE);
                        SetVboState activateVbo(*m_vbo, Vbo::VboActive);
                        shaderProgram.setUniformVariable("Color", color);
                        shaderProgram.setUniformVariable("Texture", 0);
                        m_font.activate();
                        vertexArray.render();
                        m_font.deactivate();
                        glDepthMask(GL_TRUE);
                    }
                }
            public:
                TextRenderer(TexturedFont& font) :
                m_font(font),
                m_fadeDistance(100.0f),
                m_hInset(2.5f),
                m_vInset(2.5f),
                m_vbo(NULL) {}
                
                ~TextRenderer() {
                    clear();
                    delete m_vbo;
                    m_vbo = NULL;
                }
                
                inline void addString(Key key, const String& string, const Anchor& anchor) {
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
                        entry.setVertices(m_font.quads(string));
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
                
                void render(RenderContext& context, const TextRendererFilter& filter, ShaderProgram& textProgram, const Color& textColor, ShaderProgram& backgroundProgram, const Color& backgroundColor) {
                    if (m_entries.empty())
                        return;
                    
                    EntryList entries = visibleEntries(context, filter);
                    if (entries.empty())
                        return;
                    
                    if (m_vbo == NULL)
                        m_vbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
                    
                    renderBackground(entries, context, backgroundProgram, backgroundColor);
                    renderText(entries, context, textProgram, textColor);
                }
            };
        }
    }
}

#endif
