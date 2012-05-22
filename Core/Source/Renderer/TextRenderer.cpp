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
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        void TextRenderer::addString(int key, const TextEntry& entry) {
            removeString(key);
            m_entries[key] = entry;
        }

        void TextRenderer::renderTextBackground(float x, float y, float width, float height, float hPadding, float vPadding) {
            glBegin(GL_QUADS);
            glVertex3f(x - hPadding, y - vPadding, 0);
            glVertex3f(x + width + hPadding, y - vPadding, 0);
            glVertex3f(x + width + hPadding, y + height + vPadding, 0);
            glVertex3f(x - hPadding, y + height + vPadding, 0);
            glEnd();
        }

        TextRenderer::TextRenderer(FontManager& fontManager, float fadeDistance) : m_fontManager(fontManager), m_fadeDistance(fadeDistance) {}
        
        TextRenderer::~TextRenderer() {
            clear();
        }

        void TextRenderer::addString(int key, const string& str, const FontDescriptor& descriptor, AnchorPtr anchor) {
            FontPtr font = m_fontManager.font(descriptor);
            FTBBox bounds = font->BBox(str.c_str());
            float x = bounds.Lower().Xf();
            float y = bounds.Lower().Yf();
            float width = bounds.Upper().Xf() - bounds.Lower().Xf();
            float height = bounds.Upper().Yf() - bounds.Lower().Yf();
            addString(key, TextEntry(str, font, descriptor, anchor, x, y, width, height));
        }
        
        void TextRenderer::removeString(int key) {
            TextMap::iterator textIt = m_entries.find(key);
            if (textIt != m_entries.end())
                m_entries.erase(textIt);
        }
        
        void TextRenderer::transferString(int key, TextRenderer& destination) {
            TextMap::iterator textIt = m_entries.find(key);
            if (textIt != m_entries.end()) {
                TextEntry& entry = textIt->second;
                destination.addString(key, entry.text, entry.descriptor, entry.anchor);
                m_entries.erase(textIt);
            }
        }
        
        void TextRenderer::clear() {
            m_entries.clear();
        }
        
        void TextRenderer::setFadeDistance(float fadeDistance) {
            m_fadeDistance = fadeDistance;
        }

        void TextRenderer::render(RenderContext& context, const Vec4f& color) {
            glPushAttrib(GL_TEXTURE_BIT);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
            
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
            
            glFrontFace(GL_CCW);
            glPolygonMode(GL_FRONT, GL_FILL);
            float cutoff = (m_fadeDistance + 100) * (m_fadeDistance + 100);

            TextMap::iterator textIt;
            for (textIt = m_entries.begin(); textIt != m_entries.end(); ++textIt) {
                TextEntry& entry = textIt->second;
                const Vec3f& position = entry.anchor->position();
                
                float dist2 = context.camera.squaredDistanceTo(position);
                if (dist2 <= cutoff) {
                    float dist = sqrt(dist2);
                    float factor = dist / 300;
                    
                    glPushMatrix();
                    glTranslatef(position.x, position.y, position.z);
                    context.camera.setBillboard();
                    glScalef(factor, factor, 0);
                    glTranslatef(-entry.width / 2, 0, 0);
                    
                    float alphaFactor = 1 - Math::fmax((dist - m_fadeDistance), 0) / 100;

                    /*
                    glColor4f(0, 0, 0, 0.6f * alphaFactor);
                    renderTextBackground(entry.x, entry.y, entry.width, entry.height, 2, 1);
                     */
                    
                    glSetEdgeOffset(0.5f);
                    glColorV4f(color, alphaFactor);
                    entry.font->Render(entry.text.c_str());
                    glResetEdgeOffset();
                    glPopMatrix();
                }
            }
            glPopAttrib();
            glFrontFace(GL_CW);
        }
    }
}