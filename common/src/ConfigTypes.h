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

#ifndef TrenchBroom_ConfigTypes_h
#define TrenchBroom_ConfigTypes_h

#include "StringUtils.h"

#include <map>
#include <set>
#include <vector>

namespace TrenchBroom {
    class ConfigValue;
    class ConfigList;
    class ConfigTable;
    
    class ConfigEntry {
    public:
        typedef enum {
            Type_Value  = 1 << 0,
            Type_List   = 1 << 1,
            Type_Table  = 1 << 2
        } Type;
    private:
        Type m_type;
        size_t m_line;
        size_t m_column;
    public:
        virtual ~ConfigEntry();
        
        size_t line() const;
        size_t column() const;
        
        ConfigEntry* clone() const;
        
        Type type() const;
        
        operator const String&() const;
        operator const ConfigValue&() const;
        operator const ConfigList&() const;
        operator const ConfigTable&() const;
        
        void appendToStream(std::ostream& stream) const;
        String asString() const;
    public:
        friend void swap(ConfigEntry& lhs, ConfigEntry& rhs);
    private:
        virtual ConfigEntry* doClone() const = 0;
        virtual void doAppendToStream(std::ostream& stream) const = 0;
    protected:
        ConfigEntry(const Type type, size_t line, size_t column);
    };
    
    std::ostream& operator<<(std::ostream& stream, const ConfigEntry* entry);
    
    class ConfigValue : public ConfigEntry {
    private:
        String m_value;
    public:
        ConfigValue(const String& value, size_t line, size_t column);
        
        operator const String&() const;
    private:
        ConfigEntry* doClone() const;
        void doAppendToStream(std::ostream& stream) const;
    };
    
    class ConfigList : public ConfigEntry {
    private:
        typedef std::vector<ConfigEntry*> EntryList;
        EntryList m_entries;
    public:
        ConfigList(size_t line, size_t column);
        ConfigList(const ConfigList& other);
        ~ConfigList();
        
        ConfigList& operator=(ConfigList other);
        friend void swap(ConfigList& lhs, ConfigList& rhs);
        
        const ConfigEntry& operator[](const size_t index) const;
        size_t count() const;
        
        void addEntry(ConfigEntry* entry);
    private:
        ConfigEntry* doClone() const;
        void doAppendToStream(std::ostream& stream) const;
    };
    
    class ConfigTable : public ConfigEntry {
    private:
        typedef std::map<String, ConfigEntry*> EntryMap;
        StringSet m_keys;
        EntryMap m_entries;
    public:
        ConfigTable(size_t line, size_t column);
        ConfigTable(const ConfigTable& other);
        ~ConfigTable();
        
        ConfigTable& operator=(ConfigTable other);
        friend void swap(ConfigTable& lhs, ConfigTable& rhs);
        
        const StringSet& keys() const;
        const ConfigEntry& operator[](const String& key) const;
        size_t count() const;
        bool contains(const String& key) const;
        
        void addEntry(const String& key, ConfigEntry* entry);
    private:
        ConfigEntry* doClone() const;
        void doAppendToStream(std::ostream& stream) const;
    };
}

#endif
