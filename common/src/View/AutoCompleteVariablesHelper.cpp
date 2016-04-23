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

#include "AutoCompleteVariablesHelper.h"

namespace TrenchBroom {
    namespace View {
        AutoCompleteVariablesHelper::AutoCompleteVariablesHelper(const VariableTable& variableTable) :
        m_variableTable(variableTable) {}

        bool AutoCompleteVariablesHelper::DoStartCompletion(const wxString& str, size_t index) const {
            return str[index] == '$';
        }
        
        AutoCompleteTextControl::CompletionResult AutoCompleteVariablesHelper::DoGetCompletions(const wxString& str, const size_t index) const {
            const size_t dollarIndex = findLastDollar(str, index);
            if (dollarIndex == str.Len())
                return AutoCompleteTextControl::CompletionResult();
            
            AutoCompleteTextControl::CompletionResult result(dollarIndex);

            const wxString prefix = str.Mid(dollarIndex, index - dollarIndex + 1);
            const StringSet variables = m_variableTable.declaredVariables(prefix.ToStdString(), false);
            StringSet::const_iterator it, end;
            for (it = variables.begin(), end = variables.end(); it != end; ++it) {
                const String& variableName = *it;
                const String variableStr = m_variableTable.buildVariableString(variableName);
                const String& variableValue = m_variableTable.value(variableName);
                result.Add(variableStr, variableValue);
            }
            
            return result;
        }

        size_t AutoCompleteVariablesHelper::findLastDollar(const wxString& str, const size_t startIndex) const {
            size_t curIndex = startIndex;
            do {
                --curIndex;
                if (str[curIndex] == '$')
                    return curIndex;
            } while (curIndex > 0);
            return str.Len();
        }
    }
}
