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

#include "EntityAttributeSnapshot.h"

#include "Model/AttributableNode.h"
#include "Model/Entity.h"

namespace TrenchBroom {
    namespace Model {
        EntityAttributeSnapshot::EntityAttributeSnapshot(const std::string& name, const std::string& value) :
        m_name(name),
        m_value(value),
        m_present(true) {}

        EntityAttributeSnapshot::EntityAttributeSnapshot(const std::string& name) :
        m_name(name),
        m_value(""),
        m_present(false) {}

        void EntityAttributeSnapshot::restore(AttributableNode* node) const {
            auto entity = node->entity();
            if (!m_present) {
                entity.removeAttribute(m_name);
            } else {
                entity.addOrUpdateAttribute(m_name, m_value);
            }
            node->setEntity(std::move(entity));
        }
    }
}
