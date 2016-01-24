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

#include "StandardMapParser.h"

#include "Logger.h"
#include "SetAny.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace IO {
        const String QuakeMapTokenizer::NumberDelim = Whitespace + ")";

        QuakeMapTokenizer::QuakeMapTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end),
        m_skipEol(true) {}
        
        QuakeMapTokenizer::QuakeMapTokenizer(const String& str) :
        Tokenizer(str),
        m_skipEol(true) {}
        
        void QuakeMapTokenizer::setSkipEol(bool skipEol) {
            m_skipEol = skipEol;
        }
        
        QuakeMapTokenizer::Token QuakeMapTokenizer::emitToken() {
            while (!eof()) {
                size_t startLine = line();
                size_t startColumn = column();
                const char* c = curPos();
                switch (*c) {
                    case '/':
                        advance();
                        if (curChar() == '/') {
                            advance();
                            if (curChar() == '/') {
                                advance();
                                return Token(QuakeMapToken::Comment, c, c+3, offset(c), startLine, startColumn);
                            }
                            discardUntil("\n\r");
                        }
                        break;
                    case '{':
                        advance();
                        return Token(QuakeMapToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        return Token(QuakeMapToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                    case '(':
                        advance();
                        return Token(QuakeMapToken::OParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case ')':
                        advance();
                        return Token(QuakeMapToken::CParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case '[':
                        advance();
                        return Token(QuakeMapToken::OBracket, c, c+1, offset(c), startLine, startColumn);
                    case ']':
                        advance();
                        return Token(QuakeMapToken::CBracket, c, c+1, offset(c), startLine, startColumn);
                    case '"': { // quoted string
                        advance();
                        c = curPos();
                        const char* e = readQuotedString();
                        return Token(QuakeMapToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    case '\n':
                        if (!m_skipEol) {
                            advance();
                            return Token(QuakeMapToken::Eol, c, c+1, offset(c), startLine, startColumn);
                        }
                    case '\r':
                    case ' ':
                    case '\t':
                        discardWhile(Whitespace);
                        break;
                    default: { // whitespace, integer, decimal or word
                        const char* e = readInteger(NumberDelim);
                        if (e != NULL)
                            return Token(QuakeMapToken::Integer, c, e, offset(c), startLine, startColumn);
                        
                        e = readDecimal(NumberDelim);
                        if (e != NULL)
                            return Token(QuakeMapToken::Decimal, c, e, offset(c), startLine, startColumn);
                        
                        e = readString(Whitespace);
                        if (e == NULL)
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        return Token(QuakeMapToken::String, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(QuakeMapToken::Eof, NULL, NULL, length(), line(), column());
        }

        StandardMapParser::StandardMapParser(const char* begin, const char* end, Logger* logger) :
        m_tokenizer(QuakeMapTokenizer(begin, end)),
        m_logger(logger),
        m_format(Model::MapFormat::Unknown) {}
        
        StandardMapParser::StandardMapParser(const String& str, Logger* logger) :
        m_tokenizer(QuakeMapTokenizer(str)),
        m_logger(logger),
        m_format(Model::MapFormat::Unknown) {}
        
        StandardMapParser::~StandardMapParser() {}

        Logger* StandardMapParser::logger() const {
            return m_logger;
        }

        Model::MapFormat::Type StandardMapParser::detectFormat() {
            Model::MapFormat::Type format = Model::MapFormat::Unknown;
            
            // try to find an opening parenthesis
            Token token = m_tokenizer.nextToken();
            while (token.type() != QuakeMapToken::OParenthesis &&
                   token.type() != QuakeMapToken::Eof)
                token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                format = Model::MapFormat::Standard;
            
            if (format == Model::MapFormat::Unknown) {
                m_tokenizer.pushToken(token);
                for (size_t i = 0; i < 3; ++i) {
                    expect(QuakeMapToken::OParenthesis, token = m_tokenizer.nextToken());
                    parseVector();
                    expect(QuakeMapToken::CParenthesis, token = m_tokenizer.nextToken());
                }
                
                // texture names can contain braces etc, so we just read everything until the next opening bracket or number
                m_tokenizer.readRemainder(QuakeMapToken::OBracket | QuakeMapToken::Integer | QuakeMapToken::Decimal);
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal | QuakeMapToken::OBracket, token = m_tokenizer.nextToken());
                if (token.type() == QuakeMapToken::OBracket)
                    format = Model::MapFormat::Valve;
            }
            
            if (format == Model::MapFormat::Unknown) {
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // y offset
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // rotation
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // x scale
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // y scale
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, token = m_tokenizer.nextToken());
                if (token.type() == QuakeMapToken::OParenthesis || token.type() == QuakeMapToken::CBrace)
                    format = Model::MapFormat::Standard;
            }
            
            if (format == Model::MapFormat::Unknown) {
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, token = m_tokenizer.nextToken()); // unknown Hexen 2 flag or Quake 2 surface contents
                if (token.type() == QuakeMapToken::OParenthesis || token.type() == QuakeMapToken::CBrace)
                    format = Model::MapFormat::Hexen2;
                else
                    format = Model::MapFormat::Quake2;
            }
            
            m_tokenizer.reset();
            return format;
        }
        
        void StandardMapParser::parseEntities(const Model::MapFormat::Type format) {
            setFormat(format);

            Token token = m_tokenizer.nextToken();
            while (token.type() != QuakeMapToken::Eof) {
                expect(QuakeMapToken::OBrace, token);
                m_tokenizer.pushToken(token);
                parseEntity();
                token = m_tokenizer.nextToken();
            }
        }
        
        void StandardMapParser::parseBrushes(const Model::MapFormat::Type format) {
            setFormat(format);

            Token token = m_tokenizer.nextToken();
            while (token.type() != QuakeMapToken::Eof) {
                expect(QuakeMapToken::OBrace, token);
                m_tokenizer.pushToken(token);
                parseBrush();
                token = m_tokenizer.nextToken();
            }
        }
        
        void StandardMapParser::parseBrushFaces(const Model::MapFormat::Type format) {
            setFormat(format);

            Token token = m_tokenizer.nextToken();
            while (token.type() != QuakeMapToken::Eof) {
                expect(QuakeMapToken::OParenthesis, token);
                m_tokenizer.pushToken(token);
                parseFace();
                token = m_tokenizer.nextToken();
            }
        }

        void StandardMapParser::reset() {
            m_tokenizer.reset();
        }

        void StandardMapParser::setFormat(const Model::MapFormat::Type format) {
            assert(format != Model::MapFormat::Unknown);
            m_format = format;
            formatSet(format);
        }
        
        void StandardMapParser::parseEntityOrBrush() {
            Token first = m_tokenizer.nextToken();
            assert(first.type() == QuakeMapToken::OBrace);
            Token second = m_tokenizer.nextToken();
            expect(QuakeMapToken::String | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, second);
            if (second.type() == QuakeMapToken::String) {
                m_tokenizer.pushToken(second);
                m_tokenizer.pushToken(first);
                parseEntity();
            } else if (second.type() == QuakeMapToken::OParenthesis) {
                m_tokenizer.pushToken(second);
                m_tokenizer.pushToken(first);
                parseBrush();
            }
        }

        void StandardMapParser::parseEntity() {
            Token token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                return;
            
            expect(QuakeMapToken::OBrace, token);

            bool beginEntityCalled = false;
            Model::EntityAttribute::List attributes;
            ExtraAttributes extraAttributes;
            const size_t startLine = token.line();
            
            token = m_tokenizer.nextToken();
            while (token.type() != QuakeMapToken::Eof) {
                switch (token.type()) {
                    case QuakeMapToken::Comment:
                        parseExtraAttributes(extraAttributes);
                        break;
                    case QuakeMapToken::String:
                        m_tokenizer.pushToken(token);
                        parseEntityAttribute(attributes);
                        break;
                    case QuakeMapToken::OBrace:
                        m_tokenizer.pushToken(token);
                        if (!beginEntityCalled) {
                            beginEntity(startLine, attributes, extraAttributes);
                            beginEntityCalled = true;
                        }
                        parseBrush();
                        break;
                    case QuakeMapToken::CBrace:
                        if (!beginEntityCalled)
                            beginEntity(startLine, attributes, extraAttributes);
                        endEntity(startLine, token.line() - startLine);
                        return;
                    default:
                        expect(QuakeMapToken::Comment | QuakeMapToken::String | QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
                }
                
                token = m_tokenizer.nextToken();
            }
        }
        
        void StandardMapParser::parseEntityAttribute(Model::EntityAttribute::List& attributes) {
            Token token = m_tokenizer.nextToken();
            assert(token.type() == QuakeMapToken::String);
            const String name = token.data();
            
            expect(QuakeMapToken::String, token = m_tokenizer.nextToken());
            const String value = token.data();
            
            attributes.push_back(Model::EntityAttribute(name, value, NULL));
        }
        
        void StandardMapParser::parseBrush() {
            Token token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                return;
            
            expect(QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
            if (token.type() == QuakeMapToken::CBrace)
                return;
            
            bool beginBrushCalled = false;
            ExtraAttributes extraAttributes;
            const size_t startLine = token.line();

            token = m_tokenizer.nextToken();
            while (token.type() != QuakeMapToken::Eof) {
                switch (token.type()) {
                    case QuakeMapToken::Comment:
                        parseExtraAttributes(extraAttributes);
                        break;
                    case QuakeMapToken::OParenthesis:
                        m_tokenizer.pushToken(token);
                        if (!beginBrushCalled) {
                            beginBrush(startLine);
                            beginBrushCalled = true;
                        }
                        parseFace();
                        break;
                    case QuakeMapToken::CBrace:
                        if (!beginBrushCalled)
                            beginBrush(startLine);
                        endBrush(startLine, token.line() - startLine, extraAttributes);
                        return;
                    default: {
                        expect(QuakeMapToken::OParenthesis | QuakeMapToken::CParenthesis, token);
                    }
                }
                
                token = m_tokenizer.nextToken();
            }
        }
        
        void StandardMapParser::parseFace() {
            Vec3 texAxisX, texAxisY;
            
            Token token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                return;
            
            const size_t line = token.line();
            
            expect(QuakeMapToken::OParenthesis, token);
            const Vec3 p1 = parseVector().corrected();
            expect(QuakeMapToken::CParenthesis, token = m_tokenizer.nextToken());
            expect(QuakeMapToken::OParenthesis, token = m_tokenizer.nextToken());
            const Vec3 p2 = parseVector().corrected();
            expect(QuakeMapToken::CParenthesis, token = m_tokenizer.nextToken());
            expect(QuakeMapToken::OParenthesis, token = m_tokenizer.nextToken());
            const Vec3 p3 = parseVector().corrected();
            expect(QuakeMapToken::CParenthesis, token = m_tokenizer.nextToken());
            
            const Vec3 normal = crossed(p3 - p1, p2 - p1).normalized();
            if (normal.null()) {
                m_logger->warn("Skipping face at line %u: face points are colinear", static_cast<unsigned int>(token.line()));
                return;
            }
            
            // texture names can contain braces etc, so we just read everything until the next opening bracket or number
            String textureName = m_tokenizer.readAnyString(QuakeMapTokenizer::Whitespace);
            if (textureName == Model::BrushFace::NoTextureName)
                textureName = "";
            
            Model::BrushFaceAttributes attribs(textureName);
            if (m_format == Model::MapFormat::Valve) {
                expect(QuakeMapToken::OBracket, m_tokenizer.nextToken());
                texAxisX = parseVector();
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                attribs.setXOffset(token.toFloat<float>());
                expect(QuakeMapToken::CBracket, m_tokenizer.nextToken());
                
                expect(QuakeMapToken::OBracket, m_tokenizer.nextToken());
                texAxisY = parseVector();
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                attribs.setYOffset(token.toFloat<float>());
                expect(QuakeMapToken::CBracket, m_tokenizer.nextToken());
            } else {
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                attribs.setXOffset(token.toFloat<float>());
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                attribs.setYOffset(token.toFloat<float>());
            }
            
            expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
            attribs.setRotation(token.toFloat<float>());
            expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
            attribs.setXScale(token.toFloat<float>());
            expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
            attribs.setYScale(token.toFloat<float>());
            
            if (m_format == Model::MapFormat::Hexen2) {
                // noone seems to know what the extra face attribute in Hexen 2 maps does, so we discard it
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
            } else if (m_format == Model::MapFormat::Quake2) {
                // if there are additional values, then these are content and surface flags and surface value
                if (m_tokenizer.peekToken().type() == QuakeMapToken::Integer) {
                    token = m_tokenizer.nextToken();
                    attribs.setSurfaceContents(token.toInteger<int>());
                    expect(QuakeMapToken::Integer, token = m_tokenizer.nextToken());
                    attribs.setSurfaceFlags(token.toInteger<int>());
                    expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                    attribs.setSurfaceValue(token.toFloat<float>());
                }
            }
            
            brushFace(line, p1, p2, p3, attribs, texAxisX, texAxisY);
        }
        
        Vec3 StandardMapParser::parseVector() {
            Token token;
            Vec3 vec;
            
            for (size_t i = 0; i < 3; i++) {
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                vec[i] = token.toFloat<double>();
            }
            return vec;
        }

        void StandardMapParser::parseExtraAttributes(ExtraAttributes& attributes) {
            const SetBoolFun<QuakeMapTokenizer> parseEof(&m_tokenizer, &QuakeMapTokenizer::setSkipEol, false);
            Token token = m_tokenizer.nextToken();
            expect(QuakeMapToken::String | QuakeMapToken::Eol | QuakeMapToken::Eof, token);
            while (token.type() != QuakeMapToken::Eol && token.type() != QuakeMapToken::Eof) {
                const String name = token.data();
                expect(QuakeMapToken::String | QuakeMapToken::Integer, token = m_tokenizer.nextToken());
                const String value = token.data();
                const ExtraAttribute::Type type = token.type() == QuakeMapToken::String ? ExtraAttribute::Type_String : ExtraAttribute::Type_Integer;
                attributes.insert(std::make_pair(name, ExtraAttribute(type, name, value, token.line(), token.column())));
                expect(QuakeMapToken::String | QuakeMapToken::Eol | QuakeMapToken::Eof, token = m_tokenizer.nextToken());
            }
        }

        StandardMapParser::TokenNameMap StandardMapParser::tokenNames() const {
            using namespace QuakeMapToken;
            
            TokenNameMap names;
            names[Integer]      = "integer";
            names[Decimal]      = "decimal";
            names[String]       = "string";
            names[OParenthesis] = "'('";
            names[CParenthesis] = "')'";
            names[OBrace]       = "'{'";
            names[CBrace]       = "'}'";
            names[OBracket]     = "'['";
            names[CBracket]     = "']'";
            names[Comment]      = "comment";
            names[Eof]          = "end of file";
            return names;
        }
    }
}
