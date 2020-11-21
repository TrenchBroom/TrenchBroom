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

#include "EntityAttributesVariableStore.h"

#include "EL/ELExceptions.h"
#include "EL/Value.h"
#include "Model/Entity.h"

#include <string>

namespace TrenchBroom {
    namespace Model {
        EntityAttributesVariableStore::EntityAttributesVariableStore(const Entity& entity) :
        m_entity(entity) {}

        EL::VariableStore* EntityAttributesVariableStore::doClone() const {
            return new EntityAttributesVariableStore(m_entity);
        }

        size_t EntityAttributesVariableStore::doGetSize() const {
            return m_entity.attributes().size();
        }

        EL::Value EntityAttributesVariableStore::doGetValue(const std::string& name) const {
            static const EL::Value DefaultValue("");
            const std::string* value = m_entity.attribute(name);
            if (value == nullptr) {
                return DefaultValue;
            } else {
                return EL::Value(*value);
            }
        }

        std::vector<std::string> EntityAttributesVariableStore::doGetNames() const {
            return m_entity.attributeNames();
        }

        void EntityAttributesVariableStore::doDeclare(const std::string& /* name */, const EL::Value& /* value */) {
            throw EL::EvaluationError("Declaring attributes directly is unsafe");
        }

        void EntityAttributesVariableStore::doAssign(const std::string& /* name */, const EL::Value& /* value */) {
            throw EL::EvaluationError("Changing attributes directly is unsafe");
        }
    }
}
