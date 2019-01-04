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

#include "StandardMapParser.h"

#include "Logger.h"
#include "TemporarilySetAny.h"
#include "Model/BrushFace.h"

#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace IO {
        const String& QuakeMapTokenizer::NumberDelim() {
            static const String numberDelim(Whitespace() + ")");
            return numberDelim;
        }

        QuakeMapTokenizer::QuakeMapTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end, "\"", '\\'),
        m_skipEol(true) {}
        
        QuakeMapTokenizer::QuakeMapTokenizer(const String& str) :
        Tokenizer(str, "\"", '\\'),
        m_skipEol(true) {}
        
        void QuakeMapTokenizer::setSkipEol(bool skipEol) {
            m_skipEol = skipEol;
        }
        
        QuakeMapTokenizer::Token QuakeMapTokenizer::emitToken() {
            while (!eof()) {
                auto startLine = line();
                auto startColumn = column();
                const auto* c = curPos();
                switch (*c) {
                    case '/':
                        advance();
                        if (curChar() == '/') {
                            advance();
                            if (curChar() == '/' && lookAhead(1) == ' ') {
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
                        const auto* e = readQuotedString('"', "\n}");
                        return Token(QuakeMapToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    case '\n':
                        if (!m_skipEol) {
                            advance();
                            return Token(QuakeMapToken::Eol, c, c+1, offset(c), startLine, startColumn);
                        }
                        switchFallthrough();
                    case '\r':
                    case ' ':
                    case '\t':
                        discardWhile(Whitespace());
                        break;
                    default: { // whitespace, integer, decimal or word
                        const auto* e = readInteger(NumberDelim());
                        if (e != nullptr) {
                            return Token(QuakeMapToken::Integer, c, e, offset(c), startLine, startColumn);
                        }

                        e = readDecimal(NumberDelim());
                        if (e != nullptr) {
                            return Token(QuakeMapToken::Decimal, c, e, offset(c), startLine, startColumn);
                        }

                        e = readUntil(Whitespace());
                        if (e == nullptr) {
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        }

                        return Token(QuakeMapToken::String, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(QuakeMapToken::Eof, nullptr, nullptr, length(), line(), column());
        }

        StandardMapParser::StandardMapParser(const char* begin, const char* end) :
        m_tokenizer(QuakeMapTokenizer(begin, end)),
        m_format(Model::MapFormat::Unknown) {}
        
        StandardMapParser::StandardMapParser(const String& str) :
        m_tokenizer(QuakeMapTokenizer(str)),
        m_format(Model::MapFormat::Unknown) {}
        
        StandardMapParser::~StandardMapParser() {}

        Model::MapFormat StandardMapParser::detectFormat() {
            auto format = Model::MapFormat::Unknown;
            
            // try to find an opening parenthesis
            auto token = m_tokenizer.peekToken();
            while (token.type() != QuakeMapToken::OParenthesis &&
                   token.type() != QuakeMapToken::Eof) {
                m_tokenizer.nextToken();
                token = m_tokenizer.peekToken();
            }

            if (token.type() == QuakeMapToken::Eof) {
                format = Model::MapFormat::Standard;
            }

            if (format == Model::MapFormat::Unknown) {
                for (size_t i = 0; i < 3; ++i) {
                    expect(QuakeMapToken::OParenthesis, token = m_tokenizer.nextToken());
                    parseVector();
                    expect(QuakeMapToken::CParenthesis, token = m_tokenizer.nextToken());
                }
                
                // texture names can contain braces etc, so we just read everything until the next opening bracket or number
                m_tokenizer.readRemainder(QuakeMapToken::OBracket | QuakeMapToken::Number);
                expect(QuakeMapToken::Number | QuakeMapToken::OBracket, token = m_tokenizer.nextToken());
                if (token.type() == QuakeMapToken::OBracket) {
                    format = Model::MapFormat::Valve;
                }
            }
            
            if (format == Model::MapFormat::Unknown) {
                expect(QuakeMapToken::Number, token = m_tokenizer.nextToken()); // y offset
                expect(QuakeMapToken::Number, token = m_tokenizer.nextToken()); // rotation
                expect(QuakeMapToken::Number, token = m_tokenizer.nextToken()); // x scale
                expect(QuakeMapToken::Number, token = m_tokenizer.nextToken()); // y scale
                expect(QuakeMapToken::Number | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, token = m_tokenizer.nextToken());

                if (token.type() == QuakeMapToken::OParenthesis || token.type() == QuakeMapToken::CBrace) {
                    format = Model::MapFormat::Standard;
                }
            }
            
            if (format == Model::MapFormat::Unknown) {
                expect(QuakeMapToken::Number | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, token = m_tokenizer.nextToken()); // unknown Hexen 2 flag or Quake 2 surface contents
                if (token.type() == QuakeMapToken::OParenthesis || token.type() == QuakeMapToken::CBrace) {
                    format = Model::MapFormat::Hexen2;
                } else {
                    format = Model::MapFormat::Quake2;
                }
            }
            
            m_tokenizer.reset();
            return format;
        }
        
        void StandardMapParser::parseEntities(const Model::MapFormat format, ParserStatus& status) {
            setFormat(format);

            auto token = m_tokenizer.peekToken();
            while (token.type() != QuakeMapToken::Eof) {
                expect(QuakeMapToken::OBrace, token);
                parseEntity(status);
                token = m_tokenizer.peekToken();
            }
        }
        
        void StandardMapParser::parseBrushes(const Model::MapFormat format, ParserStatus& status) {
            setFormat(format);

            auto token = m_tokenizer.peekToken();
            while (token.type() != QuakeMapToken::Eof) {
                expect(QuakeMapToken::OBrace, token);
                parseBrushOrBrushPrimitive(status);
                token = m_tokenizer.peekToken();
            }
        }
        
        void StandardMapParser::parseBrushFaces(const Model::MapFormat format, ParserStatus& status) {
            setFormat(format);

            auto token = m_tokenizer.peekToken();
            while (token.type() != QuakeMapToken::Eof) {
                expect(QuakeMapToken::OParenthesis, token);
                parseFace(status, false);
                token = m_tokenizer.peekToken();
            }
        }

        void StandardMapParser::reset() {
            m_tokenizer.reset();
        }

        void StandardMapParser::setFormat(const Model::MapFormat format) {
            assert(format != Model::MapFormat::Unknown);
            m_format = format;
            formatSet(format);
        }

        void StandardMapParser::parseEntity(ParserStatus& status) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof) {
                return;
            }

            expect(QuakeMapToken::OBrace, token);

            auto beginEntityCalled = false;
            
            auto attributes = Model::EntityAttribute::List();
            auto attributeNames = AttributeNames();
        
            auto extraAttributes = ExtraAttributes();
            const auto startLine = token.line();
            
            token = m_tokenizer.peekToken();
            while (token.type() != QuakeMapToken::Eof) {
                switch (token.type()) {
                    case QuakeMapToken::Comment:
                        m_tokenizer.nextToken();
                        parseExtraAttributes(extraAttributes, status);
                        break;
                    case QuakeMapToken::String:
                        parseEntityAttribute(attributes, attributeNames, status);
                        break;
                    case QuakeMapToken::OBrace:
                        if (!beginEntityCalled) {
                            beginEntity(startLine, attributes, extraAttributes, status);
                            beginEntityCalled = true;
                        }
                        parseBrushOrBrushPrimitive(status);
                        break;
                    case QuakeMapToken::CBrace:
                        m_tokenizer.nextToken();
                        if (!beginEntityCalled) {
                            beginEntity(startLine, attributes, extraAttributes, status);
                        }
                        endEntity(startLine, token.line() - startLine, status);
                        return;
                    default:
                        expect(QuakeMapToken::Comment | QuakeMapToken::String | QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
                }
                
                token = m_tokenizer.peekToken();
            }
        }
        
        void StandardMapParser::parseEntityAttribute(Model::EntityAttribute::List& attributes, AttributeNames& names, ParserStatus& status) {
            auto token = m_tokenizer.nextToken();
            assert(token.type() == QuakeMapToken::String);
            const auto name = token.data();
            
            const auto line = token.line();
            const auto column = token.column();
            
            expect(QuakeMapToken::String, token = m_tokenizer.nextToken());
            const auto value = token.data();
            
            if (names.count(name) == 0) {
                attributes.push_back(Model::EntityAttribute(name, value, nullptr));
                names.insert(name);
            } else {
                status.warn(line, column, "Ignoring duplicate entity property '" + name + "'");
            }
        }
        
        void StandardMapParser::parseBrushOrBrushPrimitive(ParserStatus& status) {
            // consume initial opening brace
            auto token = expect(QuakeMapToken::OBrace | QuakeMapToken::CBrace | QuakeMapToken::Eof, m_tokenizer.nextToken());

            if (token.hasType(QuakeMapToken::Eof | QuakeMapToken::CBrace)) {
                return;
            }

            const auto startLine = token.line();

            token = m_tokenizer.peekToken();
            if (m_format == Model::MapFormat::Quake3) {
                // We expect either a brush primitive or a regular brush.
                expect(QuakeMapToken::String | QuakeMapToken::OParenthesis, token);
                if (token.hasType(QuakeMapToken::String)) {
                    parseBrushPrimitive(status, startLine);
                } else {
                    parseBrush(status, startLine, false);
                }
            } else {
                expect(QuakeMapToken::OParenthesis, token);
                parseBrush(status, startLine, false);
            }

            // consume final closing brace
            expect(QuakeMapToken::CBrace, m_tokenizer.nextToken());
        }

        void StandardMapParser::parseBrushPrimitive(ParserStatus& status, const size_t startLine) {
            auto token = expect(QuakeMapToken::String, m_tokenizer.nextToken());
            expect("brushDef", token);
            expect(QuakeMapToken::OBrace, m_tokenizer.nextToken());
            parseBrush(status, startLine, true);
            expect(QuakeMapToken::CBrace, m_tokenizer.nextToken());
        }

        void StandardMapParser::parseBrush(ParserStatus& status, const size_t startLine, const bool primitive) {
            auto beginBrushCalled = false;
            auto extraAttributes = ExtraAttributes();

            auto token = m_tokenizer.peekToken();
            while (!token.hasType(QuakeMapToken::Eof)) {
                switch (token.type()) {
                    case QuakeMapToken::Comment:
                        m_tokenizer.nextToken();
                        parseExtraAttributes(extraAttributes, status);
                        break;
                    case QuakeMapToken::OParenthesis:
                        if (!beginBrushCalled) {
                            beginBrush(startLine, status);
                            beginBrushCalled = true;
                        }
                        parseFace(status, primitive);
                        break;
                    case QuakeMapToken::CBrace:
                        if (!beginBrushCalled) {
                            beginBrush(startLine, status);
                        }
                        endBrush(startLine, token.line() - startLine, extraAttributes, status);
                        return;
                    default: {
                        expect(QuakeMapToken::OParenthesis | QuakeMapToken::CParenthesis, token);
                    }
                }
                
                token = m_tokenizer.peekToken();
            }
        }
        
        void StandardMapParser::parseFace(ParserStatus& status, const bool primitive) {
            switch (m_format) {
                case Model::MapFormat::Standard:
                    parseQuakeFace(status);
                    break;
                case Model::MapFormat::Quake2:
                case Model::MapFormat::Quake3_Legacy:
                    parseQuake2Face(status);
                    break;
                case Model::MapFormat::Hexen2:
                    parseHexen2Face(status);
                    break;
                case Model::MapFormat::Daikatana:
                    parseDaikatanaFace(status);
                    break;
                case Model::MapFormat::Valve:
                    parseValveFace(status);
                    break;
                case Model::MapFormat::Quake3:
                    if (primitive) {
                        parsePrimitiveFace(status);
                    } else {
                        parseQuake2Face(status);
                    }
                case Model::MapFormat::Unknown:
                    // cannot happen
                    break;
                switchDefault()
            }
        }

        void StandardMapParser::parseQuakeFace(ParserStatus& status) {
            const auto line = m_tokenizer.line();

            const auto [p1, p2, p3] = parseFacePoints(status);
            const auto textureName = parseTextureName(status);

            auto attribs = Model::BrushFaceAttributes(textureName);
            attribs.setXOffset(parseFloat());
            attribs.setYOffset(parseFloat());
            attribs.setRotation(parseFloat());
            attribs.setXScale(parseFloat());
            attribs.setYScale(parseFloat());

            if (checkFacePoints(status, p1, p2, p3, line)) {
                brushFace(line, p1, p2, p3, attribs, vm::vec3::zero, vm::vec3::zero, status);
            }
        }

        void StandardMapParser::parseQuake2Face(ParserStatus& status) {
            const auto line = m_tokenizer.line();

            const auto [p1, p2, p3] = parseFacePoints(status);
            const auto textureName = parseTextureName(status);

            auto attribs = Model::BrushFaceAttributes(textureName);
            attribs.setXOffset(parseFloat());
            attribs.setYOffset(parseFloat());
            attribs.setRotation(parseFloat());
            attribs.setXScale(parseFloat());
            attribs.setYScale(parseFloat());

            // Quake 2 extra info is optional
            if (!check(QuakeMapToken::OParenthesis | QuakeMapToken::CBrace | QuakeMapToken::Eof, m_tokenizer.peekToken())) {
                attribs.setSurfaceContents(parseInteger());
                attribs.setSurfaceFlags(parseInteger());
                attribs.setSurfaceValue(parseFloat());
            }

            if (checkFacePoints(status, p1, p2, p3, line)) {
                brushFace(line, p1, p2, p3, attribs, vm::vec3::zero, vm::vec3::zero, status);
            }
        }

        void StandardMapParser::parseHexen2Face(ParserStatus& status) {
            const auto line = m_tokenizer.line();

            const auto [p1, p2, p3] = parseFacePoints(status);
            const auto textureName = parseTextureName(status);

            auto attribs = Model::BrushFaceAttributes(textureName);
            attribs.setXOffset(parseFloat());
            attribs.setYOffset(parseFloat());
            attribs.setRotation(parseFloat());
            attribs.setXScale(parseFloat());
            attribs.setYScale(parseFloat());

            // Hexen 2 extra info is optional
            if (!check(QuakeMapToken::OParenthesis | QuakeMapToken::CBrace | QuakeMapToken::Eof, m_tokenizer.peekToken())) {
                m_tokenizer.nextToken(); // noone seems to know what the extra value does in Hexen 2
            }

            if (checkFacePoints(status, p1, p2, p3, line)) {
                brushFace(line, p1, p2, p3, attribs, vm::vec3::zero, vm::vec3::zero, status);
            }
        }

        void StandardMapParser::parseDaikatanaFace(ParserStatus& status) {
            const auto line = m_tokenizer.line();

            const auto [p1, p2, p3] = parseFacePoints(status);
            const auto textureName = parseTextureName(status);

            auto attribs = Model::BrushFaceAttributes(textureName);
            attribs.setXOffset(parseFloat());
            attribs.setYOffset(parseFloat());
            attribs.setRotation(parseFloat());
            attribs.setXScale(parseFloat());
            attribs.setYScale(parseFloat());

            // Daikatana extra info is optional
            if (!check(QuakeMapToken::OParenthesis | QuakeMapToken::CBrace | QuakeMapToken::Eof, m_tokenizer.peekToken())) {
                attribs.setSurfaceContents(parseInteger());
                attribs.setSurfaceFlags(parseInteger());
                attribs.setSurfaceValue(parseFloat());

                // Daikatana color triple is optional
                if (check(QuakeMapToken::Integer, m_tokenizer.peekToken())) {
                    // red, green, blue
                    attribs.setColor(Color(parseInteger(), parseInteger(), parseInteger()));
                }
            }

            if (checkFacePoints(status, p1, p2, p3, line)) {
                brushFace(line, p1, p2, p3, attribs, vm::vec3::zero, vm::vec3::zero, status);
            }
        }

        void StandardMapParser::parseValveFace(ParserStatus& status) {
            const auto line = m_tokenizer.line();

            const auto [p1, p2, p3] = parseFacePoints(status);
            const auto textureName = parseTextureName(status);

            const auto [texX, xOffset, texY, yOffset] = parseValveTextureAxes(status);

            auto attribs = Model::BrushFaceAttributes(textureName);
            attribs.setXOffset(xOffset);
            attribs.setYOffset(yOffset);
            attribs.setRotation(parseFloat());
            attribs.setXScale(parseFloat());
            attribs.setYScale(parseFloat());

            if (checkFacePoints(status, p1, p2, p3, line)) {
                brushFace(line, p1, p2, p3, attribs, texX, texY, status);
            }
        }

        void StandardMapParser::parsePrimitiveFace(ParserStatus& status) {
            const auto line = m_tokenizer.line();

            const auto [p1, p2, p3] = parseFacePoints(status);

            expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());
            const auto [texX, texY] = parsePrimitiveTextureAxes(status);
            expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

            const auto textureName = parseTextureName(status);

            // TODO: what to set for offset, rotation, scale?!
            auto attribs = Model::BrushFaceAttributes(textureName);

            // Quake 2 extra info is optional
            if (!check(QuakeMapToken::OParenthesis | QuakeMapToken::CBrace | QuakeMapToken::Eof, m_tokenizer.peekToken())) {
                attribs.setSurfaceContents(parseInteger());
                attribs.setSurfaceFlags(parseInteger());
                attribs.setSurfaceValue(parseFloat());
            }

            if (checkFacePoints(status, p1, p2, p3, line)) {
                brushFace(line, p1, p2, p3, attribs, texX, texY, status);
            }
        }

        bool StandardMapParser::checkFacePoints(ParserStatus& status, const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3, const size_t line) const {
            const auto axis = cross(p3 - p1, p2 - p1);
            if (!isZero(axis, vm::C::almostZero())) {
                return true;
            } else {
                status.error(line, "Skipping face: face points are colinear");
                return false;
            }
        }

        std::tuple<vm::vec3, vm::vec3, vm::vec3> StandardMapParser::parseFacePoints(ParserStatus& status) {
            expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());
            const auto p1 = correct(parseVector());
            expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());
            expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());
            const auto p2 = correct(parseVector());
            expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());
            expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());
            const auto p3 = correct(parseVector());
            expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

            return std::make_tuple(p1, p2, p3);
        }

        String StandardMapParser::parseTextureName(ParserStatus& status) {
            auto textureName = m_tokenizer.readAnyString(QuakeMapTokenizer::Whitespace());
            if (textureName == Model::BrushFace::NoTextureName) {
                textureName = "";
            }
            return textureName;
        }

        std::tuple<vm::vec3, float, vm::vec3, float> StandardMapParser::parseValveTextureAxes(ParserStatus& status) {
            expect(QuakeMapToken::OBracket, m_tokenizer.nextToken());
            const auto texAxisX = parseVector();
            const auto xOffset = expect(QuakeMapToken::Number, m_tokenizer.nextToken()).toFloat<float>();
            expect(QuakeMapToken::CBracket, m_tokenizer.nextToken());

            expect(QuakeMapToken::OBracket, m_tokenizer.nextToken());
            const auto texAxisY = parseVector();
            const auto yOffset = expect(QuakeMapToken::Number, m_tokenizer.nextToken()).toFloat<float>();
            expect(QuakeMapToken::CBracket, m_tokenizer.nextToken());

            return std::make_tuple(texAxisX, xOffset, texAxisY, yOffset);
        }

        std::tuple<vm::vec3, vm::vec3> StandardMapParser::parsePrimitiveTextureAxes(ParserStatus& status) {
            expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());
            const auto texX = correct(parseVector());
            expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());
            expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());
            const auto texY = correct(parseVector());
            expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

            return std::make_tuple(texX, texY);
        }

        vm::vec3 StandardMapParser::parseVector() {
            vm::vec3 vec;
            for (size_t i = 0; i < 3; i++) {
                vec[i] = expect(QuakeMapToken::Number, m_tokenizer.nextToken()).toFloat<double>();
            }
            return vec;
        }

        float StandardMapParser::parseFloat() {
            return expect(QuakeMapToken::Number, m_tokenizer.nextToken()).toFloat<float>();
        }

        int StandardMapParser::parseInteger() {
            return expect(QuakeMapToken::Integer, m_tokenizer.nextToken()).toInteger<int>();
        }

        void StandardMapParser::parseExtraAttributes(ExtraAttributes& attributes, ParserStatus& status) {
            const TemporarilySetBoolFun<QuakeMapTokenizer> parseEof(&m_tokenizer, &QuakeMapTokenizer::setSkipEol, false);
            auto token = m_tokenizer.nextToken();
            expect(QuakeMapToken::String | QuakeMapToken::Eol | QuakeMapToken::Eof, token);
            while (token.type() != QuakeMapToken::Eol && token.type() != QuakeMapToken::Eof) {
                const auto name = token.data();
                expect(QuakeMapToken::String | QuakeMapToken::Integer, token = m_tokenizer.nextToken());
                const auto value = token.data();
                const auto type = token.type() == QuakeMapToken::String ? ExtraAttribute::Type_String : ExtraAttribute::Type_Integer;
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
