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

#include "PointGuideFigure.h"
#include "GL/GLee.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        static const unsigned int VertexSize = sizeof(Vec3f);
        static const unsigned int ColorSize = sizeof(Vec4f);
        static const unsigned int LineVertexSize = VertexSize + ColorSize;
        
        unsigned int PointGuideFigure::writeLine(RenderContext& context, VboBlock& block, unsigned int offset, const Vec4f& color, const Vec3f& anchor, const Vec3f& axis) {
            float length = 256;
            Vec3f outerOffset = axis * length;
            
            Vec4f lightColor = color;
            lightColor.w *= 0.2f;
            Vec4f strongColor = color;
            strongColor.w *= 0.6f;
            
            offset = block.writeVec(lightColor, offset);
            offset = block.writeVec(anchor - outerOffset, offset);
            offset = block.writeVec(strongColor, offset);
            offset = block.writeVec(anchor, offset);
            offset = block.writeVec(strongColor, offset);
            offset = block.writeVec(anchor, offset);
            offset = block.writeVec(lightColor, offset);
            offset = block.writeVec(anchor + outerOffset, offset);
            
            return offset;
        }
        
        PointGuideFigure::PointGuideFigure() : m_color(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), m_hiddenColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), m_valid(false), m_block(NULL) {}
        
        PointGuideFigure::~PointGuideFigure() {
            if (m_block != NULL) {
                m_block->freeBlock();
                m_block = NULL;
            }
        }
        
        void PointGuideFigure::setPosition(const Vec3f& position) {
            if (position == m_position)
                return;
            m_position = position;
            m_valid = false;
        }
        
        void PointGuideFigure::setColor(const Vec4f& color) {
            if (color == m_color)
                return;
            m_color = color;
            m_valid = false;
        }
        
        void PointGuideFigure::setHiddenColor(const Vec4f& hiddenColor) {
            if (hiddenColor == m_hiddenColor)
                return;
            m_hiddenColor = hiddenColor;
            m_valid = false;
        }
        
        void PointGuideFigure::render(RenderContext& context, Vbo& vbo) {
            unsigned int vertexCount = 3 * 4;
            if (!m_valid) {
                if (m_block == NULL)
                    m_block = vbo.allocBlock(vertexCount * LineVertexSize);
                
                vbo.map();
                
                unsigned int offset = 0;
                offset = writeLine(context, *m_block, offset, m_color, m_position, Vec3f::PosX);
                offset = writeLine(context, *m_block, offset, m_color, m_position, Vec3f::PosY);
                offset = writeLine(context, *m_block, offset, m_color, m_position, Vec3f::PosZ);
                
                vbo.unmap();
                m_valid = true;
            }
            
            glEnable(GL_DEPTH_TEST);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glEnableClientState(GL_COLOR_ARRAY);
            glEnableClientState(GL_VERTEX_ARRAY);
            glColorPointer(4, GL_FLOAT, LineVertexSize, reinterpret_cast<const GLvoid*>(m_block->address));
            glVertexPointer(3, GL_FLOAT, LineVertexSize, reinterpret_cast<const GLvoid*>(m_block->address + ColorSize));
            
            glDrawArrays(GL_LINES, 0, vertexCount);
            
            glPopClientAttrib();
        }
    }
}
