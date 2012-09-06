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
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Text/PathRenderer.h"
#include "Renderer/Text/StringManager.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <cassert>
#include <map>
#include <algorithm>

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
                    StringRenderer* m_stringRenderer;
                    TextAnchor* m_textAnchor;
                public:
                    TextEntry(const FontDescriptor& fontDescriptor, const String& string, StringRenderer* stringRenderer, TextAnchor* textAnchor) :
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
                    
                    inline StringRenderer* stringRenderer() const {
                        return m_stringRenderer;
                    }
                    
                    inline void setStringRenderer(StringRenderer* stringRenderer) const {
                        m_stringRenderer = stringRenderer;
                    }
                    
                    inline TextAnchor* textAnchor() const {
                        return m_textAnchor;
                    }
                };
                
                typedef std::map<Key, TextEntry> TextMap;
                
                float m_fadeDistance;
                StringManager& m_stringManager;
                TextMap m_entries;
                
                void addString(Key key, const FontDescriptor& fontDescriptor, const String& string, StringRenderer* stringRenderer, TextAnchor* anchor) {
                    removeString(key);
                    m_entries.insert(std::pair<Key, TextEntry>(key, TextEntry(fontDescriptor, string, stringRenderer, anchor)));
                }
            public:
                TextRenderer(StringManager& stringManager, float fadeDistance) :
                m_stringManager(stringManager),
                m_fadeDistance(fadeDistance) {}
                
                ~TextRenderer() {
                    clear();
                }
                
                void addString(Key key, const FontDescriptor& fontDescriptor, const String& string, TextAnchor* anchor) {
                    StringRenderer* stringRenderer = m_stringManager.createStringRenderer(fontDescriptor, string);
                    assert(stringRenderer != NULL);
                    
                    addString(key, fontDescriptor, string, stringRenderer, anchor);
                }
                
                void removeString(Key key)  {
                    typename TextMap::iterator it = m_entries.find(key);
                    if (it != m_entries.end()) {
                        TextEntry& entry = it->second;
                        m_stringManager.destroyStringRenderer(entry.stringRenderer());
                        delete entry.textAnchor();
                        m_entries.erase(it);
                    }
                }
                
                void updateString(Key key, const std::string& str) {
                    typename TextMap::iterator it = m_entries.find(key);
                    if (it != m_entries.end()) {
                        TextEntry& entry = it->second;
                        const FontDescriptor& fontDescriptor = entry.fontDescriptor();
                        const String& string = entry.string();
                        
                        m_stringManager.destroyStringRenderer(entry.stringRenderer());
                        StringRenderer* stringRenderer = m_stringManager.createStringRenderer(fontDescriptor, string);
                        entry.setStringRenderer(stringRenderer);
                    }
                }
                
                void transferString(Key key, TextRenderer& destination)  {
                    typename TextMap::iterator it = m_entries.find(key);
                    if (it != m_entries.end()) {
                        TextEntry& entry = it->second;
                        destination.addString(key, entry.fontDescriptor(), entry.string(), entry.stringRenderer(), entry.textAnchor());
                        m_entries.erase(it);
                    }
                }
                
                void clear()  {
                    typename TextMap::iterator it, end;
                    for (it = m_entries.begin(), end = m_entries.end(); it != end; ++it) {
                        TextEntry& entry = it->second;
                        m_stringManager.destroyStringRenderer(entry.stringRenderer());
                        delete entry.textAnchor();
                    }
                }
                
                void setFadeDistance(float fadeDistance)  {
                    m_fadeDistance = fadeDistance;
                }
                
                void render(RenderContext& context, TextRendererFilter& filter, const Color& color)  {
                    glDisable(GL_TEXTURE_2D);
                    glPolygonMode(GL_FRONT, GL_FILL);
                    float cutoff = (m_fadeDistance + 100) * (m_fadeDistance + 100);
                    
                    typename TextMap::iterator it, end;
                    for (it = m_entries.begin(), end = m_entries.end(); it != end; ++it) {
                        Key key = it->first;
                        if (filter.stringVisible(context, key)) {
                            TextEntry& entry = it->second;
                            StringRenderer* stringRenderer = entry.stringRenderer();
                            TextAnchor* anchor = entry.textAnchor();
                            const Vec3f& position = anchor->position();
                            
                            float dist2 = context.camera().squaredDistanceTo(position);
                            if (dist2 <= cutoff) {
                                float dist = sqrt(dist2);
                                float factor = dist / 300;
                                float width = stringRenderer->width();
                                
                                glPushMatrix();
                                glTranslatef(position.x, position.y, position.z);
                                context.camera().setBillboard();
                                glScalef(factor, factor, 0);
                                glTranslatef(-width / 2, 0, 0);
                                
                                float alphaFactor = 1.0f - (std::max)(dist - m_fadeDistance, 0.0f) / 100.0f;
                                
                                glColor4f(0, 0, 0, 0.6f * alphaFactor);
                                stringRenderer->renderBackground(context, 2.0f, 1.0f);
                                
                                glSetEdgeOffset(0.01f);
                                glColorV4f(color, alphaFactor);
                                stringRenderer->render(context);
                                glResetEdgeOffset();
                                glPopMatrix();
                            }
                        }
                    }
                }
            };
        }
    }
}

#endif
