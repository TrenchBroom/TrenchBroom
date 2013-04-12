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

#include <map>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Entity;
        
        class Map {
        protected:
            typedef std::map<String, EntitySet> TargetnameEntityMap;
            
            BBox m_worldBounds;
            bool m_forceIntegerFacePoints;
            EntityList m_entities;
            TargetnameEntityMap m_entitiesWithTargetname;
            TargetnameEntityMap m_entitiesWithTarget;
            TargetnameEntityMap m_entitiesWithKillTarget;
            Entity* m_worldspawn;
            
            void addEntityTargetname(Entity& entity, const String* targetname);
            void removeEntityTargetname(Entity& entity, const String* targetname);

            void addEntityTarget(Entity& entity, const String* targetname);
            void removeEntityTarget(Entity& entity, const String* targetname);
            void addEntityTargets(Entity& entity);
            void removeEntityTargets(Entity& entity);
            
            void addEntityKillTarget(Entity& entity, const String* targetname);
            void removeEntityKillTarget(Entity& entity, const String* targetname);
            void addEntityKillTargets(Entity& entity);
            void removeEntityKillTargets(Entity& entity);
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

            EntityList entitiesWithTargetname(const String& targetname) const;
            void updateEntityTargetname(Entity& entity, const String* newTargetname, const String* oldTargetname);
            
            EntityList entitiesWithTarget(const String& targetname) const;
            void updateEntityTarget(Entity& entity, const String* newTargetname, const String* oldTargetname);
            
            EntityList entitiesWithKillTarget(const String& targetname) const;
            void updateEntityKillTarget(Entity& entity, const String* newTargetname, const String* oldTargetname);
            
            inline const EntityList& entities() const {
                return m_entities;
            }
            
            Entity* worldspawn();
            
            void clear();
        };
    }
}

#endif /* defined(__TrenchBroom__Map__) */
