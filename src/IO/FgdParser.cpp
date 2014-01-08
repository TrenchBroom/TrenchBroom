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
#include "Assets/PropertyDefinition.h"
#include "Assets/ModelDefinition.h"

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

        Assets::EntityDefinitionList FgdParser::doParseDefinitions() {
            Assets::EntityDefinitionList definitions;
            try {
                Assets::EntityDefinition* definition = parseDefinition();
                while (definition != NULL) {
                    definitions.push_back(definition);
                    definition = parseDefinition();
                }
                return definitions;
            } catch (...) {
                VectorUtils::clearAndDelete(definitions);
                throw;
            }
        }
        
        Assets::EntityDefinition* FgdParser::parseDefinition() {
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Eof)
                return NULL;
            
            const String classname = token.data();
            if (StringUtils::caseInsensitiveEqual(classname, "@SolidClass")) {
                return parseSolidClass();
            } else if (StringUtils::caseInsensitiveEqual(classname, "@PointClass")) {
                return parsePointClass();
            } else if (StringUtils::caseInsensitiveEqual(classname, "@BaseClass")) {
                const EntityDefinitionClassInfo baseClass = parseBaseClass();
                m_baseClasses[baseClass.name()] = baseClass;
                return parseDefinition();
            } else {
                throw ParserException(token.line(), token.column(), "Unknown entity definition class '" + classname + "'");
            }
        }
        
        Assets::EntityDefinition* FgdParser::parseSolidClass() {
            EntityDefinitionClassInfo classInfo = parseClass();
            if (classInfo.hasSize())
                throw ParserException(classInfo.line(), classInfo.column(), "Solid entity definition must not have a size");
            if (!classInfo.models().empty())
                throw ParserException(classInfo.line(), classInfo.column(), "Solid entity definition must not have model definitions");
            return new Assets::BrushEntityDefinition(classInfo.name(), classInfo.color(), classInfo.description(), classInfo.propertyList());
        }
        
        Assets::EntityDefinition* FgdParser::parsePointClass() {
            EntityDefinitionClassInfo classInfo = parseClass();
            return new Assets::PointEntityDefinition(classInfo.name(), classInfo.color(), classInfo.size(), classInfo.description(), classInfo.propertyList(), classInfo.models());
        }
        
        EntityDefinitionClassInfo FgdParser::parseBaseClass() {
            EntityDefinitionClassInfo classInfo = parseClass();
            if (m_baseClasses.count(classInfo.name()) > 0)
                throw ParserException(classInfo.line(), classInfo.column(), "Redefinition of base class '" + classInfo.name() + "'");
            return classInfo;
        }
        
        EntityDefinitionClassInfo FgdParser::parseClass() {
            Token token;
            expect(FgdToken::Word | FgdToken::Equality, token = m_tokenizer.nextToken());
            
            StringList superClasses;
            EntityDefinitionClassInfo classInfo(token.line(), token.column(), m_defaultEntityColor);
            
            while (token.type() == FgdToken::Word) {
                const String typeName = token.data();
                if (StringUtils::caseInsensitiveEqual(typeName, "base")) {
                    if (!superClasses.empty())
                        throw ParserException(token.line(), token.column(), "Found multiple base properties");
                    superClasses = parseSuperClasses();
                } else if (StringUtils::caseInsensitiveEqual(typeName, "color")) {
                    if (classInfo.hasColor())
                        throw ParserException(token.line(), token.column(), "Found multiple color properties");
                    classInfo.setColor(parseColor());
                } else if (StringUtils::caseInsensitiveEqual(typeName, "size")) {
                    if (classInfo.hasSize())
                        throw ParserException(token.line(), token.column(), "Found multiple size properties");
                    classInfo.setSize(parseSize());
                } else if (StringUtils::caseInsensitiveEqual(typeName, "model")) {
                    if (!classInfo.models().empty())
                        throw ParserException(token.line(), token.column(), "Found multiple model properties");
                    classInfo.addModelDefinitions(parseModels());
                } else {
                    throw ParserException(token.line(), token.column(), "Unknown entity definition header property '" + typeName + "'");
                }
                expect(FgdToken::Equality | FgdToken::Word, token = m_tokenizer.nextToken());
            }
            
            expect(FgdToken::Word, token = m_tokenizer.nextToken());
            classInfo.setName(token.data());
            
            expect(FgdToken::Colon | FgdToken::OBracket, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Colon) {
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                classInfo.setDescription(StringUtils::trim(token.data()));
            } else {
                m_tokenizer.pushToken(token);
            }
            
            classInfo.addPropertyDefinitions(parseProperties());
            classInfo.resolveBaseClasses(m_baseClasses, superClasses);
            return classInfo;
        }
        
        StringList FgdParser::parseSuperClasses() {
            StringList superClasses;
            Token token;
            expect(FgdToken::OParenthesis, token = m_tokenizer.nextToken());
            expect(FgdToken::Word | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Word) {
                m_tokenizer.pushToken(token);
                do {
                    expect(FgdToken::Word, token = m_tokenizer.nextToken());
                    superClasses.push_back(token.data());
                    expect(FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                } while (token.type() == FgdToken::Comma);
            }
            return superClasses;
        }
        
        Assets::ModelDefinitionList FgdParser::parseModels() {
            Assets::ModelDefinitionList result;
            Token token;
            expect(FgdToken::OParenthesis, token = m_tokenizer.nextToken());
            expect(FgdToken::String | FgdToken::Word | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::String || token.type() == FgdToken::Word) {
                m_tokenizer.pushToken(token);
                do {
                    expect(FgdToken::String | FgdToken::Word, token = m_tokenizer.peekToken());
                    if (token.type() == FgdToken::String)
                        result.push_back(parseStaticModel());
                    else
                        result.push_back(parseDynamicModel());
                    expect(FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                } while (token.type() == FgdToken::Comma);
            }
            return result;
        }
        
        Assets::ModelDefinitionPtr FgdParser::parseStaticModel() {
            Token token;
            expect(FgdToken::String, token = m_tokenizer.nextToken());
            const String pathStr = token.data();
            const IO::Path path(!pathStr.empty() && pathStr[0] == ':' ? pathStr.substr(1) : pathStr);
            
            std::vector<size_t> indices;
            
            expect(FgdToken::Integer | FgdToken::Word | FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Integer) {
                indices.push_back(token.toInteger<size_t>());
                expect(FgdToken::Integer | FgdToken::Word | FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::Integer) {
                    indices.push_back(token.toInteger<size_t>());
                    expect(FgdToken::Word | FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
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
                const String propertyKey = token.data();
                expect(FgdToken::Equality, token = m_tokenizer.nextToken());
                expect(FgdToken::String | FgdToken::Integer, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::String) {
                    const String propertyValue = token.data();
                    return Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(path, skinIndex, frameIndex, propertyKey, propertyValue));
                } else {
                    const int flagValue = token.toInteger<int>();
                    return Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(path, skinIndex, frameIndex, propertyKey, flagValue));
                }
            } else {
                m_tokenizer.pushToken(token);
                return Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(path, skinIndex, frameIndex));
            }
        }
        
        Assets::ModelDefinitionPtr FgdParser::parseDynamicModel() {
            Token token;
            String pathKey, skinKey, frameKey;
            
            expect(FgdToken::Word, token = m_tokenizer.nextToken());
            if (!StringUtils::caseInsensitiveEqual("pathKey", token.data()))
                throw ParserException(token.line(), token.column(), "Expected 'pathKey', but found '" + token.data() + "'");
            
            expect(FgdToken::Equality, token = m_tokenizer.nextToken());
            expect(FgdToken::String, token = m_tokenizer.nextToken());
            pathKey = token.data();
            
            expect(FgdToken::Word | FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Word) {
                if (!StringUtils::caseInsensitiveEqual("skinKey", token.data()))
                    throw ParserException(token.line(), token.column(), "Expected 'skinKey', but found '" + token.data() + "'");
                
                expect(FgdToken::Equality, token = m_tokenizer.nextToken());
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                skinKey = token.data();
                
                expect(FgdToken::Word | FgdToken::Comma | FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::Word) {
                    if (!StringUtils::caseInsensitiveEqual("frameKey", token.data()))
                        throw ParserException(token.line(), token.column(), "Expected 'frameKey', but found '" + token.data() + "'");
                    
                    expect(FgdToken::Equality, token = m_tokenizer.nextToken());
                    expect(FgdToken::String, token = m_tokenizer.nextToken());
                    frameKey = token.data();
                } else {
                    m_tokenizer.pushToken(token);
                }
            } else {
                m_tokenizer.pushToken(token);
            }
            
            return Assets::ModelDefinitionPtr(new Assets::DynamicModelDefinition(pathKey, skinKey, frameKey));
        }
        
        Assets::PropertyDefinitionMap FgdParser::parseProperties() {
            Assets::PropertyDefinitionMap properties;
            
            Token token;
            expect(FgdToken::OBracket, token = m_tokenizer.nextToken());
            expect(FgdToken::Word | FgdToken::CBracket, token = m_tokenizer.nextToken());
            while (token.type() != FgdToken::CBracket) {
                const String propertyKey = token.data();
                
                expect(FgdToken::OParenthesis, token = m_tokenizer.nextToken());
                expect(FgdToken::Word, token = m_tokenizer.nextToken());
                const String typeName = token.data();
                expect(FgdToken::CParenthesis, token = m_tokenizer.nextToken());
                
                if (StringUtils::caseInsensitiveEqual(typeName, "target_source")) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitions for property '" + propertyKey + "'");
                    properties[propertyKey] = parseTargetSourceProperty(propertyKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "target_destination")) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitions for property '" + propertyKey + "'");
                    properties[propertyKey] = parseTargetDestinationProperty(propertyKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "string")) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitions for property '" + propertyKey + "'");
                    properties[propertyKey] = parseStringProperty(propertyKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "integer")) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitions for property '" + propertyKey + "'");
                    properties[propertyKey] = parseIntegerProperty(propertyKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "float")) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitions for property '" + propertyKey + "'");
                    properties[propertyKey] = parseFloatProperty(propertyKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "choices")) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitions for property '" + propertyKey + "'");
                    properties[propertyKey] = parseChoicesProperty(propertyKey);
                } else if (StringUtils::caseInsensitiveEqual(typeName, "flags")) {
                    if (properties.count(propertyKey) > 0)
                        throw ParserException(token.line(), token.column(), "Multiple definitions for property '" + propertyKey + "'");
                    properties[propertyKey] = parseFlagsProperty(propertyKey);
                } else {
                    throw ParserException(token.line(), token.column(), "Unknown entity definition property '" + typeName + "'");
                }
                
                expect(FgdToken::Word | FgdToken::CBracket, token = m_tokenizer.nextToken());
            }
            
            return properties;
        }
        
        Assets::PropertyDefinitionPtr FgdParser::parseTargetSourceProperty(const String& name) {
            String description;
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Colon) {
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                description = token.data();
            } else {
                m_tokenizer.pushToken(token);
            }
            return Assets::PropertyDefinitionPtr(new Assets::PropertyDefinition(name, Assets::PropertyDefinition::TargetSourceProperty, description));
        }
        
        Assets::PropertyDefinitionPtr FgdParser::parseTargetDestinationProperty(const String& name) {
            String description;
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Colon) {
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                description = token.data();
            } else {
                m_tokenizer.pushToken(token);
            }
            return Assets::PropertyDefinitionPtr(new Assets::PropertyDefinition(name, Assets::PropertyDefinition::TargetDestinationProperty, description));
        }
        
        Assets::PropertyDefinitionPtr FgdParser::parseStringProperty(const String& name) {
            String description;
            String defaultValue;
            bool hasDefaultValue = false;
            
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Colon) {
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                description = token.data();
                
                token = m_tokenizer.nextToken();
                if (token.type() == FgdToken::Colon) {
                    expect(FgdToken::String, token = m_tokenizer.nextToken());
                    hasDefaultValue = true;
                    defaultValue = token.data();
                } else {
                    m_tokenizer.pushToken(token);
                }
            } else {
                m_tokenizer.pushToken(token);
            }
            
            if (hasDefaultValue)
                return Assets::PropertyDefinitionPtr(new Assets::StringPropertyDefinition(name, description, defaultValue));
            return Assets::PropertyDefinitionPtr(new Assets::StringPropertyDefinition(name, description));
        }
        
        Assets::PropertyDefinitionPtr FgdParser::parseIntegerProperty(const String& name) {
            String description;
            int defaultValue;
            bool hasDefaultValue = false;
            
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Colon) {
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                description = token.data();
                
                token = m_tokenizer.nextToken();
                if (token.type() == FgdToken::Colon) {
                    expect(FgdToken::Integer, token = m_tokenizer.nextToken());
                    hasDefaultValue = true;
                    defaultValue = token.toInteger<int>();
                } else {
                    m_tokenizer.pushToken(token);
                }
            } else {
                m_tokenizer.pushToken(token);
            }
            
            if (hasDefaultValue)
                return Assets::PropertyDefinitionPtr(new Assets::IntegerPropertyDefinition(name, description, defaultValue));
            return Assets::PropertyDefinitionPtr(new Assets::IntegerPropertyDefinition(name, description));
        }
        
        Assets::PropertyDefinitionPtr FgdParser::parseFloatProperty(const String& name) {
            String description;
            float defaultValue;
            bool hasDefaultValue = false;
            
            Token token = m_tokenizer.nextToken();
            if (token.type() == FgdToken::Colon) {
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                description = token.data();
                
                token = m_tokenizer.nextToken();
                if (token.type() == FgdToken::Colon) {
                    // the default value must have quotes around it
                    expect(FgdToken::String, token = m_tokenizer.nextToken());
                    hasDefaultValue = true;
                    defaultValue = token.toFloat<float>();
                } else {
                    m_tokenizer.pushToken(token);
                }
            } else {
                m_tokenizer.pushToken(token);
            }
            
            if (hasDefaultValue)
                return Assets::PropertyDefinitionPtr(new Assets::FloatPropertyDefinition(name, description, defaultValue));
            return Assets::PropertyDefinitionPtr(new Assets::FloatPropertyDefinition(name, description));
        }
        
        Assets::PropertyDefinitionPtr FgdParser::parseChoicesProperty(const String& name) {
            String description;
            int defaultValue = 0;
            bool hasDefaultValue = false;
            
            Token token;
            expect(FgdToken::Colon | FgdToken::Equality, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Colon) {
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                description = token.data();
                
                expect(FgdToken::Colon | FgdToken::Equality, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::Colon) {
                    expect(FgdToken::Integer, token = m_tokenizer.nextToken());
                    hasDefaultValue = true;
                    defaultValue = token.toInteger<int>();
                    expect(FgdToken::Equality, token = m_tokenizer.nextToken());
                }
            }
            
            assert(token.type() == FgdToken::Equality);
            expect(FgdToken::OBracket, token = m_tokenizer.nextToken());
            expect(FgdToken::Integer | FgdToken::String | FgdToken::CBracket, token = m_tokenizer.nextToken());
            
            Assets::ChoicePropertyOption::List options;
            
            while (token.type() != FgdToken::CBracket) {
                const String value = token.data();
                expect(FgdToken::Colon, token = m_tokenizer.nextToken());
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                const String caption = token.data();
                options.push_back(Assets::ChoicePropertyOption(value, caption));
                expect(FgdToken::Integer | FgdToken::String | FgdToken::CBracket, token = m_tokenizer.nextToken());
            }
            
            if (hasDefaultValue)
                return Assets::PropertyDefinitionPtr(new Assets::ChoicePropertyDefinition(name, description, options, defaultValue));
            return Assets::PropertyDefinitionPtr(new Assets::ChoicePropertyDefinition(name, description, options));
        }
        
        Assets::PropertyDefinitionPtr FgdParser::parseFlagsProperty(const String& name) {
            String description;
            
            Token token;
            expect(FgdToken::Colon | FgdToken::Equality, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Colon) {
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                description = token.data();
            }
            
            expect(FgdToken::OBracket, token = m_tokenizer.nextToken());
            expect(FgdToken::Integer | FgdToken::CBracket, token = m_tokenizer.nextToken());
            
            Assets::FlagsPropertyDefinition* definition = new Assets::FlagsPropertyDefinition(name, description);
            
            while (token.type() != FgdToken::CBracket) {
                const int value = token.toInteger<int>();
                expect(FgdToken::Colon, token = m_tokenizer.nextToken());
                
                expect(FgdToken::String, token = m_tokenizer.nextToken());
                const String caption = token.data();
                
                bool defaultValue = false;
                expect(FgdToken::Colon | FgdToken::Integer | FgdToken::CBracket, token = m_tokenizer.nextToken());
                if (token.type() == FgdToken::Colon) {
                    expect(FgdToken::Integer, token = m_tokenizer.nextToken());
                    defaultValue = token.toInteger<int>() != 0;
                } else {
                    m_tokenizer.pushToken(token);
                }
                
                definition->addOption(value, caption, defaultValue);
                expect(FgdToken::Integer | FgdToken::CBracket, token = m_tokenizer.nextToken());
            }
            
            return Assets::PropertyDefinitionPtr(definition);
        }
        
        Vec3 FgdParser::parseVector() {
            Token token;
            Vec3 vec;
            for (size_t i = 0; i < 3; i++) {
                expect(FgdToken::Integer | FgdToken::Decimal, token = m_tokenizer.nextToken());
                vec[i] = token.toFloat<double>();
            }
            return vec;
        }
        
        BBox3 FgdParser::parseSize() {
            Token token;
            BBox3 size;
            expect(FgdToken::OParenthesis, token = m_tokenizer.nextToken());
            size.min = parseVector();
            expect(FgdToken::CParenthesis | FgdToken::Comma, token = m_tokenizer.nextToken());
            if (token.type() == FgdToken::Comma) {
                size.max = parseVector();
                expect(FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            } else {
                const Vec3 halfSize = size.min / 2.0;
                size.min = -halfSize;
                size.max =  halfSize;
            }
            return size;
        }
        
        Color FgdParser::parseColor() {
            Color color;
            Token token;
            expect(FgdToken::OParenthesis, token = m_tokenizer.nextToken());
            for (size_t i = 0; i < 3; i++) {
                expect(FgdToken::Decimal | FgdToken::Integer, token = m_tokenizer.nextToken());
                color[i] = token.toFloat<float>();
                if (color[i] > 1.0f)
                    color[i] /= 255.0f;
            }
            expect(FgdToken::CParenthesis, token = m_tokenizer.nextToken());
            color[3] = 1.0f;
            return color;
        }
    }
}
