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
#include "Renderer/Camera.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/PushMatrix.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RingFigure.h"
#include "Renderer/VertexArray.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        RotateObjectsHandleFigure::RotateObjectsHandleFigure(Controller::RotateObjectsHandle& handle, float axisLength) :
        m_handle(handle),
        m_axisLength(axisLength) {}

        void RotateObjectsHandleFigure::render(Vbo& vbo, RenderContext& context) {
            PushMatrix pushMatrix(context.transformation());
            Mat4f matrix = pushMatrix.matrix();
            
            matrix.translate(m_handle.position());
            pushMatrix.load(matrix);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            Vec3f xAxis, yAxis, zAxis;
            m_handle.axes(context.camera().position(), xAxis, yAxis, zAxis);

            SetVboState activateVbo(vbo, Vbo::VboActive);
            if (m_handle.hit()) {
                VertexArray axisArray(vbo, GL_LINES, 2,
                                      VertexAttribute::position3f(),
                                      VertexAttribute::color4f());
                if (m_handle.hitArea() == Model::RotateObjectsHandleHit::HAXAxis) {
                    RingFigure(Axis::AX, yAxis, zAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8, Color(1.0f, 1.0f, 1.0f, 0.6f)).render(vbo, context);
                    CircleFigure(Axis::AX, 0.0f, 2 * Math::Pi, m_handle.handleRadius() + m_handle.handleThickness(), 32, Color(1.0f, 1.0f, 1.0f, 1.0f), false).render(vbo, context);
                    SetVboState mapVbo(vbo, Vbo::VboMapped);
                    axisArray.addAttribute(Vec3f(-m_axisLength, 0.0f, 0.0f));
                    axisArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 1.0f));
                    axisArray.addAttribute(Vec3f(+m_axisLength, 0.0f, 0.0f));
                    axisArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 1.0f));
                } else if (m_handle.hitArea() == Model::RotateObjectsHandleHit::HAYAxis) {
                    RingFigure(Axis::AY, xAxis, zAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8, Color(1.0f, 1.0f, 1.0f, 0.6f)).render(vbo, context);
                    CircleFigure(Axis::AY, 0.0f, 2 * Math::Pi, m_handle.handleRadius() + m_handle.handleThickness(), 32, Color(1.0f, 1.0f, 1.0f, 1.0f), false).render(vbo, context);
                    SetVboState mapVbo(vbo, Vbo::VboMapped);
                    axisArray.addAttribute(Vec3f(0.0f, -m_axisLength, 0.0f));
                    axisArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 1.0f));
                    axisArray.addAttribute(Vec3f(0.0f, +m_axisLength, 0.0f));
                    axisArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 1.0f));
                } else {
                    RingFigure(Axis::AZ, xAxis, yAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8, Color(1.0f, 1.0f, 1.0f, 0.6f)).render(vbo, context);
                    CircleFigure(Axis::AZ, 0.0f, 2 * Math::Pi, m_handle.handleRadius() + m_handle.handleThickness(), 32, Color(1.0f, 1.0f, 1.0f, 1.0f), false).render(vbo, context);
                    SetVboState mapVbo(vbo, Vbo::VboMapped);
                    axisArray.addAttribute(Vec3f(0.0f, 0.0f, -m_axisLength));
                    axisArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 1.0f));
                    axisArray.addAttribute(Vec3f(0.0f, 0.0f, +m_axisLength));
                    axisArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 1.0f));
                }
                
                SetVboState activateVbo(vbo, Vbo::VboActive);
                axisArray.render();
            } else {
                RingFigure(Axis::AX, yAxis, zAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8, Color(1.0f, 1.0f, 1.0f, 0.6f)).render(vbo, context);
                RingFigure(Axis::AY, xAxis, zAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8, Color(1.0f, 1.0f, 1.0f, 0.6f)).render(vbo, context);
                RingFigure(Axis::AZ, xAxis, yAxis, m_handle.handleRadius(), m_handle.handleThickness(), 8, Color(1.0f, 1.0f, 1.0f, 0.6f)).render(vbo, context);
            }
            
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);

        }
    }
}