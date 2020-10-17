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

#include "ComputeNodeBoundsVisitor.h"

#include "Model/BrushNode.h"
#include "Model/GroupNode.h"
#include "Model/EntityNode.h"

#include <kdl/overload.h>

namespace TrenchBroom {
    namespace Model {
        vm::bbox3 computeLogicalBounds(const std::vector<Node*>& nodes, const vm::bbox3& defaultBounds) {
            vm::bbox3::builder builder;
            Node::visitAll(nodes, kdl::overload(
                [] (const WorldNode*)         {},
                [] (const LayerNode*)         {},
                [&](const GroupNode* group)   { builder.add(group->logicalBounds()); },
                [&](const EntityNode* entity) { builder.add(entity->logicalBounds()); },
                [&](const BrushNode* brush)   { builder.add(brush->logicalBounds()); }
            ));
            return builder.initialized() ? builder.bounds() : defaultBounds;
        }

        vm::bbox3 computePhysicalBounds(const std::vector<Node*>& nodes, const vm::bbox3& defaultBounds) {
            vm::bbox3::builder builder;
            Node::visitAll(nodes, kdl::overload(
                [] (const WorldNode*)         {},
                [] (const LayerNode*)         {},
                [&](const GroupNode* group)   { builder.add(group->physicalBounds()); },
                [&](const EntityNode* entity) { builder.add(entity->physicalBounds()); },
                [&](const BrushNode* brush)   { builder.add(brush->physicalBounds()); }
            ));
            return builder.initialized() ? builder.bounds() : defaultBounds;
        }
    }
}
