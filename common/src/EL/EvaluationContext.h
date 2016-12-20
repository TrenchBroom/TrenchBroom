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

#ifndef EvaluationContext_h
#define EvaluationContext_h

#include "EL/Value.h"

namespace TrenchBroom {
    namespace EL {
        class VariableStore;
        
        class EvaluationContext {
        private:
            VariableStore* m_store;
        public:
            EvaluationContext();
            EvaluationContext(const VariableStore& store);
            virtual ~EvaluationContext();
            
            virtual Value variableValue(const String& name) const;
            virtual void declareVariable(const String& name, const Value& value = Value::Undefined);
        };
        
        class EvaluationStack : public EvaluationContext {
        private:
            const EvaluationContext& m_next;
        public:
            EvaluationStack(const EvaluationContext& next);
            
            Value variableValue(const String& name) const;
        };
    }
}

#endif /* EvaluationContext_h */
