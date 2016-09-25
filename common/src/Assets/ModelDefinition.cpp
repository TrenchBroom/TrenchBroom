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

        ModelDefinition::ModelDefinition(const EL::Expression& expression) :
        m_expression(expression) {}
        
        ModelSpecification ModelDefinition::modelSpecification(const Model::EntityAttributes& attributes) const {
            const EL::Value result = evaluateExpression(attributes);
            switch (result.type()) {
                case EL::Type_Map:
                    return ModelSpecification(result["path"].stringValue(),
                                              static_cast<size_t>(result["skin"].numberValue()),
                                              static_cast<size_t>(result["frame"].numberValue()));
                case EL::Type_String:
                    return ModelSpecification(result.stringValue());
                case EL::Type_Boolean:
                case EL::Type_Number:
                case EL::Type_Array:
                case EL::Type_Range:
                case EL::Type_Null:
                case EL::Type_Undefined:
                    return ModelSpecification();
            }
        }

        EL::Value ModelDefinition::evaluateExpression(const Model::EntityAttributes& attributes) const {
            const Model::EntityAttributesVariableStore store(attributes);
            const EL::EvaluationContext context(store);
            return m_expression.evaluate(context);
        }
    }
}
