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

#include "MapParser.h"

#include "Exceptions.h"
#include "Model/EntityProperties.h"

#include <list>

namespace TrenchBroom {
    namespace IO {
        MapParser::ExtraAttribute::ExtraAttribute(const Type type, const std::string& name, const std::string& value, const size_t line, const size_t column) :
        m_type(type),
        m_name(name),
        m_value(value),
        m_line(line),
        m_column(column) {}

        MapParser::ExtraAttribute::Type MapParser::ExtraAttribute::type() const {
            return m_type;
        }

        const std::string& MapParser::ExtraAttribute::name() const {
            return m_name;
        }

        const std::string& MapParser::ExtraAttribute::strValue() const {
            return m_value;
        }

        void MapParser::ExtraAttribute::assertType(const Type expected) const {
            if (expected != m_type)
                throw ParserException(m_line, m_column, "Invalid extra property type");
        }

        MapParser::~MapParser() = default;
    }
}
