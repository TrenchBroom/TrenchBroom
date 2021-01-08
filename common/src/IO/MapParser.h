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

#pragma once

#include "FloatType.h"
#include "Model/MapFormat.h"

#include <vecmath/forward.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EntityProperty;
        class BrushFaceAttributes;
    }

    namespace IO {
        class ParserStatus;

        class MapParser {
        protected:
            class ExtraAttribute {
            public:
                typedef enum {
                    Type_String,
                    Type_Integer
                } Type;
            private:
                Type m_type;
                std::string m_name;
                std::string m_value;
                size_t m_line;
                size_t m_column;
            public:
                ExtraAttribute(Type type, const std::string& name, const std::string& value, size_t line, size_t column);

                Type type() const;
                const std::string& name() const;
                const std::string& strValue() const;

                void assertType(Type expected) const;

                template <typename T>
                T intValue() const {
                    assert(m_type == Type_Integer);
                    return static_cast<T>(std::atoi(m_value.c_str()));
                }
            };

            using ExtraAttributes = std::map<std::string, ExtraAttribute>;
        public:
            virtual ~MapParser();
        protected: // subclassing interface for users of the parser
            virtual void onBeginEntity(size_t line, const std::vector<Model::EntityProperty>& properties, const ExtraAttributes& extraAttributes, ParserStatus& status) = 0;
            virtual void onEndEntity(size_t startLine, size_t lineCount, ParserStatus& status) = 0;
            virtual void onBeginBrush(size_t line, ParserStatus& status) = 0;
            virtual void onEndBrush(size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes, ParserStatus& status) = 0;
            virtual void onStandardBrushFace(size_t line, Model::MapFormat targetMapFormat, const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const Model::BrushFaceAttributes& attribs, ParserStatus& status) = 0;
            virtual void onValveBrushFace(size_t line, Model::MapFormat targetMapFormat, const vm::vec3& point1, const vm::vec3& point2, const vm::vec3& point3, const Model::BrushFaceAttributes& attribs, const vm::vec3& texAxisX, const vm::vec3& texAxisY, ParserStatus& status) = 0;
        };
    }
}
