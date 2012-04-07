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

#ifndef TrenchBroom_Groups_h
#define TrenchBroom_Groups_h

#include <vector>
#include "Observer.h"

using namespace std;

namespace TrenchBroom {
    namespace Model {
        static const string GroupsChanged = "GroupsChanged";
        
        class Entity;
        class Map;
        class GroupManager : protected Observer, public Observable {
        private:
            Map& m_map;
            vector<Entity*> m_groups;
            int m_visibleGroupCount;
        protected:
            void notify(const string &name, const void *data);
        public:
            GroupManager(Map& map);
            ~GroupManager();
            const vector<Entity*>& groups() const;
            void setGroupName(Entity& group, const string& name);
            void setGroupVisibility(Entity& group, bool visibility);
            bool visible(const Entity& group) const;
            bool allGroupsVisible() const;
        };
    }
}
#endif
