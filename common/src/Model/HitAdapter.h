/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_HitAdapter
#define TrenchBroom_HitAdapter

#include <stdio.h>

namespace TrenchBroom {
    namespace Model {
        class Hit;
        
        class Brush;
        class BrushFace;
        class Entity;
        class Group;
        class Object;
        class Node;
        
        Node* hitToNode(const Hit& hit);
        Object* hitToObject(const Hit& hit);
        Group* hitToGroup(const Hit& hit);
        Entity* hitToEntity(const Hit& hit);
        Brush* hitToBrush(const Hit& hit);
        BrushFace* hitToFace(const Hit& hit);
    }
}

#endif /* defined(TrenchBroom_HitAdapter) */
