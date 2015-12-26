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

#include "FgdParser.h"

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"
#include "Assets/ModelDefinition.h"
#include "IO/ParserStatus.h"

namespace TrenchBroom {
    namespace IO {
        FgdTokenizer::FgdTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end) {}
        
        FgdTokenizer::FgdTokenizer(const String& str) :
        Tokenizer(str) {}
        
        const String FgdTokenizer::WordDelims = " \t\n\r()[]?;:,=";
        
        FgdTokenizer::Token FgdTokenizer::emitToken() {
            while (!eof()) {
                size_t startLine = line();
                size_t startColumn = column();
                const char* c = curPos();
                
                switch (*c) {
                    case '/':
                        advance();
                        if (curChar() == '/')
                            discardUntil("\n\r");
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
                        const char* e = readQuotedString();
                        return Token(FgdToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        discardWhile(Whitespace);
                        break;
                    default: {
                        const char* e = readInteger(WordDelims);
                        if (e != NULL)
                            return Token(FgdToken::Integer, c, e, offset(c), startLine, startColumn);
                        
                        e = readDecimal(WordDelims);
                        if (e != NULL)
                            return Token(FgdToken::Decimal, c, e, offset(c), startLine, startColumn);
                        
                        e = readString(WordDelims);
                        if (e == NULL)
                            throw ParserException(startLine, startColumn, "Unexpected character: '" + String(c, 1) + "'");
                        return Token(FgdToken::Word, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(FgdToken::Eof, NULL, NULL, length(), line(), column());
        }
        
        FgdParser::FgdParser(const char* begin, const char* end, const Color& defaultEntityColor) :
        m_defaultEntityColor(defaultEntityColor),
        m_tokenizer(FgdTokenizer(begin, end)) {}
        
        FgdParser::FgdParser(const String& str, const Color& defaultEntityColor) :
        m_defaultEntityColor(defaultEntityColor),
        m_tokenizer(FgdTokenizer(str)) {}
        
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
            names[Eof]          = "end of file";
            return names;
        }

        Assets::EntityDefinitionList FgdParser::doParseDefinitions(ParserStatus& status) {
            Assets::EntityDefinitionList definitions;
            try {
                Assets::EntityDefinition* definition = parseDefinition(status);
                status.progress(m_tokenizer.progress());
                while (definition != NULL) {
                    definitions.push_back(definition);
                    definition = parseDefinition(status);
                    status.progress(m_tokenizer.progress());
                }
                return definitions;
            } catch (...) {
                VectorUtils::clearAndDelete(definitions);
                throw;
            }
        }
        
        Assets::EntityDefinition* FgdParser::parseDefinition(ParserStatus& status) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Eof)
                return NULL;
            
            const String classname = token.data();
            if (StringUtils::caseInsensitiveEqual(classname, "@SolidClass")) {
                return parseSolidClass(status);
            } else if (StringUtils::caseInsensitiveEqual(classname, "@PointClass")) {
                return parsePointClass(status);
            } else if (StringUtils::caseInsensitiveEqual(classname, "@BaseClass")) {
                const EntityDefinitionClassInfo baseClass = parseBaseClass(status);
                m_baseClasses[baseClass.name()] = baseClass;
                return parseDefinition(status);
            } else if (StringUtils::caseInsensitiveEqual(classname, "@Main")) {
                skipMainClass(status);
                return parseDefinition(status);
            } else {
                const String msg = "Unknown entity definition class '" + classname + "'";
                status.error(token.line(), token.column(), msg);
                throw ParserException(token.line(), token.column(), msg);
            }
        }
        
        Assets::EntityDefinition* FgdParser::parseSolidClass(ParserStatus& status) {
            EntityDefinitionClassInfo classInfo = parseClass(status);
            if (classInfo.hasSize())
                status.warn(classInfo.line(), classInfo.column(), "Solid entity definition must not have a size");
            if (!classInfo.models().empty())
                status.warn(classInfo.line(), classInfo.column(), "Solid entity definition must not have model definitions");
            return new Assets::BrushEntityDefinition(classInfo.name(), classInfo.color(), classInfo.description(), classInfo.attributeList());
        }
        
        Assets::EntityDefinition* FgdParser::parsePointClass(ParserStatus& status) {
            EntityDefinitionClassInfo classInfo = parseClass(status);
            return new Assets::PointEntityDefinition(classInfo.name(), classInfo.color(), classInfo.size(), classInfo.description(), classInfo.attributeList(), classInfo.models());
        }
        
        EntityDefinitionClassInfo FgdParser::parseBaseClass(ParserStatus& status) {
            EntityDefinitionClassInfo classInfo = parseClass(status);
            if (m_baseClasses.count(classInfo.name()) > 0)
                status.warn(classInfo.line(), classInfo.column(), "Redefinition of base class '" + classInfo.name() + "'");
            return classInfo;
        }
        
        EntityDefinitionClassInfo FgdParser::parseClass(ParserStatus& status) {
            Token token;
            expect(status, FgdToken::Word | FgdToken::Equality, token = m_tokenizer.nextToken());
            
            StringList superClasses;
            EntityDefinitionClassInfo classInfo(token.line(), token.column(), m_defaultEntityColor);
            
            while (token.type() == FgdToken::Word) {
                const String typeName = token.data();
                if (StringUtils::caseInsensitiveEqual(typeName, "base")) {
                    if (!superClasses.empty())
                        status.warn(token.line(), token.column(), "Found multiple base attributes");
                    superClasses = parseSuperClasses(status);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "color")) {
                    if (classInfo.hasColor())
                        status.warn(token.line(), token.column(), "Found multiple color attributes");
                    classInfo.setColor(parseColor(status));
                } else if (StringUtils::caseInsensitiveEqual(typeName, "size")) {
                    if (classInfo.hasSize())
                        status.warn(token.line(), token.column(), "Found multiple size attributes");
                    classInfo.setSize(parseSize(status));
                } else if (StringUtils::caseInsensitiveEqual(typeName, "model") ||
                           StringUtils::caseInsensitiveEqual(typeName, "studio")) {
                    if (!classInfo.models().empty())
                        status.warn(token.line(), token.column(), "Found multiple model attributes");
                    classInfo.addModelDefinitions(parseModels(status));
                } else {
                    status.warn(token.line(), token.column(), "Unknown entity definition header attribute '" + typeName + "'");
                    skipClassAttribute(status);
                }
                expect(status, FgdToken::Equality | FgdToken::Word, token = m_tokenizer.nextToken());
            }
            
            expect(status, FgdToken::Word, token = m_tokenizer.nextToken());
            classInfo.setName(token.data());

            expect(status, FgdToken::Colon | FgdToken::OBracket, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Colon) {
                expect(status, FgdToken::String, token = m_tokenizer.nextToken());
                classInfo.setDescription(StringUtils::trim(token.data()));
            } else {
                m_tokenizer.pushToken(token);
            }
            
            classInfo.addAttributeDefinitions(parseProperties(status));
            classInfo.resolveBaseClasses(m_baseClasses, superClasses);
            return classInfo;
        }

        void FgdParser::skipMainClass(ParserStatus& status) {
            Token token;
            expect(status, FgdToken::Equality, token = m_tokenizer.nextToken());
            expect(status, FgdToken::OBracket, token = m_tokenizer.nextToken());
            do {
                token = m_tokenizer.nextToken();
            } while (token.type() != FgdToken::CBracket);
        }

        StringList FgdParser::parseSuperClasses(ParserStatus& status) {
            StringList superClasses;
            Token token;
            expect(status, FgdToken::OParenthesis, token = m_tokenizer.nextToken());
            expect(status, FgdToken::Word | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Word) {
                m_tokenizer.pushToken(token);
                do {
                    expect(status, FgdToken::Word, token = m_tokenizer.nextToken());
                    superClasses.push_back(token.data());
                    expect(status, FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                } while (token.type() == FgdToken::Comma);
            }
            return superClasses;
        }
        
        Assets::ModelDefinitionList FgdParser::parseModels(ParserStatus& status) {
            Assets::ModelDefinitionList result;
            Token token;
            expect(status, FgdToken::OParenthesis, token = m_tokenizer.nextToken());
            expect(status, FgdToken::String | FgdToken::Word | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::String || token.type() == FgdToken::Word) {
                m_tokenizer.pushToken(token);
                do {
                    expect(status, FgdToken::String | FgdToken::Word, token = m_tokenizer.peekToken());
                    if (token.type() == FgdToken::String)
                        result.push_back(parseStaticModel(status));
                    else
                        result.push_back(parseDynamicModel(status));
                    expect(status, FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                } while (token.type() == FgdToken::Comma);
            }
            return result;
        }
        
        Assets::ModelDefinitionPtr FgdParser::parseStaticModel(ParserStatus& status) {
            Token token;
            expect(status, FgdToken::String, token = m_tokenizer.nextToken());
            const String pathStr = token.data();
            const IO::Path path(!pathStr.empty() && pathStr[0] == ':' ? pathStr.substr(1) : pathStr);
            
            std::vector<size_t> indices;
            
            expect(status, FgdToken::Integer | FgdToken::Word | FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Integer) {
                indices.push_back(token.toInteger<size_t>());
                expect(status, FgdToken::Integer | FgdToken::Word | FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::Integer) {
                    indices.push_back(token.toInteger<size_t>());
                    expect(status, FgdToken::Word | FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                }
            }
            
            size_t skinIndex = 0;
            size_t frameIndex = 0;
            if (!indices.empty()) {
                skinIndex = indices[0];
                if (indices.size() > 1)
                    frameIndex = indices[1];
            }
            
            if (token.type() == FgdToken::Word) {
                const String attributeKey = token.data();
                expect(status, FgdToken::Equality, token = m_tokenizer.nextToken());
                expect(status, FgdToken::String | FgdToken::Integer, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::String) {
                    const String attributeValue = token.data();
                    return Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(path, skinIndex, frameIndex, attributeKey, attributeValue));
                } else {
                    const int flagValue = token.toInteger<int>();
                    return Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(path, skinIndex, frameIndex, attributeKey, flagValue));
                }
            } else {
                m_tokenizer.pushToken(token);
                return Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(path, skinIndex, frameIndex));
            }
        }
        
        Assets::ModelDefinitionPtr FgdParser::parseDynamicModel(ParserStatus& status) {
            Token token;
            String pathKey, skinKey, frameKey;
            
            expect(status, FgdToken::Word, token = m_tokenizer.nextToken());
            if (!StringUtils::caseInsensitiveEqual("pathKey", token.data())) {
                const String msg = "Expected 'pathKey', but found '" + token.data() + "'";
                status.error(token.line(), token.column(), msg);
                throw ParserException(token.line(), token.column(), msg);
            }
            
            expect(status, FgdToken::Equality, token = m_tokenizer.nextToken());
            expect(status, FgdToken::String, token = m_tokenizer.nextToken());
            pathKey = token.data();
            
            expect(status, FgdToken::Word | FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Word) {
                if (!StringUtils::caseInsensitiveEqual("skinKey", token.data())) {
                    const String msg = "Expected 'skinKey', but found '" + token.data() + "'";
                    status.error(token.line(), token.column(), msg);
                    throw ParserException(token.line(), token.column(), msg);
                }
                
                expect(status, FgdToken::Equality, token = m_tokenizer.nextToken());
                expect(status, FgdToken::String, token = m_tokenizer.nextToken());
                skinKey = token.data();
                
                expect(status, FgdToken::Word | FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::Word) {
                    if (!StringUtils::caseInsensitiveEqual("frameKey", token.data())) {
                        const String msg = "Expected 'frameKey', but found '" + token.data() + "'";
                        status.error(token.line(), token.column(), msg);
                        throw ParserException(token.line(), token.column(), msg);
                    }
                    
                    expect(status, FgdToken::Equality, token = m_tokenizer.nextToken());
                    expect(status, FgdToken::String, token = m_tokenizer.nextToken());
                    frameKey = token.data();
                } else {
                    m_tokenizer.pushToken(token);
                }
            } else {
                m_tokenizer.pushToken(token);
            }
            
            return Assets::ModelDefinitionPtr(new Assets::DynamicModelDefinition(pathKey, skinKey, frameKey));
        }
        
        void FgdParser::skipClassAttribute(ParserStatus& status) {
            size_t depth = 0;
            Token token;
            do {
                token = m_tokenizer.nextToken();
                if (token.type() == FgdToken::OParenthesis)
                    ++depth;
                else if (token.type() == FgdToken::CParenthesis)
                    --depth;
            } while (depth > 0 && token.type() != FgdToken::Eof);
        }

        Assets::AttributeDefinitionMap FgdParser::parseProperties(ParserStatus& status) {
            Assets::AttributeDefinitionMap attributes;
            
            Token token;
            expect(status, FgdToken::OBracket, token = m_tokenizer.nextToken());
            expect(status, FgdToken::Word | FgdToken::CBracket, token = m_tokenizer.nextToken());
            while (token.type() != FgdToken::CBracket) {
                const String attributeKey = token.data();
                
                if (attributes.count(attributeKey) > 0) {
                    status.warn(token.line(), token.column(), "Redefinition of property declaration '" + attributeKey + "'");
                }

                expect(status, FgdToken::OParenthesis, token = m_tokenizer.nextToken());
                expect(status, FgdToken::Word, token = m_tokenizer.nextToken());
                const String typeName = token.data();
                expect(status, FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                
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
                    status.debug(token.line(), token.column(), "Unknown property definition type '" + typeName + "' for attribute '" + attributeKey + "'");
                    attributes[attributeKey] = parseUnknownAttribute(status, attributeKey);
                }
                
                expect(status, FgdToken::Word | FgdToken::CBracket, token = m_tokenizer.nextToken());
            }
            
            return attributes;
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseTargetSourceAttribute(ParserStatus& status, const String& name) {
            const String shortDescription = parseAttributeDescription(status);
            parseDefaultStringValue(status);
            const String longDescription = parseAttributeDescription(status);
            return Assets::AttributeDefinitionPtr(new Assets::AttributeDefinition(name, Assets::AttributeDefinition::Type_TargetSourceAttribute, shortDescription, longDescription));
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseTargetDestinationAttribute(ParserStatus& status, const String& name) {
            const String shortDescription = parseAttributeDescription(status);
            parseDefaultStringValue(status);
            const String longDescription = parseAttributeDescription(status);
            return Assets::AttributeDefinitionPtr(new Assets::AttributeDefinition(name, Assets::AttributeDefinition::Type_TargetDestinationAttribute, shortDescription, longDescription));
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseStringAttribute(ParserStatus& status, const String& name) {
            const String shortDescription = parseAttributeDescription(status);
            const DefaultValue<String> defaultValue = parseDefaultStringValue(status);
            const String longDescription = parseAttributeDescription(status);

            if (defaultValue.present)
                return Assets::AttributeDefinitionPtr(new Assets::StringAttributeDefinition(name, shortDescription, longDescription, defaultValue.value));
            return Assets::AttributeDefinitionPtr(new Assets::StringAttributeDefinition(name, shortDescription, longDescription));
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseIntegerAttribute(ParserStatus& status, const String& name) {
            const String shortDescription = parseAttributeDescription(status);
            const DefaultValue<int> defaultValue = parseDefaultIntegerValue(status);
            const String longDescription = parseAttributeDescription(status);

            if (defaultValue.present)
                return Assets::AttributeDefinitionPtr(new Assets::IntegerAttributeDefinition(name, shortDescription, longDescription, defaultValue.value));
            return Assets::AttributeDefinitionPtr(new Assets::IntegerAttributeDefinition(name, shortDescription, longDescription));
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseFloatAttribute(ParserStatus& status, const String& name) {
            const String shortDescription = parseAttributeDescription(status);
            const DefaultValue<float> defaultValue = parseDefaultFloatValue(status);
            const String longDescription = parseAttributeDescription(status);

            if (defaultValue.present)
                return Assets::AttributeDefinitionPtr(new Assets::FloatAttributeDefinition(name, shortDescription, longDescription, defaultValue.value));
            return Assets::AttributeDefinitionPtr(new Assets::FloatAttributeDefinition(name, shortDescription, longDescription));
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseChoicesAttribute(ParserStatus& status, const String& name) {
            const String shortDescription = parseAttributeDescription(status);
            const DefaultValue<int> defaultValue = parseDefaultIntegerValue(status);
            const String longDescription = parseAttributeDescription(status);

            Token token;
            expect(status, FgdToken::Equality, token = m_tokenizer.nextToken());
            expect(status, FgdToken::OBracket, token = m_tokenizer.nextToken());
            expect(status, FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket, token = m_tokenizer.nextToken());
            
            Assets::ChoiceAttributeOption::List options;
            while (token.type() != FgdToken::CBracket) {
                const String value = token.data();
                expect(status, FgdToken::Colon, token = m_tokenizer.nextToken());
                expect(status, FgdToken::String, token = m_tokenizer.nextToken());
                const String caption = token.data();
                options.push_back(Assets::ChoiceAttributeOption(value, caption));
                expect(status, FgdToken::Integer | FgdToken::Decimal | FgdToken::String | FgdToken::CBracket, token = m_tokenizer.nextToken());
            }
            
            if (defaultValue.present)
                return Assets::AttributeDefinitionPtr(new Assets::ChoiceAttributeDefinition(name, shortDescription, longDescription, options, static_cast<size_t>(defaultValue.value)));
            return Assets::AttributeDefinitionPtr(new Assets::ChoiceAttributeDefinition(name, shortDescription, longDescription, options));
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseFlagsAttribute(ParserStatus& status, const String& name) {
            // Flag attributes do not have descriptions or defaults, see https://developer.valvesoftware.com/wiki/FGD
            
            Token token;
            expect(status, FgdToken::Equality, token = m_tokenizer.nextToken());
            expect(status, FgdToken::OBracket, token = m_tokenizer.nextToken());
            expect(status, FgdToken::Integer | FgdToken::CBracket, token = m_tokenizer.nextToken());
            
            Assets::FlagsAttributeDefinition* definition = new Assets::FlagsAttributeDefinition(name);
            
            while (token.type() != FgdToken::CBracket) {
                const int value = token.toInteger<int>();
                expect(status, FgdToken::Colon, token = m_tokenizer.nextToken());
                
                expect(status, FgdToken::String, token = m_tokenizer.nextToken());
                const String shortDescription = token.data();
                
                bool defaultValue = false;
                expect(status, FgdToken::Colon | FgdToken::Integer | FgdToken::CBracket, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::Colon) {
                    expect(status, FgdToken::Integer, token = m_tokenizer.nextToken());
                    defaultValue = token.toInteger<int>() != 0;
                } else {
                    m_tokenizer.pushToken(token);
                }
                
                expect(status, FgdToken::Integer | FgdToken::CBracket | FgdToken::Colon, token = m_tokenizer.nextToken());
                
                String longDescription;
                if (token.type() == FgdToken::Colon) {
                    expect(status, FgdToken::String, token = m_tokenizer.nextToken());
                    longDescription = token.data();
                    expect(status, FgdToken::Integer | FgdToken::CBracket, token = m_tokenizer.nextToken());
                }
                
                definition->addOption(value, shortDescription, longDescription, defaultValue);
            }
            
            return Assets::AttributeDefinitionPtr(definition);
        }
        
        Assets::AttributeDefinitionPtr FgdParser::parseUnknownAttribute(ParserStatus& status, const String& name) {
            const String shortDescription = parseAttributeDescription(status);
            const DefaultValue<String> defaultValue = parseDefaultStringValue(status);
            const String longDescription = parseAttributeDescription(status);
            
            if (defaultValue.present)
                return Assets::AttributeDefinitionPtr(new Assets::UnknownAttributeDefinition(name, shortDescription, longDescription, defaultValue.value));
            return Assets::AttributeDefinitionPtr(new Assets::UnknownAttributeDefinition(name, shortDescription, longDescription));
        }

        String FgdParser::parseAttributeDescription(ParserStatus& status) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Colon) {
                expect(status, FgdToken::String | FgdToken::Colon, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::String)
                    return token.data();
            }
            m_tokenizer.pushToken(token);
            return EmptyString;
        }

        FgdParser::DefaultValue<String> FgdParser::parseDefaultStringValue(ParserStatus& status) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Colon) {
                expect(status, FgdToken::String | FgdToken::Colon, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::String)
                    return DefaultValue<String>(token.data());
            }
            
            m_tokenizer.pushToken(token);
            return DefaultValue<String>();
        }

        FgdParser::DefaultValue<int> FgdParser::parseDefaultIntegerValue(ParserStatus& status) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Colon) {
                expect(status, FgdToken::Integer | FgdToken::Decimal | FgdToken::Colon, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::Integer)
                    return DefaultValue<int>(token.toInteger<int>());
                else if (token.type() == FgdToken::Decimal) { // be graceful for DaZ
                    status.warn(token.line(), token.column(), "Found float default value for integer property");
                    return DefaultValue<int>(static_cast<int>(token.toFloat<float>()));
                }
            }
            
            m_tokenizer.pushToken(token);
            return DefaultValue<int>();
        }

        FgdParser::DefaultValue<float> FgdParser::parseDefaultFloatValue(ParserStatus& status) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Colon) {
                // the default value should have quotes around it, but sometimes they're missing
                expect(status, FgdToken::String | FgdToken::Decimal | FgdToken::Integer | FgdToken::Colon, token = m_tokenizer.nextToken());
                if (token.type() != FgdToken::Colon) {
                    if (token.type() != FgdToken::String)
                        status.warn(token.line(), token.column(), "Unquoted float default value " + token.data());
                    return DefaultValue<float>(token.toFloat<float>());
                }
            }

            m_tokenizer.pushToken(token);
            return DefaultValue<float>();
        }
        
