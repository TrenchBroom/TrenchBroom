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


#include "VertexToolFigure.h"

#include "Controller/VertexTool.h"
#include "Renderer/Figures/HandleFigure.h"
#include "Renderer/Figures/PointGuideFigure.h"

namespace TrenchBroom {
    namespace Renderer {
        void VertexToolFigure::createFigures() {
            if (m_handleFigure == NULL) {
                m_handleFigure = new Renderer::HandleFigure();
                m_handleFigure->setColor(m_vertexTool.handleColor());
                m_handleFigure->setHiddenColor(m_vertexTool.hiddenHandleColor());
            }
            
            if (m_selectedHandleFigure == NULL) {
                m_selectedHandleFigure = new Renderer::HandleFigure();
                m_selectedHandleFigure->setColor(m_vertexTool.selectedHandleColor());
                m_selectedHandleFigure->setHiddenColor(m_vertexTool.hiddenSelectedHandleColor());
            }
            
            if (m_guideFigure == NULL) {
                m_guideFigure = new Renderer::PointGuideFigure();
                m_guideFigure->setColor(m_vertexTool.selectedHandleColor());
                m_guideFigure->setHiddenColor(m_vertexTool.hiddenSelectedHandleColor());
            }
        }

        void VertexToolFigure::deleteFigures() {
            if (m_handleFigure != NULL) {
                delete m_handleFigure;
                m_handleFigure = NULL;
            }
            
            if (m_selectedHandleFigure != NULL) {
                delete m_selectedHandleFigure;
                m_selectedHandleFigure = NULL;
            }
            
            if (m_guideFigure != NULL) {
                delete m_guideFigure;
                m_guideFigure = NULL;
            }
        }

        VertexToolFigure::VertexToolFigure(Controller::VertexTool& vertexTool) : m_vertexTool(vertexTool), m_handleFigure(NULL), m_selectedHandleFigure(NULL), m_guideFigure(NULL) {}
        
        VertexToolFigure::~VertexToolFigure() {
            deleteFigures();
        }
        
        void VertexToolFigure::render(RenderContext& context, Vbo& vbo) {
            if (!m_vertexTool.active()) {
                deleteFigures();
                return;
            }
         
            createFigures();
            
            bool valid = m_vertexTool.checkFigureDataValid();
            
            if (m_vertexTool.state() == Controller::Tool::TS_DRAG) {
                if (!valid) {
                    m_selectedHandleFigure->setPositions(m_vertexTool.selectedHandlePositions());
                    m_guideFigure->setPosition(m_vertexTool.draggedHandlePosition());
                }
                
                m_selectedHandleFigure->render(context,vbo);
                m_guideFigure->render(context, vbo);
            } else if (m_vertexTool.selected()) {
                if (!valid)
                    m_selectedHandleFigure->setPositions(m_vertexTool.selectedHandlePositions());
                m_selectedHandleFigure->render(context,vbo);
            } else {
                if (!valid)
                    m_handleFigure->setPositions(m_vertexTool.handlePositions());
                m_handleFigure->render(context, vbo);
            }
        }
    }
}