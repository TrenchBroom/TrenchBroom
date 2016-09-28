/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include <cassert>

#include "Model/EntityAttributesVariableStore.h"

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
            const int pathCmp = path.compare(rhs.path);
            if (pathCmp < 0)
                return true;
            if (pathCmp > 0)
                return false;
            if (skinIndex < rhs.skinIndex)
                return true;
            if (skinIndex > rhs.skinIndex)
                return false;
            return frameIndex < rhs.frameIndex;
        }

        const String ModelSpecification::asString() const {
            StringStream str;
            str << path.asString() << ":" << skinIndex << ":" << frameIndex;
            return str.str();
        }

        ModelDefinition::ModelDefinition() :
        m_expression(EL::LiteralExpression::create(EL::Value::Undefined, 0, 0)) {}

        ModelDefinition::ModelDefinition(const size_t line, const size_t column) :
        m_expression(EL::LiteralExpression::create(EL::Value::Undefined, line, column)) {}

        ModelDefinition::ModelDefinition(const EL::Expression& expression) :
        m_expression(expression) {}
        
        void ModelDefinition::append(const ModelDefinition& other) {
            EL::ExpressionBase::List cases;
            cases.push_back(m_expression.clone());
            cases.push_back(other.m_expression.clone());

            const size_t line = m_expression.line();
            const size_t column = m_expression.column();
            m_expression = EL::SwitchOperator::create(cases, line, column);
        }

        ModelSpecification ModelDefinition::modelSpecification(const Model::EntityAttributes& attributes) const {
            const Model::EntityAttributesVariableStore store(attributes);
            const EL::EvaluationContext context(store);
            return convertToModel(m_expression.evaluate(context));
        }

        ModelSpecification ModelDefinition::defaultModelSpecification() const {
            const EL::NullVariableStore store;
            const EL::EvaluationContext context(store);
            return convertToModel(m_expression.evaluate(context));
        }

        ModelSpecification ModelDefinition::convertToModel(const EL::Value& value) const {
            switch (value.type()) {
                case EL::Type_Map:
                    return ModelSpecification(value["path"].stringValue(),
                                              static_cast<size_t>(value["skin"].numberValue()),
                                              static_cast<size_t>(value["frame"].numberValue()));
                case EL::Type_String:
                    return ModelSpecification(value.stringValue());
                case EL::Type_Boolean:
                case EL::Type_Number:
                case EL::Type_Array:
                case EL::Type_Range:
                case EL::Type_Null:
                case EL::Type_Undefined:
                    return ModelSpecification();
            }
        }
    }
}
