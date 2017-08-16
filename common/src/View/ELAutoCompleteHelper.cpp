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

#include "ELAutoCompleteHelper.h"

namespace TrenchBroom {
    namespace View {
        ELAutoCompleteHelper::ELAutoCompleteHelper(const EL::VariableStore& variables) :
        m_variables(variables.clone()) {}

        size_t ELAutoCompleteHelper::DoShouldStartCompletionAfterInput(const wxString& str, const wxUniChar c, const size_t insertPos) const {
            return str.Length() + 1;
        }
        
        size_t ELAutoCompleteHelper::DoShouldStartCompletionAfterRequest(const wxString& str, const size_t insertPos) const {
            return insertPos;
        }

        AutoCompleteTextControl::CompletionResult ELAutoCompleteHelper::DoGetCompletions(const wxString& str, const size_t startIndex, const size_t count) const {
            AutoCompleteTextControl::CompletionResult result;
            
            for (const String& variableName : m_variables->names()) {
                const String variableValue = m_variables->value(variableName).describe();
                result.Add("{" + variableName + "}", variableValue);
            }
            
            return result;
        }
    }
}
