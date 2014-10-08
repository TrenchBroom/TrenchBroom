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

#include "Snapshot.h"

#include "CollectionUtils.h"
#include "Model/BrushFaceSnapshot.h"
#include "Model/Node.h"
#include "Model/NodeSnapshot.h"

namespace TrenchBroom {
    namespace Model {
        Snapshot::~Snapshot() {
            VectorUtils::clearAndDelete(m_nodeSnapshots);
            VectorUtils::clearAndDelete(m_brushFaceSnapshots);
        }

        void Snapshot::restoreNodes(const BBox3& worldBounds) {
            NodeSnapshotList::const_iterator it, end;
            for (it = m_nodeSnapshots.begin(), end = m_nodeSnapshots.end(); it != end; ++it) {
                NodeSnapshot* snapshot = *it;
                snapshot->restore(worldBounds);
            }
        }

        void Snapshot::restoreBrushFaces() {
            BrushFaceSnapshotList::const_iterator it, end;
            for (it = m_brushFaceSnapshots.begin(), end = m_brushFaceSnapshots.end(); it != end; ++it) {
                BrushFaceSnapshot* snapshot = *it;
                snapshot->restore();
            }
        }

        void Snapshot::takeSnapshot(Node* node) {
            NodeSnapshot* snapshot = node->takeSnapshot();
            if (snapshot != NULL)
                m_nodeSnapshots.push_back(snapshot);
        }

        void Snapshot::takeSnapshot(BrushFace* face) {
            BrushFaceSnapshot* snapshot = face->takeSnapshot();
            if (snapshot != NULL)
                m_brushFaceSnapshots.push_back(snapshot);
        }
    }
}
