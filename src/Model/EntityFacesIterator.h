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

#ifndef TrenchBroom_EntityFacesIterator_h
#define TrenchBroom_EntityFacesIterator_h

#include "NestedIterator.h"
#include "Model/BrushFacesIterator.h"
#include "Model/ModelTypes.h"
#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        struct EntityFacesIterator {
            typedef BrushFacesIterator::OuterIterator InnerIterator;
            typedef NestedIterator<EntityList::const_iterator, EntityFacesIterator> OuterIterator;
            
            static bool isInnerEmpty(EntityList::const_iterator it) {
                Entity* entity = *it;
                return entity->brushes().empty();
            }

            static OuterIterator begin(const EntityList& entities) {
                return OuterIterator(entities.begin(), entities.end());
            }
            
            static OuterIterator end(const EntityList& entities) {
                return OuterIterator(entities.end());
            }
            
            static InnerIterator beginInner(EntityList::const_iterator it) {
                Entity* entity = *it;
                return BrushFacesIterator::begin(entity->brushes());
            }
            
            static InnerIterator endInner(EntityList::const_iterator it) {
                Entity* entity = *it;
                return BrushFacesIterator::end(entity->brushes());
            }
        };
    }
}

#endif
