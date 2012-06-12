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

#include "EntityFigure.h"

#include "GL/GLee.h"
#include "Controller/Editor.h"
#include "Model/Map/EntityDefinition.h"
#include "Model/Map/Map.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/EntityRendererManager.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Renderer {
        EntityFigure::EntityFigure(Controller::Editor& editor, Model::EntityDefinition& entityDefinition) : Figure(), m_editor(editor), m_entityDefinition(entityDefinition), m_valid(false), m_entityRenderer(NULL), m_boundsVbo(NULL), m_boundsBlock(NULL) {
        }
        
        EntityFigure::~EntityFigure() {
            if (m_boundsVbo != NULL) {
                if (m_boundsBlock != NULL)
                    m_boundsBlock->freeBlock();
                delete m_boundsVbo;
            }
        }
        
        void EntityFigure::setPosition(const Vec3f& position) {
            m_position = position;
        }

        void EntityFigure::render(RenderContext& context) {
            if (m_entityDefinition.type != Model::TB_EDT_POINT)
                return;
            
            if (!m_valid) {
                std::vector<Vec3f> edges = bboxEdgeVertices(m_entityDefinition.bounds);
                m_vertexCount = edges.size();
                m_boundsVbo = new Vbo(GL_ARRAY_BUFFER, 3 * 4 * m_vertexCount);
                m_boundsBlock = m_boundsVbo->allocBlock(3 * 4 * m_vertexCount);
                
                m_boundsVbo->activate();
                m_boundsVbo->map();
                unsigned int offset = 0;
                for (unsigned int i = 0; i < m_vertexCount; i++)
                    offset = m_boundsBlock->writeVec(edges[i], offset);
                m_boundsVbo->unmap();

                EntityRendererManager& rendererManager = m_editor.renderer()->entityRendererManager();
                m_entityRenderer = rendererManager.entityRenderer(m_entityDefinition, m_editor.map().mods());
                
                m_valid = true;
            } else {
                m_boundsVbo->activate();
            }

            glPushAttrib(GL_TEXTURE_BIT);
            glPushMatrix();
            glTranslatef(m_position.x, m_position.y, m_position.z);
            
            glDisable(GL_TEXTURE_2D);
            glColorV4f(m_entityDefinition.color);

            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, 0);
            glDrawArrays(GL_LINES, 0, m_vertexCount);
            glPopClientAttrib();

            m_boundsVbo->deactivate();

            if (m_entityRenderer != NULL) {
                glEnable(GL_TEXTURE_2D);
                EntityRendererManager& rendererManager = m_editor.renderer()->entityRendererManager();
                rendererManager.activate();
                m_entityRenderer->render();
                rendererManager.deactivate();
            }
            glPopMatrix();
            glPopAttrib();
            
        }
    }
}