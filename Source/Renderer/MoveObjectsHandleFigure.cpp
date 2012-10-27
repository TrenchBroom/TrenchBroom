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

#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        void MoveObjectsHandleFigure::setPosition(const Vec3f& position) {
            m_position = position;
        }
        
        void MoveObjectsHandleFigure::render(Vbo& vbo, RenderContext& context) {
            VertexArray axisArray(vbo, GL_LINES, 6,
                                  VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                  VertexAttribute(4, GL_FLOAT, VertexAttribute::Color));
            VertexArray hiddenAxisArray(vbo, GL_LINES, 6,
                                        VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                        VertexAttribute(4, GL_FLOAT, VertexAttribute::Color));
            VertexArray quadArray(vbo, GL_QUADS, 8,
                                  VertexAttribute(3, GL_FLOAT, VertexAttribute::Position),
                                  VertexAttribute(4, GL_FLOAT, VertexAttribute::Color));
            
            vbo.activate();
            vbo.map();
            axisArray.addAttribute(Vec3f(m_position.x - 64.0f, m_position.y, m_position.z));
            axisArray.addAttribute(Color(1.0f, 0.0f, 0.0f, 1.0f));
            axisArray.addAttribute(Vec3f(m_position.x + 64.0f, m_position.y, m_position.z));
            axisArray.addAttribute(Color(1.0f, 0.0f, 0.0f, 1.0f));
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y - 64.0f, m_position.z));
            axisArray.addAttribute(Color(0.0f, 1.0f, 0.0f, 1.0f));
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y + 64.0f, m_position.z));
            axisArray.addAttribute(Color(0.0f, 1.0f, 0.0f, 1.0f));
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y, m_position.z - 64.0f));
            axisArray.addAttribute(Color(0.0f, 0.0f, 1.0f, 1.0f));
            axisArray.addAttribute(Vec3f(m_position.x, m_position.y, m_position.z + 64.0f));
            axisArray.addAttribute(Color(0.0f, 0.0f, 1.0f, 1.0f));

            hiddenAxisArray.addAttribute(Vec3f(m_position.x - 64.0f, m_position.y, m_position.z));
            hiddenAxisArray.addAttribute(Color(1.0f, 0.0f, 0.0f, 0.35f));
            hiddenAxisArray.addAttribute(Vec3f(m_position.x + 64.0f, m_position.y, m_position.z));
            hiddenAxisArray.addAttribute(Color(1.0f, 0.0f, 0.0f, 0.35f));
            hiddenAxisArray.addAttribute(Vec3f(m_position.x, m_position.y - 64.0f, m_position.z));
            hiddenAxisArray.addAttribute(Color(0.0f, 1.0f, 0.0f, 0.35f));
            hiddenAxisArray.addAttribute(Vec3f(m_position.x, m_position.y + 64.0f, m_position.z));
            hiddenAxisArray.addAttribute(Color(0.0f, 1.0f, 0.0f, 0.35f));
            hiddenAxisArray.addAttribute(Vec3f(m_position.x, m_position.y, m_position.z - 64.0f));
            hiddenAxisArray.addAttribute(Color(0.0f, 0.0f, 1.0f, 0.35f));
            hiddenAxisArray.addAttribute(Vec3f(m_position.x, m_position.y, m_position.z + 64.0f));
            hiddenAxisArray.addAttribute(Color(0.0f, 0.0f, 1.0f, 0.35f));
            
            Vec3f xAxis, yAxis;
            Vec3f view = m_position - context.camera().position();
            view.normalize();
            
            if (eq(fabsf(view.z), 1.0f)) {
                xAxis = context.camera().right().firstAxis();
                if (eq(fabsf(context.camera().direction().z), 1.0f))
                    yAxis = context.camera().up().firstAxis();
                else
                    yAxis = -1.0f * context.camera().direction().firstAxis();
            } else {
                xAxis = view.x > 0.0f ? Vec3f::NegX : Vec3f::PosX;
                yAxis = view.y > 0.0f ? Vec3f::NegY : Vec3f::PosY;
            }
            
            quadArray.addAttribute(m_position);
            quadArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadArray.addAttribute(m_position + 32.0f * xAxis);
            quadArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadArray.addAttribute(m_position + 32.0f * (xAxis + yAxis));
            quadArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadArray.addAttribute(m_position + 32.0f * yAxis);
            quadArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            
            if (fabsf(view.x) < fabsf(view.y))
                xAxis = view.x > 0.0f ? Vec3f::NegX : Vec3f::PosX;
            else
                xAxis = view.y > 0.0f ? Vec3f::NegY : Vec3f::PosY;
            if (view.z >= 0.0f)
                yAxis = Vec3f::NegZ;
            else
                yAxis = Vec3f::PosZ;
            
            quadArray.addAttribute(m_position);
            quadArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadArray.addAttribute(m_position + 32.0f * xAxis);
            quadArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadArray.addAttribute(m_position + 32.0f * (xAxis + yAxis));
            quadArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));
            quadArray.addAttribute(m_position + 32.0f * yAxis);
            quadArray.addAttribute(Color(1.0f, 1.0f, 1.0f, 0.7f));

            vbo.unmap();
            
            axisArray.render();
            glDisable(GL_DEPTH_TEST);
            hiddenAxisArray.render();
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            quadArray.render();
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            
            vbo.deactivate();
        }
    }
}