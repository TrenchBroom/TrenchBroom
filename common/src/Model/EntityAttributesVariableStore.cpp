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

#include "Model/EntityAttributes.h"

namespace TrenchBroom {
    namespace Model {
        EntityAttributesVariableStore::EntityAttributesVariableStore(const EntityAttributes& attributes) :
        m_attributes(attributes) {}

        EL::VariableStore* EntityAttributesVariableStore::doClone() const {
            return new EntityAttributesVariableStore(m_attributes);
        }

        EL::Value EntityAttributesVariableStore::doGetValue(const String& name) const {
            static const EL::Value DefaultValue("");
            const AttributeValue* value = m_attributes.attribute(name);
            if (value == nullptr)
                return DefaultValue;
            return EL::Value::ref(*value);
        }

        StringSet EntityAttributesVariableStore::doGetNames() const {
            return m_attributes.names();
        }

        void EntityAttributesVariableStore::doDeclare(const String& name, const EL::Value& value) {
            throw EL::EvaluationError("Declaring attributes directly is unsafe");
        }

        void EntityAttributesVariableStore::doAssign(const String& name, const EL::Value& value) {
            throw EL::EvaluationError("Changing attributes directly is unsafe");
        }
    }
}
