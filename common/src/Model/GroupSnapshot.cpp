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

#include "GroupSnapshot.h"

#include "CollectionUtils.h"
#include "Model/Group.h"
#include "Model/ModelTypes.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/TakeSnapshotVisitor.h"

namespace TrenchBroom {
    namespace Model {
        GroupSnapshot::GroupSnapshot(Group* group) :
        m_group(group) {
            takeSnapshot(group);
        }

        GroupSnapshot::~GroupSnapshot() {
            VectorUtils::clearAndDelete(m_snapshots);
        }

        void GroupSnapshot::takeSnapshot(Group* group) {
            const NodeList& children = group->children();
            
            TakeSnapshotVisitor visitor;
            Node::acceptAndRecurse(children.begin(), children.end(), visitor);
            m_snapshots = visitor.result();
        }
        
        void GroupSnapshot::doRestore(const BBox3& worldBounds) {
            SnapshotList::const_iterator it, end;
            for (it = m_snapshots.begin(), end = m_snapshots.end(); it != end; ++it) {
                ObjectSnapshot* snapshot = *it;
                snapshot->restore(worldBounds);
            }
        }
    }
}
