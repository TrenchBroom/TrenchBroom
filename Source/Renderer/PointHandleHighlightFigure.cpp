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

#include "PointHandleHighlightFigure.h"

#include "Renderer/ApplyMatrix.h"
#include "Renderer/Camera.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        PointHandleHighlightFigure::PointHandleHighlightFigure(const Vec3f& position, const Color& color, float radius, float scalingFactor) :
        m_position(position),
        m_color(color),
        m_radius(radius),
        m_scalingFactor(scalingFactor) {
            assert(radius > 0.0f);
            assert(scalingFactor > 0.0f);
        }
        
        void PointHandleHighlightFigure::render(Vbo& vbo, RenderContext& context) {
            float factor = context.camera().distanceTo(m_position) * m_scalingFactor;
            
            Mat4f billboardMatrix = context.camera().billboardMatrix();
            Mat4f matrix = Mat4f::Identity;
            matrix.translate(m_position);
            matrix *= billboardMatrix;
            matrix.scale(Vec3f(factor, factor, 0.0f));
            ApplyMatrix applyBillboard(context.transformation(), matrix);

            ActivateShader shader(context.shaderManager(), Shaders::HandleShader);
            shader.currentShader().setUniformVariable("Color", m_color);
            
            CircleFigure circle(Axis::AZ, 0.0f, 2.0f * Math::Pi, 2.0f * m_radius, 16, false);
            circle.render(vbo, context);
        }
    }
}
