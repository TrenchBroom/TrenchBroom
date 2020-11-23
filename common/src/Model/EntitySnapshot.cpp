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

#include "EntitySnapshot.h"

#include "Exceptions.h"
#include "Model/EntityNode.h"

#include <kdl/result.h>

namespace TrenchBroom {
    namespace Model {
        EntitySnapshot::EntitySnapshot(EntityNode* entityNode) :
        m_entityNode(entityNode),
        m_entitySnapshot(m_entityNode->entity()) {
            m_entitySnapshot.setDefinition(nullptr);
            m_entitySnapshot.setModel(nullptr);
        }

        kdl::result<void, SnapshotErrors> EntitySnapshot::doRestore(const vm::bbox3& /* worldBounds */) {
            m_entityNode->setEntity(std::move(m_entitySnapshot));
            return kdl::result<void, SnapshotErrors>::success();
        }
    }
}
