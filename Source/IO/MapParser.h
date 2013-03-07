/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MapParser__
#define __TrenchBroom__MapParser__

#include "IO/ByteBuffer.h"
#include "IO/StreamTokenizer.h"
#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"
#include "Model/FaceTypes.h"
#include "Utility/MessageException.h"
#include "Utility/VecMath.h"

#include <memory>
#include <vector>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Entity;
        class Face;
        class Map;
    }

    namespace Utility {
        class Console;
        class ProgressIndicator;
    }

    namespace IO {
        namespace TokenType {
            static const unsigned int Integer       = 1 <<  0; // integer number
            static const unsigned int Decimal       = 1 <<  1; // decimal number
            static const unsigned int String        = 1 <<  2; // string
            static const unsigned int OParenthesis  = 1 <<  3; // opening parenthesis: (
            static const unsigned int CParenthesis  = 1 <<  4; // closing parenthesis: )
            static const unsigned int OBrace        = 1 <<  5; // opening brace: {
            static const unsigned int CBrace        = 1 <<  6; // closing brace: }
            static const unsigned int OBracket      = 1 <<  7; // opening bracket: [
            static const unsigned int CBracket      = 1 <<  8; // closing bracket: ]
            static const unsigned int Comment       = 1 <<  9; // line comment starting with //
            static const unsigned int Eof           = 1 << 10; // end of file
        }

        class MapTokenEmitter : public TokenEmitter<MapTokenEmitter> {
        private:
            StringStream m_buffer;
        protected:
            bool isDelimiter(char c) {
                return isWhitespace(c) || c == '(' || c == ')' || c == '{' || c == '}' || c == '?' || c == ';' || c == ',' || c == '=';
            }

            Token doEmit(Tokenizer& tokenizer, size_t line, size_t column);
        };
        
        class MapParserException : public TrenchBroom::Utility::MessageException {
        private:
            String type(unsigned int type) {
                StringList names;
                if ((type & TokenType::Integer) != 0)
                    names.push_back("integer number");
                if ((type & TokenType::Decimal) != 0)
                    names.push_back("decimal number");
                if ((type & TokenType::String) != 0)
                    names.push_back("string");
                if ((type & TokenType::OParenthesis) != 0)
                    names.push_back("opening parenthesis");
                if ((type & TokenType::CParenthesis) != 0)
                    names.push_back("closing parenthesis");
                if ((type & TokenType::OBrace) != 0)
                    names.push_back("opening brace");
                if ((type & TokenType::CBrace) != 0)
                    names.push_back("closing brace");
                if ((type & TokenType::OBracket) != 0)
                    names.push_back("opening bracket");
                if ((type & TokenType::CBracket) != 0)
                    names.push_back("closing bracket");
                if ((type & TokenType::Comment) != 0)
                    names.push_back("comment");

                if (names.empty())
                    return "unknown token type";
                if (names.size() == 1)
                    return names[0];

                StringStream str;
                str << names[0];
                for (unsigned int i = 1; i < names.size() - 1; i++)
                    str << ", " << names[i];
                str << ", or " << names[names.size() - 1];
                return str.str();
            }

            std::string buildMessage(const Token& token, unsigned int expectedType) {
                std::stringstream msgStream;
                msgStream << "Malformed map file: expected token of type " << type(expectedType) << ", but found " << type(token.type()) << " at line " << token.line() << ", column " << token.column();
                return msgStream.str();
            }
        public:
            MapParserException() : MessageException("Reached unexpected end of file") {}
            MapParserException(const Token& token, unsigned int expectedType) : MessageException(buildMessage(token, expectedType)) {}
        };

        class MapParser {
        public:
            class CreateBrushStrategy {
            public:
                virtual ~CreateBrushStrategy() {}
                virtual Model::Brush* operator()(const BBox& worldBounds, const Model::FaceList& faces) = 0;
            };
        private:
            enum MapFormat {
                Undefined,
                Standard,
                Valve
            };

            Utility::Console& m_console;
            CreateBrushStrategy& m_createBrushStrategy;
            StreamTokenizer<MapTokenEmitter> m_tokenizer;
            MapFormat m_format;
            size_t m_size;

            inline void expect(unsigned int expectedType, const Token& actualToken) const {
                if ((actualToken.type() & expectedType) == 0)
                    throw MapParserException(actualToken, expectedType);
            }
        public:
            MapParser(std::istream& stream, Utility::Console& console, CreateBrushStrategy& createBrushStrategy);
            
            void parseMap(Model::Map& map, Utility::ProgressIndicator* indicator);
            Model::Entity* parseEntity(const BBox& worldBounds, Utility::ProgressIndicator* indicator);
            Model::Brush* parseBrush(const BBox& worldBounds, Utility::ProgressIndicator* indicator);
            Model::Face* parseFace(const BBox& worldBounds);

            bool parseEntities(const BBox& worldBounds, Model::EntityList& entities);
            bool parseBrushes(const BBox& worldBounds, Model::BrushList& brushes);
            bool parseFaces(const BBox& worldBounds, Model::FaceList& faces);
        };

    }
}

#endif /* defined(__TrenchBroom__MapParser__) */
