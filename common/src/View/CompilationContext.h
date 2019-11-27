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

#ifndef CompilationContext_h
#define CompilationContext_h

#include "StringType.h"
#include "EL/VariableStore.h"
#include "View/TextOutputAdapter.h"

#include <memory>

namespace TrenchBroom {
    namespace View {
        class MapDocument;

        class CompilationContext {
        private:
            std::weak_ptr<MapDocument> m_document;
            EL::VariableTable m_variables;

            TextOutputAdapter m_output;
            bool m_test;
        public:
            CompilationContext(std::weak_ptr<MapDocument> document, const EL::VariableTable& variables, const TextOutputAdapter& output, bool test);

            std::shared_ptr<MapDocument> document() const;
            bool test() const;

            String interpolate(const String& input) const;
            String variableValue(const String& variableName) const;

            template <typename T>
            CompilationContext& operator<<(const T& t) {
                m_output << t;
                return *this;
            }
        };
    }
}


#endif /* CompilationContext_h */
