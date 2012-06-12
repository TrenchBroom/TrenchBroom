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

#include "DragEntityTargetTool.h"
#include "Controller/Camera.h"
#include "Controller/Editor.h"
#include "Controller/Grid.h"
#include "Controller/Tool.h"
#include "Model/Map/Entity.h"
#include "Model/Map/EntityDefinition.h"
#include "Model/Map/Map.h"
#include "Model/Map/Picker.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"
#include "Renderer/Figures/EntityFigure.h"

namespace TrenchBroom {
    namespace Controller {
        void DragEntityTargetTool::updateFeedbackFigure(const DragInfo& info) {
            Controller::Grid& grid = m_editor.grid();
            Vec3f delta = grid.moveDelta(m_bounds, m_editor.map().worldBounds(), m_bounds.center(), m_editor.camera().defaultPoint(info.event.ray.direction));
            
            if (!delta.null()) {
                m_bounds = m_bounds.translate(delta);
                m_feedbackFigure->setPosition(m_bounds.center());
            }
        }

        bool DragEntityTargetTool::accepts(const DragInfo& info) {
            return info.name == "Entity";
        }
    
        bool DragEntityTargetTool::activate(const DragInfo& info) {
            if (m_feedbackFigure != NULL) {
                removeFigure(*m_feedbackFigure);
                delete m_feedbackFigure;
                m_feedbackFigure = NULL;
            }
            
            Model::EntityDefinition* definition = static_cast<Model::EntityDefinition*>(info.payload);
            m_bounds = definition->bounds;

            m_feedbackFigure = new Renderer::EntityFigure(m_editor, *definition);
            updateFeedbackFigure(info);
            addFigure(*m_feedbackFigure);
            
            return false;
        }
        
        void DragEntityTargetTool::deactivate(const DragInfo& info) {
            if (m_feedbackFigure != NULL) {
                removeFigure(*m_feedbackFigure);
                delete m_feedbackFigure;
                m_feedbackFigure = NULL;
            }
        }
        
        bool DragEntityTargetTool::move(const DragInfo& info) {
            updateFeedbackFigure(info);
            return false;
        }
        
        bool DragEntityTargetTool::drop(const DragInfo& info) {
            if (m_feedbackFigure != NULL) {
                removeFigure(*m_feedbackFigure);
                delete m_feedbackFigure;
                m_feedbackFigure = NULL;
            }
            
            Model::EntityDefinition* definition = static_cast<Model::EntityDefinition*>(info.payload);
            
            m_editor.map().undoManager().begin("Create Entity");
            m_editor.map().createEntity(definition->name);
            m_editor.map().setEntityProperty(Model::OriginKey, m_bounds.center(), true);
            m_editor.map().undoManager().end();
            
            return true;
        }
    }
}
