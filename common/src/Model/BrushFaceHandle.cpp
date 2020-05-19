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
#include "Model/Brush.h"
#include "Model/BrushNode.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace Model {
        BrushFaceHandle::BrushFaceHandle(BrushNode* node, const size_t faceIndex) :
        m_node(node),
        m_faceIndex(faceIndex) {
            assert(m_node != nullptr);
            ensure(m_faceIndex < m_node->brush().faceCount(), "face index must be valid");
        }

        BrushNode* BrushFaceHandle::node() const {
            return m_node;
        }

        size_t BrushFaceHandle::faceIndex() const {
            return m_faceIndex;
        }
        
        BrushFace* BrushFaceHandle::face() const {
            return m_node->brush().face(m_faceIndex);
        }

        bool operator==(const BrushFaceHandle& lhs, const BrushFaceHandle& rhs) {
            return lhs.m_node == rhs.m_node && lhs.m_faceIndex == rhs.m_faceIndex;
        }

        std::vector<BrushNode*> toNodes(const std::vector<BrushFaceHandle>& handles) {
            return kdl::vec_transform(handles, [](const auto& handle) { return handle.node(); });
        }

        std::vector<BrushFace*> toFaces(const std::vector<BrushFaceHandle>& handles) {
            return kdl::vec_transform(handles, [](const auto& handle) { return handle.face(); });
        }

        std::vector<BrushFaceHandle> toHandles(BrushNode* brushNode) {
            std::vector<BrushFaceHandle> result;
            result.reserve(brushNode->brush().faceCount());
            for (size_t i = 0u; i < brushNode->brush().faceCount(); ++i) {
                result.emplace_back(brushNode, i);
            }
            return result;
        }
    }
}
