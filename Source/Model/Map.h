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

#ifndef __TrenchBroom__Map__
#define __TrenchBroom__Map__

#include "Model/EntityTypes.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Entity;
        
        class Map {
        protected:
            EntityList m_entities;
            Entity* m_worldspawn;
            BBox m_worldBounds;

            void setEntityDefinition(Entity* entity);
        public:
            Map(const BBox& worldBounds);
            ~Map();
            
            void addEntity(Entity* entity);

            inline const EntityList& entities() const {
                return m_entities;
            }
            
            Entity* createEntity(const PropertyValue& classname);
            
            Entity* worldspawn(bool create);
            
            inline const BBox& worldBounds() const {
                return m_worldBounds;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Map__) */
