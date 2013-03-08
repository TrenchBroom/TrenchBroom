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

#ifndef TrenchBroom_BrushTypes_h
#define TrenchBroom_BrushTypes_h

#include "Model/FaceTypes.h"

#include <map>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Entity;
        class Brush;
        
        typedef std::set<Brush*> BrushSet;
        static const BrushSet EmptyBrushSet;
        typedef std::vector<Brush*> BrushList;
        static const BrushList EmptyBrushList;
        typedef std::map<Brush*, Entity*> BrushParentMap;
        typedef std::pair<Brush*, Entity*> BrushParentMapEntry;
        typedef std::pair<BrushParentMap::iterator, bool> BrushParentMapInsertResult;
        typedef std::map<Entity*, BrushList> EntityBrushesMap;
        typedef std::pair<Entity*, BrushList> EntityBrushesMapEntry;
        typedef std::pair<EntityBrushesMap::iterator, bool> EntityBrushesMapInsertResult;

        class BrushFunctor {
        public:
            virtual ~BrushFunctor() {}
            virtual void operator()(const Model::Brush& brush) = 0;
        };
    }
}

#endif
