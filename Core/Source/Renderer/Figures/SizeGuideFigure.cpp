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

#include "SizeGuideFigure.h"

#include "Controller/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"

#include <sstream>

namespace TrenchBroom {
    namespace Renderer {
        SizeGuideFigure::SizeGuideFigure (FontManager& fontManager, const FontDescriptor& fontDescriptor) : Figure(), m_fontManager(fontManager), m_fontDescriptor(fontDescriptor), m_offset(5.0f), m_cutoffDistance(512.0f), m_stringsValid(false) {}
        SizeGuideFigure::~SizeGuideFigure() {
            while (!m_strings.empty()) m_fontManager.destroyStringRenderer(m_strings.back()), m_strings.pop_back();
        }
        
        void SizeGuideFigure::setBounds(const BBox& bounds) {
            if (bounds == m_bounds)
                return;
            
            m_bounds = bounds;
            m_stringsValid = false;
        }
        
        void SizeGuideFigure::setColor(const Vec4f& color) {
            m_color = color;
        }
        
        void SizeGuideFigure::setHiddenColor(const Vec4f& hiddenColor) {
            m_hiddenColor = hiddenColor;
        }
        
        void SizeGuideFigure::render(RenderContext& context, Vbo& vbo) {
            const Vec3f& cameraPos = context.camera.position();
            Vec3f center = m_bounds.center();
            Vec3f centerDir = center - cameraPos;
            
            Vec3fList vertices;
            Vec3fList labelPositions;
            
            // X guide
            if (centerDir.y >= 0) {
                vertices.push_back(m_bounds.min);
                vertices.back().y -= m_offset;
                vertices.push_back(vertices.back());
                vertices.back().y -= m_offset;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().x = m_bounds.max.x;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().y += m_offset;
                
                labelPositions.push_back(center);
                labelPositions.back().y = m_bounds.min.y - 2 * m_offset;
                labelPositions.back().z = m_bounds.min.z;
            } else {
                vertices.push_back(m_bounds.min);
                vertices.back().y = m_bounds.max.y + m_offset;
                vertices.push_back(vertices.back());
                vertices.back().y += m_offset;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().x = m_bounds.max.x;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().y -= m_offset;

                labelPositions.push_back(center);
                labelPositions.back().y = m_bounds.max.y + 2 * m_offset;
                labelPositions.back().z = m_bounds.min.z;
            }
            
            // Y guide
            if (centerDir.x >= 0) {
                vertices.push_back(m_bounds.min);
                vertices.back().x -= m_offset;
                vertices.push_back(vertices.back());
                vertices.back().x -= m_offset;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().y = m_bounds.max.y;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().x += m_offset;

                labelPositions.push_back(center);
                labelPositions.back().x = m_bounds.min.x - 2 * m_offset;
                labelPositions.back().z = m_bounds.min.z;
            } else {
                vertices.push_back(m_bounds.min);
                vertices.back().x = m_bounds.max.x + m_offset;
                vertices.push_back(vertices.back());
                vertices.back().x += m_offset;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().y = m_bounds.max.y;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().x -= m_offset;

                labelPositions.push_back(center);
                labelPositions.back().x = m_bounds.max.x + 2 * m_offset;
                labelPositions.back().z = m_bounds.min.z;
            }
            
            if (centerDir.z >= 0) {
                for (unsigned int i = 0; i < vertices.size(); i++)
                    vertices[i].z = m_bounds.max.z;
                for (unsigned int i = 0; i < labelPositions.size(); i++)
                    labelPositions[i].z = m_bounds.max.z;
            }
            
            // Z Guide
            if (cameraPos.x <= m_bounds.min.x && cameraPos.y <= m_bounds.max.y) {
                vertices.push_back(m_bounds.min);
                vertices.back().x -= m_offset * 0.71f;
                vertices.back().y = m_bounds.max.y + m_offset * 0.71f;
                vertices.push_back(vertices.back());
                vertices.back().x -= m_offset * 0.71f;
                vertices.back().y += m_offset * 0.71f;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().z = m_bounds.max.z;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().x += m_offset * 0.71f;
                vertices.back().y -= m_offset * 0.71f;

                labelPositions.push_back(center);
                labelPositions.back().x = m_bounds.min.x - 2 * 0.71f * m_offset;
                labelPositions.back().y = m_bounds.max.y + 2 * 0.71f * m_offset;
            } else if (cameraPos.x <= m_bounds.max.x && cameraPos.y >= m_bounds.max.y) {
                vertices.push_back(m_bounds.max);
                vertices.back().x += m_offset * 0.71f;
                vertices.back().y += m_offset * 0.71f;
                vertices.push_back(vertices.back());
                vertices.back().x += m_offset * 0.71f;
                vertices.back().y += m_offset * 0.71f;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().z = m_bounds.min.z;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().x -= m_offset * 0.71f;
                vertices.back().y -= m_offset * 0.71f;

                labelPositions.push_back(center);
                labelPositions.back().x = m_bounds.max.x + 2 * 0.71f * m_offset;
                labelPositions.back().y = m_bounds.max.y + 2 * 0.71f * m_offset;
            } else if (cameraPos.x >= m_bounds.max.x && cameraPos.y >= m_bounds.min.y) {
                vertices.push_back(m_bounds.max);
                vertices.back().x += m_offset * 0.71f;
                vertices.back().y = m_bounds.min.y - m_offset * 0.71f;
                vertices.push_back(vertices.back());
                vertices.back().x += m_offset * 0.71f;
                vertices.back().y -= m_offset * 0.71f;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().z = m_bounds.min.z;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().x -= m_offset * 0.71f;
                vertices.back().y += m_offset * 0.71f;

                labelPositions.push_back(center);
                labelPositions.back().x = m_bounds.max.x + 2 * 0.71f * m_offset;
                labelPositions.back().y = m_bounds.min.y - 2 * 0.71f * m_offset;
            } else if (cameraPos.x >= m_bounds.min.x && cameraPos.y <= m_bounds.min.y) {
                vertices.push_back(m_bounds.min);
                vertices.back().x -= m_offset * 0.71f;
                vertices.back().y -= m_offset * 0.71f;
                vertices.push_back(vertices.back());
                vertices.back().x -= m_offset * 0.71f;
                vertices.back().y -= m_offset * 0.71f;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().z = m_bounds.max.z;
                
                vertices.push_back(vertices.back());
                vertices.push_back(vertices.back());
                vertices.back().x += m_offset * 0.71f;
                vertices.back().y += m_offset * 0.71f;

                labelPositions.push_back(center);
                labelPositions.back().x = m_bounds.min.x - 2 * 0.71f * m_offset;
                labelPositions.back().y = m_bounds.min.y - 2 * 0.71f * m_offset;
            }            

            if (!m_stringsValid) {
                while (!m_strings.empty()) m_fontManager.destroyStringRenderer(m_strings.back()), m_strings.pop_back();
                
                Vec3f size = m_bounds.size();
                for (unsigned int i = 0; i < 3; i++) {
                    std::stringstream str;
                    str.precision(0);
                    str << std::fixed << size[i];
                    m_strings.push_back(m_fontManager.createStringRenderer(m_fontDescriptor, str.str()));
                }
                
                m_stringsValid = true;
            }
            
            VboBlock* block = vbo.allocBlock(vertices.size() * sizeof(Vec3f));
            vbo.map();
            block->writeVecs(vertices, 0);
            vbo.unmap();
            
            glDisable(GL_DEPTH_TEST);

            // initialize the stencil buffer to cancel out the guides in those areas where the strings will be rendered
            glPolygonMode(GL_FRONT, GL_FILL);
            glClear(GL_STENCIL_BUFFER_BIT);
            glColorMask(false, false, false, false);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

            for (unsigned int i = 0; i < labelPositions.size(); i++) {
                float dist = context.camera.distanceTo(labelPositions[i]);
                float factor = dist / 300;
                
                float width = m_strings[i]->width;
                float height = m_strings[i]->height;
                
                glPushMatrix();
                glTranslatef(labelPositions[i].x, labelPositions[i].y, labelPositions[i].z);
                context.camera.setBillboard();
                glScalef(factor, factor, 0);
                glTranslatef(-width / 2, -height / 2, 0);
                m_strings[i]->renderBackground(1, 1);
                glPopMatrix();
            }
            
            glColorMask(true, true, true, true);
            glStencilFunc(GL_NOTEQUAL, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, reinterpret_cast<const GLvoid*>(block->address));
            
            glColorV4f(m_color);
            glDrawArrays(GL_LINES, 0, vertices.size());
            
            glPopClientAttrib();
            block->freeBlock();

            glDisable(GL_STENCIL_TEST);

            vbo.deactivate();
            m_fontManager.activate();
            for (unsigned int i = 0; i < labelPositions.size(); i++) {
                glColorV4f(m_color);
                
                float dist = context.camera.distanceTo(labelPositions[i]);
                float factor = dist / 300;
                float width = m_strings[i]->width;
                float height = m_strings[i]->height;
                
                glPushMatrix();
                glTranslatef(labelPositions[i].x, labelPositions[i].y, labelPositions[i].z);
                context.camera.setBillboard();
                glScalef(factor, factor, 0);
                glTranslatef(-width / 2, -height / 2, 0);
                m_strings[i]->render();
                glPopMatrix();
            }
            m_fontManager.deactivate();
            vbo.activate();
        }
    }
}