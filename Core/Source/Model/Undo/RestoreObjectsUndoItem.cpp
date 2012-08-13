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
        RestoreObjectsUndoItem::RestoreObjectsUndoItem(Map& map, const EntityList& entities, const BrushParentMap& removedBrushes, const BrushParentMap& movedBrushes) : UndoItem(map), m_entities(entities), m_removedBrushes(removedBrushes), m_movedBrushes(movedBrushes) {}
        
        RestoreObjectsUndoItem::~RestoreObjectsUndoItem() {
            // because we are the owners of deleted objects, we must delete them
            while (!m_entities.empty()) delete m_entities.back(), m_entities.pop_back();
            for (BrushParentMap::iterator it = m_removedBrushes.begin(); it != m_removedBrushes.end(); ++it) delete it->first;
            for (BrushParentMap::iterator it = m_movedBrushes.begin(); it != m_movedBrushes.end(); ++it) delete it->first;
        }
        
        void RestoreObjectsUndoItem::undo() {
            m_map.restoreObjects(m_entities, m_removedBrushes, m_movedBrushes);
            
            // make sure that the objects are not freed when the undo item is destroyed
            m_entities.clear();
            m_removedBrushes.clear();
            m_movedBrushes.clear();
        }
    }
}