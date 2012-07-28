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

#include "DragEntityFigure.h"

namespace TrenchBroom {
    namespace Renderer {
        void DragEntityFigure::dragStarted(Model::EntityDefinitionPtr& entityDefinition) {
            m_entityDefinition = entityDefinition;
            m_valid = false;
        }
        
        void DragEntityFigure::dragged(const BBox& bounds) {
            m_bounds = bounds;
            m_valid = false;
        }
        
        void DragEntityFigure::dragEnded(const BBox& bounds) {
            // nothing to do
        }
        
        DragEntityFigure::DragEntityFigure(Controller::DragEntityTargetTool& dragEntityTool) : m_dragEntityTool(dragEntityTool), m_entityDefinition(EntityDefinitionPtr()), m_bounds(BBox()), m_valid(false), m_entityFigure(NULL), m_guideFigure(NULL) {
            m_dragEntityTool.dragStarted += new DragStartListener(this, &DragEntityFigure::dragStarted);
            m_dragEntityTool.dragged += new DragListener(this, &DragEntityFigure::dragged);
            m_dragEntityTool.dragEnded += new DragListener(this, &DragEntityFigure::dragEnded);
        }
        
        DragEntityFigure::~DragEntityFigure() {
            m_dragEntityTool.dragStarted -= new DragStartListener(this, &DragEntityFigure::dragStarted);
            m_dragEntityTool.dragged -= new DragListener(this, &DragEntityFigure::dragged);
            m_dragEntityTool.dragEnded -= new DragListener(this, &DragEntityFigure::dragEnded);
            
            if (m_entityFigure != NULL) {
                delete m_entityFigure;
                m_entityFigure = NULL;
            }
            
            if (m_guideFigure != NULL) {
                delete m_guideFigure;
                m_guideFigure = NULL;
            }
        }
        
        void DragEntityFigure::render(RenderContext& context, Vbo& vbo) {
            if (m_entityDefinition.get() == NULL)
                return;
            
            if (m_entityFigure == NULL) {
            }
        }
    }
}