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

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"
#include "Assets/ModelDefinition.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/ELParser.h"
#include "IO/LegacyModelDefinitionParser.h"
#include "IO/ParserStatus.h"

namespace TrenchBroom {
    namespace IO {
        FgdTokenizer::FgdTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end, "", 0) {}
        
        FgdTokenizer::FgdTokenizer(const String& str) :
        Tokenizer(str, "", 0) {}
        
        const String FgdTokenizer::WordDelims = " \t\n\r()[]?;:,=";
        
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
                            throw ParserException(startLine, startColumn, "Unexpected character: '" + String(c, 1) + "'");
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
        
        FgdParser::FgdParser(const String& str, const Color& defaultEntityColor, const Path& path) :
        FgdParser(str.c_str(), str.c_str() + str.size(), defaultEntityColor, path) {}

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
            m_fileSystem.pushFileSystem(new DiskFileSystem(folder));
            m_paths.push_back(path);
        }

        void FgdParser::popIncludePath() {
            m_fileSystem.popFileSystem();
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

        Assets::EntityDefinitionList FgdParser::doParseDefinitions(ParserStatus& status) {
            Assets::EntityDefinitionList definitions;
            try {
                auto token = m_tokenizer.peekToken();
                while (!token.hasType(FgdToken::Eof)) {
                    parseDefinitionOrInclude(status, definitions);
                    token = m_tokenizer.peekToken();
                }
                return definitions;
            } catch (...) {
                VectorUtils::clearAndDelete(definitions);
                throw;
            }
        }

        void FgdParser::parseDefinitionOrInclude(ParserStatus& status, Assets::EntityDefinitionList& definitions) {
            auto token = expect(status, FgdToken::Eof | FgdToken::Word, m_tokenizer.peekToken());
            if (token.hasType(FgdToken::Eof)) {
                return;
            }

            if (StringUtils::caseInsensitiveEqual(token.data(), "@include")) {
                const auto includedDefinitions = parseInclude(status);
                VectorUtils::append(definitions, includedDefinitions);
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
            if (StringUtils::caseInsensitiveEqual(classname, "@SolidClass")) {
                return parseSolidClass(status);
            } else if (StringUtils::caseInsensitiveEqual(classname, "@PointClass")) {
                return parsePointClass(status);
            } else if (StringUtils::caseInsensitiveEqual(classname, "@BaseClass")) {
                const auto baseClass = parseBaseClass(status);
                m_baseClasses[baseClass.name()] = baseClass;
                return nullptr;
            } else if (StringUtils::caseInsensitiveEqual(classname, "@Main")) {
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
            
            StringList superClasses;
            EntityDefinitionClassInfo classInfo(token.line(), token.column(), m_defaultEntityColor);
            
            while (token.type() == FgdToken::Word) {
                const auto typeName = token.data();
                if (StringUtils::caseInsensitiveEqual(typeName, "base")) {
                    if (!superClasses.empty()) {
                        status.warn(token.line(), token.column(), "Found multiple base attributes");
                    }
                    superClasses = parseSuperClasses(status);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "color")) {
                    if (classInfo.hasColor()) {
                        status.warn(token.line(), token.column(), "Found multiple color attributes");
                    }
                    classInfo.setColor(parseColor(status));
                } else if (StringUtils::caseInsensitiveEqual(typeName, "size")) {
                    if (classInfo.hasSize()) {
                        status.warn(token.line(), token.column(), "Found multiple size attributes");
                    }
                    classInfo.setSize(parseSize(status));
                } else if (StringUtils::caseInsensitiveEqual(typeName, "model") ||
                           StringUtils::caseInsensitiveEqual(typeName, "studio") ||
                           StringUtils::caseInsensitiveEqual(typeName, "studioprop")) {
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
                classInfo.setDescription(StringUtils::trim(description));
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

        StringList FgdParser::parseSuperClasses(ParserStatus& status) {
            expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());

            auto token = expect(status, FgdToken::Word | FgdToken::CParenthesis, m_tokenizer.peekToken());

            StringList superClasses;
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

        void FgdParser::skipClassAttribute(ParserStatus& status) {
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

        Assets::AttributeDefinitionMap FgdParser::parseProperties(ParserStatus& status) {
            Assets::AttributeDefinitionMap attributes;
            
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
                
                if (StringUtils::caseInsensitiveEqual(typeName, "target_source")) {
                    attributes[attributeKey] = parseTargetSourceAttribute(status, attributeKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "target_destination")) {
                    attributes[attributeKey] = parseTargetDestinationAttribute(status, attributeKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "string")) {
                    attributes[attributeKey] = parseStringAttribute(status, attributeKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "integer")) {
                    attributes[attributeKey] = parseIntegerAttribute(status, attributeKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "float")) {
                    attributes[attributeKey] = parseFloatAttribute(status, attributeKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "choices")) {
                    attributes[attributeKey] = parseChoicesAttribute(status, attributeKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "flags")) {
                    attributes[attributeKey] = parseFlagsAttribute(status, attributeKey);
                } else {
                    StringStream msg;
                    msg << "Unknown property definition type '" << typeName << "' for attribute '" << attributeKey << "'";
                    status.debug(token.line(), token.column(), msg.str());
                    attributes[attributeKey] = parseUnknownAttribute(status, attributeKey);
                }
                
                token = expect(status, FgdToken::Word | FgdToken::CBracket, m_tokenizer.nextToken());
            }
            
            return attributes;
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseTargetSourceAttribute(ParserStatus& status, const String& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            parseDefaultStringValue(status);
            const auto longDescription = parseAttributeDescription(status);
            return Assets::AttributeDefinitionPtr(new Assets::AttributeDefinition(name, Assets::AttributeDefinition::Type_TargetSourceAttribute, shortDescription, longDescription, readOnly));
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseTargetDestinationAttribute(ParserStatus& status, const String& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            parseDefaultStringValue(status);
            const auto longDescription = parseAttributeDescription(status);
            return Assets::AttributeDefinitionPtr(new Assets::AttributeDefinition(name, Assets::AttributeDefinition::Type_TargetDestinationAttribute, shortDescription, longDescription, readOnly));
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseStringAttribute(ParserStatus& status, const String& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            const auto defaultValue = parseDefaultStringValue(status);
            const auto longDescription = parseAttributeDescription(status);

            if (defaultValue.present) {
                return Assets::AttributeDefinitionPtr(new Assets::StringAttributeDefinition(name, shortDescription, longDescription, defaultValue.value, readOnly));
            } else {
                return Assets::AttributeDefinitionPtr(new Assets::StringAttributeDefinition(name, shortDescription, longDescription, readOnly));
            }
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseIntegerAttribute(ParserStatus& status, const String& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            const auto defaultValue = parseDefaultIntegerValue(status);
            const auto longDescription = parseAttributeDescription(status);

            if (defaultValue.present) {
                return Assets::AttributeDefinitionPtr(new Assets::IntegerAttributeDefinition(name, shortDescription, longDescription, defaultValue.value, readOnly));
            } else {
                return Assets::AttributeDefinitionPtr(new Assets::IntegerAttributeDefinition(name, shortDescription, longDescription, readOnly));
            }
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseFloatAttribute(ParserStatus& status, const String& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            const auto defaultValue = parseDefaultFloatValue(status);
            const auto longDescription = parseAttributeDescription(status);

            if (defaultValue.present) {
                return Assets::AttributeDefinitionPtr(new Assets::FloatAttributeDefinition(name, shortDescription, longDescription, defaultValue.value, readOnly));
            } else {
                return Assets::AttributeDefinitionPtr(new Assets::FloatAttributeDefinition(name, shortDescription, longDescription, readOnly));
            }
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseChoicesAttribute(ParserStatus& status, const String& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            const auto defaultValue = parseDefaultIntegerValue(status);
            const auto longDescription = parseAttributeDescription(status);

            expect(status, FgdToken::Equality, m_tokenizer.nextToken());
            expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

            auto token = expect(status, FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket, m_tokenizer.nextToken());
            
            Assets::ChoiceAttributeOption::List options;
            while (token.type() != FgdToken::CBracket) {
                const auto value = token.data();
                expect(status, FgdToken::Colon, m_tokenizer.nextToken());
                const auto caption = parseString(status);
                options.push_back(Assets::ChoiceAttributeOption(value, caption));
                token = expect(status, FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket, m_tokenizer.nextToken());
            }
            
            if (defaultValue.present) {
                return Assets::AttributeDefinitionPtr(new Assets::ChoiceAttributeDefinition(name, shortDescription, longDescription, options, static_cast<size_t>(defaultValue.value), readOnly));
            } else {
                return Assets::AttributeDefinitionPtr(new Assets::ChoiceAttributeDefinition(name, shortDescription, longDescription, options, readOnly));
            }
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseFlagsAttribute(ParserStatus& status, const String& name) {
            // Flag attributes do not have descriptions or defaults, see https://developer.valvesoftware.com/wiki/FGD
            
            expect(status, FgdToken::Equality, m_tokenizer.nextToken());
            expect(status, FgdToken::OBracket, m_tokenizer.nextToken());

            auto token = expect(status, FgdToken::Integer | FgdToken::CBracket, m_tokenizer.nextToken());
            
            auto* definition = new Assets::FlagsAttributeDefinition(name);
            
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
                
                String longDescription;
                if (token.type() == FgdToken::Colon) {
                    longDescription = parseString(status);
                    token = expect(status, FgdToken::Integer | FgdToken::CBracket, m_tokenizer.nextToken());
                }
                
                definition->addOption(value, shortDescription, longDescription, defaultValue);
            }
            
            return Assets::AttributeDefinitionPtr(definition);
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseUnknownAttribute(ParserStatus& status, const String& name) {
            const auto readOnly = parseReadOnlyFlag(status);
            const auto shortDescription = parseAttributeDescription(status);
            const auto defaultValue = parseDefaultStringValue(status);
            const auto longDescription = parseAttributeDescription(status);
            
            if (defaultValue.present) {
                return Assets::AttributeDefinitionPtr(new Assets::UnknownAttributeDefinition(name, shortDescription, longDescription, defaultValue.value, readOnly));
            } else {
                return Assets::AttributeDefinitionPtr(new Assets::UnknownAttributeDefinition(name, shortDescription, longDescription, readOnly));
            }
        }

        bool FgdParser::parseReadOnlyFlag(ParserStatus& status) {
            auto token = m_tokenizer.peekToken();
            if (token.hasType(FgdToken::Word) && token.data() == "readonly") {
                m_tokenizer.nextToken();
                return true;
            } else {
                return false;
            }
        }

        String FgdParser::parseAttributeDescription(ParserStatus& status) {
            auto token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                token = expect(status, FgdToken::String | FgdToken::Colon, m_tokenizer.peekToken());
                if (token.type() == FgdToken::String) {
                    return parseString(status);
                }
            }
            return EmptyString;
        }

        FgdParser::DefaultValue<String> FgdParser::parseDefaultStringValue(ParserStatus& status) {
            auto token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                token = expect(status, FgdToken::String | FgdToken::Colon, m_tokenizer.peekToken());
                if (token.type() == FgdToken::String) {
                    token = m_tokenizer.nextToken();
                    return DefaultValue<String>(token.data());
                }
            }
            return DefaultValue<String>();
        }

        FgdParser::DefaultValue<int> FgdParser::parseDefaultIntegerValue(ParserStatus& status) {
            auto token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                token = expect(status, FgdToken::Integer | FgdToken::Decimal | FgdToken::Colon, m_tokenizer.peekToken());
                if (token.type() == FgdToken::Integer) {
                    token = m_tokenizer.nextToken();
                    return DefaultValue<int>(token.toInteger<int>());
                } else if (token.type() == FgdToken::Decimal) { // be graceful for DaZ
                    token = m_tokenizer.nextToken();
                    status.warn(token.line(), token.column(), "Found float default value for integer property");
                    return DefaultValue<int>(static_cast<int>(token.toFloat<float>()));
                }
            }
            return DefaultValue<int>();
        }

        FgdParser::DefaultValue<float> FgdParser::parseDefaultFloatValue(ParserStatus& status) {
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
                    return DefaultValue<float>(token.toFloat<float>());
                }
            }
            return DefaultValue<float>();
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

        String FgdParser::parseString(ParserStatus& status) {
            auto token = expect(status, FgdToken::String, m_tokenizer.nextToken());
            if (m_tokenizer.peekToken().hasType(FgdToken::Plus)) {
                StringStream str;
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

        Assets::EntityDefinitionList FgdParser::parseInclude(ParserStatus& status) {
            auto token = expect(status, FgdToken::Word, m_tokenizer.nextToken());
            assert(StringUtils::caseInsensitiveEqual(token.data(), "@include"));

            expect(status, FgdToken::String, token = m_tokenizer.nextToken());
            const auto path = Path(token.data());
            return handleInclude(status, path);
        }

        Assets::EntityDefinitionList FgdParser::handleInclude(ParserStatus& status, const Path& path) {
            const auto snapshot = m_tokenizer.snapshot();
            auto result = Assets::EntityDefinitionList(0);
            try {
                status.debug(m_tokenizer.line(), "Parsing included file '" + path.asString() + "'");
                const auto file = m_fileSystem.openFile(path);
                const auto filePath = file->path();
                status.debug(m_tokenizer.line(), "Resolved '" + path.asString() + "' to '" + filePath.asString() + "'");

                if (!isRecursiveInclude(filePath)) {
                    const PushIncludePath pushIncludePath(this, filePath);
                    m_tokenizer.replaceState(file->begin(), file->end());
                    result = doParseDefinitions(status);
                } else {
                    auto str = StringStream();
                    str << "Skipping recursively included file: " << path.asString() << " (" << filePath << ")";
                    status.error(m_tokenizer.line(), str.str());
                }
            } catch (const Exception &e) {
                auto str = StringStream();
                str << "Failed to parse included file: " << e.what();
                status.error(m_tokenizer.line(), str.str());
            }

            m_tokenizer.restore(snapshot);
            return result;
        }
    }
}
