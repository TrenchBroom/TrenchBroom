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
#include "Model/Map/Picker.h"
#include "Model/Map/Face.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Map.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"
#include "Utilities/Console.h"

namespace TrenchBroom {
    namespace Controller {
        bool MoveObjectTool::doBeginLeftDrag(ToolEvent& event, Vec3f& lastPoint) {
            Model::Hit* hit = event.hits->first(Model::TB_HT_ENTITY | Model::TB_HT_FACE, true);
            if (hit == NULL)
                return false;
            
            if (hit->type == Model::TB_HT_FACE) {
                Model::Face& face = hit->face();
                Model::Brush& brush = *face.brush();
                if (!brush.selected())
                    return false;
            } else {
                Model::Entity& entity = hit->entity();
                if (!entity.selected())
                    return false;
            }
            lastPoint = hit->hitPoint;
            
            m_editor.map().undoManager().begin("Move Objects");
            
            return true;
        }
        
        bool MoveObjectTool::doLeftDrag(ToolEvent& event, Vec3f& delta, Vec3f& lastPoint) {
            Grid& grid = m_editor.grid();
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();

            log(TB_LL_INFO, "%f %f %f\n", delta.x, delta.y, delta.z);
            grid.moveDelta(selection.bounds(), map.worldBounds(), delta, &lastPoint);
            log(TB_LL_INFO, "%f %f %f\n", delta.x, delta.y, delta.z);
            
            if (delta.null())
                return true;
            
            map.translateObjects(delta, true);
            return true;
        }
        
        void MoveObjectTool::doEndLeftDrag(ToolEvent& event) {
            m_editor.map().undoManager().end();
        }
    }
}