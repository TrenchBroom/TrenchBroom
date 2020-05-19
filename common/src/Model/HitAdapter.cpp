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

#include "HitAdapter.h"

#include "Hit.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"

namespace TrenchBroom {
    namespace Model {
        Node* hitToNode(const Hit& hit) {
            if (hit.type() == EntityNode::EntityHitType) {
                return hit.target<EntityNode*>();
            } else if (hit.type() == BrushNode::BrushHitType) {
                BrushFace* face = hit.target<BrushFace*>();
                return face->brush()->node();
            } else {
                return nullptr;
            }
        }

        Object* hitToObject(const Hit& hit) {
            if (hit.type() == EntityNode::EntityHitType) {
                return hit.target<EntityNode*>();
            } else if (hit.type() == BrushNode::BrushHitType) {
                BrushFace* face = hit.target<BrushFace*>();
                return face->brush()->node();
            } else {
                return nullptr;
            }
        }

        EntityNode* hitToEntity(const Hit& hit) {
            if (hit.type() == EntityNode::EntityHitType) {
                return hit.target<EntityNode*>();
            } else {
                return nullptr;
            }
        }

        BrushNode* hitToBrush(const Hit& hit) {
            if (hit.type() == BrushNode::BrushHitType) {
                return hit.target<BrushFace*>()->brush()->node();
            } else {
                return nullptr;
            }
        }

        BrushFace* hitToFace(const Hit& hit) {
            if (hit.type() == BrushNode::BrushHitType) {
                return hit.target<BrushFace*>();
            } else {
                return nullptr;
            }
        }
    }
}
