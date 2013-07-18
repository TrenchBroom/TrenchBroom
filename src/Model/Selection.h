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

#ifndef __TrenchBroom__Selection__
#define __TrenchBroom__Selection__

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        class Map;
        
        class Selection {
        private:
            Map* m_map;
        public:
            Selection(Map* map = NULL);

            Object::List selectedObjects() const;
            Entity::List selectedEntities() const;
            Brush::List selectedBrushes() const;
            BrushFace::List selectedFaces() const;
            void selectObjects(const Object::List& objects);
            void deselectObjects(const Object::List& objects);
            void selectAllObjects();
            void selectFaces(const BrushFace::List& faces);
            void deselectFaces(const BrushFace::List& faces);
            void deselectAll();
        private:
            void deselectAllObjects();
            void deselectAllFaces();
        };
    }
}

#endif /* defined(__TrenchBroom__Selection__) */
