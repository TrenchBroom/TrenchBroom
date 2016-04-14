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
    GetVariableValue::~GetVariableValue() {}
    
    String GetVariableValue::operator()(const String& variableName) const {
        return doGetValue(variableName);
    }

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
    
    const String VariableTable::translate(const String& string, const GetVariableValue& getValue) const {
        String result = string;
        StringSet::const_iterator it, end;
        for (it = m_variables.begin(), end = m_variables.end(); it != end; ++it) {
            const String& variableName   = *it;
            const String  variableString = buildVariableString(variableName);
            const String  variableValue  = getValue(variableName);
            result = StringUtils::replaceAll(result, variableString, variableValue);
        }
        return result;
    }

    String VariableTable::buildVariableString(const String& variableName) const {
        return m_prefix + variableName + m_suffix;
    }

    VariableValueTable::VariableValueTable(const VariableTable& variableTable) :
    m_variableTable(variableTable) {}
    
    void VariableValueTable::define(const String& variableName, const String& variableValue) {
        assert(m_variableTable.declared(variableName));
        m_variableValues.insert(std::make_pair(variableName, variableValue));
    }
    
    void VariableValueTable::undefine(const String& variableName) {
        m_variableValues.erase(variableName);
    }

    const String VariableValueTable::translate(const String& string) const {
        return m_variableTable.translate(string, *this);
    }

    String VariableValueTable::doGetValue(const String& variableName) const {
        StringMap::const_iterator it = m_variableValues.find(variableName);
        if (it == m_variableValues.end())
            return "";
        return it->second;
    }
}
