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

#ifndef __TrenchBroom__Map__
#define __TrenchBroom__Map__

#include "SharedPointer.h"
#include "Model/Entity.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        class Map {
        public:
            typedef std::tr1::shared_ptr<Map> Ptr;
        private:
            Entity::List m_entities;
            Entity::Ptr m_worldspawn;
            
            Map();
        public:
            static Ptr newMap();
            ~Map();
            
            const Entity::List& entities() const;
            void addEntity(Entity::Ptr entity);
            Entity::Ptr worldspawn();
            
            template <class Operator, class Filter>
            inline void eachBrushFace(Operator& op, Filter& filter) {
                Entity::List::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    Entity::Ptr entity = *it;
                    if (filter(entity))
                        entity->eachBrushFace(op, filter);
                }
            }
        private:
            Entity::Ptr findWorldspawn() const;
        };
    }
}

#endif /* defined(__TrenchBroom__Map__) */
