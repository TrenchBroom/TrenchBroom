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
#include "Controller/DragPlane.h"
#include "Controller/Editor.h"
#include "Controller/Grid.h"
#include "Controller/Tool.h"
#include "Model/Map/Entity.h"
#include "Model/Map/EntityDefinition.h"
#include "Model/Map/Map.h"
#include "Model/Map/Picker.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"
#include "Renderer/Figures/DragEntityTargetToolFigure.h"

namespace TrenchBroom {
    namespace Controller {
        
        void DragEntityTargetTool::update(const DragInfo& info) {
            Vec3f delta;
            Controller::Grid& grid = m_editor.grid();

            Model::Hit* hit = info.event.hits->first(Model::TB_HT_FACE, true);
            if (hit == NULL) {
                Vec3f newPos = m_editor.camera().defaultPoint(info.event.ray.direction);
                delta = grid.moveDeltaForEntity(m_bounds.center(), m_editor.map().worldBounds(), newPos - m_bounds.center());
            } else {
                Model::Face& face = hit->face();
                DragPlane dragPlane = DragPlane::orthogonal(face.boundary.normal, true);

                Vec3f halfSize = m_bounds.size() * 0.5f;
                float offsetLength = halfSize | dragPlane.normal();
                if (offsetLength < 0)
                    offsetLength *= -1.0f;
                Vec3f offset = dragPlane.normal() * offsetLength;
                
                float dist = dragPlane.intersect(info.event.ray, hit->hitPoint);
                Vec3f newPos = info.event.ray.pointAtDistance(dist);
                delta = grid.moveDeltaForEntity(m_bounds.center(), m_editor.map().worldBounds(), newPos - (m_bounds.center() - offset));

                EAxis a = dragPlane.normal().firstComponent();
                if (dragPlane.normal()[a] > 0) delta[a] = hit->hitPoint[a] - m_bounds.min[a];
                else delta[a] = hit->hitPoint[a] - m_bounds.max[a];
            }
            
            if (delta.null())
                return;
            
            m_bounds = m_bounds.translate(delta);
            m_position = m_bounds.center() - m_entityDefinition->bounds.center();
            refreshFigure(true);
        }

        bool DragEntityTargetTool::accepts(const DragInfo& info) {
            return info.name == "Entity";
        }
    
        bool DragEntityTargetTool::handleActivate(const DragInfo& info) {
            m_entityDefinition = static_cast<Model::EntityDefinition*>(info.payload);
            m_bounds = m_entityDefinition->bounds;

            if (!m_figureCreated) {
                Renderer::DragEntityTargetToolFigure* figure = new Renderer::DragEntityTargetToolFigure(*this);
                addFigure(*figure);
                m_figureCreated = true;
            }
            
            update(info);

            return false;
        }
        
        void DragEntityTargetTool::handleDeactivate(const DragInfo& info) {
            m_entityDefinition = NULL;
        }
        
        bool DragEntityTargetTool::handleMove(const DragInfo& info) {
            assert(m_entityDefinition == static_cast<Model::EntityDefinition*>(info.payload));

            update(info);
            return false;
        }
        
        bool DragEntityTargetTool::handleDrop(const DragInfo& info) {
            assert(m_entityDefinition == static_cast<Model::EntityDefinition*>(info.payload));

            m_editor.map().undoManager().begin("Create Entity");
            m_editor.map().createEntity(m_entityDefinition->name);
            m_editor.map().setEntityProperty(Model::OriginKey, m_bounds.center() - m_entityDefinition->bounds.center(), true);
            m_editor.map().undoManager().end();
            
            refreshFigure(false);
            
            return true;
        }
    }
}
