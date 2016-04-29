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

#include "ConfigParserBase.h"

#include "CollectionUtils.h"
#include "Exceptions.h"

#include <cstdarg>

namespace TrenchBroom {
    namespace IO {
        ConfigParserBase::ConfigParserBase(const char* begin, const char* end, const Path& path) :
        m_parser(begin, end),
        m_path(path) {}
        
        ConfigParserBase::ConfigParserBase(const String& str, const Path& path) :
        m_parser(str),
        m_path(path) {}

        ConfigParserBase::~ConfigParserBase() {}

        ConfigParserBase::ConfigPtr ConfigParserBase::parseConfigFile() {
            return ConfigPtr(m_parser.parse());
        }

        StringSet ConfigParserBase::parseSet(const ConfigList& list) const {
            StringSet result;
            for (size_t i = 0; i < list.count(); ++i) {
                const ConfigEntry& entry = list[i];
                expectEntry(ConfigEntry::Type_Value, entry);
                result.insert(static_cast<const String&>(entry));
            }
            return result;
        }
        
        StringList ConfigParserBase::parseList(const ConfigList& list) const {
            StringList result;
            for (size_t i = 0; i < list.count(); ++i) {
                const ConfigEntry& entry = list[i];
                expectEntry(ConfigEntry::Type_Value, entry);
                result.push_back(static_cast<const String&>(entry));
            }
            return result;
        }
        
        void ConfigParserBase::expectEntry(const int typeMask, const ConfigEntry& entry) const {
            if ((typeMask & entry.type()) == 0)
                throw ParserException(entry.line(), entry.column(), "Expected " + typeNames(typeMask) + ", but got " + typeNames(entry.type()));
        }
        
        void ConfigParserBase::expectListEntry(const size_t index, int typeMask, const ConfigList& list) const {
            if (index >= list.count()) {
                ParserException e(list.line(), list.column());
                e << "Expected list entry with type " << typeNames(typeMask) << " at index " << index;
                throw e;
            }
            
            if ((list[index].type() & typeMask) == 0) {
                ParserException e(list.line(), list.column());
                e << "Expected list entry with type " << typeNames(typeMask) << " at index " << index << ", but got list entry with type " << typeNames(list[index].type());
                throw e;
            }
        }

        void ConfigParserBase::expectTableEntry(const String& key, const int typeMask, const ConfigTable& parent) const {
            if (!parent.contains(key))
                throw ParserException(parent.line(), parent.column(), "Expected table entry '" + key + "' with type " + typeNames(typeMask));
            if ((parent[key].type() & typeMask) == 0)
                throw ParserException(parent.line(), parent.column(), "Expected table entry '" + key + "' with type " + typeNames(typeMask) + ", but got table entry with type " + typeNames(parent[key].type()));
        }
        
        void ConfigParserBase::expectTableEntries(const ConfigTable& table, const StringSet& mandatory, const StringSet& optional) const {
            const StringSet missing = SetUtils::minus(mandatory, table.keys());
            if (!missing.empty()) {
                const String keyStr = StringUtils::join(missing.begin(), missing.end(), ", ", ", and ", " and ", StringUtils::StringToSingleQuotedString());
                throw ParserException(table.line(), table.column(), "Missing table keys " + keyStr);
            }
            
            const StringSet unknown = SetUtils::minus(table.keys(), SetUtils::merge(mandatory, optional));
            if (!unknown.empty()) {
                const String keyStr = StringUtils::join(unknown.begin(), unknown.end(), ", ", ", and ", " and ", StringUtils::StringToSingleQuotedString());
                throw ParserException(table.line(), table.column(), "Unexpected table keys " + keyStr);
            }
        }
        
        String ConfigParserBase::typeNames(const int typeMask) const {
            StringList result;
            if ((typeMask & ConfigEntry::Type_Value) != 0)
                result.push_back("value");
            if ((typeMask & ConfigEntry::Type_List) != 0)
                result.push_back("list");
            if ((typeMask & ConfigEntry::Type_Table) != 0)
                result.push_back("table");
            
            if (result.empty())
                return "none";
            if (result.size() == 1)
                return result.front();
            return StringUtils::join(result, ", ", ", or ", " or ");
        }
    }
}
