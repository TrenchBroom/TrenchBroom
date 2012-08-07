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

#include "MoveObjectTool.h"
#include "Controller/Editor.h"
#include "Controller/Grid.h"
#include "Controller/Tool.h"
#include "Controller/Options.h"
#include "Model/Map/Picker.h"
#include "Model/Map/Face.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Map.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"
#include "Renderer/Figures/MoveObjectToolFigure.h"
#include "Utilities/Console.h"

namespace TrenchBroom {
    namespace Controller {
        bool MoveObjectTool::handleBeginPlaneDrag(InputEvent& event, Vec3f& initialPoint) {
            if (event.mouseButton != TB_MB_LEFT)
                return false;
            
            Model::Hit* hit = event.hits->first(Model::TB_HT_ENTITY | Model::TB_HT_FACE, true);
            if (hit == NULL)
                return false;
            
            if (hit->type == Model::TB_HT_FACE) {
                Model::Face& face = hit->face();
                Model::Brush& brush = *face.brush;
                if (!brush.selected)
                    return false;
                m_referenceObject = &brush;
            } else {
                Model::Entity& entity = hit->entity();
                if (!entity.selected())
                    return false;
                m_referenceObject = &entity;
            }
            initialPoint = hit->hitPoint;
            
            if (!m_figureCreated) {
                Renderer::MoveObjectToolFigure* figure = new Renderer::MoveObjectToolFigure(*this);
                addFigure(*figure);
                m_figureCreated = true;
            }
            refreshFigure(true);
            
            editor().map().undoManager().begin("Move Objects");
            
            return true;
        }
        
        bool MoveObjectTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            assert(event.mouseButton == TB_MB_LEFT);
            
            Grid& grid = editor().grid();
            Model::Map& map = editor().map();
            
            Vec3f delta = curMousePoint - referencePoint;
            Vec3f snapDelta;
            if (m_referenceObject->objectType() == Model::TB_MT_ENTITY) {
                Model::Entity* entity = static_cast<Model::Entity*>(m_referenceObject);
                const Vec3f& origin = entity->origin();
                Vec3f newOrigin = grid.snap(origin + curMousePoint - referencePoint);
                snapDelta = newOrigin - origin;
                
                for (unsigned int i = 0; i < 3; i++)
                    if (snapDelta[i] > 0 != delta[i] > 0)
                        snapDelta[i] = 0;
            } else {
                snapDelta = grid.snap(curMousePoint - referencePoint);
            }
            
            if (snapDelta.null())
                return true;
            
            referencePoint += snapDelta;
            map.translateObjects(snapDelta, editor().options().lockTextures());
            refreshFigure(true);
            
            return true;
        }
        
        void MoveObjectTool::handleEndPlaneDrag(InputEvent& event) {
            assert(event.mouseButton == TB_MB_LEFT);
            
            editor().map().undoManager().end();
            refreshFigure(false);
            m_referenceObject = NULL;
        }

        MoveObjectTool::MoveObjectTool(Editor& editor) : DragTool(editor), m_referenceObject(NULL), m_figureCreated(false) {}
    }
}