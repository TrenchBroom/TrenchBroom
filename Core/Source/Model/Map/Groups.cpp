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

#include "Groups.h"
#include <cstdlib>
#include <algorithm>
#include "Model/Map/Entity.h"
#include "Model/Map/Map.h"

namespace TrenchBroom {
    namespace Model {
        void GroupManager::entitesAdded(const vector<Entity*>& entities) {
            bool changed = false;
            for (int i = 0; i < entities.size(); i++) {
                if (entities[i]->group()) {
                    m_groups.push_back(entities[i]);
                    if (visible(*entities[i]))
                        m_visibleGroupCount++;
                    changed |= true;
                }
            }
            if (changed)
                groupsChanged(*this);
        }

        void GroupManager::entitiesRemoved(const vector<Entity*>& entities) {
            bool changed = false;
            for (int i = 0; i < entities.size(); i++) {
                if (entities[i]->group()) {
                    if (visible(*entities[i]))
                        m_visibleGroupCount--;
                    m_groups.erase(find(m_groups.begin(), m_groups.end(), entities[i]));
                    changed |= true;
                }
            }
            if (changed)
                groupsChanged(*this);
        }

        void GroupManager::brushesChanged(const vector<Brush*>& brushes) {
            bool changed = false;
            for (int i = 0; i < brushes.size(); i++) {
                if (brushes[i]->entity()->group()) {
                    changed = true;
                    break;
                }
            }
            if (changed)
                groupsChanged(*this);
        }

        void GroupManager::mapLoaded(Map& map) {
            const vector<Entity*>& entities = map.entities();
            for (int i = 0; i < entities.size(); i++) {
                if (entities[i]->group()) {
                    m_groups.push_back(entities[i]);
                    if (visible(*entities[i]))
                        m_visibleGroupCount++;
                }
            }
            groupsChanged(*this);
        }

        void GroupManager::mapCleared(Map& map) {
            m_groups.clear();
            m_visibleGroupCount = 0;
            groupsChanged(*this);
        }

        GroupManager::GroupManager(Map& map) : m_map(map), m_visibleGroupCount(0) {
            m_map.entitiesWereAdded     += new Model::Map::EntityEvent::Listener<GroupManager>(this, &GroupManager::entitesAdded);
            m_map.entitiesWillBeRemoved += new Model::Map::EntityEvent::Listener<GroupManager>(this, &GroupManager::entitiesRemoved);
            m_map.brushesDidChange      += new Model::Map::BrushEvent::Listener<GroupManager>(this, &GroupManager::brushesChanged);
            m_map.mapLoaded             += new Model::Map::MapEvent::Listener<GroupManager>(this, &GroupManager::mapLoaded);
            m_map.mapCleared            += new Model::Map::MapEvent::Listener<GroupManager>(this, &GroupManager::mapCleared);
        }

        GroupManager::~GroupManager() {
            m_map.entitiesWereAdded     -= new Model::Map::EntityEvent::Listener<GroupManager>(this, &GroupManager::entitesAdded);
            m_map.entitiesWillBeRemoved -= new Model::Map::EntityEvent::Listener<GroupManager>(this, &GroupManager::entitiesRemoved);
            m_map.brushesDidChange      -= new Model::Map::BrushEvent::Listener<GroupManager>(this, &GroupManager::brushesChanged);
            m_map.mapLoaded             -= new Model::Map::MapEvent::Listener<GroupManager>(this, &GroupManager::mapLoaded);
            m_map.mapCleared            -= new Model::Map::MapEvent::Listener<GroupManager>(this, &GroupManager::mapCleared);
        }

        const vector<Entity*>& GroupManager::groups() const {
            return m_groups;
        }

        void GroupManager::setGroupName(Entity& group, const string& name) {
            group.setProperty(GroupNameKey, name);
            groupsChanged(*this);
        }

        void GroupManager::setGroupVisibility(Entity& group, bool visibility) {
            if (visibility == visible(group)) return;
            group.setProperty(GroupNameKey, "" + visibility);
            if (visibility) m_visibleGroupCount++;
            else m_visibleGroupCount--;
            groupsChanged(*this);
        }

        bool GroupManager::visible(const Entity& group) const {
            const string* value = group.propertyForKey(GroupVisibilityKey);
            if (value == NULL)
                return false;
            return atoi(value->c_str()) != 0;
        }

        bool GroupManager::allGroupsVisible() const {
            return m_visibleGroupCount == 0;
        }
    }
}
