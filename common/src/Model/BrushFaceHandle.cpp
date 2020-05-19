/*
Copyright (C) 2020 Kristian Duske

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

#include "BrushFaceHandle.h"

#include "Ensure.h"
#include "Model/BrushNode.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace Model {
        BrushFaceHandle::BrushFaceHandle(BrushNode* node, BrushFace* face) :
        m_node(node),
        m_face(face) {
            assert(m_node != nullptr);
            assert(m_face != nullptr);
            ensure(kdl::vec_contains(m_node->faces(), face), "face must belong to node");
        }

        BrushNode* BrushFaceHandle::node() const {
            return m_node;
        }

        BrushFace* BrushFaceHandle::face() const {
            return m_face;
        }

        bool operator==(const BrushFaceHandle& lhs, const BrushFaceHandle& rhs) {
            return lhs.m_node == rhs.m_node && lhs.m_face == rhs.m_face;
        }

        std::vector<BrushFace*> toFaces(const std::vector<BrushFaceHandle>& handles) {
            return kdl::vec_transform(handles, [](const auto& handle) { return handle.face(); });
        }

        std::vector<BrushFaceHandle> toHandles(BrushNode* brush) {
            return kdl::vec_transform(brush->faces(), [&](auto* face) { return BrushFaceHandle(brush, face); });
        }
    }
}
