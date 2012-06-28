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

#include "ResizeBrushTool.h"

#include "Controller/DragPlane.h"
#include "Controller/Grid.h"
#include "Controller/Options.h"
#include "Model/Map/Face.h"
#include "Model/Map/Map.h"
#include "Model/Map/Picker.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"

namespace TrenchBroom {
    namespace Controller {
        void ResizeBrushTool::updateDragPlane(ToolEvent& event) {
            m_dragPlane = DragPlane::parallel(m_referenceFace->boundary.normal, event.ray.direction);
        }

        bool ResizeBrushTool::doBeginLeftDrag(ToolEvent& event, Vec3f& initialPoint) {
            if (!resizeBrushModiferPressed(event))
                return false;
                
            Model::Hit* hit = event.hits->first(Model::TB_HT_FACE, true);
            if (hit == NULL)
                return false;
            
            Model::Face& face = hit->face();
            Model::Brush& brush = *face.brush;
            if (!face.selected && !brush.selected)
                return false;
            
            m_referenceFace = &face;
            initialPoint = hit->hitPoint;
            m_editor.map().undoManager().begin("Resize Brushes");
            return true;
        }
        
        bool ResizeBrushTool::doLeftDrag(ToolEvent& event, const Vec3f& lastMousePoint, const Vec3f& curMousePoint, Vec3f& referencePoint) {
            Vec3f delta = curMousePoint - referencePoint;
            float dist = m_editor.grid().moveDistance(*m_referenceFace, delta);

            if (!Math::isnan(dist)) {
                Model::Selection& selection = m_editor.map().selection();
                Model::FaceList faces;
                if (selection.mode() == Model::TB_SM_FACES)
                    faces = selection.faces();
                else
                    faces.push_back(m_referenceFace);
                
                m_editor.map().resizeBrushes(faces, dist, true);
                referencePoint += delta;
            }
            
            return true;
        }
        
        void ResizeBrushTool::doEndLeftDrag(ToolEvent& event) {
            m_editor.map().undoManager().end();
            m_referenceFace = NULL;
        }
        
        ResizeBrushTool::ResizeBrushTool(Editor& editor) : DragTool(editor) {
        }
        
        ResizeBrushTool::~ResizeBrushTool() {
        }

        bool ResizeBrushTool::resizeBrushModiferPressed(ToolEvent& event) {
            return event.modifierKeys == TB_MK_CMD;
        }
    }
}