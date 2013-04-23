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

#include "Map.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Utility/List.h"

namespace TrenchBroom {
    namespace Model {
        void Map::addEntityTargetname(Entity& entity, const String* targetname) {
            if (targetname != NULL && !targetname->empty())
                m_entitiesWithTargetname[*targetname].insert(&entity);
        }
        
        void Map::removeEntityTargetname(Entity& entity, const String* targetname) {
            if (targetname != NULL && !targetname->empty()) {
                typedef TargetnameEntityMap::iterator MapIt;
                MapIt it = m_entitiesWithTargetname.find(*targetname);
                if (it != m_entitiesWithTargetname.end()) {
                    it->second.erase(&entity);
                    if (it->second.empty())
                        m_entitiesWithTargetname.erase(it);
                }
            }
        }

        void Map::addEntityTarget(Entity& entity, const String* targetname) {
            if (targetname != NULL && !targetname->empty())
                m_entitiesWithTarget[*targetname].insert(&entity);
        }
        
        void Map::removeEntityTarget(Entity& entity, const String* targetname) {
            if (targetname != NULL && !targetname->empty()) {
                typedef TargetnameEntityMap::iterator MapIt;
                MapIt it = m_entitiesWithTarget.find(*targetname);
                if (it != m_entitiesWithTarget.end()) {
                    it->second.erase(&entity);
                    if (it->second.empty())
                        m_entitiesWithTarget.erase(it);
                }
            }
        }
        
        void Map::addEntityTargets(Entity& entity) {
            const StringList targetnames = entity.linkTargetnames();
            StringList::const_iterator it, end;
            for (it = targetnames.begin(), end = targetnames.end(); it != end; ++it)
                addEntityTarget(entity, &*it);
        }
        
        void Map::removeEntityTargets(Entity& entity) {
            const StringList targetnames = entity.linkTargetnames();
            StringList::const_iterator it, end;
            for (it = targetnames.begin(), end = targetnames.end(); it != end; ++it)
                removeEntityTarget(entity, &*it);
        }

        void Map::addEntityKillTarget(Entity& entity, const String* targetname) {
            if (targetname != NULL && !targetname->empty())
                m_entitiesWithKillTarget[*targetname].insert(&entity);
        }
        
        void Map::removeEntityKillTarget(Entity& entity, const String* targetname) {
            if (targetname != NULL && !targetname->empty()) {
                typedef TargetnameEntityMap::iterator MapIt;
                MapIt it = m_entitiesWithKillTarget.find(*targetname);
                if (it != m_entitiesWithKillTarget.end()) {
                    it->second.erase(&entity);
                    if (it->second.empty())
                        m_entitiesWithKillTarget.erase(it);
                }
            }
        }

        void Map::addEntityKillTargets(Entity& entity) {
            const StringList targetnames = entity.killTargetnames();
            StringList::const_iterator it, end;
            for (it = targetnames.begin(), end = targetnames.end(); it != end; ++it)
                addEntityKillTarget(entity, &*it);
        }
        
        void Map::removeEntityKillTargets(Entity& entity) {
            const StringList targetnames = entity.killTargetnames();
            StringList::const_iterator it, end;
            for (it = targetnames.begin(), end = targetnames.end(); it != end; ++it)
                removeEntityKillTarget(entity, &*it);
        }

        Map::Map(const BBoxf& worldBounds, bool forceIntegerFacePoints) :
        m_worldBounds(worldBounds),
        m_forceIntegerFacePoints(forceIntegerFacePoints),
        m_worldspawn(NULL) {}

        Map::~Map() {
            clear();
        }

        void Map::setForceIntegerFacePoints(bool forceIntegerFacePoints) {
            EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                const Model::BrushList& brushes = entity.brushes();
                BrushList::const_iterator brushIt, brushEnd;
                for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                    Model::Brush& brush = **brushIt;
                    brush.setForceIntegerFacePoints(forceIntegerFacePoints);
                }
            }
            
            m_forceIntegerFacePoints = forceIntegerFacePoints;
        }

        void Map::addEntity(Entity& entity) {
            if (!entity.worldspawn() || worldspawn() == NULL) {
                m_entities.push_back(&entity);
                addEntityTargetname(entity, entity.propertyForKey(Entity::TargetnameKey));
                addEntityTargets(entity);
                addEntityKillTargets(entity);
                entity.setMap(this);
            }
        }
        
        void Map::removeEntity(Entity& entity) {
            if (entity.worldspawn())
                m_worldspawn = NULL;
            entity.setMap(NULL);
            removeEntityTargetname(entity, entity.propertyForKey(Entity::TargetnameKey));
            removeEntityTargets(entity);
            removeEntityKillTargets(entity);
            Utility::erase(m_entities, &entity);
        }

        EntityList Map::entitiesWithTargetname(const String& targetname) const {
            typedef TargetnameEntityMap::const_iterator MapIt;
            MapIt it = m_entitiesWithTargetname.find(targetname);
            if (it == m_entitiesWithTargetname.end())
                return EmptyEntityList;
            return Utility::makeList(it->second);
        }
        
        void Map::updateEntityTargetname(Entity& entity, const String* newTargetname, const String* oldTargetname) {
            removeEntityTargetname(entity, oldTargetname);
            addEntityTargetname(entity, newTargetname);
        }

        
        EntityList Map::entitiesWithTarget(const String& targetname) const {
            typedef TargetnameEntityMap::const_iterator MapIt;
            MapIt it = m_entitiesWithTarget.find(targetname);
            if (it == m_entitiesWithTarget.end())
                return EmptyEntityList;
            return Utility::makeList(it->second);
        }
        
        void Map::updateEntityTarget(Entity& entity, const String* newTargetname, const String* oldTargetname) {
            removeEntityTarget(entity, oldTargetname);
            addEntityTarget(entity, newTargetname);
        }
        
        EntityList Map::entitiesWithKillTarget(const String& targetname) const {
            typedef TargetnameEntityMap::const_iterator MapIt;
            MapIt it = m_entitiesWithKillTarget.find(targetname);
            if (it == m_entitiesWithKillTarget.end())
                return EmptyEntityList;
            return Utility::makeList(it->second);
        }
        
        void Map::updateEntityKillTarget(Entity& entity, const String* newTargetname, const String* oldTargetname) {
            removeEntityKillTarget(entity, oldTargetname);
            addEntityKillTarget(entity, newTargetname);
        }

        Entity* Map::worldspawn() {
            for (unsigned int i = 0; i < m_entities.size() && m_worldspawn == NULL; i++) {
                Entity* entity = m_entities[i];
                if (entity->worldspawn())
                    m_worldspawn = entity;
            }
            
            return m_worldspawn;
        }

        void Map::clear() {
            m_entitiesWithTargetname.clear();
            m_entitiesWithTarget.clear();
            m_entitiesWithKillTarget.clear();
            Utility::deleteAll(m_entities);
            m_worldspawn = NULL;
        }
    }
}
