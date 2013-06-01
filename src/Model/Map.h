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
#include "Model/EntityTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Map;
        typedef std::tr1::shared_ptr<Map> MapPtr;
        
        class Map {
        private:
            EntityList m_entities;
            EntityPtr m_worldspawn;
            
            Map();
        public:
            static MapPtr newMap();
            ~Map();
            
            inline const EntityList& entities() const {
                return m_entities;
            }
            
            void addEntity(EntityPtr entity);
            
            EntityPtr worldspawn();
        private:
            EntityPtr findWorldspawn() const;
        };
    }
}

#endif /* defined(__TrenchBroom__Map__) */
