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
#include "Renderer/Camera.h"
#include "Renderer/CircleFigure.h"
#include "Renderer/PushMatrix.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexArray.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        MoveObjectsHandleFigure::MoveObjectsHandleFigure(Controller::MoveObjectsHandle& handle) :
        m_handle(handle) {}
        
        void MoveObjectsHandleFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState mapVbo(vbo, Vbo::VboMapped);
            
            PushMatrix pushMatrix(context.transformation());
            Mat4f matrix = pushMatrix.matrix();
            
            matrix.translate(m_handle.position());
            pushMatrix.load(matrix);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            VertexArray axisArray(vbo, GL_LINES, 6,
                                  VertexAttribute::position3f(),
                                  VertexAttribute::color4f());
            
            Color color;
            if (m_handle.hit() && (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXAxis ||
                                   m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXYPlane ||
                                   m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXZPlane))
                color = Color(1.0f, 1.0f, 1.0f, 1.0f);
            else
                color = Color(1.0f, 0.0f, 0.0f, 1.0f);
            
            axisArray.addAttribute(Vec3f(-m_handle.axisLength(), 0.0f, 0.0f));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(m_handle.axisLength(), 0.0f, 0.0f));
            axisArray.addAttribute(color);
            
            if (m_handle.hit() && (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAYAxis ||
                                   m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXYPlane ||
                                   m_handle.hitArea()== Model::MoveObjectsHandleHit::HAYZPlane))
                color = Color(1.0f, 1.0f, 1.0f, 1.0f);
            else
                color = Color(0.0f, 1.0f, 0.0f, 1.0f);
            
            axisArray.addAttribute(Vec3f(0.0f, -m_handle.axisLength(), 0.0f));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(0.0f, +m_handle.axisLength(), 0.0f));
            axisArray.addAttribute(color);
            
            if (m_handle.hit() && (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAZAxis ||
                                   m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXZPlane ||
                                   m_handle.hitArea()== Model::MoveObjectsHandleHit::HAYZPlane))
                color = Color(1.0f, 1.0f, 1.0f, 1.0f);
            else
                color = Color(0.0f, 0.0f, 1.0f, 1.0f);
            
            axisArray.addAttribute(Vec3f(0.0f, 0.0f, -m_handle.axisLength()));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(0.0f, 0.0f, +m_handle.axisLength()));
            axisArray.addAttribute(color);
            
            Vec3f xAxis, yAxis, zAxis;
            m_handle.axes(context.camera().position(), xAxis, yAxis, zAxis);
            
            if (m_handle.hit()) {
                if (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXYPlane)
                    CircleFigure(Axis::AZ, xAxis, yAxis, m_handle.planeRadius(), 8, Color(1.0f, 1.0f, 1.0f, 0.25f), true).render(vbo, context);
                if (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAXZPlane)
                    CircleFigure(Axis::AY, xAxis, zAxis, m_handle.planeRadius(), 8, Color(1.0f, 1.0f, 1.0f, 0.25f), true).render(vbo, context);
                if (m_handle.hitArea()== Model::MoveObjectsHandleHit::HAYZPlane)
                    CircleFigure(Axis::AX, yAxis, zAxis, m_handle.planeRadius(), 8, Color(1.0f, 1.0f, 1.0f, 0.25f), true).render(vbo, context);
            }
            
            CircleFigure(Axis::AZ, xAxis, yAxis, m_handle.planeRadius(), 8, Color(1.0f, 1.0f, 1.0f, 0.6f), false).render(vbo, context);
            CircleFigure(Axis::AY, xAxis, zAxis, m_handle.planeRadius(), 8, Color(1.0f, 1.0f, 1.0f, 0.6f), false).render(vbo, context);
            CircleFigure(Axis::AX, yAxis, zAxis, m_handle.planeRadius(), 8, Color(1.0f, 1.0f, 1.0f, 0.6f), false).render(vbo, context);
            
            SetVboState activateVbo(vbo, Vbo::VboActive);
            axisArray.render();
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }
    }
}