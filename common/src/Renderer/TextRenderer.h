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

#ifndef TrenchBroom_TextRenderer_h
#define TrenchBroom_TextRenderer_h

#include "Color.h"
#include "VecMath.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/TextureFont.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <map>

class Color;

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
        public:
            virtual ~TextAnchor() {}
            Vec3f offset(const Camera& camera, const Vec2f& size) const;
            Vec3f position() const;
        private:
            virtual Vec3f basePosition() const = 0;
            virtual Alignment::Type alignment() const = 0;
            virtual Vec2f extraOffsets(const Alignment::Type a) const;
            Vec2f alignmentFactors(const Alignment::Type a) const;
        };
        
        class SimpleTextAnchor : public TextAnchor {
        private:
            Vec3f m_position;
            Alignment::Type m_alignment;
            Vec2f m_extraOffsets;
        protected:
            Vec3f basePosition() const;
            Alignment::Type alignment() const;
            Vec2f extraOffsets(const Alignment::Type a) const;
        public:
            SimpleTextAnchor(const Vec3f& position, const Alignment::Type alignment, const Vec2f& extraOffsets = Vec2f::Null);
        };
        
        template <typename Key>
        class DefaultKeyComparator {
        public:
            bool operator()(const Key& lhs, const Key& rhs) const {
                return lhs < rhs;
            }
        };
                
        template <typename Key, typename Comparator = DefaultKeyComparator<Key> >
        class TextRenderer {
        public:
            class TextRendererFilter {
            public:
                virtual ~TextRendererFilter() {}
                virtual bool stringVisible(RenderContext& context, const Key& key) const = 0;
            };
            
            class SimpleTextRendererFilter : public TextRendererFilter {
            public:
                bool stringVisible(RenderContext& context, const Key& key) const {
                    return true;
                }
            };
            
            class TextColorProvider {
            public:
                virtual ~TextColorProvider() {}
                virtual Color textColor(RenderContext& context, const Key& key) const = 0;
                virtual Color backgroundColor(RenderContext& context, const Key& key) const = 0;
            };
            
            class PrefTextColorProvider : public TextColorProvider {
            private:
                Preference<Color>& m_textColorPref;
                Preference<Color>& m_backgroundColorPref;
            public:
                PrefTextColorProvider(Preference<Color>& textColorPref, Preference<Color>& backgroundColorPref) :
                m_textColorPref(textColorPref),
                m_backgroundColorPref(backgroundColorPref) {}
            public:
                Color textColor(RenderContext& context, const Key& key) const {
                    PreferenceManager& prefs = PreferenceManager::instance();
                    return prefs.get(m_textColorPref);
                }
                
                Color backgroundColor(RenderContext& context, const Key& key) const {
                    PreferenceManager& prefs = PreferenceManager::instance();
                    return prefs.get(m_backgroundColorPref);
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
                
                const Vec2f::List& vertices() const {
                    return m_vertices;
                }
                
                void updateVertices(const Vec2f::List& vertices, const Vec2f& size) {
                    m_vertices = vertices;
                    m_size = size;
                }
                
                const Vec2f& size() const {
                    return m_size;
                }
                
                const TextAnchor& textAnchor() const {
                    return *m_textAnchor.get();
                }
                
                void setAnchor(TextAnchor::Ptr textAnchor) {
                    m_textAnchor = textAnchor;
                }
            };
            
            typedef std::map<Key, TextEntry, Comparator> TextMap;
            typedef std::pair<Key, TextEntry> TextMapItem;
            typedef std::vector<typename TextMap::const_iterator> EntryList;
            
            TextureFont& m_font;
            float m_fadeDistance;
            float m_hInset;
            float m_vInset;
            
            TextMap m_entries;
            TextMap m_renderOnce;
            Vbo m_vbo;
        public:
            TextRenderer(TextureFont& font) :
            m_font(font),
            m_fadeDistance(100.0f),
            m_hInset(4.0f),
            m_vInset(4.0f),
            m_vbo(0xFFFF) {}
            
            ~TextRenderer() {
                clear();
            }
            
            void renderOnce(Key key, const String& string, TextAnchor::Ptr anchor) {
                const Vec2f::List vertices = m_font.quads(string, true);
                const Vec2f size = m_font.measure(string);
                addString(key, vertices, size, anchor, m_renderOnce);
            }
            
            void addString(Key key, const String& string, TextAnchor::Ptr anchor) {
                const Vec2f::List vertices = m_font.quads(string, true);
                const Vec2f size = m_font.measure(string);
                addString(key, vertices, size, anchor, m_entries);
            }
            
            void removeString(Key key)  {
                typename TextMap::iterator it = m_entries.find(key);
                if (it != m_entries.end()) {
                    m_entries.erase(it);
                }
            }
            
            void updateString(Key key, const String& string) {
                typename TextMap::iterator it = m_entries.find(key);
                if (it != m_entries.end()) {
                    TextEntry& entry = it->second;
                    const Vec2f::List vertices = m_font.quads(string, true);
                    const Vec2f size = m_font.measure(string);
                    entry.updateVertices(vertices, size);
                }
            }
            
            void updateAnchor(Key key, TextAnchor::Ptr anchor) {
                typename TextMap::iterator it = m_entries.find(key);
                if (it != m_entries.end()) {
                    TextEntry& entry = it->second;
                    entry.setAnchor(anchor);
                }
            }
            
            void transferString(Key key, TextRenderer& destination)  {
                typename TextMap::iterator it = m_entries.find(key);
                if (it != m_entries.end()) {
                    TextEntry& entry = it->second;
                    destination.addString(key, entry.vertices(), entry.textAnchor());
                    m_entries.erase(it);
                }
            }
            
            bool empty() const {
                return m_entries.empty();
            }
            
            void clear()  {
                m_entries.clear();
                clearRenderOnce();
            }
            
            void setFadeDistance(float fadeDistance)  {
                m_fadeDistance = fadeDistance;
            }
            
            void render(RenderContext& context, const TextRendererFilter& filter, const TextColorProvider& colorProvider, const ShaderConfig& textProgram, const ShaderConfig& backgroundProgram) {
                if (m_entries.empty() && m_renderOnce.empty())
                    return;
                
                EntryList entries = visibleEntries(context, filter);
                if (entries.empty())
                    return;

                typedef VertexSpecs::P3T2C4::Vertex FontVertex;
                FontVertex::List fontVertices;
                
                typedef VertexSpecs::P3C4::Vertex RectVertex;
                RectVertex::List rectVertices;
                rectVertices.reserve(3 * 16 * entries.size());
                
                typename EntryList::const_iterator it, end;
                for (it = entries.begin(), end = entries.end(); it != end; ++it) {
                    const Key& key = (*it)->first;
                    const TextEntry& entry = (*it)->second;
                    const Vec2f& size = entry.size().rounded();
                    const TextAnchor& anchor = entry.textAnchor();
                    const Vec3f offset = anchor.offset(context.camera(), size);
                    
                    const Vec2f::List& textVertices = entry.vertices();
                    for (size_t j = 0; j < textVertices.size() / 2; ++j) {
                        const Vec2f& position2 = textVertices[2 * j];
                        const Vec2f& texCoords = textVertices[2 * j + 1];
                        const Vec3f position3(position2.x() + offset.x(),
                                              position2.y() + offset.y(),
                                              -offset.z());
                        fontVertices.push_back(FontVertex(position3, texCoords, colorProvider.textColor(context, key)));
                    }

                    const Vec2f::List tempRect = roundedRect2D(size.x() + 2.0f * m_hInset, size.y() + 2.0f * m_vInset, 3.0f, 3);
                    for (size_t j = 0; j < tempRect.size(); ++j) {
                        const Vec2f& vertex = tempRect[j];
                        const Vec3f position = Vec3f(vertex.x() + offset.x() + size.x() / 2.0f,
                                                     vertex.y() + offset.y() + size.y() / 2.0f,
                                                     -offset.z());
                        rectVertices.push_back(RectVertex(position, colorProvider.backgroundColor(context, key)));
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
                VertexArray fontArray = VertexArray::swap(GL_QUADS, fontVertices);
                VertexArray rectArray = VertexArray::swap(GL_TRIANGLES, rectVertices);

                SetVboState vboState(m_vbo);
                vboState.mapped();
                fontArray.prepare(m_vbo);
                rectArray.prepare(m_vbo);
                vboState.active();
                
                // glDepthMask(GL_FALSE);
                glDisable(GL_TEXTURE_2D);
                
                ActiveShader backgroundShader(context.shaderManager(), backgroundProgram);
                rectArray.render();
                
                glEnable(GL_TEXTURE_2D);
                ActiveShader textShader(context.shaderManager(), textProgram);
                textShader.set("Texture", 0);
                m_font.activate();
                fontArray.render();
                m_font.deactivate();
                
                // glDepthMask(GL_TRUE);
                
                clearRenderOnce();
            }
        private:
            void addString(Key key, const Vec2f::List& vertices, const Vec2f& size, TextAnchor::Ptr anchor, TextMap& entries) {
                removeString(key);
                entries.insert(TextMapItem(key, TextEntry(vertices, size, anchor)));
            }
            
            EntryList visibleEntries(RenderContext& context, const TextRendererFilter& filter) const {
                EntryList result;
                visibleEntries(context, filter, m_entries, result);
                visibleEntries(context, filter, m_renderOnce, result);
                return result;
            }
            
            void visibleEntries(RenderContext& context, const TextRendererFilter& filter, const TextMap& entries, EntryList& result) const {
                const float cutoff = (m_fadeDistance + 100) * (m_fadeDistance + 100);

                typename TextMap::const_iterator it, end;
                for (it = entries.begin(), end = entries.end(); it != end; ++it) {
                    Key key = it->first;
                    if (filter.stringVisible(context, key)) {
                        const TextEntry& entry = it->second;
                        const TextAnchor& anchor = entry.textAnchor();
                        const Vec3f position = anchor.position();
                        
                        const float dist2 = context.camera().squaredDistanceTo(position);
                        if (dist2 <= cutoff)
                            result.push_back(it);
                    }
                }
            }
            
            void clearRenderOnce() {
                m_renderOnce.clear();
            }
        };
    }
}

#endif
