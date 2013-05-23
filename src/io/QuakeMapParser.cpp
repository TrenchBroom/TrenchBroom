/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "QuakeMapParser.h"

#include "Model/Entity.h"

namespace TrenchBroom {
    namespace IO {
        QuakeMapTokenizer::QuakeMapTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end) {}
        
        QuakeMapTokenizer::QuakeMapTokenizer(const String& str) :
        Tokenizer(str) {}

        QuakeMapTokenizer::Token QuakeMapTokenizer::emitToken() {
            while (!eof()) {
                size_t startLine = line();
                size_t startColumn = column();
                const char* c = nextChar();
                switch (*c) {
                    case '/':
                        if (peekChar() == '/')
                            discardUntil("\n\r");
                        break;
                    case '{':
                        return Token(QuakeMapToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                    case '}':
                        return Token(QuakeMapToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                    case '(':
                        return Token(QuakeMapToken::OParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case ')':
                        return Token(QuakeMapToken::CParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case '[':
                        return Token(QuakeMapToken::OBracket, c, c+1, offset(c), startLine, startColumn);
                    case ']':
                        return Token(QuakeMapToken::CBracket, c, c+1, offset(c), startLine, startColumn);
                    case '"': { // quoted string
                        const char* begin = nextChar();
                        const char* end = readQuotedString(begin);
                        return Token(QuakeMapToken::String, begin, end, offset(begin), startLine, startColumn);
                    }
                    default: { // whitespace, integer, decimal or word
                        if (isWhitespace(*c)) {
                            discardWhile(Whitespace);
                        } else {
                            const char* begin = c;
                            const char* end = readInteger(begin, Whitespace);
                            if (end > begin)
                                return Token(QuakeMapToken::Integer, begin, end, offset(begin), startLine, startColumn);
                            
                            end = readDecimal(begin, Whitespace);
                            if (end > begin)
                                return Token(QuakeMapToken::Decimal, begin, end, offset(begin), startLine, startColumn);
                            
                            end = readString(begin, Whitespace);
                            assert(end > begin);
                            return Token(QuakeMapToken::String, begin, end, offset(begin), startLine, startColumn);
                        }
                    }
                }
            }
            return Token(QuakeMapToken::Eof, NULL, NULL, length(), line(), column());
        }

        QuakeMapParser::QuakeMapParser(const char* begin, const char* end) :
        m_tokenizer(QuakeMapTokenizer(begin, end)) {}
                    
        QuakeMapParser::QuakeMapParser(const String& str) :
        m_tokenizer(QuakeMapTokenizer(str)) {}
        
        String QuakeMapParser::tokenName(const QuakeMapToken::Type typeMask) const {
            StringList names;
            if ((typeMask & QuakeMapToken::Integer) != 0)
                names.push_back("integer number");
            if ((typeMask & QuakeMapToken::Decimal) != 0)
                names.push_back("decimal number");
            if ((typeMask & QuakeMapToken::String) != 0)
                names.push_back("string");
            if ((typeMask & QuakeMapToken::OParenthesis) != 0)
                names.push_back("opening parenthesis");
            if ((typeMask & QuakeMapToken::CParenthesis) != 0)
                names.push_back("closing parenthesis");
            if ((typeMask & QuakeMapToken::OBrace) != 0)
                names.push_back("opening brace");
            if ((typeMask & QuakeMapToken::CBrace) != 0)
                names.push_back("closing brace");
            if ((typeMask & QuakeMapToken::OBracket) != 0)
                names.push_back("opening bracket");
            if ((typeMask & QuakeMapToken::CBracket) != 0)
                names.push_back("closing bracket");
            if ((typeMask & QuakeMapToken::Comment) != 0)
                names.push_back("comment");
            
            if (names.empty())
                return "unknown token type";
            if (names.size() == 1)
                return names[0];
            
            StringStream str;
            str << names[0];
            for (size_t i = 1; i < names.size() - 1; i++)
                str << ", " << names[i];
            if (names.size() > 2)
                str << ",";
            str << " or " << names[names.size() - 1];
            return str.str();
        }

        Model::MapPtr QuakeMapParser::doParseMap(const BBox3& worldBounds) {
            Model::MapPtr map = Model::Map::newMap();
            Model::EntityPtr entity = parseEntity(worldBounds);
            while (entity != NULL) {
                map->addEntity(entity);
                entity = parseEntity(worldBounds);
            }
            return map;
        }

        Model::EntityPtr QuakeMapParser::parseEntity(const BBox3& worldBounds) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                return Model::EntityPtr();
            
            expect(QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
            if (token.type() == QuakeMapToken::CBrace)
                return Model::EntityPtr();
            
            Model::EntityPtr entity = Model::Entity::newEntity();
            const size_t firstLine = token.line();
            
            while ((token = m_tokenizer.nextToken()).type() != QuakeMapToken::Eof) {
                switch (token.type()) {
                    case QuakeMapToken::String: {
                        String key = token.data();
                        expect(QuakeMapToken::String, token = m_tokenizer.nextToken());
                        String value = token.data();
                        entity->addOrUpdateProperty(key, value);
                        break;
                    }
                    case QuakeMapToken::OBrace: {
                        m_tokenizer.pushToken(token);
                        bool moreBrushes = true;
                        while (moreBrushes) {
                            Model::BrushPtr brush = parseBrush(worldBounds);
                            if (brush != NULL)
                                entity->addBrush(brush);
                            expect(QuakeMapToken::OBrace | QuakeMapToken::CBrace, token = m_tokenizer.nextToken());
                            moreBrushes = (token.type() == QuakeMapToken::OBrace);
                            m_tokenizer.pushToken(token);
                        }
                        break;
                    }
                    case QuakeMapToken::CBrace: {
                        entity->setFilePosition(firstLine, token.line() - firstLine);
                        return entity;
                    }
                    default:
                        expect(QuakeMapToken::String | QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
                }
            }
            
            return entity;
        }
        
        Model::BrushPtr QuakeMapParser::parseBrush(const BBox3& worldBounds) {
        }
        
        Model::BrushFacePtr QuakeMapParser::parseFace(const BBox3& worldBounds) {
        }
    }
}
