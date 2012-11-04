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

#include "ClipHandleFigure.h"

#include "Controller/ClipHandle.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SphereFigure.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"

namespace TrenchBroom {
    namespace Renderer {
        ClipHandleFigure::ClipHandleFigure(Controller::ClipHandle& handle) :
        m_handle(handle) {}
        
        void ClipHandleFigure::render(Vbo& vbo, RenderContext& context) {
            ActivateShader shader(context.shaderManager(), Shaders::HandleShader);

            bool wasHandleHit = false;
            const Vec3f& currentPoint = m_handle.currentPoint();
            for (unsigned int i = 0; i < m_handle.numPoints(); i++) {
                const Vec3f& point = m_handle.point(i);
                ApplyMatrix applyTranslation(context.transformation(), Mat4f().translate(point));
                
                if (point.equals(currentPoint)) {
                    shader.currentShader().setUniformVariable("Color", Color(1.0f, 0.0f, 0.0f, 1.0f));
                    wasHandleHit = true;
                } else {
                    shader.currentShader().setUniformVariable("Color", Color(0.0f, 1.0f, 0.0f, 1.0f));
                }
                SphereFigure(m_handle.handleRadius(), 1).render(vbo, context);
            }
            
            if (m_handle.numPoints() < 3 && m_handle.hasCurrentHit() && !wasHandleHit) {
                ApplyMatrix applyTranslation(context.transformation(), Mat4f().translate(currentPoint));
                
                shader.currentShader().setUniformVariable("Color", Color(0.0f, 1.0f, 0.0f, 1.0f));
                SphereFigure(m_handle.handleRadius(), 1).render(vbo, context);
            }
        }
    }
}