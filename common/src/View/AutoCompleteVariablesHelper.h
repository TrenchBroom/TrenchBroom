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

#ifndef AutoCompleteVariablesHelper_h
#define AutoCompleteVariablesHelper_h

#include "EL.h"
#include "SharedPointer.h"
#include "View/AutoCompleteTextControl.h"

namespace TrenchBroom {
    namespace View {
        class AutoCompleteVariablesHelper : public AutoCompleteTextControl::Helper {
        private:
            typedef std::tr1::shared_ptr<EL::VariableStore> VariableStorePtr;
            VariableStorePtr m_variables;
        public:
            AutoCompleteVariablesHelper(const EL::VariableStore& variables);
        private:
            size_t DoShouldStartCompletionAfterInput(const wxString& str, wxUniChar c, size_t insertPos) const;
            size_t DoShouldStartCompletionAfterRequest(const wxString& str, size_t insertPos) const;
            AutoCompleteTextControl::CompletionResult DoGetCompletions(const wxString& str, size_t startIndex, size_t count) const;
            
            size_t findLastDollar(const wxString& str, size_t startIndex) const;
        };
    }
}

#endif /* AutoCompleteVariablesHelper_h */
