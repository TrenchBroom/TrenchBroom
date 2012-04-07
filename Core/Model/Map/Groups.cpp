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
#include "Map.h"
#include "Entity.h"
#include <cstdlib>

namespace TrenchBroom {
    namespace Model {
        void GroupManager::notify(const string &name, const void *data) {
            bool changed = false;
            if (name == EntitiesAdded) {
                const vector<Entity*>& entities = *(const vector<Entity*>*)data;
                for (int i = 0; i < entities.size(); i++) {
                    if (entities[i]->group()) {
                        m_groups.push_back(entities[i]);
                        if (visible(*entities[i]))
                            m_visibleGroupCount++;
                        changed |= true;
                    }
                }
            } else if (name == EntitiesWillBeRemoved) {
                const vector<Entity*>& entities = *(const vector<Entity*>*)data;
                for (int i = 0; i < entities.size(); i++) {
                    if (entities[i]->group()) {
                        if (visible(*entities[i]))
                            m_visibleGroupCount--;
                        m_groups.erase(find(m_groups.begin(), m_groups.end(), entities[i]));
                        changed |= true;
                    }
                }
            } else if (name == BrushesDidChange) {
                const vector<Brush*>& brushes = *(const vector<Brush*>*)data;
                for (int i = 0; i < brushes.size(); i++) {
                    if (brushes[i]->entity()->group()) {
                        changed = true;
                        break;
                    }
                }
            } else if (name == MapCleared) {
                m_groups.clear();
                m_visibleGroupCount = 0;
            } else if (name == MapLoaded) {
                const vector<Entity*>& entities = m_map.entities();
                for (int i = 0; i < entities.size(); i++) {
                    if (entities[i]->group()) {
                        m_groups.push_back(entities[i]);
                        if (visible(*entities[i]))
                            m_visibleGroupCount++;
                    }
                }
            }
            
            if (changed)
                postNotification(GroupsChanged, NULL);
        }
        
        GroupManager::GroupManager(Map& map) : Observer(), m_map(map), m_visibleGroupCount(0) {
            m_map.addObserver(EntitiesAdded, *this);
            m_map.addObserver(EntitiesWillBeRemoved, *this);
            m_map.addObserver(BrushesDidChange, *this);
            m_map.addObserver(MapCleared, *this);
            m_map.addObserver(MapLoaded, *this);
        }
        
        GroupManager::~GroupManager() {
            m_map.removeObserver(*this);
        }
        
        const vector<Entity*>& GroupManager::groups() const {
            return m_groups;
        }
        
        void GroupManager::setGroupName(Entity& group, const string& name) {
            group.setProperty(GroupNameKey, name);
            postNotification(GroupsChanged, NULL);
        }
        
        void GroupManager::setGroupVisibility(Entity& group, bool visibility) {
            if (visibility == visible(group)) return;
            group.setProperty(GroupNameKey, "" + visibility);
            if (visibility) m_visibleGroupCount++;
            else m_visibleGroupCount--;
            postNotification(GroupsChanged, NULL);
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