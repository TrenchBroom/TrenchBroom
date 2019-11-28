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

#include "CompilationContext.h"

#include "EL/EvaluationContext.h"
#include "EL/Interpolator.h"
#include "EL/Value.h"
#include "EL/Types.h"

namespace TrenchBroom {
    namespace View {
        CompilationContext::CompilationContext(std::weak_ptr<MapDocument> document, const EL::VariableTable& variables, const TextOutputAdapter& output, bool test) :
        m_document(document),
        m_variables(variables),
        m_output(output),
        m_test(test) {}

        std::shared_ptr<MapDocument> CompilationContext::document() const {
            return lock(m_document);
        }

        bool CompilationContext::test() const {
            return m_test;
        }

        String CompilationContext::interpolate(const String& input) const {
            return EL::interpolate(input, EL::EvaluationContext(m_variables));
        }

        String CompilationContext::variableValue(const String& variableName) const {
            return m_variables.value(variableName).convertTo(EL::ValueType::String).stringValue();
        }
    }
}
