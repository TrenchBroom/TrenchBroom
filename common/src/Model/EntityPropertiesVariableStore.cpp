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

#include "EntityPropertiesVariableStore.h"

#include "EL/ELExceptions.h"
#include "EL/Value.h"
#include "Model/Entity.h"

#include <string>

namespace TrenchBroom {
    namespace Model {
        EntityPropertiesVariableStore::EntityPropertiesVariableStore(const Entity& entity) :
        m_entity(entity) {}

        EL::VariableStore* EntityPropertiesVariableStore::doClone() const {
            return new EntityPropertiesVariableStore(m_entity);
        }

        size_t EntityPropertiesVariableStore::doGetSize() const {
            return m_entity.properties().size();
        }

        EL::Value EntityPropertiesVariableStore::doGetValue(const std::string& name) const {
            static const EL::Value DefaultValue("");
            const std::string* value = m_entity.property(name);
            if (value == nullptr) {
                return DefaultValue;
            } else {
                return EL::Value(*value);
            }
        }

        std::vector<std::string> EntityPropertiesVariableStore::doGetNames() const {
            return m_entity.propertyKeys();
        }

        void EntityPropertiesVariableStore::doDeclare(const std::string& /* name */, const EL::Value& /* value */) {
            throw EL::EvaluationError("Declaring properties directly is unsafe");
        }

        void EntityPropertiesVariableStore::doAssign(const std::string& /* name */, const EL::Value& /* value */) {
            throw EL::EvaluationError("Changing properties directly is unsafe");
        }
    }
}
