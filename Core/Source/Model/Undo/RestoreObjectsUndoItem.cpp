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

#include "RestoreObjectsUndoItem.h"

#include "Model/Map/Map.h"
#include "Model/Selection.h"

namespace TrenchBroom {
    namespace Model {
        RestoreObjectsUndoItem::RestoreObjectsUndoItem(Map& map, const std::vector<Entity*>& entities, const BrushList& brushes) : UndoItem(map), m_entities(entities), m_brushes(brushes) {}
        
        void RestoreObjectsUndoItem::undo() {
            Selection& selection = m_map.selection();
            selection.removeAll();
            
            for (unsigned int i = 0; i < m_entities.size(); i++) {
                Entity* entity = m_entities[i];
                m_map.addEntity(entity);
            }
            
            for (unsigned int i = 0; i < m_brushes.size(); i++) {
            }
        }
    }
}