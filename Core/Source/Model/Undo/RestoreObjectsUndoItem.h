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

#ifndef TrenchBroom_RestoreObjectsUndoItem_h
#define TrenchBroom_RestoreObjectsUndoItem_h

#include "Model/Undo/UndoItem.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Map;
        class Entity;
        class Brush;
        
        class RestoreObjectsUndoItem : public UndoItem {
        protected:
            std::vector<Entity*> m_entities;
            BrushList m_brushes;
        public:
            RestoreObjectsUndoItem(Map& map, const std::vector<Entity*>& entities, const BrushList& brushes);
            virtual ~RestoreObjectsUndoItem() {}
            
            virtual void undo();
        };
    }
}

#endif
