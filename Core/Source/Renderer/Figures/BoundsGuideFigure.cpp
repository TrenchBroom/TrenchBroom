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

#include "BoundsGuideFigure.h"
#include "GL/GLee.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        static const unsigned int VertexSize = sizeof(Vec3f);
        static const unsigned int ColorSize = sizeof(Vec4f);
        static const unsigned int LineVertexSize = VertexSize + ColorSize;
        
        unsigned int BoundsGuideFigure::writeLine(RenderContext& context, VboBlock& block, unsigned int offset, const Vec4f& color, const Vec3f& anchor, float size, const Vec3f& axis) {
            float length = 256;
            Vec3f outerOffset = axis * ((size / 2.0f) + length);
            Vec3f innerOffset = axis * (size / 2.0f);

            Vec4f lightColor = color;
            lightColor.w *= 0.2f;
            Vec4f strongColor = color;
            strongColor.w *= 0.6f;
            
            offset = block.writeVec(lightColor, offset);
            offset = block.writeVec(anchor - outerOffset, offset);
            offset = block.writeVec(strongColor, offset);
            offset = block.writeVec(anchor - innerOffset, offset);
            offset = block.writeVec(strongColor, offset);
            offset = block.writeVec(anchor - innerOffset, offset);
            offset = block.writeVec(strongColor, offset);
            offset = block.writeVec(anchor + innerOffset, offset);
            offset = block.writeVec(strongColor, offset);
            offset = block.writeVec(anchor + innerOffset, offset);
            offset = block.writeVec(lightColor, offset);
            offset = block.writeVec(anchor + outerOffset, offset);

            return offset;
        }

        BoundsGuideFigure::BoundsGuideFigure() : m_color(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), m_hiddenColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), m_valid(false), m_block(NULL) {}
        
        BoundsGuideFigure::~BoundsGuideFigure() {
            if (m_block != NULL) {
                m_block->freeBlock();
                m_block = NULL;
            }
        }
        
        void BoundsGuideFigure::setBounds(const BBox& bounds) {
            if (bounds == m_bounds)
                return;
            m_bounds = bounds;
            m_valid = false;
        }
        
        void BoundsGuideFigure::setColor(const Vec4f& color) {
            if (color == m_color)
                return;
            m_color = color;
            m_valid = false;
        }
        
        void BoundsGuideFigure::setHiddenColor(const Vec4f& hiddenColor) {
            if (hiddenColor == m_hiddenColor)
                return;
            m_hiddenColor = hiddenColor;
            m_valid = false;
        }

        void BoundsGuideFigure::render(RenderContext& context, Vbo& vbo) {
            unsigned int vertexCount = 12 * 6;
            if (!m_valid) {
                if (m_block == NULL)
                    m_block = vbo.allocBlock(vertexCount * LineVertexSize);
                
                vbo.map();
                
                Vec3f v;
                Vec3f size = m_bounds.size();
                unsigned int offset = 0;
                
                v = m_bounds.min;
                v.x += size.x / 2;
                offset = writeLine(context, *m_block, offset, m_color, v, size.x, Vec3f::PosX);
                v.y += size.y;
                offset = writeLine(context, *m_block, offset, m_color, v, size.x, Vec3f::PosX);
                v.z += size.z;
                offset = writeLine(context, *m_block, offset, m_color, v, size.x, Vec3f::PosX);
                v.y -= size.y;
                offset = writeLine(context, *m_block, offset, m_color, v, size.x, Vec3f::PosX);
                
                v = m_bounds.min;
                v.y += size.y / 2;
                offset = writeLine(context, *m_block, offset, m_color, v, size.y, Vec3f::PosY);
                v.x += size.x;
                offset = writeLine(context, *m_block, offset, m_color, v, size.y, Vec3f::PosY);
                v.z += size.z;
                offset = writeLine(context, *m_block, offset, m_color, v, size.y, Vec3f::PosY);
                v.x -= size.x;
                offset = writeLine(context, *m_block, offset, m_color, v, size.y, Vec3f::PosY);
                
                v = m_bounds.min;
                v.z += size.z / 2;
                offset = writeLine(context, *m_block, offset, m_color, v, size.z, Vec3f::PosZ);
                v.x += size.x;
                offset = writeLine(context, *m_block, offset, m_color, v, size.z, Vec3f::PosZ);
                v.y += size.y;
                offset = writeLine(context, *m_block, offset, m_color, v, size.z, Vec3f::PosZ);
                v.x -= size.x;
                offset = writeLine(context, *m_block, offset, m_color, v, size.z, Vec3f::PosZ);
                
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
