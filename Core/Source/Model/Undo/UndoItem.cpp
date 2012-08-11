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

#include "UndoItem.h"
#include "Model/Map/Map.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Face.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"

namespace TrenchBroom {
    namespace Model {
        SelectionUndoItem::SelectionUndoItem(Map& map) : UndoItem(map) {
            Selection& selection = m_map.selection();
            m_selectedEntities = selection.selectedEntities();
            m_selectedBrushes = selection.selectedBrushes();
            m_selectedFaces = selection.selectedFaces();
        }

        void SelectionUndoItem::undo() {
            Selection& selection = m_map.selection();
            
            selection.deselectAll();
            selection.selectEntities(m_selectedEntities);
            selection.selectBrushes(m_selectedBrushes);
            selection.selectFaces(m_selectedFaces);
            performUndo();
        }

        void SelectionSnapshotUndoItem::undo() {
            Selection& selection = m_map.selection();
            
            m_map.undoManager().addSelection(m_map);
            
            selection.deselectAll();
            selection.selectEntities(m_selectedEntities);
            selection.selectBrushes(m_selectedBrushes);
            selection.selectFaces(m_selectedFaces);
        }
    }
}
