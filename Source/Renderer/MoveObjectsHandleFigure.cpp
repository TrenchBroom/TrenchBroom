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

namespace TrenchBroom {
    namespace Renderer {
        MoveObjectsHandleFigure::MoveObjectsHandleFigure(Controller::MoveObjectsHandle& handle) :
        m_handle(handle),
        m_hit(NULL) {}

        void MoveObjectsHandleFigure::render(Vbo& vbo, RenderContext& context) {
            SetVboState mapVbo(vbo, Vbo::VboMapped);
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            VertexArray axisArray(vbo, GL_LINES, 6,
                                  VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                  VertexAttribute(4, GL_FLOAT, VertexAttribute::Color));

            Color color;
            if (m_hit != NULL && (m_hit->hitArea() == Model::MoveObjectsHandleHit::HAXAxis ||
                                  m_hit->hitArea() == Model::MoveObjectsHandleHit::HAXYPlane ||
                                  m_hit->hitArea() == Model::MoveObjectsHandleHit::HAXZPlane))
                color = Color(1.0f, 1.0f, 1.0f, 1.0f);
            else
                color = Color(1.0f, 0.0f, 0.0f, 1.0f);
            
            axisArray.addAttribute(Vec3f(m_handle.position().x - m_handle.axisLength(), m_handle.position().y, m_handle.position().z));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(m_handle.position().x + m_handle.axisLength(), m_handle.position().y, m_handle.position().z));
            axisArray.addAttribute(color);
            
            if (m_hit != NULL && (m_hit->hitArea() == Model::MoveObjectsHandleHit::HAYAxis ||
                                  m_hit->hitArea() == Model::MoveObjectsHandleHit::HAXYPlane ||
                                  m_hit->hitArea() == Model::MoveObjectsHandleHit::HAYZPlane))
                color = Color(1.0f, 1.0f, 1.0f, 1.0f);
            else
                color = Color(0.0f, 1.0f, 0.0f, 1.0f);

            axisArray.addAttribute(Vec3f(m_handle.position().x, m_handle.position().y - m_handle.axisLength(), m_handle.position().z));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(m_handle.position().x, m_handle.position().y + m_handle.axisLength(), m_handle.position().z));
            axisArray.addAttribute(color);
            
            if (m_hit != NULL && (m_hit->hitArea() == Model::MoveObjectsHandleHit::HAZAxis ||
                                  m_hit->hitArea() == Model::MoveObjectsHandleHit::HAXZPlane ||
                                  m_hit->hitArea() == Model::MoveObjectsHandleHit::HAYZPlane))
                color = Color(1.0f, 1.0f, 1.0f, 1.0f);
            else
                color = Color(0.0f, 0.0f, 1.0f, 1.0f);

            axisArray.addAttribute(Vec3f(m_handle.position().x, m_handle.position().y, m_handle.position().z - m_handle.axisLength()));
            axisArray.addAttribute(color);
            axisArray.addAttribute(Vec3f(m_handle.position().x, m_handle.position().y, m_handle.position().z + m_handle.axisLength()));
            axisArray.addAttribute(color);

            Vec3f xAxis, yAxis, zAxis;
            m_handle.axes(context.camera().position(), xAxis, yAxis, zAxis);

            if (m_hit != NULL && (m_hit->hitArea() == Model::MoveObjectsHandleHit::HAXYPlane ||
                                  m_hit->hitArea() == Model::MoveObjectsHandleHit::HAXZPlane ||
                                  m_hit->hitArea() == Model::MoveObjectsHandleHit::HAYZPlane)) {
                PushMatrix pushMatrix(context.transformation());
                Mat4f matrix = pushMatrix.matrix();
                matrix.translate(m_handle.position());
                
                if (m_hit->hitArea() == Model::MoveObjectsHandleHit::HAXZPlane)
                    matrix.rotate(Math::Pi / 2.0f, Vec3f::PosX);
                else if (m_hit->hitArea() == Model::MoveObjectsHandleHit::HAYZPlane)
                    matrix.rotate(Math::Pi / 2.0f, Vec3f::PosY);
                pushMatrix.load(matrix);

                CircleFigure filledCircle(m_handle.planeRadius(), 24, Color(1.0f, 1.0f, 1.0f, 0.25f), true);
                filledCircle.render(vbo, context);
            }
            
            {
                CircleFigure outlinedCircle(m_handle.planeRadius(), 24, Color(1.0f, 1.0f, 1.0f, 0.6f), false);
                PushMatrix pushMatrix(context.transformation());
                Mat4f matrix = pushMatrix.matrix();
                
                matrix.translate(m_handle.position());
                pushMatrix.load(matrix);
                outlinedCircle.render(vbo, context);
                
                matrix.rotate(Math::Pi / 2.0f, Vec3f::PosX);
                pushMatrix.load(matrix);
                outlinedCircle.render(vbo, context);
                
                matrix.rotate(Math::Pi / 2.0f, Vec3f::PosY);
                pushMatrix.load(matrix);
                outlinedCircle.render(vbo, context);
            }
            
            SetVboState activateVbo(vbo, Vbo::VboActive);
            axisArray.render();
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
        }
    }
}