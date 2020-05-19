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

#include "BoundsContainsNodeVisitor.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"

namespace TrenchBroom {
    namespace Model {
        BoundsContainsNodeVisitor::BoundsContainsNodeVisitor(const vm::bbox3& bounds) :
        m_bounds(bounds) {}

        void BoundsContainsNodeVisitor::doVisit(const WorldNode*)         { setResult(false); }
        void BoundsContainsNodeVisitor::doVisit(const LayerNode*)         { setResult(false); }
        void BoundsContainsNodeVisitor::doVisit(const GroupNode* group)   { setResult(m_bounds.contains(group->logicalBounds())); }
        void BoundsContainsNodeVisitor::doVisit(const EntityNode* entity) { setResult(m_bounds.contains(entity->logicalBounds())); }
        void BoundsContainsNodeVisitor::doVisit(const BrushNode* brush)   { setResult(m_bounds.contains(brush->logicalBounds())); }
    }
}
