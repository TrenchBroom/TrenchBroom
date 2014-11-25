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

#include "TextRenderer.h"

#include "CollectionUtils.h"
#include "AttrString.h"
#include "Renderer/Camera.h"
#include "Renderer/FontManager.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/TextAnchor.h"
#include "Renderer/TextureFont.h"

namespace TrenchBroom {
    namespace Renderer {
        const size_t TextRenderer::RectCornerSegments = 3;
        
        TextRenderer::CachedString::CachedString(Vec2f::List& i_vertices, const Vec2f& i_size) :
        size(i_size) {
            using std::swap;
            swap(i_vertices, vertices);
        }

        TextRenderer::Entry::Entry(StringCache::iterator i_string, const Vec3f& i_offset, const Color& i_textColor, const Color& i_backgroundColor) :
        string(i_string),
        offset(i_offset),
        textColor(i_textColor),
        backgroundColor(i_backgroundColor) {}

        TextRenderer::TextRenderer(const FontDescriptor& fontDescriptor, const Vec2f& inset) :
        m_fontDescriptor(fontDescriptor),
        m_inset(inset),
        m_textVertexCount(0),
        m_rectVertexCount(0) {}

        void TextRenderer::renderString(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position) {
            renderString(renderContext, textColor, backgroundColor, string, position, false);
        }
        
        void TextRenderer::renderStringOnTop(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position) {
            renderString(renderContext, textColor, backgroundColor, string, position, true);
        }

        void TextRenderer::renderString(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position, const bool onTop) {
            if (!isVisible(renderContext, string, position))
                return;
            
            const StringCache::iterator cachedString = findOrCreateCachedString(renderContext, string);
            const Vec3f offset = position.offset(renderContext.camera(), cachedString->second.size);
            m_entries.push_back(Entry(cachedString, offset, textColor, backgroundColor));
            
            m_textVertexCount += cachedString->second.vertices.size();
            m_rectVertexCount += roundedRect2DVertexCount(RectCornerSegments);
        }

        bool TextRenderer::isVisible(RenderContext& renderContext, const AttrString& string, const TextAnchor& position) const {
            const Camera& camera = renderContext.camera();
            const Camera::Viewport& viewport = camera.unzoomedViewport();
            
            const Vec2f size = stringSize(renderContext, string);
            const Vec2f offset = Vec2f(position.offset(camera, size)) - m_inset;
            const Vec2f actualSize = size + 2.0f * m_inset;
            
            return viewport.contains(offset.x(), offset.y(), actualSize.x(), actualSize.y());
        }

        Vec2f TextRenderer::stringSize(RenderContext& renderContext, const AttrString& string) const {
            const StringCache::const_iterator it = m_cache.find(string);
            if (it != m_cache.end())
                return it->second.size;
            
            FontManager& fontManager = renderContext.fontManager();
            TextureFont& font = fontManager.font(m_fontDescriptor);
            return font.measure(string);
        }

        TextRenderer::StringCache::iterator TextRenderer::findOrCreateCachedString(RenderContext& renderContext, const AttrString& string) {
            typedef std::pair<bool, StringCache::iterator> InsertPos;
            InsertPos insertPos = MapUtils::findInsertPos(m_cache, string);
            if (insertPos.first)
                return insertPos.second;
            
            FontManager& fontManager = renderContext.fontManager();
            TextureFont& font = fontManager.font(m_fontDescriptor);
            
            Vec2f::List vertices = font.quads(string, true);
            const Vec2f size = font.measure(string);

            return m_cache.insert(insertPos.second, std::make_pair(string, CachedString(vertices, size)));
        }

        void TextRenderer::doPrepare(Vbo& vbo) {
            TextVertex::List textVertices;
            textVertices.reserve(m_textVertexCount);
            
            RectVertex::List rectVertices;
            rectVertices.reserve(m_rectVertexCount);

            EntryList::const_iterator it, end;
            for (it = m_entries.begin(), end = m_entries.end(); it != end; ++it) {
                const Entry& entry = *it;
                addEntry(entry, textVertices, rectVertices);
            }
            
            m_textArray = VertexArray::swap(GL_QUADS, textVertices);
            m_rectArray = VertexArray::swap(GL_TRIANGLES, rectVertices);
            
            m_textArray.prepare(vbo);
            m_rectArray.prepare(vbo);
        }
        
        void TextRenderer::addEntry(const Entry& entry, TextVertex::List textVertices, RectVertex::List rectVertices) {
            const CachedString& string = entry.string->second;
            const Vec2f::List& stringVertices = string.vertices;
            const Vec2f& stringSize = string.size;
            
            const Vec3f& offset = entry.offset;
            const Color& textColor = entry.textColor;
            const Color& rectColor = entry.backgroundColor;
            
            for (size_t j = 0; j < stringVertices.size() / 2; ++j) {
                const Vec2f& position2 = stringVertices[2 * j];
                const Vec2f& texCoords = stringVertices[2 * j + 1];
                const Vec3f position3(position2.x() + offset.x(),
                                      position2.y() + offset.y(),
                                      -offset.z());
                textVertices.push_back(TextVertex(position3, texCoords, textColor));
            }

            const Vec2f::List tempRect = roundedRect2D(stringSize.x() + 2.0f * m_inset.x(), stringSize.y() + 2.0f * m_inset.y(), 3.0f, 3);
            for (size_t j = 0; j < tempRect.size(); ++j) {
                const Vec2f& vertex = tempRect[j];
                const Vec3f position = Vec3f(vertex.x() + offset.x() + stringSize.x() / 2.0f,
                                             vertex.y() + offset.y() + stringSize.y() / 2.0f,
                                             -offset.z());
                rectVertices.push_back(RectVertex(position, rectColor));
            }
        }

        void TextRenderer::doRender(RenderContext& renderContext) {
            FontManager& fontManager = renderContext.fontManager();
            TextureFont& font = fontManager.font(m_fontDescriptor);

            const Camera::Viewport& viewport = renderContext.camera().unzoomedViewport();
            const Mat4x4f projection = orthoMatrix(0.0f, 1.0f,
                                                   static_cast<float>(viewport.x),
                                                   static_cast<float>(viewport.height),
                                                   static_cast<float>(viewport.width),
                                                   static_cast<float>(viewport.y));
            const Mat4x4f view = viewMatrix(Vec3f::NegZ, Vec3f::PosY);
            ReplaceTransformation ortho(renderContext.transformation(), projection, view);
            
            glDisable(GL_TEXTURE_2D);
            
            ActiveShader backgroundShader(renderContext.shaderManager(), Shaders::TextBackgroundShader);
            m_rectArray.render();
            
            glEnable(GL_TEXTURE_2D);
            
            ActiveShader textShader(renderContext.shaderManager(), Shaders::ColoredTextShader);
            textShader.set("Texture", 0);
            font.activate();
            m_textArray.render();
            font.deactivate();
            
            clear();
        }

        void TextRenderer::clear() {
            m_entries.clear();
        }
    }
}
