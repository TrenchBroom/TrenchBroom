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

#include "VariableHelper.h"

namespace TrenchBroom {
    VariableTable::VariableTable() :
    m_prefix("${"),
    m_suffix("}") {}
    
    VariableTable::VariableTable(const String& prefix, const String& suffix) :
    m_prefix(prefix),
    m_suffix(suffix) {}

    const StringSet& VariableTable::declaredVariables() const {
        return m_variables;
    }

    bool VariableTable::declared(const String& variable) const {
        return m_variables.count(variable) > 0;
    }

    void VariableTable::declare(const String& variable) {
        assert(!StringUtils::isBlank(variable));
        m_variables.insert(variable);
    }
    
    void VariableTable::undeclare(const String& variable) {
        m_variables.erase(variable);
    }
    
    void VariableTable::define(const String& variableName, const String& variableValue) {
        assert(declared(variableName));
        m_values.insert(std::make_pair(variableName, variableValue));
    }
    
    void VariableTable::undefine(const String& variableName) {
        assert(declared(variableName));
        m_values.erase(variableName);
    }

    const String& VariableTable::value(const String& variableName) const {
        StringMap::const_iterator it = m_values.find(variableName);
        if (it == m_values.end())
            return EmptyString;
        return it->second;
    }

    String VariableTable::translate(const String& string) const {
        String result = string;
        StringSet::const_iterator it, end;
        for (it = m_variables.begin(), end = m_variables.end(); it != end; ++it) {
            const String& variableName   = *it;
            const String  variableString = buildVariableString(variableName);
            const String& variableValue  = value(variableName);
            result = StringUtils::replaceAll(result, variableString, variableValue);
        }
        return result;
    }

    String VariableTable::buildVariableString(const String& variableName) const {
        return m_prefix + variableName + m_suffix;
    }
}
