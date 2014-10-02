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
#include "AttrString.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Renderer/Camera.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontManager.h"
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
                virtual bool stringVisible(RenderContext& renderContext, const Key& key) const = 0;
            };
            
            class SimpleTextRendererFilter : public TextRendererFilter {
            public:
                bool stringVisible(RenderContext& renderContext, const Key& key) const {
                    return true;
                }
            };
            
            class TextColorProvider {
            public:
                virtual ~TextColorProvider() {}
                virtual Color textColor(RenderContext& renderContext, const Key& key) const = 0;
                virtual Color backgroundColor(RenderContext& renderContext, const Key& key) const = 0;
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
                Color textColor(RenderContext& renderContext, const Key& key) const {
                    PreferenceManager& prefs = PreferenceManager::instance();
                    return prefs.get(m_textColorPref);
                }
                
                Color backgroundColor(RenderContext& renderContext, const Key& key) const {
                    PreferenceManager& prefs = PreferenceManager::instance();
                    return prefs.get(m_backgroundColorPref);
                }
            };
            
        protected:
            class PreparedEntry {
            private:
                Vec2f::List m_vertices;
                Vec2f m_size;
                TextAnchor::Ptr m_anchor;
            public:
                PreparedEntry(const Vec2f::List vertices, const Vec2f& size, TextAnchor::Ptr anchor) :
                m_vertices(vertices),
                m_size(size),
                m_anchor(anchor) {}
                
                const Vec2f::List& vertices() const {
                    return m_vertices;
                }
                
                const Vec2f& size() const {
                    return m_size;
                }
                
                TextAnchor::Ptr anchor() const {
                    return m_anchor;
                }
                
                void setAnchor(TextAnchor::Ptr anchor) {
                    m_anchor = anchor;
                }
            };
            
            class UnpreparedEntry {
            private:
                AttrString m_string;
                TextAnchor::Ptr m_anchor;
            public:
                UnpreparedEntry(const AttrString& string, TextAnchor::Ptr anchor) :
                m_string(string),
                m_anchor(anchor) {}

                PreparedEntry prepare(TextureFont& font) const {
                    const Vec2f::List vertices = font.quads(m_string, true);
                    const Vec2f size = font.measure(m_string);
                    return PreparedEntry(vertices, size, m_anchor);
                }
                
                TextAnchor::Ptr anchor() const {
                    return m_anchor;
                }
                
                void setAnchor(TextAnchor::Ptr anchor) {
                    m_anchor = anchor;
                }
            };
            
            typedef std::map<Key, UnpreparedEntry, Comparator> UnpreparedEntryMap;
            typedef std::map<Key, PreparedEntry, Comparator> PreparedEntryMap;
            typedef std::vector<typename PreparedEntryMap::const_iterator> EntryList;
            
            FontDescriptor m_fontDescriptor;
            float m_fadeDistance;
            float m_hInset;
            float m_vInset;

            UnpreparedEntryMap m_unpreparedEntries;
            PreparedEntryMap m_preparedEntries;
            
            Vbo m_vbo;
        public:
            TextRenderer(const FontDescriptor& fontDescriptor) :
            m_fontDescriptor(fontDescriptor),
            m_fadeDistance(100.0f),
            m_hInset(4.0f),
            m_vInset(4.0f),
            m_vbo(0xFFFF) {}
            
            ~TextRenderer() {
                clear();
            }
        public:
            void addString(const Key& key, const AttrString& string, TextAnchor::Ptr anchor) {
                addString(key, string, anchor, m_unpreparedEntries);
            }
        private:
            void addString(const Key& key, const AttrString& string, TextAnchor::Ptr anchor, UnpreparedEntryMap& entries) {
                removeString(key);
                entries.insert(std::make_pair(key, UnpreparedEntry(string, anchor)));
            }
        public:
            void removeString(const Key& key)  {
                typename PreparedEntryMap::iterator pIt = m_preparedEntries.find(key);
                if (pIt != m_preparedEntries.end()) {
                    m_preparedEntries.erase(pIt);
                } else {
                    typename UnpreparedEntryMap::iterator upIt = m_unpreparedEntries.find(key);
                    if (upIt != m_unpreparedEntries.end())
                        m_unpreparedEntries.erase(upIt);
                }
            }
            
            void updateString(const Key& key, const AttrString& string) {
                TextAnchor::Ptr anchor;
                
                typename PreparedEntryMap::iterator pIt = m_preparedEntries.find(key);
                if (pIt != m_preparedEntries.end()) {
                    const PreparedEntry& entry = pIt->second;
                    anchor = entry.anchor();
                    m_preparedEntries.erase(pIt);
                } else {
                    typename UnpreparedEntryMap::iterator upIt = m_unpreparedEntries.find(key);
                    if (upIt != m_unpreparedEntries.end()) {
                        const UnpreparedEntry& entry = upIt->second;
                        anchor = entry.anchor();
                        m_unpreparedEntries.erase(upIt);
                    } else {
                        return;
                    }
                }
                
                addString(key, string, anchor);
            }
            
            void updateAnchor(const Key& key, TextAnchor::Ptr anchor) {
                typename PreparedEntryMap::iterator pIt = m_preparedEntries.find(key);
                if (pIt != m_preparedEntries.end()) {
                    const PreparedEntry& entry = pIt->second;
                    entry.setAnchor(anchor);
                } else {
                    typename UnpreparedEntryMap::iterator upIt = m_unpreparedEntries.find(key);
                    if (upIt != m_unpreparedEntries.end()) {
                        const UnpreparedEntry& entry = pIt->second;
                        entry.setAnchor(anchor);
                    }
                }
            }
        public:
            bool empty() const {
                return m_unpreparedEntries.empty() && m_preparedEntries.empty();
            }
            
            void clear()  {
                m_unpreparedEntries.clear();
                m_preparedEntries.clear();
            }
            
            void setFadeDistance(const float fadeDistance)  {
                m_fadeDistance = fadeDistance;
            }
            
            void render(RenderContext& renderContext, const TextRendererFilter& filter, const TextColorProvider& colorProvider, const ShaderConfig& textProgram, const ShaderConfig& backgroundProgram) {
                if (empty())
                    return;
                
                FontManager& fontManager = renderContext.fontManager();
                TextureFont& font = fontManager.font(m_fontDescriptor);

                prepareEntries(font);
                
                EntryList entries = visibleEntries(renderContext, filter);
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
                    const PreparedEntry& entry = (*it)->second;
                    const Vec2f& size = entry.size().rounded();
                    const TextAnchor::Ptr anchor = entry.anchor();
                    const Vec3f offset = anchor->offset(renderContext.camera(), size);
                    
                    const Vec2f::List& textVertices = entry.vertices();
                    for (size_t j = 0; j < textVertices.size() / 2; ++j) {
                        const Vec2f& position2 = textVertices[2 * j];
                        const Vec2f& texCoords = textVertices[2 * j + 1];
                        const Vec3f position3(position2.x() + offset.x(),
                                              position2.y() + offset.y(),
                                              -offset.z());
                        fontVertices.push_back(FontVertex(position3, texCoords, colorProvider.textColor(renderContext, key)));
                    }

                    const Vec2f::List tempRect = roundedRect2D(size.x() + 2.0f * m_hInset, size.y() + 2.0f * m_vInset, 3.0f, 3);
                    for (size_t j = 0; j < tempRect.size(); ++j) {
                        const Vec2f& vertex = tempRect[j];
                        const Vec3f position = Vec3f(vertex.x() + offset.x() + size.x() / 2.0f,
                                                     vertex.y() + offset.y() + size.y() / 2.0f,
                                                     -offset.z());
                        rectVertices.push_back(RectVertex(position, colorProvider.backgroundColor(renderContext, key)));
                    }
                }
                
                const Camera::Viewport& viewport = renderContext.camera().viewport();
                const Mat4x4f projection = orthoMatrix(0.0f, 1.0f,
                                                       static_cast<float>(viewport.x),
                                                       static_cast<float>(viewport.height),
                                                       static_cast<float>(viewport.width),
                                                       static_cast<float>(viewport.y));
                const Mat4x4f view = viewMatrix(Vec3f::NegZ, Vec3f::PosY);
                
                ReplaceTransformation ortho(renderContext.transformation(), projection, view);
                VertexArray fontArray = VertexArray::swap(GL_QUADS, fontVertices);
                VertexArray rectArray = VertexArray::swap(GL_TRIANGLES, rectVertices);

                SetVboState vboState(m_vbo);
                vboState.mapped();
                fontArray.prepare(m_vbo);
                rectArray.prepare(m_vbo);
                vboState.active();
                
                // glDepthMask(GL_FALSE);
                glDisable(GL_TEXTURE_2D);
                
                ActiveShader backgroundShader(renderContext.shaderManager(), backgroundProgram);
                rectArray.render();
                
                glEnable(GL_TEXTURE_2D);
                ActiveShader textShader(renderContext.shaderManager(), textProgram);
                textShader.set("Texture", 0);
                font.activate();
                fontArray.render();
                font.deactivate();
                
                // glDepthMask(GL_TRUE);
            }
        private:
            void prepareEntries(TextureFont& font) {
                typename UnpreparedEntryMap::iterator it, end;
                for (it = m_unpreparedEntries.begin(), end = m_unpreparedEntries.end(); it != end; ++it) {
                    const Key& key = it->first;
                    const UnpreparedEntry& unpreparedEntry = it->second;
                    const PreparedEntry preparedEntry = unpreparedEntry.prepare(font);
                    m_preparedEntries.insert(std::make_pair(key, preparedEntry));
                }
                m_unpreparedEntries.clear();
            }
            
            EntryList visibleEntries(RenderContext& renderContext, const TextRendererFilter& filter) const {
                EntryList result;
                visibleEntries(renderContext, filter, m_preparedEntries, result);
                return result;
            }
            
            void visibleEntries(RenderContext& renderContext, const TextRendererFilter& filter, const PreparedEntryMap& entries, EntryList& result) const {
                
                typename PreparedEntryMap::const_iterator it, end;
                for (it = entries.begin(), end = entries.end(); it != end; ++it) {
                    const Key& key = it->first;
                    const PreparedEntry& entry = it->second;
                    if (entryVisible(renderContext, filter, key, entry))
                        result.push_back(it);
                }
            }
            
            bool entryVisible(RenderContext& renderContext, const TextRendererFilter& filter, const Key& key, const PreparedEntry& entry) const {
                const float cutoff = (m_fadeDistance + 100) * (m_fadeDistance + 100);

                if (filter.stringVisible(renderContext, key)) {
                    const TextAnchor::Ptr anchor = entry.anchor();
                    const Vec3f position = anchor->position();
                    const float dist2 = renderContext.camera().squaredDistanceTo(position);
                    if (dist2 <= cutoff)
                        return true;
                }
                return false;
            }
        };
    }
}

#endif
