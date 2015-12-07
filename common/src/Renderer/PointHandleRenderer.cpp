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

#include "PointHandleRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexSpec.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        PointHandleRenderer::PointHandleRenderer() :
        m_handle(pref(Preferences::HandleRadius), 16, true),
        m_highlight(2.0f * pref(Preferences::HandleRadius), 16, false) {}
        
        void PointHandleRenderer::addPoint(const Color& color, const Vec3f& position) {
            m_pointHandles[color].push_back(position);
        }

        void PointHandleRenderer::addHighlight(const Color& color, const Vec3f& position) {
            m_highlights[color].push_back(position);
        }
        
        void PointHandleRenderer::doPrepareVertices(Vbo& vertexVbo) {
            m_handle.prepare(vertexVbo);
            m_highlight.prepare(vertexVbo);
        }
        
        void PointHandleRenderer::doRender(RenderContext& renderContext) {
            const Camera& camera = renderContext.camera();
            const Camera::Viewport& viewport = camera.unzoomedViewport();
            
            const Mat4x4f projection = orthoMatrix(-1.0f, 1.0f,
                                                   static_cast<float>(viewport.x),
                                                   static_cast<float>(viewport.height),
                                                   static_cast<float>(viewport.width),
                                                   static_cast<float>(viewport.y));
            const Mat4x4f view = viewMatrix(Vec3f::NegZ, Vec3f::PosY);
            ReplaceTransformation ortho(renderContext.transformation(), projection, view);

            glAssert(glDisable(GL_DEPTH_TEST));
            renderHandles(renderContext, m_pointHandles, m_handle);
            renderHandles(renderContext, m_highlights, m_highlight);
            glAssert(glEnable(GL_DEPTH_TEST));
            
            clear();
        }

        void PointHandleRenderer::renderHandles(RenderContext& renderContext, const HandleMap& map, Circle& circle) {
            const Camera& camera = renderContext.camera();
            ActiveShader shader(renderContext.shaderManager(), Shaders::HandleShader);

            HandleMap::const_iterator hIt, hEnd;
            Vec3f::List::const_iterator pIt, pEnd;
            
            for (hIt = map.begin(), hEnd = map.end(); hIt != hEnd; ++hIt) {
                const Color& color = hIt->first;
                shader.set("Color", color);
                
                const Vec3f::List& positions = hIt->second;
                for (pIt = positions.begin(), pEnd = positions.end(); pIt != pEnd; ++pIt) {
                    const Vec3f& position = *pIt;
                    const Vec3f offset = camera.project(position);
                    MultiplyModelMatrix translate(renderContext.transformation(), translationMatrix(offset));
                    circle.render();
                }
            }
        }

        void PointHandleRenderer::clear() {
            m_pointHandles.clear();
            m_highlights.clear();
        }
    }
}
