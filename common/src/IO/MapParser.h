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

#ifndef TrenchBroom_MapParser_h
#define TrenchBroom_MapParser_h

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/EntityAttributes.h"
#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFaceAttributes;
    }
    
    namespace IO {
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
                String m_name;
                String m_value;
                size_t m_line;
                size_t m_column;
            public:
                ExtraAttribute(Type type, const String& name, const String& value, size_t line, size_t column);
                
                Type type() const;
                const String& name() const;
                const String& strValue() const;
                
                void assertType(Type expected) const;
                
                template <typename T>
                T intValue() const {
                    assert(m_type == Type_Integer);
                    return static_cast<T>(std::atoi(m_value.c_str()));
                }
            };
            
            typedef std::map<String, ExtraAttribute> ExtraAttributes;
        public:
            virtual ~MapParser();
        protected:
            void formatSet(Model::MapFormat::Type format);
            void beginEntity(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes);
            void endEntity(size_t startLine, size_t lineCount);
            void beginBrush(size_t line);
            void endBrush(size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes);
            void brushFace(size_t line, const Vec3& point1, const Vec3& point2, const Vec3& point3, const Model::BrushFaceAttributes& attribs, const Vec3& texAxisX, const Vec3& texAxisY);
        private: // subclassing interface for users of the parser
            virtual void onFormatSet(Model::MapFormat::Type format) = 0;
            virtual void onBeginEntity(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes) = 0;
            virtual void onEndEntity(size_t startLine, size_t lineCount) = 0;
            virtual void onBeginBrush(size_t line) = 0;
            virtual void onEndBrush(size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes) = 0;
            virtual void onBrushFace(size_t line, const Vec3& point1, const Vec3& point2, const Vec3& point3, const Model::BrushFaceAttributes& attribs, const Vec3& texAxisX, const Vec3& texAxisY) = 0;
        };
    }
}

#endif
