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

#ifndef TrenchBroom_Snapshot
#define TrenchBroom_Snapshot

#include "FloatType.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class BrushFaceHandle;
        class BrushFaceSnapshot;
        class Node;
        class NodeSnapshot;

        class Snapshot {
        private:
            std::vector<NodeSnapshot*> m_nodeSnapshots;
            std::vector<BrushFaceSnapshot*> m_brushFaceSnapshots;
        public:
            template <typename I>
            Snapshot(I cur, I end) {
                while (cur != end) {
                    takeSnapshot(*cur);
                    ++cur;
                }
            }

            ~Snapshot();

            void restoreNodes(const vm::bbox3& worldBounds);
            void restoreBrushFaces();
        private:
            void takeSnapshot(Node* node);
            void takeSnapshot(const BrushFaceHandle& pair);
        private:
            Snapshot(const Snapshot&);
            Snapshot& operator=(const Snapshot&);
        };
    }
}

#endif /* defined(TrenchBroom_Snapshot) */
