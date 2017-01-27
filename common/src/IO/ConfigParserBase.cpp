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

        EL::Expression ConfigParserBase::parseConfigFile() {
            return m_parser.parse();
        }
        
        void ConfigParserBase::expectType(const EL::Value& value, const EL::ValueType type) const {
            if (value.type() != type)
                throw ParserException(value.line(), value.column(), "Expected value of type '" + EL::typeName(type) + "', but got type '" + value.typeName() + "'");
        }
        
        void ConfigParserBase::expectStructure(const EL::Value& value, const String& structure) const {
            ELParser parser(structure);
            const EL::Value expected = parser.parse().evaluate(EL::EvaluationContext());
            assert(expected.type() == EL::Type_Array);
            
            const EL::Value& mandatory = expected[0];
            assert(mandatory.type() == EL::Type_Map);
            
            const EL::Value& optional = expected[1];
            assert(optional.type() == EL::Type_Map);
            
            for (const String& key : mandatory.keys()) {
                const String& typeName = mandatory[key].stringValue();
                const EL::ValueType type = EL::typeForName(typeName);
                expectMapEntry(value, key, type);
            }
            
            for (const String& key : value.keys()) {
                if (!mandatory.contains(key) && !optional.contains(key))
                    throw ParserException(value.line(), value.column(), "Unexpected map entry '" + key + "'");
            }
        }
        
        void ConfigParserBase::expectMapEntry(const EL::Value& value, const String& key, EL::ValueType type) const {
            const EL::MapType& map = value.mapValue();
            const EL::MapType::const_iterator it = map.find(key);
            if (it == std::end(map))
                throw ParserException(value.line(), value.column(), "Expected map entry '" + key + "'");
            expectType(it->second, type);
        }
    }
}
