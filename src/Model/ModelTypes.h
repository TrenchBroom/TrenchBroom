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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_ModelTypes_h
#define TrenchBroom_ModelTypes_h

#include "SharedPointer.h"
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        typedef std::vector<BrushFace*> BrushFaceList;
        static const BrushFaceList EmptyBrushFaceList;
        
        class Object;
        typedef std::vector<Object*> ObjectList;
        static const ObjectList EmptyObjectList;
        
        class Brush;
        typedef std::vector<Brush*> BrushList;
        static const BrushList EmptyBrushList;
        
        class Entity;
        typedef std::vector<Entity*> EntityList;
        static const EntityList EmptyEntityList;
        
        class Texture;
        typedef std::vector<Texture*> TextureList;
        
        class TextureCollection;
        typedef std::vector<TextureCollection*> TextureCollectionList;
        
        class Pickable;
        typedef std::vector<Pickable*> PickableList;
        
        class Game;
        typedef std::tr1::shared_ptr<Game> GamePtr;
    }
}

#endif
