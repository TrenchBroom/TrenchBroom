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
#include "Renderer/Figures/BoundsGuideFigure.h"
#include "Utilities/Console.h"

namespace TrenchBroom {
    namespace Controller {
        bool MoveObjectTool::handleBeginPlaneDrag(InputEvent& event, Vec3f& initialPoint) {
            if (event.mouseButton != MB_LEFT)
                return false;
            
            Model::Hit* hit = event.hits->first(Model::TB_HT_ENTITY | Model::TB_HT_FACE, true);
            if (hit == NULL)
                return false;
            
            if (hit->type == Model::TB_HT_FACE) {
                Model::Face& face = hit->face();
                Model::Brush& brush = *face.brush;
                if (!brush.selected)
                    return false;
            } else {
                Model::Entity& entity = hit->entity();
                if (!entity.selected())
                    return false;
            }
            initialPoint = hit->hitPoint;
            
            if (m_guideFigure == NULL) {
                Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
                m_guideFigure = new Renderer::BoundsGuideFigure();
                m_guideFigure->setColor(prefs.selectionGuideColor());
                m_guideFigure->setHiddenColor(prefs.hiddenSelectionGuideColor());
            }

            Model::Map& map = m_editor.map();
            m_guideFigure->setBounds(map.selection().bounds());

            addFigure(*m_guideFigure);
            m_editor.map().undoManager().begin("Move Objects");
            
            return true;
        }
        
        bool MoveObjectTool::handlePlaneDrag(InputEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            assert(event.mouseButton == MB_LEFT);
            
            Grid& grid = m_editor.grid();
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            
            Vec3f delta = grid.moveDelta(selection.bounds(), map.worldBounds(), curMousePoint - referencePoint);
            if (delta.null())
                return true;
            
            referencePoint += delta;
            map.translateObjects(delta, m_editor.options().lockTextures());
            m_guideFigure->setBounds(selection.bounds());
            
            return true;
        }
        
        void MoveObjectTool::handleEndPlaneDrag(InputEvent& event) {
            assert(event.mouseButton == MB_LEFT);
            
            removeFigure(*m_guideFigure);
            delete m_guideFigure;
            m_guideFigure = NULL;
            m_editor.map().undoManager().end();
        }

        MoveObjectTool::MoveObjectTool(Editor& editor) : DragTool(editor), m_guideFigure(NULL) {}
    }
}