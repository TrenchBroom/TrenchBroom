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

#include "MoveObjectsHandleFigure.h"

#include "Controller/MoveObjectsHandle.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/AxisFigure.h"
#include "Renderer/Camera.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/VertexArray.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        void MoveObjectsHandleFigure::renderAxes(Vbo& vbo, RenderContext& context) {
            ActivateShader activateShader(context.shaderManager(), Shaders::ColoredHandleShader);
            AxisFigure axisFigure(m_handle.axisLength());
            if (m_handle.hit() && (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXAxis ||
                                   m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXYPlane ||
                                   m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXZPlane))
                axisFigure.setXColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            if (m_handle.hit() && (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAYAxis ||
                                        m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXYPlane ||
                                        m_handle.hitArea()== Model::MoveObjectsHandleHit::HAYZPlane))
                axisFigure.setYColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            if (m_handle.hit() && (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAZAxis ||
                                        m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXZPlane ||
                                        m_handle.hitArea()== Model::MoveObjectsHandleHit::HAYZPlane))
                axisFigure.setZColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            axisFigure.render(vbo, context);
        }
        
        void MoveObjectsHandleFigure::renderPlanes(Vbo& vbo, RenderContext& context) {
            ActivateShader activateShader(context.shaderManager(), Shaders::EdgeShader);
            
            Vec3f xAxis, yAxis, zAxis;
            m_handle.axes(context.camera().position(), xAxis, yAxis, zAxis);
            
            if (m_handle.hit()) {
                activateShader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.25f));
                if (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXYPlane)
                    CircleFigure(Axis::AZ, xAxis, yAxis, m_handle.planeRadius(), 8, true).render(vbo, context);
                if (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXZPlane)
                    CircleFigure(Axis::AY, xAxis, zAxis, m_handle.planeRadius(), 8, true).render(vbo, context);
                if (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAYZPlane)
                    CircleFigure(Axis::AX, yAxis, zAxis, m_handle.planeRadius(), 8,  true).render(vbo, context);
            }
            
            activateShader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.6f));
            CircleFigure(Axis::AZ, xAxis, yAxis, m_handle.planeRadius(), 8, false).render(vbo, context);
            CircleFigure(Axis::AY, xAxis, zAxis, m_handle.planeRadius(), 8, false).render(vbo, context);
            CircleFigure(Axis::AX, yAxis, zAxis, m_handle.planeRadius(), 8, false).render(vbo, context);
        }

        MoveObjectsHandleFigure::MoveObjectsHandleFigure(Controller::MoveObjectsHandle& handle) :
        m_handle(handle) {}
        
        void MoveObjectsHandleFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState mapVbo(vbo, Vbo::VboMapped);
            
            Mat4f translation;
            translation.translate(m_handle.position());
            ApplyMatrix applyTranslation(context.transformation(), translation);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            renderAxes(vbo, context);
            renderPlanes(vbo, context);
            
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }
    }
}