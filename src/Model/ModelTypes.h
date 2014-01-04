/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef TrenchBroom_ModelTypes_h
#define TrenchBroom_ModelTypes_h

#include "StringUtils.h"
#include "SharedPointer.h"
#include <map>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        namespace MapFormat {
            typedef size_t Type;
            static const Type Unknown = 1 << 0;
            static const Type Quake   = 1 << 1;
            static const Type Quake2  = 1 << 2;
            static const Type Valve   = 1 << 3;
            static const Type Hexen2  = 1 << 4;
        }
        
        class Map;
        
        typedef String PropertyKey;
        typedef std::vector<PropertyKey> PropertyKeyList;
        typedef std::set<PropertyKey> PropertyKeySet;
        typedef String PropertyValue;
        typedef std::vector<PropertyValue> PropertyValueList;

        class BrushFace;
        typedef std::vector<BrushFace*> BrushFaceList;
        static const BrushFaceList EmptyBrushFaceList(0);
        typedef std::set<BrushFace*> BrushFaceSet;
        
        class Object;
        typedef std::vector<Object*> ObjectList;
        static const ObjectList EmptyObjectList(0);
        typedef std::set<Object*> ObjectSet;
        
        struct ObjectParentPair {
            Object* object;
            Object* parent;
            
            ObjectParentPair(Object* i_object, Object* i_parent);
            ObjectParentPair(Object* i_object);
        };
        
        typedef std::vector<ObjectParentPair> ObjectParentList;
        typedef std::map<Object*, ObjectList> ObjectChildrenMap;
        
        class Brush;
        typedef std::vector<Brush*> BrushList;
        static const BrushList EmptyBrushList(0);
        typedef std::set<Brush*> BrushSet;
        
        class Entity;
        typedef std::vector<Entity*> EntityList;
        static const EntityList EmptyEntityList(0);
        typedef std::set<Entity*> EntitySet;
        static const EntitySet EmptyEntitySet;
        
        struct EntityProperty;
        
        typedef std::map<Entity*, BrushList> EntityBrushesMap;
        typedef std::map<Brush*, Entity*> BrushEntityMap;
        
        class Pickable;
        typedef std::vector<Pickable*> PickableList;
        
        class Game;
        typedef std::tr1::shared_ptr<Game> GamePtr;
    }
}

#endif
