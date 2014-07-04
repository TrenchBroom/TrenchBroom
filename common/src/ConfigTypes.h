/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "SharedPointer.h"

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
        
        typedef TrenchBroom::shared_ptr<ConfigEntry> Ptr;
    private:
        Type m_type;
    public:
        virtual ~ConfigEntry();
        Type type() const;
        
        operator const String&() const;
        operator const ConfigValue&() const;
        operator const ConfigList&() const;
        operator const ConfigTable&() const;
    protected:
        ConfigEntry(const Type type);
    };
    
    class ConfigValue : public ConfigEntry {
    public:
        typedef TrenchBroom::shared_ptr<ConfigValue> Ptr;
    private:
        String m_value;
    public:
        ConfigValue(const String& value);

        operator const String&() const;
    };
    
    class ConfigList : public ConfigEntry {
    public:
        typedef TrenchBroom::shared_ptr<ConfigList> Ptr;
    private:
        typedef std::vector<ConfigEntry::Ptr> EntryList;
        EntryList m_entries;
    public:
        ConfigList();
        
        const ConfigEntry& operator[](const size_t index) const;
        size_t count() const;
        
        void addEntry(ConfigEntry::Ptr entry);
    };
    
    class ConfigTable : public ConfigEntry {
    public:
        typedef TrenchBroom::shared_ptr<ConfigTable> Ptr;
    private:
        typedef std::map<String, ConfigEntry::Ptr> EntryMap;
        StringSet m_keys;
        EntryMap m_entries;
    public:
        ConfigTable();
        
        const StringSet& keys() const;
        const ConfigEntry& operator[](const String& key) const;
        size_t count() const;
        bool contains(const String& key) const;
        
        void addEntry(const String& key, ConfigEntry::Ptr entry);
    };
}

#endif
