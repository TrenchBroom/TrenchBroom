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

#ifndef TrenchBroom_UndoItem_h
#define TrenchBroom_UndoItem_h

#include "Model/Map/BrushTypes.h"
#include "Model/Map/FaceTypes.h"
#include "Model/Map/EntityTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Map;
        class Entity;
        class Brush;
        class Face;
        
        class UndoItem {
        protected:
            Map& m_map;
        public:
            UndoItem(Map& map) : m_map(map) {}
            virtual ~UndoItem() {}
            virtual void undo() = 0;
        };
        
        class SelectionUndoItem : public UndoItem {
        protected:
            EntityList m_selectedEntities;
            BrushList m_selectedBrushes;
            FaceList m_selectedFaces;
        public:
            SelectionUndoItem(Map& map);
            virtual ~SelectionUndoItem() {}
            virtual void undo();
            virtual void performUndo() = 0;
        };
    }
}

#endif
