/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
                        discardWhile(Whitespace());
                        break;
                    default: {
                        const char* e = readInteger(WordDelims);
                        if (e != NULL)
                            return Token(FgdToken::Integer, c, e, offset(c), startLine, startColumn);
                        
                        e = readDecimal(WordDelims);
                        if (e != NULL)
                            return Token(FgdToken::Decimal, c, e, offset(c), startLine, startColumn);
                        
                        e = readUntil(WordDelims);
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
            if (classInfo.hasModelDefinition())
                status.warn(classInfo.line(), classInfo.column(), "Solid entity definition must not have model definitions");
            return new Assets::BrushEntityDefinition(classInfo.name(), classInfo.color(), classInfo.description(), classInfo.attributeList());
        }
        
        Assets::EntityDefinition* FgdParser::parsePointClass(ParserStatus& status) {
            EntityDefinitionClassInfo classInfo = parseClass(status);
            return new Assets::PointEntityDefinition(classInfo.name(), classInfo.color(), classInfo.size(), classInfo.description(), classInfo.attributeList(), classInfo.modelDefinition());
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
                           StringUtils::caseInsensitiveEqual(typeName, "studio") ||
                           StringUtils::caseInsensitiveEqual(typeName, "studioprop")) {
                    if (classInfo.hasModelDefinition())
                        status.warn(token.line(), token.column(), "Found multiple model attributes");
                    classInfo.setModelDefinition(parseModel(status));
                } else {
                    status.warn(token.line(), token.column(), "Unknown entity definition header attribute '" + typeName + "'");
                    skipClassAttribute(status);
                }
                expect(status, FgdToken::Equality | FgdToken::Word, token = m_tokenizer.nextToken());
            }
            
            expect(status, FgdToken::Word, token = m_tokenizer.nextToken());
            classInfo.setName(token.data());

            expect(status, FgdToken::Colon | FgdToken::OBracket, token = m_tokenizer.peekToken());
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                expect(status, FgdToken::String, token = m_tokenizer.nextToken());
                classInfo.setDescription(StringUtils::trim(token.data()));
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
            expect(status, FgdToken::Word | FgdToken::CParenthesis, token = m_tokenizer.peekToken());
            if (token.type() == FgdToken::Word) {
                do {
                    expect(status, FgdToken::Word, token = m_tokenizer.nextToken());
                    superClasses.push_back(token.data());
                    expect(status, FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                } while (token.type() == FgdToken::Comma);
            } else {
                m_tokenizer.nextToken();
            }
            return superClasses;
        }
        
        Assets::ModelDefinition FgdParser::parseModel(ParserStatus& status) {
            expect(status, FgdToken::OParenthesis, m_tokenizer.nextToken());
            
            const TokenizerState::Snapshot snapshot = m_tokenizer.snapshot();
            const size_t line = m_tokenizer.line();
            const size_t column = m_tokenizer.column();
            
            try {
                ELParser parser(m_tokenizer);
                EL::Expression expression = parser.parse();
                expect(status, FgdToken::CParenthesis, m_tokenizer.nextToken());

                expression.optimize();
                return Assets::ModelDefinition(expression);
            } catch (const ParserException& e) {
                try {
                    m_tokenizer.restore(snapshot);
                    
                    LegacyModelDefinitionParser parser(m_tokenizer);
                    EL::Expression expression = parser.parse(status);
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
                expect(status, FgdToken::Colon | FgdToken::Integer | FgdToken::CBracket, token = m_tokenizer.peekToken());
                if (token.type() == FgdToken::Colon) {
                    m_tokenizer.nextToken();
                    expect(status, FgdToken::Integer, token = m_tokenizer.nextToken());
                    defaultValue = token.toInteger<int>() != 0;
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
            Token token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                expect(status, FgdToken::String | FgdToken::Colon, token = m_tokenizer.peekToken());
                if (token.type() == FgdToken::String) {
                    token = m_tokenizer.nextToken();
                    return token.data();
                }
            }
            return EmptyString;
        }

        FgdParser::DefaultValue<String> FgdParser::parseDefaultStringValue(ParserStatus& status) {
            Token token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                expect(status, FgdToken::String | FgdToken::Colon, token = m_tokenizer.peekToken());
                if (token.type() == FgdToken::String) {
                    token = m_tokenizer.nextToken();
                    return DefaultValue<String>(token.data());
                }
            }
            return DefaultValue<String>();
        }

        FgdParser::DefaultValue<int> FgdParser::parseDefaultIntegerValue(ParserStatus& status) {
            Token token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                expect(status, FgdToken::Integer | FgdToken::Decimal | FgdToken::Colon, token = m_tokenizer.peekToken());
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
            Token token = m_tokenizer.peekToken();
            if (token.type() == FgdToken::Colon) {
                m_tokenizer.nextToken();
                // the default value should have quotes around it, but sometimes they're missing
                expect(status, FgdToken::String | FgdToken::Decimal | FgdToken::Integer | FgdToken::Colon, token = m_tokenizer.peekToken());
                if (token.type() != FgdToken::Colon) {
                    token = m_tokenizer.nextToken();
                    if (token.type() != FgdToken::String)
                        status.warn(token.line(), token.column(), "Unquoted float default value " + token.data());
                    return DefaultValue<float>(token.toFloat<float>());
                }
            }
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
