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
            BBox m_worldBounds;
            bool m_forceIntegerFacePoints;
            EntityList m_entities;
            Entity* m_worldspawn;
        public:
            Map(const BBox& worldBounds, bool forceIntegerFacePoints);
            ~Map();
            
            inline const BBox& worldBounds() const {
                return m_worldBounds;
            }
            
            inline bool forceIntegerFacePoints() const {
                return m_forceIntegerFacePoints;
            }
            
            void setForceIntegerFacePoints(bool forceIntegerFacePoints);
            
            void addEntity(Entity& entity);
            void removeEntity(Entity& entity);

            inline const EntityList& entities() const {
                return m_entities;
            }
            
            Entity* worldspawn();
            
            void clear();
        };
    }
}

#endif /* defined(__TrenchBroom__Map__) */
