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

#include "ModelDefinition.h"

#include "EL/EvaluationContext.h"
#include "EL/Types.h"
#include "EL/Value.h"
#include "EL/VariableStore.h"

#include <kdl/string_compare.h>

#include <vecmath/scalar.h>

#include <ostream>

namespace TrenchBroom {
    namespace Assets {
        ModelSpecification::ModelSpecification() :
        path(""),
        skinIndex(0),
        frameIndex(0) {}

        ModelSpecification::ModelSpecification(const IO::Path& i_path, const size_t i_skinIndex, const size_t i_frameIndex) :
        path(i_path),
        skinIndex(i_skinIndex),
        frameIndex(i_frameIndex) {}

        bool ModelSpecification::operator<(const ModelSpecification& rhs) const {
            return compare(rhs) < 0;
        }

        bool ModelSpecification::operator>(const ModelSpecification& rhs) const {
            return compare(rhs) > 0;
        }

        bool ModelSpecification::operator<=(const ModelSpecification& rhs) const {
            return compare(rhs) <= 0;
        }

        bool ModelSpecification::operator>=(const ModelSpecification& rhs) const {
            return compare(rhs) >= 0;
        }

        bool ModelSpecification::operator==(const ModelSpecification& rhs) const {
            return compare(rhs) == 0;
        }

        bool ModelSpecification::operator!=(const ModelSpecification& rhs) const {
            return compare(rhs) != 0;
        }

        int ModelSpecification::compare(const ModelSpecification& other) const {
            const int pathCmp = path.compare(other.path);
            if (pathCmp != 0)
                return pathCmp;
            if (skinIndex != other.skinIndex)
                return static_cast<int>(skinIndex) - static_cast<int>(other.skinIndex);
            if (frameIndex != other.frameIndex)
                return static_cast<int>(frameIndex) - static_cast<int>(other.frameIndex);
            return 0;
        }

        std::ostream& operator<<(std::ostream& stream, const ModelSpecification& spec) {
            stream << spec.path << ":" << spec.skinIndex << ":" << spec.frameIndex;
            return stream;
        }

        ModelDefinition::ModelDefinition() :
        m_expression(EL::LiteralExpression(EL::Value::Undefined), 0, 0) {}

        ModelDefinition::ModelDefinition(const size_t line, const size_t column) :
        m_expression(EL::LiteralExpression(EL::Value::Undefined), line, column) {}

        ModelDefinition::ModelDefinition(const EL::Expression& expression) :
        m_expression(expression) {}

        bool operator==(const ModelDefinition& lhs, const ModelDefinition& rhs) {
            return lhs.m_expression.asString() == rhs.m_expression.asString();
        }
        
        bool operator!=(const ModelDefinition& lhs, const ModelDefinition& rhs) {
            return !(lhs == rhs);
        }

        std::ostream& operator<<(std::ostream& str, const ModelDefinition& def) {
            str << "ModelDefinition{ " << def.m_expression << " }";
            return str;
        }

        void ModelDefinition::append(const ModelDefinition& other) {
            std::vector<EL::Expression> cases;
            cases.push_back(m_expression);
            cases.push_back(other.m_expression);
        
            const size_t line = m_expression.line();
            const size_t column = m_expression.column();
            m_expression = EL::Expression(EL::SwitchExpression(std::move(cases)), line, column);
        }

        ModelSpecification ModelDefinition::modelSpecification(const EL::VariableStore& variableStore) const {
            const EL::EvaluationContext context(variableStore);
            return convertToModel(m_expression.evaluate(context));
        }

        ModelSpecification ModelDefinition::defaultModelSpecification() const {
            return modelSpecification(EL::NullVariableStore());
        }

        ModelSpecification ModelDefinition::convertToModel(const EL::Value& value) const {
            switch (value.type()) {
                case EL::ValueType::Map:
                    return ModelSpecification( path(value["path"]),
                                              index(value["skin"]),
                                              index(value["frame"]));
                case EL::ValueType::String:
                    return ModelSpecification(path(value));
                case EL::ValueType::Boolean:
                case EL::ValueType::Number:
                case EL::ValueType::Array:
                case EL::ValueType::Range:
                case EL::ValueType::Null:
                case EL::ValueType::Undefined:
                    break;
            }

            return ModelSpecification();
        }

        IO::Path ModelDefinition::path(const EL::Value& value) const {
            if (value.type() != EL::ValueType::String)
                return IO::Path();
            const std::string& path = value.stringValue();
            return IO::Path(kdl::cs::str_is_prefix(path, ":") ? path.substr(1) : path);
        }

        size_t ModelDefinition::index(const EL::Value& value) const {
            if (!value.convertibleTo(EL::ValueType::Number))
                return 0;
            const EL::IntegerType intValue = value.convertTo(EL::ValueType::Number).integerValue();
            return static_cast<size_t>(vm::max(0l, intValue));
        }
    }
}
