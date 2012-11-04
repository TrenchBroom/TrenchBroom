/*
 Copyright (C) 2010-208 Kristian Duske
 
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

#include "RotateObjectsHandleFigure.h"

#include "Controller/RotateObjectsHandle.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/AxisFigure.h"
#include "Renderer/Camera.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RingFigure.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/VertexArray.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        void RotateObjectsHandleFigure::renderAxis(Vbo& vbo, RenderContext& context) {
            ActivateShader shader(context.shaderManager(), Shaders::ColoredHandleShader);
            AxisFigure axisFigure(m_axisLength);
            if (m_handle.hitArea() == Model::RotateObjectsHandleHit::HAXAxis) {
                axisFigure.setAxes(Axis::AX);
                axisFigure.setXColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            } else if (m_handle.hitArea() == Model::RotateObjectsHandleHit::HAYAxis) {
                axisFigure.setAxes(Axis::AY);
                axisFigure.setYColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            } else {
                axisFigure.setAxes(Axis::AZ);
                axisFigure.setZColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            }
            axisFigure.render(vbo, context);
        }
        
        void RotateObjectsHandleFigure::renderRing(Vbo& vbo, RenderContext& context) {
            Vec3f xAxis, yAxis, zAxis;
            m_handle.axes(context.camera().position(), xAxis, yAxis, zAxis);
            
            ActivateShader shader(context.shaderManager(), Shaders::HandleShader);
            shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.6f));
            
            Mat4f rotation;
            if (m_handle.hitArea() == Model::RotateObjectsHandleHit::HAXAxis) {
                rotation.rotate(m_handle.angle(), Vec3f::PosX);
                ApplyMatrix applyRotation(context.transformation(), rotation);
                
                RingFigure(Axis::AX, yAxis, zAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8).render(vbo, context);
                CircleFigure(Axis::AX, 0.0f, 2 * Math::Pi, m_handle.handleRadius() + m_handle.handleThickness(), 32, false).render(vbo, context);
            } else if (m_handle.hitArea() == Model::RotateObjectsHandleHit::HAYAxis) {
                rotation.rotate(m_handle.angle(), Vec3f::PosY);
                ApplyMatrix applyRotation(context.transformation(), rotation);

                RingFigure(Axis::AY, xAxis, zAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8).render(vbo, context);
                CircleFigure(Axis::AY, 0.0f, 2 * Math::Pi, m_handle.handleRadius() + m_handle.handleThickness(), 32, false).render(vbo, context);
            } else {
                rotation.rotate(m_handle.angle(), Vec3f::PosZ);
                ApplyMatrix applyRotation(context.transformation(), rotation);
                
                RingFigure(Axis::AZ, xAxis, yAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8).render(vbo, context);
                CircleFigure(Axis::AZ, 0.0f, 2 * Math::Pi, m_handle.handleRadius() + m_handle.handleThickness(), 32, false).render(vbo, context);
            }
        }
        
        RotateObjectsHandleFigure::RotateObjectsHandleFigure(Controller::RotateObjectsHandle& handle, float axisLength) :
        m_handle(handle),
        m_axisLength(axisLength) {}
        
        void RotateObjectsHandleFigure::render(Vbo& vbo, RenderContext& context) {
            Mat4f translation;
            translation.translate(m_handle.position());
            ApplyMatrix applyTranslation(context.transformation(), translation);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            SetVboState activateVbo(vbo, Vbo::VboActive);
            if (m_handle.hit()) {
                renderAxis(vbo, context);
                renderRing(vbo, context);
            } else {
                Vec3f xAxis, yAxis, zAxis;
                m_handle.axes(context.camera().position(), xAxis, yAxis, zAxis);

                ActivateShader shader(context.shaderManager(), Shaders::HandleShader);
                shader.currentShader().setUniformVariable("Color", Color(1.0f, 1.0f, 1.0f, 0.6f));

                RingFigure(Axis::AX, yAxis, zAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8).render(vbo, context);
                RingFigure(Axis::AY, xAxis, zAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8).render(vbo, context);
                RingFigure(Axis::AZ, xAxis, yAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8).render(vbo, context);
            }
            
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }
    }
}