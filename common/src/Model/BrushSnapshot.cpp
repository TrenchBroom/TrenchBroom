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

#include "BrushSnapshot.h"

#include "Model/BrushNode.h"
#include "Model/BrushFace.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace Model {
        BrushSnapshot::BrushSnapshot(BrushNode* brushNode) :
        m_brushNode(brushNode) {
            const Brush& brush = m_brushNode->brush();
            for (BrushFace* face : brush.faces()) {
                BrushFace* faceClone = face->clone();
                faceClone->setTexture(nullptr);
                m_faces.push_back(faceClone);
            }
        }

        BrushSnapshot::~BrushSnapshot() {
            kdl::vec_clear_and_delete(m_faces);
        }

        void BrushSnapshot::doRestore(const vm::bbox3& worldBounds) {
            m_brushNode->setBrush(Brush(worldBounds, m_faces));
            m_faces.clear();
        }
    }
}
