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

#ifndef TrenchBroom_MapFacesIterator_h
#define TrenchBroom_MapFacesIterator_h

#include "NestedIterator.h"
#include "Model/EntityFacesIterator.h"
#include "Model/ModelTypes.h"
#include "Model/Map.h"

namespace TrenchBroom {
    namespace Model {
        struct MapFacesIterator {
            typedef BrushFacesIterator::OuterIterator InnerIterator;
            typedef NestedIterator<EntityList::const_iterator, MapFacesIterator> OuterIterator;
            
            static OuterIterator begin(const Map& map) {
                return OuterIterator(map.entities().begin(), map.entities().end());
            }
            
            static OuterIterator end(const Map& map) {
                return OuterIterator(map.entities().end());
            }
            
            InnerIterator beginInner(Model::EntityList::const_iterator it) {
                Entity* entity = *it;
                return BrushFacesIterator::begin(entity->brushes());
            }
            
            InnerIterator endInner(Model::EntityList::const_iterator it) {
                Entity* entity = *it;
                return BrushFacesIterator::end(entity->brushes());
            }
        };
    }
}

#endif
