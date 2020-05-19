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

#ifndef TrenchBroom_CollectMatchingBrushFacesVisitor
#define TrenchBroom_CollectMatchingBrushFacesVisitor

#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushFacePredicates.h"
#include "Model/BrushNode.h"
#include "Model/NodeVisitor.h"

#include <tuple>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        template <typename P>
        class CollectMatchingBrushFacesVisitor : public NodeVisitor {
        private:
            P m_p;
            std::vector<BrushFaceHandle> m_faces;
        public:
            CollectMatchingBrushFacesVisitor(const P& p = P()) : m_p(p) {}
            const std::vector<BrushFaceHandle>& faces() const { return m_faces; }
        private:
            void doVisit(WorldNode*)  override {}
            void doVisit(LayerNode*)  override {}
            void doVisit(GroupNode*)  override {}
            void doVisit(EntityNode*) override {}
            void doVisit(BrushNode* brush) override {
                for (BrushFace* face : brush->faces()) {
                    if (m_p(face)) {
                        m_faces.push_back(BrushFaceHandle(brush, face));
                    }
                }
            }
        };

        using CollectBrushFacesVisitor = CollectMatchingBrushFacesVisitor<BrushFacePredicates::True>;
    }
}

#endif /* defined(TrenchBroom_CollectMatchingBrushFacesVisitor) */
