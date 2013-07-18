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

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Map;
        
        class Selection {
        private:
            Map* m_map;
        public:
            Selection(Map* map = NULL);

            ObjectList selectedObjects() const;
            EntityList selectedEntities() const;
            BrushList selectedBrushes() const;
            BrushFaceList selectedFaces() const;
            void selectObjects(const ObjectList& objects);
            void deselectObjects(const ObjectList& objects);
            void selectAllObjects();
            void selectFaces(const BrushFaceList& faces);
            void deselectFaces(const BrushFaceList& faces);
            void deselectAll();
        private:
            void deselectAllObjects();
            void deselectAllFaces();
        };
    }
}

#endif /* defined(__TrenchBroom__Selection__) */
