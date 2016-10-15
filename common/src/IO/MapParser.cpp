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

#include "MapParser.h"
#include "Exceptions.h"

namespace TrenchBroom {
    namespace IO {
        MapParser::ExtraAttribute::ExtraAttribute(const Type type, const String& name, const String& value, const size_t line, const size_t column) :
        m_type(type),
        m_name(name),
        m_value(value),
        m_line(line),
        m_column(column) {}
        
        MapParser::ExtraAttribute::Type MapParser::ExtraAttribute::type() const {
            return m_type;
        }
        
        const String& MapParser::ExtraAttribute::name() const {
            return m_name;
        }
        
        const String& MapParser::ExtraAttribute::strValue() const {
            return m_value;
        }
        
        void MapParser::ExtraAttribute::assertType(const Type expected) const {
            if (expected != m_type)
                throw ParserException(m_line, m_column, "Invalid extra property type");
        }
        
        MapParser::~MapParser() {}

        void MapParser::formatSet(const Model::MapFormat::Type format) {
            onFormatSet(format);
        }

        void MapParser::beginEntity(const size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) {
            onBeginEntity(line, attributes, extraAttributes);
        }
        
        void MapParser::endEntity(const size_t startLine, const size_t lineCount) {
            onEndEntity(startLine, lineCount);
        }
        
        void MapParser::beginBrush(const size_t line) {
            onBeginBrush(line);
        }
        
        void MapParser::endBrush(const size_t startLine, const size_t lineCount, const ExtraAttributes& extraAttributes) {
            onEndBrush(startLine, lineCount, extraAttributes);
        }
        
        void MapParser::brushFace(size_t line, const Vec3& point1, const Vec3& point2, const Vec3& point3, const Model::BrushFaceAttributes& attribs, const Vec3& texAxisX, const Vec3& texAxisY) {
            onBrushFace(line, point1, point2, point3, attribs, texAxisX, texAxisY);
        }
    }
}
