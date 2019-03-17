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

#include "EL.h"
#include "Logger.h"
#include "StringUtils.h"
#include "View/TextCtrlOutputAdapter.h"
#include "View/ViewTypes.h"

#include <wx/string.h>
#include <wx/thread.h>

namespace TrenchBroom {
    namespace View {
        class CompilationContext {
        private:
            MapDocumentWPtr m_document;
            EL::VariableTable m_variables;

            TextCtrlOutputAdapter m_output;
            bool m_test;
        public:
            CompilationContext(MapDocumentWPtr document, const EL::VariableTable& variables, const TextCtrlOutputAdapter& output, bool test);

            MapDocumentSPtr document() const;
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
