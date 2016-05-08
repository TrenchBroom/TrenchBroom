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

#ifndef VariableTable_h
#define VariableTable_h

#include "StringUtils.h"

namespace TrenchBroom {
    class VariableTable {
    private:
        StringSet m_variables;
        ::StringMap m_values;
        const String m_prefix;
        const String m_suffix;
    public:
        VariableTable();
        VariableTable(const String& prefix, const String& suffix);

        const StringSet& declaredVariables() const;
        StringSet declaredVariables(const String& prefix, bool caseSensitive) const;
        bool declared(const String& variable) const;
        
        void declare(const String& variable);
        void undeclare(const String& variable);
        
        void define(const String& variableName, const String& variableValue);
        void undefine(const String& variableName);
        
        const String& value(const String& variableName) const;
        String translate(const String& string) const;
        String buildVariableString(const String& variableName) const;
        
    };
}

#endif /* VariableTable_h */
