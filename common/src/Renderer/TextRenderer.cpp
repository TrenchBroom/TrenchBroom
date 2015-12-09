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
        const float TextRenderer::DefaultMaxViewDistance = 768.0f;
        const float TextRenderer::DefaultMinZoomFactor = 0.5f;
        const Vec2f TextRenderer::DefaultInset = Vec2f(4.0f, 4.0f);
        const size_t TextRenderer::RectCornerSegments = 3;
        const float TextRenderer::RectCornerRadius = 3.0f;
        
        TextRenderer::Entry::Entry(Vec2f::List& i_vertices, const Vec2f& i_size, const Vec3f& i_offset, const Color& i_textColor, const Color& i_backgroundColor) :
        size(i_size),
        offset(i_offset),
        textColor(i_textColor),
        backgroundColor(i_backgroundColor) {
            using std::swap;
            swap(vertices, i_vertices);
        }

        TextRenderer::EntryCollection::EntryCollection() :
        textVertexCount(0),
        rectVertexCount(0) {}
        
        TextRenderer::TextRenderer(const FontDescriptor& fontDescriptor, const float maxViewDistance, const float minZoomFactor, const Vec2f& inset) :
        m_fontDescriptor(fontDescriptor),
        m_maxViewDistance(maxViewDistance),
        m_minZoomFactor(minZoomFactor),
        m_inset(inset) {}

        void TextRenderer::renderString(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position) {
            renderString(renderContext, textColor, backgroundColor, string, position, false);
        }
        
        void TextRenderer::renderStringOnTop(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position) {
            renderString(renderContext, textColor, backgroundColor, string, position, true);
        }

        void TextRenderer::renderString(RenderContext& renderContext, const Color& textColor, const Color& backgroundColor, const AttrString& string, const TextAnchor& position, const bool onTop) {
            
            const Camera& camera = renderContext.camera();
            const float distance = camera.perpendicularDistanceTo(position.position());
            if (distance <= 0.0f)
                return;
            
            if (!isVisible(renderContext, string, position, distance, onTop))
                return;
            
            FontManager& fontManager = renderContext.fontManager();
            TextureFont& font = fontManager.font(m_fontDescriptor);

            Vec2f::List vertices = font.quads(string, true);
            const float alphaFactor = computeAlphaFactor(renderContext, distance, onTop);
            const Vec2f size = font.measure(string);
            const Vec3f offset = position.offset(camera, size);
            
            if (onTop)
                addEntry(m_entriesOnTop, Entry(vertices, size, offset,
                                               Color(textColor, alphaFactor * textColor.a()),
                                               Color(backgroundColor, alphaFactor * backgroundColor.a())));
            else
                addEntry(m_entries, Entry(vertices, size, offset,
                                          Color(textColor, alphaFactor * textColor.a()),
                                          Color(backgroundColor, alphaFactor * backgroundColor.a())));
        }

        bool TextRenderer::isVisible(RenderContext& renderContext, const AttrString& string, const TextAnchor& position, const float distance, const bool onTop) const {
            if (!onTop) {
                if (renderContext.render3D() && distance > m_maxViewDistance)
                    return false;
                if (renderContext.render2D() && renderContext.camera().zoom() < m_minZoomFactor)
                    return false;
            }
            
            const Camera& camera = renderContext.camera();
            const Camera::Viewport& viewport = camera.unzoomedViewport();
            
            const Vec2f size = stringSize(renderContext, string);
            const Vec2f offset = Vec2f(position.offset(camera, size)) - m_inset;
            const Vec2f actualSize = size + 2.0f * m_inset;
            
            return viewport.contains(offset.x(), offset.y(), actualSize.x(), actualSize.y());
        }

        float TextRenderer::computeAlphaFactor(const RenderContext& renderContext, const float distance, const bool onTop) const {
            if (onTop)
                return 1.0f;

            if (renderContext.render3D()) {
                const float a = m_maxViewDistance - distance;
                if (a > 128.0f)
                    return 1.0f;
                return a / 128.0f;
            } else {
                const float z = renderContext.camera().zoom();
                const float d = z - m_minZoomFactor;
                if (d > 0.3f)
                    return 1.0f;
                return d / 0.3f;
            }
        }
        
        void TextRenderer::addEntry(EntryCollection& collection, const Entry& entry) {
            collection.entries.push_back(entry);
            collection.textVertexCount += entry.vertices.size();
            collection.rectVertexCount += roundedRect2DVertexCount(RectCornerSegments);
        }
        
        Vec2f TextRenderer::stringSize(RenderContext& renderContext, const AttrString& string) const {
            FontManager& fontManager = renderContext.fontManager();
            TextureFont& font = fontManager.font(m_fontDescriptor);
            return font.measure(string).rounded();
        }

        void TextRenderer::doPrepareVertices(Vbo& vertexVbo) {
            prepare(m_entries, false, vertexVbo);
            prepare(m_entriesOnTop, true, vertexVbo);
        }
        
        void TextRenderer::prepare(EntryCollection& collection, const bool onTop, Vbo& vbo) {
            TextVertex::List textVertices;
            textVertices.reserve(collection.textVertexCount);
            
            RectVertex::List rectVertices;
            rectVertices.reserve(collection.rectVertexCount);
            
            EntryList::const_iterator it, end;
            for (it = collection.entries.begin(), end = collection.entries.end(); it != end; ++it) {
                const Entry& entry = *it;
                addEntry(entry, onTop, textVertices, rectVertices);
            }
            
            collection.textArray = VertexArray::swap(textVertices);
            collection.rectArray = VertexArray::swap(rectVertices);
            
            collection.textArray.prepare(vbo);
            collection.rectArray.prepare(vbo);
        }

        void TextRenderer::addEntry(const Entry& entry, const bool onTop, TextVertex::List& textVertices, RectVertex::List& rectVertices) {
            const Vec2f::List& stringVertices = entry.vertices;
            const Vec2f& stringSize = entry.size;
            
            const Vec3f& offset = entry.offset;
            
            const Color& textColor = entry.textColor;
            const Color& rectColor = entry.backgroundColor;
            
            for (size_t i = 0; i < stringVertices.size() / 2; ++i) {
                const Vec2f& position2 = stringVertices[2 * i];
                const Vec2f& texCoords = stringVertices[2 * i + 1];
                textVertices.push_back(TextVertex(Vec3f(position2 + offset, -offset.z()), texCoords, textColor));
            }

            const Vec2f::List rect = roundedRect2D(stringSize + 2.0f * m_inset, RectCornerRadius, RectCornerSegments);
            for (size_t i = 0; i < rect.size(); ++i) {
                const Vec2f& vertex = rect[i];
                rectVertices.push_back(RectVertex(Vec3f(vertex + offset + stringSize / 2.0f, -offset.z()), rectColor));
            }
        }

        void TextRenderer::doRender(RenderContext& renderContext) {
            const Camera::Viewport& viewport = renderContext.camera().unzoomedViewport();
            const Mat4x4f projection = orthoMatrix(0.0f, 1.0f,
                                                   static_cast<float>(viewport.x),
                                                   static_cast<float>(viewport.height),
                                                   static_cast<float>(viewport.width),
                                                   static_cast<float>(viewport.y));
            const Mat4x4f view = viewMatrix(Vec3f::NegZ, Vec3f::PosY);
            ReplaceTransformation ortho(renderContext.transformation(), projection, view);
            
            render(m_entries, renderContext);
            
            glAssert(glDisable(GL_DEPTH_TEST));
            render(m_entriesOnTop, renderContext);
            glAssert(glEnable(GL_DEPTH_TEST));
        }

        void TextRenderer::render(EntryCollection& collection, RenderContext& renderContext) {
            FontManager& fontManager = renderContext.fontManager();
            TextureFont& font = fontManager.font(m_fontDescriptor);
            
            glAssert(glDisable(GL_TEXTURE_2D));
            
            ActiveShader backgroundShader(renderContext.shaderManager(), Shaders::TextBackgroundShader);
            collection.rectArray.render(GL_TRIANGLES);
            
            glAssert(glEnable(GL_TEXTURE_2D));
            
            ActiveShader textShader(renderContext.shaderManager(), Shaders::ColoredTextShader);
            textShader.set("Texture", 0);
            font.activate();
            collection.textArray.render(GL_QUADS);
            font.deactivate();
        }
    }
}
