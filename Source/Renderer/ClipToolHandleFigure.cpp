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

#include "ClipToolHandleFigure.h"

#include "Controller/ClipHandle.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SphereFigure.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"

namespace TrenchBroom {
    namespace Renderer {
        ClipToolHandleFigure::ClipToolHandleFigure(Controller::ClipHandle& handle) :
        m_handle(handle) {}
        
        void ClipToolHandleFigure::render(Vbo& vbo, RenderContext& context) {
            bool wasHandleHit = false;
            if (m_handle.numPoints() > 0) {
                
            }
            
            if (m_handle.numPoints() < 3 && m_handle.hasCurrentHit() && !wasHandleHit) {
                const Vec3f& point = m_handle.currentPoint();
                ApplyMatrix applyTranslation(context.transformation(), Mat4f().translate(point));
                
                ActivateShader shader(context.shaderManager(), Shaders::HandleShader);
                shader.currentShader().setUniformVariable("Color", Color(0.0f, 1.0f, 0.0f, 1.0f));
                SphereFigure(m_handle.handleRadius()).render(vbo, context);
            }
        }
    }
}