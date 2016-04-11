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

#include "ConfigTypes.h"

#include "CollectionUtils.h"

#include <cassert>

namespace TrenchBroom {
    ConfigEntry::~ConfigEntry() {}
    
    ConfigEntry* ConfigEntry::clone() const {
        return doClone();
    }

    size_t ConfigEntry::line() const {
        return m_line;
    }
    
    size_t ConfigEntry::column() const {
        return m_column;
    }

    ConfigEntry::Type ConfigEntry::type() const {
        return m_type;
    }

    ConfigEntry::operator const String&() const {
        const ConfigValue& asValue = *this;
        return static_cast<const String&>(asValue);
    }
    
    ConfigEntry::operator const ConfigValue&() const {
        assert(m_type == Type_Value);
        return static_cast<const ConfigValue&>(*this);
    }
    
    ConfigEntry::operator const ConfigList&() const {
        assert(m_type == Type_List);
        return static_cast<const ConfigList&>(*this);
    }
    
    ConfigEntry::operator const ConfigTable&() const {
        assert(m_type == Type_Table);
        return static_cast<const ConfigTable&>(*this);
    }

    void ConfigEntry::appendToStream(std::ostream& stream) const {
        doAppendToStream(stream, "");
    }

    String ConfigEntry::asString() const {
        StringStream stream;
        appendToStream(stream);
        return stream.str();
    }

    void ConfigEntry::appendToStream(std::ostream& stream, const String& indent) const {
        doAppendToStream(stream, indent);
    }

    void swap(ConfigEntry& lhs, ConfigEntry& rhs) {
        using std::swap;
        swap(lhs.m_line, rhs.m_line);
        swap(lhs.m_column, rhs.m_column);
    }

    ConfigEntry::ConfigEntry(const Type type, const size_t line, const size_t column) :
    m_type(type),
    m_line(line),
    m_column(column) {}

    std::ostream& operator<<(std::ostream& stream, const ConfigEntry* entry) {
        entry->appendToStream(stream);
        return stream;
    }
    
    ConfigValue::ConfigValue(const String& value) :
    ConfigEntry(Type_Value, 0, 0),
    m_value(value) {}
    
    ConfigValue::ConfigValue(const String& value, const size_t line, const size_t column) :
    ConfigEntry(Type_Value, line, column),
    m_value(value) {}
    
    ConfigValue::operator const String&() const {
        return m_value;
    }

    ConfigEntry* ConfigValue::doClone() const {
        return new ConfigValue(m_value, line(), column());
    }

    void ConfigValue::doAppendToStream(std::ostream& stream, const String& indent) const {
        stream << "\"" << m_value << "\"";
    }

    ConfigList::ConfigList() :
    ConfigEntry(Type_List, 0, 0) {}
    
    ConfigList::ConfigList(const size_t line, const size_t column) :
    ConfigEntry(Type_List, line, column) {}

    ConfigList::ConfigList(const ConfigList& other) :
    ConfigEntry(Type_List, other.line(), other.column()) {
        EntryList::const_iterator it, end;
        for (it = other.m_entries.begin(), end = other.m_entries.end(); it != end; ++it) {
            const ConfigEntry* original = *it;
            m_entries.push_back(original->clone());
        }
    }
    
    ConfigList::~ConfigList() {
        VectorUtils::clearAndDelete(m_entries);
    }

    ConfigList& ConfigList::operator=(ConfigList other) {
        using std::swap;
        swap(*this, other);
        return *this;
    }
    
    void swap(ConfigList& lhs, ConfigList& rhs) {
        using std::swap;
        swap(static_cast<ConfigEntry&>(lhs), static_cast<ConfigEntry&>(rhs));
        swap(lhs.m_entries, rhs.m_entries);
    }

    const ConfigEntry& ConfigList::operator[](const size_t index) const {
        assert(index < count());
        return *m_entries[index];
    }
    
    size_t ConfigList::count() const {
        return m_entries.size();
    }
    
    void ConfigList::addEntry(ConfigEntry* entry) {
        m_entries.push_back(entry);
    }

    ConfigEntry* ConfigList::doClone() const {
        return new ConfigList(*this);
    }

    void ConfigList::doAppendToStream(std::ostream& stream, const String& indent) const {
        if (m_entries.empty()) {
            stream << "{}";
        } else {
            stream << "{\n";
            const String childIndent = indent + "    ";
            for (size_t i = 0; i < m_entries.size(); ++i) {
                stream << childIndent;
                m_entries[i]->appendToStream(stream, childIndent);
                if (i < m_entries.size() - 1)
                    stream << ",";
                stream << "\n";
            }
            stream << indent << "}";
        }
    }

    ConfigTable::ConfigTable() :
    ConfigEntry(Type_Table, 0, 0) {}

    ConfigTable::ConfigTable(const size_t line, const size_t column) :
    ConfigEntry(Type_Table, line, column) {}

    ConfigTable::ConfigTable(const ConfigTable& other) :
    ConfigEntry(Type_Table, other.line(), other.column()) {
        EntryMap::const_iterator it, end;
        for (it = other.m_entries.begin(), end = other.m_entries.end(); it != end; ++it) {
            const String& key = it->first;
            const ConfigEntry* original = it->second;
            m_entries.insert(std::make_pair(key, original->clone()));
            m_keys.insert(key);
        }
    }
    
    ConfigTable::~ConfigTable() {
        MapUtils::clearAndDelete(m_entries);
    }
    
    ConfigTable& ConfigTable::operator=(ConfigTable other) {
        using std::swap;
        swap(*this, other);
        return *this;
    }
    
    void swap(ConfigTable& lhs, ConfigTable& rhs) {
        using std::swap;
        swap(static_cast<ConfigEntry&>(lhs), static_cast<ConfigEntry&>(rhs));
        swap(lhs.m_keys, rhs.m_keys);
        swap(lhs.m_entries, rhs.m_entries);
    }

    const StringSet& ConfigTable::keys() const {
        return m_keys;
    }
    
    const ConfigEntry& ConfigTable::operator[](const String& key) const {
        EntryMap::const_iterator it = m_entries.find(key);
        assert(it != m_entries.end());
        return *it->second;
    }
    
    size_t ConfigTable::count() const {
        return m_entries.size();
    }
    
    bool ConfigTable::contains(const String& key) const {
        return m_keys.count(key) > 0;
    }

    void ConfigTable::addEntry(const String& key, ConfigEntry* entry) {
        if (MapUtils::insertOrReplace(m_entries, key, entry))
            m_keys.insert(key);
    }

    ConfigEntry* ConfigTable::doClone() const {
        return new ConfigTable(*this);
    }

    void ConfigTable::doAppendToStream(std::ostream& stream, const String& indent) const {
        if (m_entries.empty()) {
            stream << indent << "{}";
        } else {
            const String childIndent = indent + "    ";
            stream << "{\n";
            EntryMap::const_iterator it = m_entries.begin();
            EntryMap::const_iterator end = m_entries.end();
            while (it != end) {
                const String& key = it->first;
                const ConfigEntry* entry = it->second;
                stream << childIndent << key << " = ";
                entry->appendToStream(stream, childIndent);
                ++it;
                if (it != end)
                    stream << ",";
                stream << "\n";
            }
        }
        stream << indent << "}";
    }
}
