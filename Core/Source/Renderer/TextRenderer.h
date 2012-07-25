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

#include "Controller/Camera.h"
#include "Renderer/FontManager.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Utilities/VecMath.h"
#include "Utilities/SharedPointer.h"

#include <cassert>
#include <map>
#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        
        class TextAnchor {
        public:
            virtual ~TextAnchor() {}
            virtual const Vec3f& position() = 0;
        };

        typedef std::tr1::shared_ptr<TextAnchor> TextAnchorPtr;

        template <typename Key>
        class TextRenderer {
        public:
            class TextRendererFilter {
            public:
                TextRendererFilter() {}
                virtual ~TextRendererFilter() {}
                virtual bool stringVisible(RenderContext& context, const Key& key) = 0;
            };
        private:
            typedef std::pair<StringRendererPtr, TextAnchorPtr> TextEntry;
            typedef std::map<Key, TextEntry> TextMap;
            
            float m_fadeDistance;
            FontManager& m_fontManager;
            TextMap m_entries;
            
            void addString(Key key, StringRendererPtr stringRenderer, TextAnchorPtr anchor) {
                removeString(key);
                m_entries[key] = TextEntry(stringRenderer, anchor);
            }
        public:
            TextRenderer(FontManager& fontManager, float fadeDistance) : m_fontManager(fontManager), m_fadeDistance(fadeDistance) {}
            
            ~TextRenderer() {
                clear();
            }
            
            void addString(Key key, const std::string& str, const FontDescriptor& descriptor, TextAnchorPtr anchor) {
                StringRendererPtr stringRenderer = m_fontManager.createStringRenderer(descriptor, str);
                addString(key, stringRenderer, anchor);
            }
            
            void removeString(Key key)  {
                typename TextMap::iterator textIt = m_entries.find(key);
                if (textIt != m_entries.end()) {
                    m_fontManager.destroyStringRenderer(textIt->second.first);
                    m_entries.erase(textIt);
                }
            }
            
            void updateString(Key key, const std::string& str) {
                typename TextMap::iterator textIt = m_entries.find(key);
                if (textIt != m_entries.end()) {
                    const FontDescriptor& descriptor = textIt->second.first->fontDescriptor;
                    StringRendererPtr stringRenderer = m_fontManager.createStringRenderer(descriptor, str);
                    m_fontManager.destroyStringRenderer(textIt->second.first);
                    textIt->second.first = stringRenderer;
                }
            }
            
            void transferString(Key key, TextRenderer& destination)  {
                typename TextMap::iterator textIt = m_entries.find(key);
                if (textIt != m_entries.end()) {
                    destination.addString(key, textIt->second.first, textIt->second.second);
                    m_entries.erase(textIt);
                }
            }
            
            void clear()  {
                typename TextMap::iterator textIt;
                for (textIt = m_entries.begin(); textIt != m_entries.end(); ++textIt)
                    m_fontManager.destroyStringRenderer(textIt->second.first);
                m_entries.clear();
            }
            
            void setFadeDistance(float fadeDistance)  {
                m_fadeDistance = fadeDistance;
            }
            
            void render(RenderContext& context, TextRendererFilter& filter, const Vec4f& color)  {
                glDisable(GL_TEXTURE_2D);
                glPolygonMode(GL_FRONT, GL_FILL);
                float cutoff = (m_fadeDistance + 100) * (m_fadeDistance + 100);
                
                typename TextMap::iterator textIt;
                for (textIt = m_entries.begin(); textIt != m_entries.end(); ++textIt) {
                    Key key = textIt->first;
                    if (filter.stringVisible(context, key)) {
                        StringRendererPtr renderer = textIt->second.first;
                        TextAnchorPtr anchor = textIt->second.second;
                        const Vec3f& position = anchor->position();
                        
                        float dist2 = context.camera.squaredDistanceTo(position);
                        if (dist2 <= cutoff) {
                            float dist = sqrt(dist2);
                            float factor = dist / 300;
                            float width = renderer->width;
                            
                            glPushMatrix();
                            glTranslatef(position.x, position.y, position.z);
                            context.camera.setBillboard();
                            glScalef(factor, factor, 0);
                            glTranslatef(-width / 2, 0, 0);
                            
                            float alphaFactor = 1.0f - (std::max)(dist - m_fadeDistance, 0.0f) / 100.0f;
                            
                            glColor4f(0, 0, 0, 0.6f * alphaFactor);
                            renderer->renderBackground(2, 1);
                            
                            glSetEdgeOffset(0.5f);
                            glColorV4f(color, alphaFactor);
                            renderer->render();
                            glResetEdgeOffset();
                            glPopMatrix();
                        }
                    }
                }
            }
        };
    }
}

#endif
