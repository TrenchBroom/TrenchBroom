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

#include "Exceptions.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace Model {
        BrushSnapshot::BrushSnapshot(BrushNode* brushNode) :
        m_brushNode(brushNode) {
            const Brush& brush = m_brushNode->brush();
            for (const BrushFace& face : brush.faces()) {
                BrushFace& copy = m_faces.emplace_back(face);
                copy.setTexture(nullptr);
            }
        }

        kdl::result<void, SnapshotErrors> BrushSnapshot::doRestore(const vm::bbox3& worldBounds) {
            return Brush::create(worldBounds, std::move(m_faces))
                .visit(kdl::overload(
                    [&](Brush&& b) {
                        m_brushNode->setBrush(std::move(b));
                        return kdl::result<void, SnapshotErrors>::success();
                    },
                    [](const BrushError e) {
                        return kdl::result<void, SnapshotErrors>::error(SnapshotErrors{e});
                    }
                ));
        }
    }
}
