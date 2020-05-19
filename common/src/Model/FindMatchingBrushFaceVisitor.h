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

#ifndef TrenchBroom_FindMatchingBrushFaceVisitor
#define TrenchBroom_FindMatchingBrushFaceVisitor

#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        template <typename P>
        class FindMatchingBrushFaceVisitor : public ConstNodeVisitor, public NodeQuery<const BrushFace*> {
        private:
            P m_p;
        public:
            FindMatchingBrushFaceVisitor(const P& p = P()) : m_p(p) {}
        private:
            void doVisit(const WorldNode*)  override {}
            void doVisit(const LayerNode*)  override {}
            void doVisit(const GroupNode*)  override {}
            void doVisit(const EntityNode*) override {}
            void doVisit(const BrushNode* brushNode)   override {
                const Brush& brush = brushNode->brush();
                for (const BrushFace& face : brush.faces()) {
                    if (m_p(brushNode, face)) {
                        setResult(&face);
                        cancel();
                        return;
                    }
                }
            }
        };
    }
}

#endif /* defined(TrenchBroom_FindMatchingBrushFaceVisitor) */
