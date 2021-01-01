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

#include "DefParser.h"

#include "Exceptions.h"
#include "Assets/ModelDefinition.h"
#include "Assets/PropertyDefinition.h"
#include "IO/ELParser.h"
#include "IO/EntityDefinitionClassInfo.h"
#include "IO/LegacyModelDefinitionParser.h"
#include "IO/ParserStatus.h"
#include "Model/EntityProperties.h"

#include <kdl/string_format.h>
#include <kdl/vector_utils.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        DefTokenizer::DefTokenizer(std::string_view str) :
        Tokenizer(std::move(str), "", 0) {}

        const std::string DefTokenizer::WordDelims = " \t\n\r()[]{};,=";

        DefTokenizer::Token DefTokenizer::emitToken() {
            while (!eof()) {
                const size_t startLine = line();
                const size_t startColumn = column();
                const char* c = curPos();
                switch (*c) {
                    case '/': {
                        if (lookAhead() == '*') {
                            // eat all chars immediately after the '*' because it's often followed by QUAKE
                            do { advance(); } while (!eof() && !isWhitespace(curChar()));
                            return Token(DefToken::ODefinition, c, curPos(), offset(c), startLine, startColumn);
                        } else if (lookAhead() == '/') {
                            discardUntil("\n\r");
                            break;
                        }
                        // fall through and try to read as word
                        switchFallthrough();
                    }
                    case '*': {
                        if (lookAhead() == '/') {
                            advance();
                            return Token(DefToken::CDefinition, c, curPos(), offset(c), startLine, startColumn);
                        }
                        // fall through and try to read as word
                        switchFallthrough();
                    }
                    case '(':
                        advance();
                        return Token(DefToken::OParenthesis, c, c + 1, offset(c), startLine, startColumn);
                    case ')':
                        advance();
                        return Token(DefToken::CParenthesis, c, c + 1, offset(c), startLine, startColumn);
                    case '{':
                        advance();
                        return Token(DefToken::OBrace, c, c + 1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        return Token(DefToken::CBrace, c, c + 1, offset(c), startLine, startColumn);
                    case '=':
                        advance();
                        return Token(DefToken::Equality, c, c + 1, offset(c), startLine, startColumn);
                    case ';':
                        advance();
                        return Token(DefToken::Semicolon, c, c + 1, offset(c), startLine, startColumn);
                    case '\r':
                        if (lookAhead() == '\n') {
                            advance();
                        }
                        // handle carriage return without consecutive linefeed
                        // by falling through into the line feed case
                        switchFallthrough();
                    case '\n':
                        advance();
                        return Token(DefToken::Newline, c, c + 1, offset(c), startLine, startColumn);
                    case ',':
                        advance();
                        return Token(DefToken::Comma, c, c + 1, offset(c), startLine, startColumn);
                    case ' ':
                    case '\t':
                        discardWhile(" \t");
                        break;
                    case '"': { // quoted string
                        advance();
                        c = curPos();
                        const char* e = readQuotedString();
                        return Token(DefToken::QuotedString, c, e, offset(c), startLine, startColumn);
                    }
                    case '-':
                        if (isWhitespace(lookAhead())) {
                            advance();
                            return Token(DefToken::Minus, c, c + 1, offset(c), startLine, startColumn);
                        }
                        // otherwise fallthrough, might be a negative number
                        switchFallthrough();
                    default: { // integer, decimal or word
                        const char* e = readInteger(WordDelims);
                        if (e != nullptr)
                            return Token(DefToken::Integer, c, e, offset(c), startLine, startColumn);
                        e = readDecimal(WordDelims);
                        if (e != nullptr)
                            return Token(DefToken::Decimal, c, e, offset(c), startLine, startColumn);
                        e = readUntil(WordDelims);
                        if (e == nullptr)
                            throw ParserException(startLine, startColumn, "Unexpected character: " + std::string(c, 1));
                        return Token(DefToken::Word, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(DefToken::Eof, nullptr, nullptr, length(), line(), column());
        }

        DefParser::DefParser(std::string_view str, const Color& defaultEntityColor) :
        EntityDefinitionParser(defaultEntityColor),
        m_tokenizer(DefTokenizer(std::move(str))) {}

        DefParser::TokenNameMap DefParser::tokenNames() const {
            using namespace DefToken;

            TokenNameMap names;
            names[Integer]      = "integer";
            names[Decimal]      = "decimal";
            names[QuotedString] = "quoted string";
            names[OParenthesis] = "'('";
            names[CParenthesis] = "')'";
            names[OBrace]       = "'{'";
            names[CBrace]       = "'}'";
            names[Word]         = "word";
            names[ODefinition]  = "'/*'";
            names[CDefinition]  = "'*/'";
            names[Semicolon]    = "';'";
            names[Newline]      = "newline";
            names[Comma]        = "','";
            names[Equality]     = "'='";
            names[Minus]        = "'-'";
            names[Eof]          = "end of file";
            return names;
        }

        std::vector<EntityDefinitionClassInfo> DefParser::parseClassInfos(ParserStatus& status) {
            std::vector<EntityDefinitionClassInfo> result;

            auto classInfo = parseClassInfo(status);
            status.progress(m_tokenizer.progress());
            
            while (classInfo) {
                result.push_back(std::move(*classInfo));
                classInfo = parseClassInfo(status);
                status.progress(m_tokenizer.progress());
            }
            
            return result;
        }

        std::optional<EntityDefinitionClassInfo> DefParser::parseClassInfo(ParserStatus& status) {
            Token token = m_tokenizer.nextToken();
            while (token.type() != DefToken::Eof && token.type() != DefToken::ODefinition) {
                token = m_tokenizer.nextToken();
            }
            if (token.type() == DefToken::Eof) {
                return std::nullopt;
            }

            expect(status, DefToken::ODefinition, token);
            
            EntityDefinitionClassInfo classInfo;
            classInfo.type = EntityDefinitionClassType::BaseClass;
            classInfo.line = token.line();
            classInfo.column = token.column();

            token = expect(status, DefToken::Word, m_tokenizer.nextToken());
            classInfo.name = token.data();

            token = expect(status, DefToken::OParenthesis | DefToken::Newline, m_tokenizer.peekToken());
            if (token.type() == DefToken::OParenthesis) {
                classInfo.type = EntityDefinitionClassType::BrushClass;
                classInfo.color = parseColor(status);

                token = expect(status, DefToken::OParenthesis | DefToken::Word, m_tokenizer.peekToken());
                if (token.hasType(DefToken::OParenthesis)) {
                    classInfo.size = parseBounds(status);
                    classInfo.type = EntityDefinitionClassType::PointClass;
                } else if (token.data() == "?") {
                    m_tokenizer.nextToken();
                }

                token = m_tokenizer.peekToken();
                if (token.hasType(DefToken::Word | DefToken::Minus)) {
                    if (!addPropertyDefinition(classInfo.propertyDefinitions, parseSpawnflags(status))) {
                        status.warn(token.line(), token.column(), "Skipping duplicate spawnflags property definition");
                    }
                }
            }

            expect(status, DefToken::Newline, m_tokenizer.nextToken());

            parseProperties(status, classInfo);
            classInfo.description = kdl::str_trim(parseDescription());

            expect(status, DefToken::CDefinition, m_tokenizer.nextToken());

            return classInfo;
        }

        DefParser::PropertyDefinitionPtr DefParser::parseSpawnflags(ParserStatus& /* status */) {
            auto definition = std::make_shared<Assets::FlagsPropertyDefinition>(Model::PropertyKeys::Spawnflags);
            size_t numOptions = 0;

            Token token = m_tokenizer.peekToken();
            while (token.hasType(DefToken::Word | DefToken::Minus)) {
                token = m_tokenizer.nextToken();
                const auto name = token.hasType(DefToken::Word) ? token.data() : "";
                const auto value = 1 << numOptions++;
                definition->addOption(value, name, "", false);
                token = m_tokenizer.peekToken();
            }
            
            return definition;
        }

        void DefParser::parseProperties(ParserStatus& status, EntityDefinitionClassInfo& classInfo) {
            if (m_tokenizer.peekToken().type() == DefToken::OBrace) {
                m_tokenizer.nextToken();
                while (parseProperty(status, classInfo));
            }
        }

        bool DefParser::parseProperty(ParserStatus& status, EntityDefinitionClassInfo& classInfo) {
            Token token = expect(status, DefToken::Word | DefToken::CBrace, nextTokenIgnoringNewlines());
            if (token.type() != DefToken::Word)
                return false;

            const auto line = token.line();
            const auto column = token.column();

            std::string typeName = token.data();
            if (typeName == "default") {
                // ignore these properties
                parseDefaultProperty(status);
            } else if (typeName == "base") {
                classInfo.superClasses.push_back(parseBaseProperty(status));
            } else if (typeName == "choice") {
                auto propertyDefinition = parseChoicePropertyDefinition(status);
                if (!addPropertyDefinition(classInfo.propertyDefinitions, propertyDefinition)) {
                    status.warn(line, column, "Skipping duplicate property definition: " + propertyDefinition->key());
                }
            } else if (typeName == "model") {
                classInfo.modelDefinition = parseModelDefinition(status);
            }

            expect(status, DefToken::Semicolon, nextTokenIgnoringNewlines());
            return true;
        }

        void DefParser::parseDefaultProperty(ParserStatus& status) {
            // Token token;
            expect(status, DefToken::OParenthesis, nextTokenIgnoringNewlines());
            expect(status, DefToken::QuotedString, nextTokenIgnoringNewlines());
            // const std::string propertyName = token.data();
            expect(status, DefToken::Comma, nextTokenIgnoringNewlines());
            expect(status, DefToken::QuotedString, nextTokenIgnoringNewlines());
            // const std::string propertyValue = token.data();
            expect(status, DefToken::CParenthesis, nextTokenIgnoringNewlines());
        }

        std::string DefParser::parseBaseProperty(ParserStatus& status) {
            expect(status, DefToken::OParenthesis, nextTokenIgnoringNewlines());
            Token token = expect(status, DefToken::QuotedString, nextTokenIgnoringNewlines());
            const std::string basename = token.data();
            expect(status, DefToken::CParenthesis, nextTokenIgnoringNewlines());

            return basename;
        }

        DefParser::PropertyDefinitionPtr DefParser::parseChoicePropertyDefinition(ParserStatus& status) {
            Token token = expect(status, DefToken::QuotedString, m_tokenizer.nextToken());
            const std::string propertyKey = token.data();

            Assets::ChoicePropertyOption::List options;
            expect(status, DefToken::OParenthesis, nextTokenIgnoringNewlines());
            token = nextTokenIgnoringNewlines();
            while (token.type() == DefToken::OParenthesis) {
                token = expect(status, DefToken::Integer, nextTokenIgnoringNewlines());
                const std::string name = token.data();

                expect(status, DefToken::Comma, nextTokenIgnoringNewlines());
                token = expect(status, DefToken::QuotedString, nextTokenIgnoringNewlines());
                const std::string value = token.data();
                options.push_back(Assets::ChoicePropertyOption(name, value));

                expect(status, DefToken::CParenthesis, nextTokenIgnoringNewlines());
                token = nextTokenIgnoringNewlines();
            }

            expect(status, DefToken::CParenthesis, token);

            return DefParser::PropertyDefinitionPtr(new Assets::ChoicePropertyDefinition(propertyKey, "", "", options, false));
        }

        Assets::ModelDefinition DefParser::parseModelDefinition(ParserStatus& status) {
            expect(status, DefToken::OParenthesis, m_tokenizer.nextToken());

            const auto snapshot = m_tokenizer.snapshot();
            const auto line = m_tokenizer.line();
            const auto column = m_tokenizer.column();

            try {
                ELParser parser(m_tokenizer);
                auto expression = parser.parse();
                expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());

                expression.optimize();
                return Assets::ModelDefinition(expression);
            } catch (const ParserException& e) {
                try {
                    m_tokenizer.restore(snapshot);

                    LegacyModelDefinitionParser parser(m_tokenizer);
                    auto expression = parser.parse(status);
                    expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());

                    expression.optimize();
                    status.warn(line, column, "Legacy model expressions are deprecated, replace with '" + expression.asString() + "'");
                    return Assets::ModelDefinition(expression);
                } catch (const ParserException&) {
                    m_tokenizer.restore(snapshot);
                    throw e;
                }
            }
        }

        std::string DefParser::parseDescription() {
            Token token = m_tokenizer.peekToken();
            if (token.type() == DefToken::CDefinition) {
                return "";
            }
            return m_tokenizer.readRemainder(DefToken::CDefinition);
        }

        vm::vec3 DefParser::parseVector(ParserStatus& status) {
            vm::vec3 vec;
            for (size_t i = 0; i < 3; i++) {
                Token token = expect(status, DefToken::Integer | DefToken::Decimal, m_tokenizer.nextToken());
                vec[i] = token.toFloat<double>();
            }
            return vec;
        }

        vm::bbox3 DefParser::parseBounds(ParserStatus& status) {
            vm::bbox3 bounds;
            expect(status, DefToken::OParenthesis, m_tokenizer.nextToken());
            bounds.min = parseVector(status);
            expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());
            expect(status, DefToken::OParenthesis, m_tokenizer.nextToken());
            bounds.max = parseVector(status);
            expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());
            return repair(bounds);
        }

        Color DefParser::parseColor(ParserStatus& status) {
            Color color;
            Token token;
            expect(status, DefToken::OParenthesis, m_tokenizer.nextToken());
            for (size_t i = 0; i < 3; i++) {
                token = expect(status, DefToken::Decimal | DefToken::Integer, m_tokenizer.nextToken());
                color[i] = token.toFloat<float>();
                if (color[i] > 1.0f)
                    color[i] /= 255.0f;
            }
            expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());
            color[3] = 1.0f;
            return color;
        }

        DefParser::Token DefParser::nextTokenIgnoringNewlines() {
            Token token = m_tokenizer.nextToken();
            while (token.type() == DefToken::Newline) {
                token = m_tokenizer.nextToken();
            }
            return token;
        }
    }
}
