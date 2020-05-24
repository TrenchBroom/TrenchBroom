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

#include "FgdParser.h"

#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"
#include "IO/File.h"
#include "IO/DiskFileSystem.h"
#include "IO/ELParser.h"
#include "IO/LegacyModelDefinitionParser.h"
#include "IO/ParserStatus.h"

#include <kdl/string_compare.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        FgdTokenizer::FgdTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end, "", 0) {}

        FgdTokenizer::FgdTokenizer(const std::string& str) :
        Tokenizer(str, "", 0) {}

        const std::string FgdTokenizer::WordDelims = " \t\n\r()[]?;:,=";

        FgdTokenizer::Token FgdTokenizer::emitToken() {
            while (!eof()) {
                auto startLine = line();
                auto startColumn = column();
                const auto* c = curPos();

                switch (*c) {
                    case '/':
                        advance();
                        if (curChar() == '/') {
                            discardUntil("\n\r");
                        }
                        break;
                    case '(':
                        advance();
                        return Token(FgdToken::OParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case ')':
                        advance();
                        return Token(FgdToken::CParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case '[':
                        advance();
                        return Token(FgdToken::OBracket, c, c+1, offset(c), startLine, startColumn);
                    case ']':
                        advance();
                        return Token(FgdToken::CBracket, c, c+1, offset(c), startLine, startColumn);
                    case '=':
                        advance();
                        return Token(FgdToken::Equality, c, c+1, offset(c), startLine, startColumn);
                    case ',':
                        advance();
                        return Token(FgdToken::Comma, c, c+1, offset(c), startLine, startColumn);
                    case ':':
                        advance();
                        return Token(FgdToken::Colon, c, c+1, offset(c), startLine, startColumn);
                    case '"': { // quoted string
                        advance();
                        c = curPos();
                        const auto* e = readQuotedString();
                        return Token(FgdToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        discardWhile(Whitespace());
                        break;
                    case '+': { // string continuation
                        const auto snapshot = this->snapshot();
                        advance();

                        const auto* e = curPos();
                        discardWhile(Whitespace());

                        if (curChar() == '"') {
                            return Token(FgdToken::Plus, c, e, offset(c), startLine, startColumn);
                        } else {
                            restore(snapshot);
                            // fall through to allow reading numbers
                        }
                        switchFallthrough();
                    }
                    default: {
                        const auto* e = readInteger(WordDelims);
                        if (e != nullptr) {
                            return Token(FgdToken::Integer, c, e, offset(c), startLine, startColumn);
                        }

                        e = readDecimal(WordDelims);
                        if (e != nullptr) {
                            return Token(FgdToken::Decimal, c, e, offset(c), startLine, startColumn);
                        }

                        e = readUntil(WordDelims);
                        if (e == nullptr) {
                            throw ParserException(startLine, startColumn, "Unexpected character: '" + std::string(c, 1) + "'");
                        } else {
                            return Token(FgdToken::Word, c, e, offset(c), startLine, startColumn);
                        }
                    }
                }
            }
            return Token(FgdToken::Eof, nullptr, nullptr, length(), line(), column());
        }

        FgdParser::FgdParser(const char* begin, const char* end, const Color& defaultEntityColor, const Path& path) :
        m_defaultEntityColor(defaultEntityColor),
        m_tokenizer(FgdTokenizer(begin, end)) {
            if (!path.isEmpty()) {
                pushIncludePath(path);
            }
        }

        FgdParser::FgdParser(const std::string& str, const Color& defaultEntityColor, const Path& path) :
        FgdParser(str.c_str(), str.c_str() + str.size(), defaultEntityColor, path) {}

        FgdParser::FgdParser(const std::string& str, const Color& defaultEntityColor) :
        FgdParser(str, defaultEntityColor, Path()) {}

        FgdParser::TokenNameMap FgdParser::tokenNames() const {
            using namespace FgdToken;

            TokenNameMap names;
            names[Integer]      = "integer";
            names[Decimal]      = "decimal";
            names[Word]         = "word";
            names[String]       = "string";
            names[OParenthesis] = "'('";
            names[CParenthesis] = "')'";
            names[OBracket]     = "'['";
            names[CBracket]     = "']'";
            names[Equality]     = "'='";
            names[Colon]        = "':'";
            names[Comma]        = "','";
            names[Plus]         = "'+'";
            names[Eof]          = "end of file";
            return names;
        }

        class FgdParser::PushIncludePath {
        private:
            FgdParser* m_parser;
        public:
            PushIncludePath(FgdParser* parser, const Path& path) :
                m_parser(parser) {
                m_parser->pushIncludePath(path);
            }

            ~PushIncludePath() {
                m_parser->popIncludePath();
            }
        };

        void FgdParser::pushIncludePath(const Path& path) {
            ensure(path.isAbsolute(), "include path must be absolute");
            assert(!isRecursiveInclude(path));

            const auto folder = path.deleteLastComponent();
            m_fs = std::make_shared<DiskFileSystem>(m_fs, folder);
            m_paths.push_back(path);
        }

        void FgdParser::popIncludePath() {
            assert(!m_paths.empty());
            m_fs = m_fs->releaseNext();
            m_paths.pop_back();
        }

        bool FgdParser::isRecursiveInclude(const Path& path) const {
            for (const auto& includedPath : m_paths) {
                if (path == includedPath) {
                    return true;
                }
            }
            return false;
        }

        FgdParser::EntityDefinitionList FgdParser::doParseDefinitions(ParserStatus& status) {
            EntityDefinitionList definitions;
            try {
                auto token = m_tokenizer.peekToken();
                while (!token.hasType(FgdToken::Eof)) {
                    parseDefinitionOrInclude(status, definitions);
                    token = m_tokenizer.peekToken();
                }
                return definitions;
            } catch (...) {
                kdl::vec_clear_and_delete(definitions);
                throw;
            }
        }

        void FgdParser::parseDefinitionOrInclude(ParserStatus& status, EntityDefinitionList& definitions) {
            auto token = expect(status, FgdToken::Eof | FgdToken::Word, m_tokenizer.peekToken());
            if (token.hasType(FgdToken::Eof)) {
                return;
            }

            if (kdl::ci::str_is_equal(token.data(), "@include")) {
                const auto includedDefinitions = parseInclude(status);
                kdl::vec_append(definitions, includedDefinitions);
            } else {
                auto* definition = parseDefinition(status);
                status.progress(m_tokenizer.progress());
                if (definition != nullptr) {
                    definitions.push_back(definition);
                }
            }
        }

        Assets::EntityDefinition* FgdParser::parseDefinition(ParserStatus& status) {
            auto token = expect(status, FgdToken::Word, m_tokenizer.nextToken());

            const auto classname = token.data();
            if (kdl::ci::str_is_equal(classname, "@SolidClass")) {
                return parseSolidClass(status);
            } else if (kdl::ci::str_is_equal(classname, "@PointClass")) {
                return parsePointClass(status);
            } else if (kdl::ci::str_is_equal(classname, "@BaseClass")) {
                const auto baseClass = parseBaseClass(status);
                m_baseClasses[baseClass.name()] = baseClass;
                return nullptr;
            } else if (kdl::ci::str_is_equal(classname, "@Main")) {
                skipMainClass(status);
                return nullptr;
            } else {
                const auto msg = "Unknown entity definition class '" + classname + "'";
                status.error(token.line(), token.column(), msg);
                throw ParserException(token.line(), token.column(), msg);
            }
        }

        Assets::EntityDefinition* FgdParser::parseSolidClass(ParserStatus& status) {
            EntityDefinitionClassInfo classInfo = parseClass(status);
            if (classInfo.hasSize()) {
                status.warn(classInfo.line(), classInfo.column(), "Solid entity definition must not have a size");
            }
            if (classInfo.hasModelDefinition()) {
                status.warn(classInfo.line(), classInfo.column(), "Solid entity definition must not have model definitions");
            }
            return new Assets::BrushEntityDefinition(classInfo.name(), classInfo.color(), classInfo.description(), classInfo.attributeList());
        }

        Assets::EntityDefinition* FgdParser::parsePointClass(ParserStatus& status) {
            const auto classInfo = parseClass(status);
            return new Assets::PointEntityDefinition(classInfo.name(), classInfo.color(), classInfo.size(), classInfo.description(), classInfo.attributeList(), classInfo.modelDefinition());
        }

        EntityDefinitionClassInfo FgdParser::parseBaseClass(ParserStatus& status) {
            const auto classInfo = parseClass(status);
            if (m_baseClasses.count(classInfo.name()) > 0) {
                status.warn(classInfo.line(), classInfo.column(), "Redefinition of base class '" + classInfo.name() + "'");
            }
            return classInfo;
        }

        EntityDefinitionClassInfo FgdParser::parseClass(ParserStatus& status) {
            auto token = expect(status, FgdToken::Word | FgdToken::Equality, m_tokenizer.nextToken());

            std::vector<std::string> superClasses;
            EntityDefinitionClassInfo classInfo(token.line(), token.column(), m_defaultEntityColor);

            while (token.type() == FgdToken::Word) {
                const auto typeName = token.data();
                if (kdl::ci::str_is_equal(typeName, "base")) {
                    if (!superClasses.empty()) {
                        status.warn(token.line(), token.column(), "Found multiple base attributes");
                    }
                    superClasses = parseSuperClasses(status);
                } else if (kdl::ci::str_is_equal(typeName, "color")) {
                    if (classInfo.hasColor()) {
                        status.warn(token.line(), token.column(), "Found multiple color attributes");
                    }
                    classInfo.setColor(parseColor(status));
                } else if (kdl::ci::str_is_equal(typeName, "size")) {
                    if (classInfo.hasSize()) {
                        status.warn(token.line(), token.column(), "Found multiple size attributes");
                    }
                    classInfo.setSize(parseSize(status));
                } else if (kdl::ci::str_is_equal(typeName, "model") ||
                           kdl::ci::str_is_equal(typeName, "studio") ||
                           kdl::ci::str_is_equal(typeName, "studioprop")) {
                    if (classInfo.hasModelDefinition()) {
                        status.warn(token.line(), token.column(), "Found multiple model attributes");
                    }
                    classInfo.setModelDefinition(parseModel(status));
                } else {
                    status.warn(token.line(), token.column(), "Unknown entity definition header attribute '" + typeName + "'");
                    skipClassAttribute(status);
                }
                token = expect(status, FgdToken::Equality | FgdToken::Word, m_tokenizer.nextToken());
            }

            token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
            classInfo.setName(token.data());

            token = expect(status, FgdToken::Colon | FgdToken::OBracket, m_tokenizer.peekToken());
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                const auto description = parseString(status);
                classInfo.setDescription(kdl::str_trim(description));
            }

            classInfo.addAttributeDefinitions(parseProperties(status));
            classInfo.resolveBaseClasses(m_baseClasses, superClasses);
            return classInfo;
        }

        void FgdParser::skipMainClass(ParserStatus& status) {
            expect(status, FgdToken::Equality, m_tokenizer.nextToken());
            expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

            Token token;
            do {
                token = m_tokenizer.nextToken();
            } while (token.type() != FgdToken::CBracket);
        }

        std::vector<std::string> FgdParser::parseSuperClasses(ParserStatus& status) {
            expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());

            auto token = expect(status, FgdToken::Word | FgdToken::CParenthesis, m_tokenizer.peekToken());

            std::vector<std::string> superClasses;
            if (token.type() == FgdToken::Word) {
                do {
                    token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
                    superClasses.push_back(token.data());
                    token = expect(status, FgdToken::Comma | FgdToken::CParenthesis, m_tokenizer.nextToken());
                } while (token.type() == FgdToken::Comma);
            } else {
                m_tokenizer.nextToken();
            }
            return superClasses;
        }

        Assets::ModelDefinition FgdParser::parseModel(ParserStatus& status) {
            expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());

            const auto snapshot = m_tokenizer.snapshot();
            const auto line = m_tokenizer.line();
            const auto column = m_tokenizer.column();

            try {
                ELParser parser(m_tokenizer);
                auto expression = parser.parse();
                expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

                expression.optimize();
                return Assets::ModelDefinition(expression);
            } catch (const ParserException& e) {
                try {
                    m_tokenizer.restore(snapshot);

                    LegacyModelDefinitionParser parser(m_tokenizer);
                    auto expression = parser.parse(status);
                    expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

                    expression.optimize();
                    status.warn(line, column, "Legacy model expressions are deprecated, replace with '" + expression.asString() + "'");
                    return Assets::ModelDefinition(expression);
                } catch (const ParserException&) {
                    m_tokenizer.restore(snapshot);
                    throw e;
                }
            }
        }

        void FgdParser::skipClassAttribute(ParserStatus& /* status */) {
            size_t depth = 0;
            Token token;
            do {
                token = m_tokenizer.nextToken();
                if (token.type() == FgdToken::OParenthesis) {
                    ++depth;
                } else if (token.type() == FgdToken::CParenthesis) {
                    --depth;
                }
            } while (depth > 0 && token.type() != FgdToken::Eof);
        }

        FgdParser::AttributeDefinitionMap FgdParser::parseProperties(ParserStatus& status) {
            AttributeDefinitionMap attributes;

            expect(status, FgdToken::OBracket, m_tokenizer.nextToken());
            auto token = expect(status, FgdToken::Word | FgdToken::CBracket, m_tokenizer.nextToken());

            while (token.type() != FgdToken::CBracket) {
                const auto attributeKey = token.data();

                if (attributes.count(attributeKey) > 0) {
                    status.warn(token.line(), token.column(), "Redefinition of property declaration '" + attributeKey + "'");
                }

                expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());
                token = expect(status, FgdToken::Word, m_tokenizer.nextToken());

                const auto typeName = token.data();
                token = expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

                if (kdl::ci::str_is_equal(typeName, "target_source")) {
                    attributes[attributeKey] = parseTargetSourceAttribute(status, attributeKey);
                } else if (kdl::ci::str_is_equal(typeName, "target_destination")) {
                    attributes[attributeKey] = parseTargetDestinationAttribute(status, attributeKey);
                } else if (kdl::ci::str_is_equal(typeName, "string")) {
                    attributes[attributeKey] = parseStringAttribute(status, attributeKey);
                } else if (kdl::ci::str_is_equal(typeName, "integer")) {
                    attributes[attributeKey] = parseIntegerAttribute(status, attributeKey);
                } else if (kdl::ci::str_is_equal(typeName, "float")) {
                    attributes[attributeKey] = parseFloatAttribute(status, attributeKey);
                } else if (kdl::ci::str_is_equal(typeName, "choices")) {
                    attributes[attributeKey] = parseChoicesAttribute(status, attributeKey);
                } else if (kdl::ci::str_is_equal(typeName, "flags")) {
                    attributes[attributeKey] = parseFlagsAttribute(status, attributeKey);
                } else {
                    status.debug(token.line(), token.column(), kdl::str_to_string("Unknown property definition type '", typeName, "' for attribute '", attributeKey, "'"));
                    attributes[attributeKey] = parseUnknownAttribute(status, attributeKey);
                }

                token = expect(status, FgdToken::Word | FgdToken::CBracket, m_tokenizer.nextToken());
            }

            return attributes;
        }

        FgdParser::AttributeDefinitionPtr FgdParser::parseTargetSourceAttribute(ParserStatus& status, const std::string& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            parseDefaultStringValue(status);
            const auto longDescription = parseAttributeDescription(status);
            return std::make_shared<Assets::AttributeDefinition>(name, Assets::AttributeDefinitionType::TargetSourceAttribute, shortDescription, longDescription, readOnly);
        }

        FgdParser::AttributeDefinitionPtr FgdParser::parseTargetDestinationAttribute(ParserStatus& status, const std::string& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            parseDefaultStringValue(status);
            const auto longDescription = parseAttributeDescription(status);
            return std::make_shared<Assets::AttributeDefinition>(name, Assets::AttributeDefinitionType::TargetDestinationAttribute, shortDescription, longDescription, readOnly);
        }

        FgdParser::AttributeDefinitionPtr FgdParser::parseStringAttribute(ParserStatus& status, const std::string& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            const auto defaultValue = parseDefaultStringValue(status);
            const auto longDescription = parseAttributeDescription(status);
            return std::make_shared<Assets::StringAttributeDefinition>(name, shortDescription, longDescription, readOnly, defaultValue);
        }

        FgdParser::AttributeDefinitionPtr FgdParser::parseIntegerAttribute(ParserStatus& status, const std::string& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            const auto defaultValue = parseDefaultIntegerValue(status);
            const auto longDescription = parseAttributeDescription(status);
            return std::make_shared<Assets::IntegerAttributeDefinition>(name, shortDescription, longDescription, readOnly, defaultValue);
        }

        FgdParser::AttributeDefinitionPtr FgdParser::parseFloatAttribute(ParserStatus& status, const std::string& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            const auto defaultValue = parseDefaultFloatValue(status);
            const auto longDescription = parseAttributeDescription(status);
            return std::make_shared<Assets::FloatAttributeDefinition>(name, shortDescription, longDescription, readOnly, defaultValue);
        }

        FgdParser::AttributeDefinitionPtr FgdParser::parseChoicesAttribute(ParserStatus& status, const std::string& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            const auto defaultValue = parseDefaultChoiceValue(status);
            const auto longDescription = parseAttributeDescription(status);

            expect(status, FgdToken::Equality, m_tokenizer.nextToken());
            expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

            auto token = expect(status, FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket, m_tokenizer.nextToken());

            Assets::ChoiceAttributeOption::List options;
            while (token.type() != FgdToken::CBracket) {
                const auto value = token.data();
                expect(status, FgdToken::Colon, m_tokenizer.nextToken());
                const auto caption = parseString(status);

                options.emplace_back(value, caption);
                token = expect(status, FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket, m_tokenizer.nextToken());
            }

            return std::make_shared<Assets::ChoiceAttributeDefinition>(name, shortDescription, longDescription, options, readOnly, defaultValue);
        }

        FgdParser::AttributeDefinitionPtr FgdParser::parseFlagsAttribute(ParserStatus& status, const std::string& name) {
            // Flag attributes do not have descriptions or defaults, see https://developer.valvesoftware.com/wiki/FGD

            expect(status, FgdToken::Equality, m_tokenizer.nextToken());
            expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

            auto token = expect(status, FgdToken::Integer | FgdToken::CBracket, m_tokenizer.nextToken());

            auto definition = std::make_shared<Assets::FlagsAttributeDefinition>(name);

            while (token.type() != FgdToken::CBracket) {
                const auto value = token.toInteger<int>();
                expect(status, FgdToken::Colon, m_tokenizer.nextToken());
                const auto shortDescription = parseString(status);

                auto defaultValue = false;
                token = expect(status, FgdToken::Colon | FgdToken::Integer | FgdToken::CBracket, m_tokenizer.peekToken());
                if (token.type() == FgdToken::Colon) {
                    m_tokenizer.nextToken();
                    token = expect(status, FgdToken::Integer, m_tokenizer.nextToken());
                    defaultValue = token.toInteger<int>() != 0;
                }

                token = expect(status, FgdToken::Integer | FgdToken::CBracket | FgdToken::Colon, m_tokenizer.nextToken());

                std::string longDescription;
                if (token.type() == FgdToken::Colon) {
                    longDescription = parseString(status);
                    token = expect(status, FgdToken::Integer | FgdToken::CBracket, m_tokenizer.nextToken());
                }

                definition->addOption(value, shortDescription, longDescription, defaultValue);
            }
            return definition;
        }

        FgdParser::AttributeDefinitionPtr FgdParser::parseUnknownAttribute(ParserStatus& status, const std::string& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            const auto defaultValue = parseDefaultStringValue(status);
            const auto longDescription = parseAttributeDescription(status);
            return std::make_shared<Assets::UnknownAttributeDefinition>(name, shortDescription, longDescription, readOnly, defaultValue);
        }

        bool FgdParser::parseReadOnlyFlag(ParserStatus& /* status */) {
            auto token = m_tokenizer.peekToken();
            if (token.hasType(FgdToken::Word) && token.data() == "readonly") {
                m_tokenizer.nextToken();
                return true;
            } else {
                return false;
            }
        }

        std::string FgdParser::parseAttributeDescription(ParserStatus& status) {
            auto token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                token = expect(status, FgdToken::String | FgdToken::Colon, m_tokenizer.peekToken());
                if (token.type() == FgdToken::String) {
                    return parseString(status);
                }
            }
            return "";
        }

        std::optional<std::string> FgdParser::parseDefaultStringValue(ParserStatus& status) {
            auto token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                token = expect(status, FgdToken::String | FgdToken::Colon | FgdToken::Integer | FgdToken::Decimal, m_tokenizer.peekToken());
                if (token.type() == FgdToken::String) {
                    token = m_tokenizer.nextToken();
                    return token.data();
                } else if (token.type() == FgdToken::Integer || token.type() == FgdToken::Decimal) {
                    token = m_tokenizer.nextToken();
                    status.warn(token.line(), token.column(), "Found numeric default value for string property");
                    return token.data();
                }
            }
            return std::nullopt;
        }

        std::optional<int> FgdParser::parseDefaultIntegerValue(ParserStatus& status) {
            auto token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                token = expect(status, FgdToken::Integer | FgdToken::Decimal | FgdToken::Colon, m_tokenizer.peekToken());
                if (token.type() == FgdToken::Integer) {
                    token = m_tokenizer.nextToken();
                    return token.toInteger<int>();
                } else if (token.type() == FgdToken::Decimal) { // be graceful for DaZ
                    token = m_tokenizer.nextToken();
                    status.warn(token.line(), token.column(), "Found float default value for integer property");
                    return static_cast<int>(token.toFloat<float>());
                }
            }
            return std::nullopt;
        }

        std::optional<float> FgdParser::parseDefaultFloatValue(ParserStatus& status) {
            auto token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                // the default value should have quotes around it, but sometimes they're missing
                token = expect(status, FgdToken::String | FgdToken::Decimal | FgdToken::Integer | FgdToken::Colon, m_tokenizer.peekToken());
                if (token.type() != FgdToken::Colon) {
                    token = m_tokenizer.nextToken();
                    if (token.type() != FgdToken::String) {
                        status.warn(token.line(), token.column(), "Unquoted float default value " + token.data());
                    }
                    return token.toFloat<float>();
                }
            }
            return std::nullopt;
        }

        std::optional<std::string> FgdParser::parseDefaultChoiceValue(ParserStatus& status) {
            auto token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                token = expect(status, FgdToken::String | FgdToken::Integer | FgdToken::Decimal | FgdToken::Colon, m_tokenizer.peekToken());
                if (token.hasType(FgdToken::String | FgdToken::Integer | FgdToken::Decimal)) {
                    token = m_tokenizer.nextToken();
                    return token.data();
                }
            }
            return std::nullopt;
        }

        vm::vec3 FgdParser::parseVector(ParserStatus& status) {
            vm::vec3 vec;
            for (size_t i = 0; i < 3; i++) {
                auto token = expect(status, FgdToken::Integer | FgdToken::Decimal, m_tokenizer.nextToken());
                vec[i] = token.toFloat<double>();
            }
            return vec;
        }

        vm::bbox3 FgdParser::parseSize(ParserStatus& status) {
            vm::bbox3 size;
            expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());
            size.min = parseVector(status);

            auto token = expect(status, FgdToken::CParenthesis | FgdToken::Comma, m_tokenizer.nextToken());
            if (token.type() == FgdToken::Comma) {
                size.max = parseVector(status);
                expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());
            } else {
                const auto halfSize = size.min / 2.0;
                size.min = -halfSize;
                size.max =  halfSize;
            }
            return repair(size);
        }

        Color FgdParser::parseColor(ParserStatus& status) {
            Color color;
            expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());
            for (size_t i = 0; i < 3; i++) {
                auto token = expect(status, FgdToken::Decimal | FgdToken::Integer, m_tokenizer.nextToken());
                color[i] = token.toFloat<float>();
                if (color[i] > 1.0f) {
                    color[i] /= 255.0f;
                }
            }
            expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());
            color[3] = 1.0f;
            return color;
        }

        std::string FgdParser::parseString(ParserStatus& status) {
            auto token = expect(status, FgdToken::String, m_tokenizer.nextToken());
            if (m_tokenizer.peekToken().hasType(FgdToken::Plus)) {
                std::stringstream str;
                str << token.data();
                do {
                    m_tokenizer.nextToken();
                    token = expect(status, FgdToken::String, m_tokenizer.nextToken());
                    str << token.data();
                } while (m_tokenizer.peekToken().hasType(FgdToken::Plus));
                return str.str();
            } else {
                return token.data();
            }
        }

        FgdParser::EntityDefinitionList FgdParser::parseInclude(ParserStatus& status) {
            auto token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
            assert(kdl::ci::str_is_equal(token.data(), "@include"));

            expect(status, FgdToken::String, token = m_tokenizer.nextToken());
            const auto path = Path(token.data());
            return handleInclude(status, path);
        }

        FgdParser::EntityDefinitionList FgdParser::handleInclude(ParserStatus& status, const Path& path) {
            const auto snapshot = m_tokenizer.snapshot();
            auto result = EntityDefinitionList{};
            try {
                status.debug(m_tokenizer.line(), "Parsing included file '" + path.asString() + "'");
                const auto file = m_fs->openFile(path);
                const auto filePath = file->path();
                status.debug(m_tokenizer.line(), "Resolved '" + path.asString() + "' to '" + filePath.asString() + "'");

                if (!isRecursiveInclude(filePath)) {
                    const PushIncludePath pushIncludePath(this, filePath);
                    auto reader = file->reader().buffer();
                    m_tokenizer.replaceState(std::begin(reader), std::end(reader));
                    result = doParseDefinitions(status);
                } else {
                    status.error(m_tokenizer.line(), kdl::str_to_string("Skipping recursively included file: ", path.asString(), " (", filePath, ")"));
                }
            } catch (const Exception &e) {
                status.error(m_tokenizer.line(), kdl::str_to_string("Failed to parse included file: ", e.what()));
            }

            m_tokenizer.restore(snapshot);
            return result;
        }
    }
}
