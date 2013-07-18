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
        typedef std::tr1::shared_ptr<BrushFace> BrushFacePtr;
        typedef std::vector<BrushFacePtr> BrushFaceList;
        static const BrushFaceList EmptyBrushFaceList;
        
        class Object;
        typedef std::tr1::shared_ptr<Object> ObjectPtr;
        typedef std::vector<ObjectPtr> ObjectList;
        static const ObjectList EmptyObjectList;
        
        class Brush;
        typedef std::tr1::shared_ptr<Brush> BrushPtr;
        typedef std::vector<BrushPtr> BrushList;
        static const BrushList EmptyBrushList;
        
        class Entity;
        typedef std::tr1::shared_ptr<Entity> EntityPtr;
        typedef std::vector<EntityPtr> EntityList;
        static const EntityList EmptyEntityList;
        
        class Texture;
        typedef std::tr1::shared_ptr<Texture> TexturePtr;
        typedef std::vector<TexturePtr> TextureList;
        
        class TextureCollection;
        typedef std::tr1::shared_ptr<TextureCollection> TextureCollectionPtr;
        typedef std::vector<TextureCollectionPtr> TextureCollectionList;
        
        class Map;
        typedef std::tr1::shared_ptr<Map> MapPtr;
        
        class Game;
        typedef std::tr1::shared_ptr<Game> GamePtr;
    }
}

#endif
