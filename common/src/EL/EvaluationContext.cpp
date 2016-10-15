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

#include "EvaluationContext.h"

#include "EL/VariableStore.h"

namespace TrenchBroom {
    namespace EL {
        EvaluationContext::EvaluationContext() :
        m_store(new VariableTable()) {}
        
        EvaluationContext::EvaluationContext(const VariableStore& store) :
        m_store(store.clone()) {}
        
        EvaluationContext::~EvaluationContext() {
            delete m_store;
        }
        
        Value EvaluationContext::variableValue(const String& name) const {
            return m_store->value(name);
        }
        
        void EvaluationContext::declareVariable(const String& name, const Value& value) {
            m_store->declare(name, value);
        }
        
        EvaluationStack::EvaluationStack(const EvaluationContext& next) :
        m_next(next) {}
        
        Value EvaluationStack::variableValue(const String& name) const {
            const Value& value = EvaluationContext::variableValue(name);
            if (value != Value::Undefined)
                return value;
            return m_next.variableValue(name);
        }
    }
}
