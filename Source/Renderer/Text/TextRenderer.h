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

#include "Renderer/Camera.h"
#include "Renderer/PushMatrix.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Text/PathRenderer.h"
#include "Renderer/Text/StringManager.h"
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
            
            class TextAnchor {
            public:
                virtual ~TextAnchor() {}
                virtual const Vec3f& position() = 0;
            };
            
            template <typename Key>
            class TextRenderer {
            public:
                class TextRendererFilter {
                public:
                    TextRendererFilter() {}
                    virtual ~TextRendererFilter() {}
                    virtual bool stringVisible(RenderContext& context, const Key& key) = 0;
                };
            protected:
                class TextEntry {
                protected:
                    FontDescriptor m_fontDescriptor;
                    String m_string;
                    StringRendererPtr m_stringRenderer;
                    TextAnchor* m_textAnchor;
                public:
                    TextEntry(const FontDescriptor& fontDescriptor, const String& string, StringRendererPtr stringRenderer, TextAnchor* textAnchor) :
                    m_fontDescriptor(fontDescriptor),
                    m_string(string),
                    m_stringRenderer(stringRenderer),
                    m_textAnchor(textAnchor) {}
                    
                    inline const FontDescriptor& fontDescriptor() const {
                        return m_fontDescriptor;
                    }
                    
                    inline const String& string() const {
                        return m_string;
                    }
                    
                    inline StringRendererPtr stringRenderer() const {
                        return m_stringRenderer;
                    }
                    
                    inline void setStringRenderer(StringRendererPtr stringRenderer) const {
                        m_stringRenderer = stringRenderer;
                    }
                    
                    inline TextAnchor* textAnchor() const {
                        return m_textAnchor;
                    }
                };
                
                class EntryWithDistance {
                protected:
                    TextEntry* m_entry;
                    float m_distance;
                public:
                    EntryWithDistance(TextEntry* entry, float distance) :
                    m_entry(entry),
                    m_distance(distance) {}
                    
                    inline TextEntry& entry() const {
                        return *m_entry;
                    }
                    
                    inline float distance() const {
                        return m_distance;
                    }
                };
                
                typedef std::map<Key, TextEntry> TextMap;
                typedef std::vector<EntryWithDistance> EntryList;
                
                StringManager& m_stringManager;
                float m_fadeDistance;
                float m_hInset;
                float m_vInset;

                TextMap m_entries;
                VboPtr m_backgroundVbo;
                
                inline void addString(Key key, const FontDescriptor& fontDescriptor, const String& string, StringRendererPtr stringRenderer, TextAnchor* anchor) {
                    removeString(key);
                    m_entries.insert(std::pair<Key, TextEntry>(key, TextEntry(fontDescriptor, string, stringRenderer, anchor)));
                }
                
                EntryList visibleEntries(RenderContext& context, TextRendererFilter& filter) {
                    float cutoff = (m_fadeDistance + 100) * (m_fadeDistance + 100);
                    EntryList result;
                    
                    typename TextMap::iterator it, end;
                    for (it = m_entries.begin(), end = m_entries.end(); it != end; ++it) {
                        Key key = it->first;
                        if (filter.stringVisible(context, key)) {
                            TextEntry& entry = it->second;
                            TextAnchor* anchor = entry.textAnchor();
                            const Vec3f& position = anchor->position();
                            
                            float dist2 = context.camera().squaredDistanceTo(position);
                            if (dist2 <= cutoff)
                                result.push_back(EntryWithDistance(&entry, sqrt(dist2)));
                        }
                    }
                    
                    return result;
                }
                
                void renderBackground(const EntryList& entries, RenderContext& context, ShaderProgram& shaderProgram, const Color& color) {
                    if (shaderProgram.activate()) {
                        Mat4f billboardMatrix = context.camera().billboardMatrix();
                        
                        unsigned int vertexCount = static_cast<unsigned int>(3 * 16 * entries.size()); // 16 triangles (for a rounded rect with 3 triangles per corner: 3 * 4 + 4 = 16)
                        VertexArrayPtr vertexArray = VertexArrayPtr(new VertexArray(*m_backgroundVbo, GL_TRIANGLES, vertexCount,
                                                                                    VertexAttribute::position3f(),
                                                                                    VertexAttribute::color4f()));
                        Vec2f::List vertices;
                        vertices.reserve(vertexCount);
                        
                        m_backgroundVbo->activate();
                        m_backgroundVbo->map();
                        for (unsigned int i = 0; i < entries.size(); i++) {
                            const EntryWithDistance& entryWithDistance = entries[i];
                            TextEntry& entry = entryWithDistance.entry();
                            float dist = entryWithDistance.distance();
                            float factor = dist / 300.0f;
                            
                            StringRendererPtr stringRenderer = entry.stringRenderer();
                            TextAnchor* anchor = entry.textAnchor();
                            const Vec3f& position = anchor->position();
                            
                            Mat4f matrix;
                            matrix.translate(position);
                            matrix *= billboardMatrix;
                            matrix.scale(Vec3f(factor, factor, 0.0f));
                            matrix.translate(Vec3f(0.0f, (stringRenderer->height() - m_vInset) / 2.0f, 0.0f));
                             
                            float a = 1.0f - (std::max)(dist - m_fadeDistance, 0.0f) / 100.0f;
                            roundedRect(stringRenderer->width() + m_hInset, stringRenderer->height() + m_vInset, 3.0f, 3, vertices);
                            for (unsigned int j = 0; j < vertices.size(); j++) {
                                Vec3f vertex = Vec3f(vertices[j].x, vertices[j].y, 0.0f);
                                vertexArray->addAttribute(matrix * vertex);
                                vertexArray->addAttribute(Color(color.x, color.y, color.z, color.w * a));
                            }
                            vertices.clear();
                        }
                        m_backgroundVbo->unmap();
                        
                        vertexArray->render();
                        m_backgroundVbo->deactivate();
                        shaderProgram.deactivate();
                    }
                }
                
                void renderText(const EntryList& entries, RenderContext& context, ShaderProgram& shaderProgram, const Color& color)  {
                    PushMatrix pushMatrix(context.transformation());
                    Mat4f billboardMatrix = context.camera().billboardMatrix();

                    if (shaderProgram.activate()) {
                        m_stringManager.activate();
                        glSetEdgeOffset(0.01f);
                        for (unsigned int i = 0; i < entries.size(); i++) {
                            const EntryWithDistance& entryWithDistance = entries[i];
                            TextEntry& entry = entryWithDistance.entry();
                            float dist = entryWithDistance.distance();
                            float factor = dist / 300.0f;
                            
                            StringRendererPtr stringRenderer = entry.stringRenderer();
                            TextAnchor* anchor = entry.textAnchor();
                            const Vec3f& position = anchor->position();
                            
                            Mat4f matrix = pushMatrix.matrix();
                            matrix.translate(position);
                            matrix *= billboardMatrix;
                            matrix.scale(Vec3f(factor, factor, 0.0f));
                            matrix.translate(Vec3f(-stringRenderer->width() / 2.0f, m_vInset / 2.0f, 0.0f));
                            pushMatrix.load(matrix);

                            float a = 1.0f - (std::max)(dist - m_fadeDistance, 0.0f) / 100.0f;
                            shaderProgram.setUniformVariable("Color", Vec4f(color.x, color.y, color.z, color.w * a));
                            stringRenderer->render();
                        }
                        glResetEdgeOffset();
                        m_stringManager.deactivate();
                        shaderProgram.deactivate();
                    }
                }
            public:
                TextRenderer(StringManager& stringManager) :
                m_stringManager(stringManager),
                m_fadeDistance(100.0f),
                m_hInset(3.0f),
                m_vInset(3.0f) {}
                
                ~TextRenderer() {
                    clear();
                }
                
                inline void addString(Key key, const FontDescriptor& fontDescriptor, const String& string, TextAnchor* anchor) {
                    StringRendererPtr stringRenderer = m_stringManager.stringRenderer(fontDescriptor, string);
                    assert(stringRenderer.get() != NULL);
                    
                    addString(key, fontDescriptor, string, stringRenderer, anchor);
                }
                
                inline void removeString(Key key)  {
                    typename TextMap::iterator it = m_entries.find(key);
                    if (it != m_entries.end()) {
                        TextEntry& entry = it->second;
                        delete entry.textAnchor();
                        m_entries.erase(it);
                    }
                }
                
                inline void updateString(Key key, const std::string& str) {
                    typename TextMap::iterator it = m_entries.find(key);
                    if (it != m_entries.end()) {
                        TextEntry& entry = it->second;
                        const FontDescriptor& fontDescriptor = entry.fontDescriptor();
                        const String& string = entry.string();
                        
                        StringRendererPtr stringRenderer = m_stringManager.stringRenderer(fontDescriptor, string);
                        entry.setStringRenderer(stringRenderer);
                    }
                }
                
                inline void transferString(Key key, TextRenderer& destination)  {
                    typename TextMap::iterator it = m_entries.find(key);
                    if (it != m_entries.end()) {
                        TextEntry& entry = it->second;
                        destination.addString(key, entry.fontDescriptor(), entry.string(), entry.stringRenderer(), entry.textAnchor());
                        m_entries.erase(it);
                    }
                }
                
                inline bool empty() const {
                    return m_entries.empty();
                }
                
                inline void clear()  {
                    typename TextMap::iterator it, end;
                    for (it = m_entries.begin(), end = m_entries.end(); it != end; ++it) {
                        TextEntry& entry = it->second;
                        delete entry.textAnchor();
                    }
                    m_entries.clear();
                }
                
                inline void setFadeDistance(float fadeDistance)  {
                    m_fadeDistance = fadeDistance;
                }
                
                void render(RenderContext& context, TextRendererFilter& filter, ShaderProgram& textProgram, const Color& textColor, ShaderProgram& backgroundProgram, const Color& backgroundColor) {
                    if (m_entries.empty())
                        return;
                    
                    EntryList entries = visibleEntries(context, filter);
                    if (entries.empty())
                        return;
                    
                    if (m_backgroundVbo.get() == NULL)
                        m_backgroundVbo = VboPtr(new Vbo(GL_ARRAY_BUFFER, 0xFFFF));
                    
                    renderBackground(entries, context, backgroundProgram, backgroundColor);
                    renderText(entries, context, textProgram, textColor);
                }
            };
        }
    }
}

#endif
