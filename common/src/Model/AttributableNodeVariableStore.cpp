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

#include "AttributableNodeVariableStore.h"

#include "StringUtils.h"
#include "EL/ELExceptions.h"
#include "EL/Types.h"
#include "Model/AttributableNode.h"

namespace TrenchBroom {
    namespace Model {
        AttributableNodeVariableStore::AttributableNodeVariableStore(AttributableNode* node) :
        m_node(node) {
            ensure(m_node != nullptr, "node is null");
        }

        EL::VariableStore* AttributableNodeVariableStore::doClone() const {
            return new AttributableNodeVariableStore(m_node);
        }

        size_t AttributableNodeVariableStore::doGetSize() const {
            return m_node->attributes().size();
        }

        EL::Value AttributableNodeVariableStore::doGetValue(const String& name) const {
            if (!m_node->hasAttribute(name)) {
                return EL::Value::Undefined;
            } else {
                return EL::Value(m_node->attribute(name));
            }
        }

        StringList AttributableNodeVariableStore::doGetNames() const {
            return m_node->attributeNames();
        }

        void AttributableNodeVariableStore::doDeclare(const String& name, const EL::Value& value) {
            if (m_node->hasAttribute(name)) {
                throw EL::EvaluationError("Variable '" + name + "' already declared");
            } else {
                doAssign(name, value);
            }
        }

        void AttributableNodeVariableStore::doAssign(const String& name, const EL::Value& value) {
            const EL::Value stringELValue = value.convertTo(EL::ValueType::String);
            m_node->addOrUpdateAttribute(name, StringUtils::toString(stringELValue));
        }
    }
}
