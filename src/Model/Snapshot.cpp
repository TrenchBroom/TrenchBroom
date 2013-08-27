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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Snapshot.h"

namespace TrenchBroom {
    namespace Model {
        Snapshot::Snapshot() {}

        Snapshot::Snapshot(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity& entity = **it;
                m_entitySnapshots.push_back(entity.takeSnapshot());
            }
        }
        
        void Snapshot::restore() {
            EntitySnapshotList::iterator it, end;
            for (it = m_entitySnapshots.begin(), end = m_entitySnapshots.end(); it != end; ++it) {
                Model::EntitySnapshot& snapshot = *it;
                snapshot.restore();
            }
        }
    }
}