        Vec3 FgdParser::parseVector(ParserStatus& status) {
            Token token;
            Vec3 vec;
            for (size_t i = 0; i < 3; i++) {
                expect(status, FgdToken::Integer | FgdToken::Decimal, token = m_tokenizer.nextToken());
                vec[i] = token.toFloat<double>();
            }
            return vec;
        }
        
        BBox3 FgdParser::parseSize(ParserStatus& status) {
            Token token;
            BBox3 size;
            expect(status, FgdToken::OParenthesis, token = m_tokenizer.nextToken());
            size.min = parseVector(status);
            expect(status, FgdToken::CParenthesis | FgdToken::Comma, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Comma) {
                size.max = parseVector(status);
                expect(status, FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            } else {
                const Vec3 halfSize = size.min / 2.0;
                size.min = -halfSize;
                size.max =  halfSize;
            }
            return size;
        }
        
        Color FgdParser::parseColor(ParserStatus& status) {
            Color color;
            Token token;
            expect(status, FgdToken::OParenthesis, token = m_tokenizer.nextToken());
            for (size_t i = 0; i < 3; i++) {
                expect(status, FgdToken::Decimal | FgdToken::Integer, token = m_tokenizer.nextToken());
                color[i] = token.toFloat<float>();
                if (color[i] > 1.0f)
                    color[i] /= 255.0f;
            }
            expect(status, FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            color[3] = 1.0f;
            return color;
        }
    }
}
