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

#include "IO/ParserStatus.h"
#include "Model/BrushFace.h"
#include "Model/EntityAttributes.h"

#include <kdl/invoke.h>
#include <kdl/vector_set.h>

#include <vecmath/plane.h>
#include <vecmath/vec.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        const std::string& QuakeMapTokenizer::NumberDelim() {
            static const std::string numberDelim(Whitespace() + ")");
            return numberDelim;
        }

        QuakeMapTokenizer::QuakeMapTokenizer(std::string_view str) :
        Tokenizer(std::move(str), "\"", '\\'),
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
                    case ';':
                        // Heretic2 allows semicolon to start a line comment.
                        // QuArK writes comments in this format when saving a Heretic2 .map.
                        advance();
                        discardUntil("\n\r");
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
                    case '\r':
                        if (lookAhead() == '\n') {
                            advance();
                        }
                        // handle carriage return without consecutive linefeed
                        // by falling through into the line feed case
                        switchFallthrough();
                    case '\n':
                        if (!m_skipEol) {
                            advance();
                            return Token(QuakeMapToken::Eol, c, c+1, offset(c), startLine, startColumn);
                        }
                        switchFallthrough();
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
                            throw ParserException(startLine, startColumn, "Unexpected character: " + std::string(c, 1));
                        }

                        return Token(QuakeMapToken::String, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(QuakeMapToken::Eof, nullptr, nullptr, length(), line(), column());
        }

        const std::string StandardMapParser::BrushPrimitiveId = "brushDef";
        const std::string StandardMapParser::PatchId = "patchDef2";

        StandardMapParser::StandardMapParser(std::string_view str) :
        m_tokenizer(QuakeMapTokenizer(std::move(str))),
        m_format(Model::MapFormat::Unknown) {}

        StandardMapParser::~StandardMapParser() = default;

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
                    parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis);
                }

                // texture names can contain braces etc, so we just read everything until the next opening bracket or number
                m_tokenizer.readRemainder(QuakeMapToken::OBracket | QuakeMapToken::Number);
                expect(QuakeMapToken::Number | QuakeMapToken::OBracket, token = m_tokenizer.nextToken());
                if (token.type() == QuakeMapToken::OBracket) {
                    format = Model::MapFormat::Valve;
                    // TODO: Could also be Model::MapFormat::Quake2_Valve or Model::MapFormat::Quake3_Valve, handle this case.
                }
            }

            if (format == Model::MapFormat::Unknown) {
                expect(QuakeMapToken::Number, m_tokenizer.nextToken()); // y offset
                expect(QuakeMapToken::Number, m_tokenizer.nextToken()); // rotation
                expect(QuakeMapToken::Number, m_tokenizer.nextToken()); // x scale
                expect(QuakeMapToken::Number, m_tokenizer.nextToken()); // y scale
                token = expect(QuakeMapToken::Number | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, m_tokenizer.nextToken());

                if (token.type() == QuakeMapToken::OParenthesis || token.type() == QuakeMapToken::CBrace) {
                    format = Model::MapFormat::Standard;
                }
            }

            if (format == Model::MapFormat::Unknown) {
                token = expect(QuakeMapToken::Number | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, m_tokenizer.nextToken()); // unknown Hexen 2 flag or Quake 2 surface contents
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
                parseBrushOrBrushPrimitiveOrPatch(status);
                token = m_tokenizer.peekToken();
            }
        }

        void StandardMapParser::parseBrushFaces(const Model::MapFormat format, ParserStatus& status) {
            setFormat(format);

            auto token = m_tokenizer.peekToken();
            while (token.type() != QuakeMapToken::Eof) {
                expect(QuakeMapToken::OParenthesis, token);
                // TODO 2427: detect the face type when parsing Quake3 map faces!
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

            auto attributes = std::vector<Model::EntityAttribute>();
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
                        parseBrushOrBrushPrimitiveOrPatch(status);
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

        void StandardMapParser::parseEntityAttribute(std::vector<Model::EntityAttribute>& attributes, AttributeNames& names, ParserStatus& status) {
            auto token = m_tokenizer.nextToken();
            assert(token.type() == QuakeMapToken::String);
            const auto name = token.data();

            const auto line = token.line();
            const auto column = token.column();

            expect(QuakeMapToken::String, token = m_tokenizer.nextToken());
            const auto value = token.data();

            if (names.count(name) == 0) {
                attributes.push_back(Model::EntityAttribute(name, value));
                names.insert(name);
            } else {
                status.warn(line, column, "Ignoring duplicate entity property '" + name + "'");
            }
        }

        void StandardMapParser::parseBrushOrBrushPrimitiveOrPatch(ParserStatus& status) {
            // consume initial opening brace
            auto token = expect(QuakeMapToken::OBrace | QuakeMapToken::CBrace | QuakeMapToken::Eof, m_tokenizer.nextToken());

            if (token.hasType(QuakeMapToken::Eof | QuakeMapToken::CBrace)) {
                return;
            }

            const auto startLine = token.line();

            token = m_tokenizer.peekToken();
            if (m_format == Model::MapFormat::Quake3) {
                // We expect either a brush primitive, a patch or a regular brush.
                expect(QuakeMapToken::String | QuakeMapToken::OParenthesis, token);
                if (token.hasType(QuakeMapToken::String)) {
                    expect(std::vector<std::string>({ BrushPrimitiveId, PatchId }), token);
                    if (token.data() == BrushPrimitiveId) {
                        parseBrushPrimitive(status, startLine);
                    } else {
                        parsePatch(status, startLine);
                    }
                } else {
                    parseBrush(status, startLine, false);
                }
            } else if (m_format == Model::MapFormat::Quake3_Valve ||
                       m_format == Model::MapFormat::Quake3_Legacy) {
                // We expect either a patch or a regular brush.
                expect(QuakeMapToken::String | QuakeMapToken::OParenthesis, token);
                if (token.hasType(QuakeMapToken::String)) {
                    expect(PatchId, token);
                    parsePatch(status, startLine);
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
            expect(BrushPrimitiveId, token);
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
                        // TODO 2427: handle brush primitives
                        if (!beginBrushCalled && !primitive) {
                            beginBrush(startLine, status);
                            beginBrushCalled = true;
                        }
                        parseFace(status, primitive);
                        break;
                    case QuakeMapToken::CBrace:
                        // TODO 2427: handle brush primitives
                        if (!primitive) {
                            if (!beginBrushCalled) {
                                beginBrush(startLine, status);
                            }
                            endBrush(startLine, token.line() - startLine, extraAttributes, status);
                        } else {
                            status.warn(startLine, "Skipping brush primitive: currently not supported");
                        }
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
                case Model::MapFormat::Quake2_Valve:
                case Model::MapFormat::Quake3_Valve:
                    parseQuake2ValveFace(status);
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
                    break;
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

            standardBrushFace(line, m_format, p1, p2, p3, attribs, status);
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

            standardBrushFace(line, m_format, p1, p2, p3, attribs, status);
        }

        void StandardMapParser::parseQuake2ValveFace(ParserStatus& status) {
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

            // Quake 2 extra info is optional
            if (!check(QuakeMapToken::OParenthesis | QuakeMapToken::CBrace | QuakeMapToken::Eof, m_tokenizer.peekToken())) {
                attribs.setSurfaceContents(parseInteger());
                attribs.setSurfaceFlags(parseInteger());
                attribs.setSurfaceValue(parseFloat());
            }

            valveBrushFace(line, m_format, p1, p2, p3, attribs, texX, texY, status);
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

            standardBrushFace(line, m_format, p1, p2, p3, attribs, status);
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
            if (check(QuakeMapToken::Integer, m_tokenizer.peekToken())) {
                attribs.setSurfaceContents(parseInteger());
                attribs.setSurfaceFlags(parseInteger());
                attribs.setSurfaceValue(parseFloat());

                // Daikatana color triple is optional
                if (check(QuakeMapToken::Integer, m_tokenizer.peekToken())) {
                    // red, green, blue
                    attribs.setColor(Color(parseInteger(), parseInteger(), parseInteger()));
                }
            }

            standardBrushFace(line, m_format, p1, p2, p3, attribs, status);
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

            valveBrushFace(line, m_format, p1, p2, p3, attribs, texX, texY, status);
        }

        void StandardMapParser::parsePrimitiveFace(ParserStatus& status) {
            /* const auto line = */ m_tokenizer.line();

            /* const auto [p1, p2, p3] = */ parseFacePoints(status);
            

            expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());

            /* const auto [texX, texY] = */ parsePrimitiveTextureAxes(status);
            expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

            const auto textureName = parseTextureName(status);

            // TODO 2427: what to set for offset, rotation, scale?!
            auto attribs = Model::BrushFaceAttributes(textureName);

            // Quake 2 extra info is optional
            if (!check(QuakeMapToken::OParenthesis | QuakeMapToken::CBrace | QuakeMapToken::Eof, m_tokenizer.peekToken())) {
                attribs.setSurfaceContents(parseInteger());
                attribs.setSurfaceFlags(parseInteger());
                attribs.setSurfaceValue(parseFloat());
            }

            // TODO 2427: create a brush face
            // brushFace(line, p1, p2, p3, attribs, texX, texY, status);
        }

        void StandardMapParser::parsePatch(ParserStatus& status, const size_t startLine) {
            auto token = expect(QuakeMapToken::String, m_tokenizer.nextToken());
            expect(PatchId, token);
            expect(QuakeMapToken::OBrace, m_tokenizer.nextToken());

            parseTextureName(status);
            expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());

            token = expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
            auto h = token.toInteger<int>();
            if (h < 0) {
                status.warn(token.line(), token.column(), "Negative patch height, assuming 0");
                h = 0;
            }

            token = expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
            auto w = token.toInteger<int>();
            if (w < 0) {
                status.warn(token.line(), token.column(), "Negative patch width, assuming 0");
                w = 0;
            }

            expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
            expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
            expect(QuakeMapToken::Integer, m_tokenizer.nextToken());
            expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

            expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());
            for (size_t i = 0; i < size_t(h); ++i) {
                expect(QuakeMapToken::OParenthesis, m_tokenizer.nextToken());
                for (size_t j = 0; j < size_t(w); ++j) {
                    parseFloatVector<5>(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis);
                }
                expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());
            }
            expect(QuakeMapToken::CParenthesis, m_tokenizer.nextToken());

            expect(QuakeMapToken::CBrace, m_tokenizer.nextToken());

            // TODO 2428: create the actual patch
            status.warn(startLine, "Skipping patch: currently not supported");
        }

        std::tuple<vm::vec3, vm::vec3, vm::vec3> StandardMapParser::parseFacePoints(ParserStatus& /* status */) {
            const auto p1 = correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
            const auto p2 = correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
            const auto p3 = correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));

            return std::make_tuple(p1, p2, p3);
        }

        std::string StandardMapParser::parseTextureName(ParserStatus& /* status */) {
            return m_tokenizer.readAnyString(QuakeMapTokenizer::Whitespace());
        }

        std::tuple<vm::vec3, float, vm::vec3, float> StandardMapParser::parseValveTextureAxes(ParserStatus& /* status */) {
            const auto firstAxis = parseFloatVector<4>(QuakeMapToken::OBracket, QuakeMapToken::CBracket);
            const auto texS = firstAxis.xyz();
            const auto xOffset = static_cast<float>(firstAxis.w());

            const auto secondAxis = parseFloatVector<4>(QuakeMapToken::OBracket, QuakeMapToken::CBracket);
            const auto texT = secondAxis.xyz();
            const auto yOffset = static_cast<float>(secondAxis.w());

            return std::make_tuple(texS, xOffset, texT, yOffset);
        }

        std::tuple<vm::vec3, vm::vec3> StandardMapParser::parsePrimitiveTextureAxes(ParserStatus& /* status */) {
            const auto texX = correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
            const auto texY = correct(parseFloatVector(QuakeMapToken::OParenthesis, QuakeMapToken::CParenthesis));
            return std::make_tuple(texX, texY);
        }

        float StandardMapParser::parseFloat() {
            return expect(QuakeMapToken::Number, m_tokenizer.nextToken()).toFloat<float>();
        }

        int StandardMapParser::parseInteger() {
            return expect(QuakeMapToken::Integer, m_tokenizer.nextToken()).toInteger<int>();
        }

        void StandardMapParser::parseExtraAttributes(ExtraAttributes& attributes, ParserStatus& /* status */) {
            // do not skip EOLs for the duration of this function call
            m_tokenizer.setSkipEol(false);
            const kdl::invoke_later parseEof([this]() { m_tokenizer.setSkipEol(true); });

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
