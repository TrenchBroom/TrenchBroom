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
        
        void PointHandleRenderer::addPoint(const Vec3f& position) {
            m_points.push_back(position);
        }
        
        void PointHandleRenderer::addSelectedPoint(const Vec3f& position) {
            m_selectedPoints.push_back(position);
        }

        void PointHandleRenderer::addHighlight(const Vec3f& position) {
            m_highlights.push_back(position);
        }
        
        void PointHandleRenderer::doPrepare(Vbo& vbo) {
            m_handle.prepare(vbo);
            m_highlight.prepare(vbo);
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

            ActiveShader shader(renderContext.shaderManager(), Shaders::HandleShader);
            
            glDisable(GL_DEPTH_TEST);

            shader.set("Color", pref(Preferences::HandleColor));
            Vec3f::List::const_iterator it, end;
            for (it = m_points.begin(), end = m_points.end(); it != end; ++it) {
                const Vec3f& position = *it;
                const Vec3f offset = camera.project(position);
                MultiplyModelMatrix translate(renderContext.transformation(), translationMatrix(offset));
                
                m_handle.render();
            }
            
            shader.set("Color", pref(Preferences::SelectedHandleColor));
            for (it = m_selectedPoints.begin(), end = m_selectedPoints.end(); it != end; ++it) {
                const Vec3f& position = *it;
                const Vec3f offset = camera.project(position);
                MultiplyModelMatrix translate(renderContext.transformation(), translationMatrix(offset));
                
                m_handle.render();
            }
            
            shader.set("Color", pref(Preferences::SelectedHandleColor));
            for (it = m_highlights.begin(), end = m_highlights.end(); it != end; ++it) {
                const Vec3f& position = *it;
                const Vec3f offset(camera.project(position), 0.0f);
                MultiplyModelMatrix translate(renderContext.transformation(), translationMatrix(offset));
                m_highlight.render();
            }
            glEnable(GL_DEPTH_TEST);
            
            clear();
        }

        void PointHandleRenderer::clear() {
            m_points.clear();
            m_selectedPoints.clear();
            m_highlights.clear();
        }
    }
}
