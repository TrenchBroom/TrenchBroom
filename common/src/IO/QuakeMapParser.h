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

#ifndef __TrenchBroom__QuakeMapParser__
#define __TrenchBroom__QuakeMapParser__

#include "IO/MapParser.h"
#include "IO/Parser.h"
#include "IO/Token.h"
#include "IO/Tokenizer.h"
#include "Model/ModelFactory.h"
#include "Model/ModelTypes.h"

#include <map>

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        namespace QuakeMapToken {
            typedef unsigned int Type;
            static const Type Integer       = 1 <<  0; // integer number
            static const Type Decimal       = 1 <<  1; // decimal number
            static const Type String        = 1 <<  2; // string
            static const Type OParenthesis  = 1 <<  3; // opening parenthesis: (
            static const Type CParenthesis  = 1 <<  4; // closing parenthesis: )
            static const Type OBrace        = 1 <<  5; // opening brace: {
            static const Type CBrace        = 1 <<  6; // closing brace: }
            static const Type OBracket      = 1 <<  7; // opening bracket: [
            static const Type CBracket      = 1 <<  8; // closing bracket: ]
            static const Type Comment       = 1 <<  9; // line comment starting with ///
            static const Type Eof           = 1 << 10; // end of file
            static const Type Eol           = 1 << 11; // end of line
        }

        class QuakeMapTokenizer : public Tokenizer<QuakeMapToken::Type> {
        private:
            bool m_skipEol;
        public:
            QuakeMapTokenizer(const char* begin, const char* end);
            QuakeMapTokenizer(const String& str);
            
            void setSkipEol(bool skipEol);
        private:
            Token emitToken();
        };
        
        class QuakeMapParser : public MapParser, public Parser<QuakeMapToken::Type> {
        private:
            typedef QuakeMapTokenizer::Token Token;

            class PlaneWeightOrder {
            private:
                bool m_deterministic;
            public:
                PlaneWeightOrder(const bool deterministic);
                template <typename T, size_t S>
                bool operator()(const Plane<T,S>& lhs, const Plane<T,S>& rhs) const {
                    int result = lhs.normal.weight() - rhs.normal.weight();
                    if (m_deterministic)
                        result += static_cast<int>(1000.0 * (lhs.distance - lhs.distance));
                    return result < 0;
                }
            };
            
            class FaceWeightOrder {
            private:
                const PlaneWeightOrder& m_planeOrder;
            public:
                FaceWeightOrder(const PlaneWeightOrder& planeOrder);
                bool operator()(const Model::BrushFace* lhs, const Model::BrushFace* rhs) const;
            };

            class ExtraProperty {
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
                ExtraProperty(Type type, const String& name, const String& value, size_t line, size_t column);
                
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
            
            typedef std::map<String, ExtraProperty> ExtraProperties;
            
            Logger* m_logger;
            QuakeMapTokenizer m_tokenizer;
            Model::MapFormat::Type m_format;
            Model::ModelFactory m_factory;
        public:
            QuakeMapParser(const char* begin, const char* end, Logger* logger = NULL);
            QuakeMapParser(const String& str, Logger* logger = NULL);
        private:
            TokenNameMap tokenNames() const;
            Model::Map* doParseMap(const BBox3& worldBounds);
            Model::EntityList doParseEntities(const BBox3& worldBounds, Model::MapFormat::Type format);
            Model::BrushList doParseBrushes(const BBox3& worldBounds, Model::MapFormat::Type format);
            Model::BrushFaceList doParseFaces(const BBox3& worldBounds, Model::MapFormat::Type format);
            
            void setFormat(Model::MapFormat::Type format);
            Model::MapFormat::Type detectFormat();
            Model::Entity* parseEntity(const BBox3& worldBounds);
            void parseEntityProperty(Model::Entity* entity);
            Model::Brush* parseBrush(const BBox3& worldBounds);
            Model::BrushFace* parseFace(const BBox3& worldBounds);
            Vec3 parseVector();
            
            Model::Brush* createBrush(const BBox3& worldBounds, const Model::BrushFaceList faces, const ExtraProperties& extraProperties, const size_t firstLine, const size_t lineCount) const;
            
            void parseExtraProperties(ExtraProperties& properties);
            void setExtraObjectProperties(Model::Object* object, const ExtraProperties properties) const;
        };
    }
}

#endif /* defined(__TrenchBroom__QuakeMapParser__) */
