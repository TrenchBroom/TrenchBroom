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
            
            inline const Entity::List& entities() const {
                return m_entities;
            }
            
            void addEntity(Entity::Ptr entity);
            
            Entity::Ptr worldspawn();
        private:
            Entity::Ptr findWorldspawn() const;
        };
    }
}

#endif /* defined(__TrenchBroom__Map__) */
