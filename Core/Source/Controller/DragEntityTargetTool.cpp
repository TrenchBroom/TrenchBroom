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
#include "Renderer/Figures/EntityFigure.h"
#include "Renderer/Figures/PositioningGuideFigure.h"

namespace TrenchBroom {
    namespace Controller {
        
        void DragEntityTargetTool::deleteFigures() {
            if (m_entityFigure != NULL) {
                removeFigure(*m_entityFigure);
                delete m_entityFigure;
                m_entityFigure = NULL;
            }
            if (m_guideFigure != NULL) {
                removeFigure(*m_guideFigure);
                delete m_guideFigure;
                m_guideFigure = NULL;
            }
        }
        
        void DragEntityTargetTool::updateFigures(const DragInfo& info, const Model::EntityDefinition& definition) {
            Vec3f delta;
            Controller::Grid& grid = m_editor.grid();

            Model::Hit* hit = info.event.hits->first(Model::TB_HT_FACE, true);
            if (hit == NULL) {
                Vec3f newPos = m_editor.camera().defaultPoint(info.event.ray.direction);
                delta = grid.moveDelta(m_bounds, m_editor.map().worldBounds(), m_bounds.center(), newPos);
            } else {
                Model::Face& face = hit->face();
                DragPlane dragPlane(face.boundary.normal);

                Vec3f halfSize = m_bounds.size() * 0.5f;
                float offsetLength = halfSize | dragPlane.normal();
                if (offsetLength < 0)
                    offsetLength *= -1.0f;
                Vec3f offset = dragPlane.normal() * offsetLength;
                
                float dist = dragPlane.intersect(info.event.ray, hit->hitPoint);
                Vec3f newPos = info.event.ray.pointAtDistance(dist);
                delta = grid.moveDelta(m_bounds, m_editor.map().worldBounds(), m_bounds.center() - offset, newPos);

                EAxis a = dragPlane.normal().firstComponent();
                if (dragPlane.normal()[a] > 0) delta[a] = hit->hitPoint[a] - m_bounds.min[a];
                else delta[a] = hit->hitPoint[a] - m_bounds.max[a];
            }
            
            if (!delta.null()) {
                m_bounds = m_bounds.translate(delta);
                m_entityFigure->setPosition(m_bounds.center() - definition.bounds.center());
                m_guideFigure->updateBounds(m_bounds);
            }
        }

        bool DragEntityTargetTool::accepts(const DragInfo& info) {
            return info.name == "Entity";
        }
    
        bool DragEntityTargetTool::activate(const DragInfo& info) {
            deleteFigures();
            
            Model::EntityDefinition* definition = static_cast<Model::EntityDefinition*>(info.payload);
            m_bounds = definition->bounds;

            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            
            m_entityFigure = new Renderer::EntityFigure(m_editor, *definition, false );
            m_guideFigure = new Renderer::PositioningGuideFigure(m_bounds, prefs.selectionGuideColor(), prefs.hiddenSelectionGuideColor());
            updateFigures(info, *definition);
            addFigure(*m_entityFigure);
            addFigure(*m_guideFigure);
            
            return false;
        }
        
        void DragEntityTargetTool::deactivate(const DragInfo& info) {
            deleteFigures();
        }
        
        bool DragEntityTargetTool::move(const DragInfo& info) {
            Model::EntityDefinition* definition = static_cast<Model::EntityDefinition*>(info.payload);
            updateFigures(info, *definition);
            return false;
        }
        
        bool DragEntityTargetTool::drop(const DragInfo& info) {
            deleteFigures();
            
            Model::EntityDefinition* definition = static_cast<Model::EntityDefinition*>(info.payload);
            
            m_editor.map().undoManager().begin("Create Entity");
            m_editor.map().createEntity(definition->name);
            m_editor.map().setEntityProperty(Model::OriginKey, m_bounds.center() - definition->bounds.center(), true);
            m_editor.map().undoManager().end();
            
            return true;
        }
    }
}
