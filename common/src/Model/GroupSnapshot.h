/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_GroupSnapshot
#define TrenchBroom_GroupSnapshot

#include "Model/ModelTypes.h"
#include "Model/NodeSnapshot.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Group;
        
        class GroupSnapshot : public NodeSnapshot {
        private:
            NodeSnapshotList m_snapshots;
        public:
            GroupSnapshot(Group* group);
            ~GroupSnapshot();
        private:
            void takeSnapshot(Group* group);
            void doRestore(const BBox3& worldBounds);
        };
    }
}

#endif /* defined(TrenchBroom_GroupSnapshot) */
