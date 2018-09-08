/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

        void Snapshot::restoreNodes(const vm::bbox3& worldBounds) {
            for (NodeSnapshot* snapshot : m_nodeSnapshots)
                snapshot->restore(worldBounds);
        }

        void Snapshot::restoreBrushFaces() {
            for (BrushFaceSnapshot* snapshot : m_brushFaceSnapshots)
                snapshot->restore();
        }

        void Snapshot::takeSnapshot(Node* node) {
            NodeSnapshot* snapshot = node->takeSnapshot();
            if (snapshot != nullptr)
                m_nodeSnapshots.push_back(snapshot);
        }

        void Snapshot::takeSnapshot(BrushFace* face) {
            BrushFaceSnapshot* snapshot = face->takeSnapshot();
            if (snapshot != nullptr)
                m_brushFaceSnapshots.push_back(snapshot);
        }
    }
}
