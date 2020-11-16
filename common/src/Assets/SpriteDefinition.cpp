/*
 Copyright (C) 2020 MaxED

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

#include "SpriteDefinition.h"

#include "EL/EvaluationContext.h"
#include "EL/Types.h"
#include "EL/Value.h"
#include "Model/EntityAttributesVariableStore.h"

#include <kdl/string_compare.h>

#include <vecmath/scalar.h>

#include <ostream>

namespace TrenchBroom::Assets {
    SpriteDefinition::SpriteDefinition() :
        m_expression(EL::LiteralExpression(EL::Value::Undefined), 0, 0) {}

    SpriteDefinition::SpriteDefinition(const size_t line, const size_t column) :
        m_expression(EL::LiteralExpression(EL::Value::Undefined), line, column) {}

    SpriteDefinition::SpriteDefinition(const EL::Expression& expression) :
        m_expression(expression) {}

    bool operator==(const SpriteDefinition& lhs, const SpriteDefinition& rhs) {
        return lhs.m_expression.asString() == rhs.m_expression.asString();
    }

    bool operator!=(const SpriteDefinition& lhs, const SpriteDefinition& rhs) {
        return !(lhs == rhs);
    }

    std::ostream& operator<<(std::ostream& str, const SpriteDefinition& def) {
        str << "SpriteDefinition{ " << def.m_expression << " }";
        return str;
    }

    void SpriteDefinition::append(const SpriteDefinition& other) {
        std::vector<EL::Expression> cases;
        cases.push_back(m_expression);
        cases.push_back(other.m_expression);

        const size_t line = m_expression.line();
        const size_t column = m_expression.column();
        m_expression = EL::Expression(EL::SwitchExpression(std::move(cases)), line, column);
    }

    std::string SpriteDefinition::spritePath(const Model::EntityAttributes& attributes) const {
        const Model::EntityAttributesVariableStore store(attributes);
        const EL::EvaluationContext context(store);
        return convertToSprite(m_expression.evaluate(context));
    }

    std::string SpriteDefinition::defaultSpritePath() const {
        const EL::NullVariableStore store;
        const EL::EvaluationContext context(store);
        return convertToSprite(m_expression.evaluate(context));
    }

    std::string SpriteDefinition::convertToSprite(const EL::Value& value) {
        switch (value.type()) {
            case EL::ValueType::Map:
                return toString(value["path"]);

            case EL::ValueType::String:
                return toString(value);

            case EL::ValueType::Boolean:
            case EL::ValueType::Number:
            case EL::ValueType::Array:
            case EL::ValueType::Range:
            case EL::ValueType::Null:
            case EL::ValueType::Undefined:
                break;

            switchDefault()
        }

        return std::string();
    }

    std::string SpriteDefinition::toString(const EL::Value& value) {
        if (value.type() != EL::ValueType::String) {
            return std::string();
        } else {
            return value.stringValue();
        }
    }
}
