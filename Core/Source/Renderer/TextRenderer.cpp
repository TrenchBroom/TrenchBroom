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

#include "TextRenderer.h"
#include "Controller/Camera.h"
#include "Renderer/FontManager.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        void TextRenderer::addString(int key, StringRenderer& stringRenderer, Anchor& anchor) {
            removeString(key);
            m_entries[key] = TextEntry(&stringRenderer, &anchor);
        }

        TextRenderer::TextRenderer(FontManager& fontManager, float fadeDistance) : m_fontManager(fontManager), m_fadeDistance(fadeDistance) {}
        
        TextRenderer::~TextRenderer() {
            clear();
        }

        void TextRenderer::addString(int key, const string& str, const FontDescriptor& descriptor, Anchor& anchor) {
            StringRenderer& stringRenderer = m_fontManager.createStringRenderer(descriptor, str);
            addString(key, stringRenderer, anchor);
        }
        
        void TextRenderer::removeString(int key) {
            TextMap::iterator textIt = m_entries.find(key);
            if (textIt != m_entries.end()) {
                m_fontManager.destroyStringRenderer(*textIt->second.first);
                delete textIt->second.second; // delete anchor
                m_entries.erase(textIt);
            }
        }
        
        void TextRenderer::transferString(int key, TextRenderer& destination) {
            TextMap::iterator textIt = m_entries.find(key);
            if (textIt != m_entries.end()) {
                destination.addString(key, *textIt->second.first, *textIt->second.second);
                m_entries.erase(textIt);
            }
        }
        
        void TextRenderer::clear() {
            TextMap::iterator textIt;
            for (textIt = m_entries.begin(); textIt != m_entries.end(); ++textIt) {
                m_fontManager.destroyStringRenderer(*textIt->second.first);
                delete textIt->second.second; // delete anchor
            }
            m_entries.clear();
        }
        
        void TextRenderer::setFadeDistance(float fadeDistance) {
            m_fadeDistance = fadeDistance;
        }

        void TextRenderer::render(RenderContext& context, const Vec4f& color) {
            glDisable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT, GL_FILL);
            float cutoff = (m_fadeDistance + 100) * (m_fadeDistance + 100);

            TextMap::iterator textIt;
            for (textIt = m_entries.begin(); textIt != m_entries.end(); ++textIt) {
                StringRenderer* renderer = textIt->second.first;
                Anchor* anchor = textIt->second.second;
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
                    
                    float alphaFactor = 1 - Math::fmax((dist - m_fadeDistance), 0) / 100;

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
}