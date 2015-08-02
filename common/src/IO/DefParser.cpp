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

#include "DefParser.h"

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"
#include "Assets/ModelDefinition.h"

namespace TrenchBroom {
    namespace IO {
        DefTokenizer::DefTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end) {}
        
        DefTokenizer::DefTokenizer(const String& str) :
        Tokenizer(str) {}
        
        const String DefTokenizer::WordDelims = " \t\n\r()[]{}?;,=";
        
        DefTokenizer::Token DefTokenizer::emitToken() {
            while (!eof()) {
                size_t startLine = line();
                size_t startColumn = column();
                const char* c = curPos();
                switch (*c) {
                    case '/': {
                        advance();
                        if (curChar() == '*') {
                            // eat all chars immediately after the '*' because it's often followed by QUAKE
                            do { advance(); } while (!eof() && !isWhitespace(curChar()));
                            return Token(DefToken::ODefinition, c, curPos(), offset(c), startLine, startColumn);
                        } else if (curChar() == '/') {
                            discardUntil("\n\r");
                            break;
                        }
                        // fall through and try to read as word
                        retreat();
                    }
                    case '*': {
                        advance();
                        if (curChar() == '/')
                            return Token(DefToken::CDefinition, c, curPos(), offset(c), startLine, startColumn);
                        // fall through and try to read as word
                        retreat();
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
                    case '?':
                        advance();
                        return Token(DefToken::Question, c, c + 1, offset(c), startLine, startColumn);
                    case '\r':
                        advance();
                    case '\n':
                        advance();
                        return Token(DefToken::Newline, c, c + 1, offset(c), startLine, startColumn);
                    case ',':
                        advance();
                        return Token(DefToken::Comma, c, c + 1, offset(c), startLine, startColumn);
                    case ' ':
                    case '\t':
                        discardWhile(Whitespace);
                        break;
                    case '"': { // quoted string
                        advance();
                        c = curPos();
                        const char* e = readQuotedString();
                        return Token(DefToken::QuotedString, c, e, offset(c), startLine, startColumn);
                    }
                    default: { // integer, decimal or word
                        const char* e = readInteger(WordDelims);
                        if (e != NULL)
                            return Token(DefToken::Integer, c, e, offset(c), startLine, startColumn);
                        e = readDecimal(WordDelims);
                        if (e != NULL)
                            return Token(DefToken::Decimal, c, e, offset(c), startLine, startColumn);
                        e = readString(WordDelims);
                        if (e == NULL)
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        return Token(DefToken::Word, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(DefToken::Eof, NULL, NULL, length(), line(), column());
        }
        
        DefParser::DefParser(const char* begin, const char* end, const Color& defaultEntityColor) :
        m_defaultEntityColor(defaultEntityColor),
        m_tokenizer(DefTokenizer(begin, end)) {}
        
        DefParser::DefParser(const String& str, const Color& defaultEntityColor) :
        m_defaultEntityColor(defaultEntityColor),
        m_tokenizer(DefTokenizer(str)) {}
        
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
            names[Question]     = "'?'";
            names[ODefinition]  = "'/*'";
            names[CDefinition]  = "'*/'";
            names[Semicolon]    = "';'";
            names[Comma]        = "','";
            names[Equality]     = "'='";
            names[Eof]          = "end of file";
            return names;
        }

        Assets::EntityDefinitionList DefParser::doParseDefinitions() {
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
        
        Assets::EntityDefinition* DefParser::parseDefinition() {
            Token token = m_tokenizer.nextToken();
            while (token.type() != DefToken::Eof && token.type() != DefToken::ODefinition)
                token = m_tokenizer.nextToken();
            if (token.type() == DefToken::Eof)
                return NULL;
            
            expect(DefToken::ODefinition, token);
            
            StringList baseClasses;
            EntityDefinitionClassInfo classInfo;
            
            token = m_tokenizer.nextToken();
            expect(DefToken::Word, token);
            classInfo.setName(token.data());
            
            token = m_tokenizer.peekToken();
            expect(DefToken::OParenthesis | DefToken::Newline, token);
            if (token.type() == DefToken::OParenthesis) {
                classInfo.setColor(parseColor());
                
                token = m_tokenizer.peekToken();
                expect(DefToken::OParenthesis | DefToken::Question, token);
                if (token.type() == DefToken::OParenthesis) {
                    classInfo.setSize(parseBounds());
                } else {
                    m_tokenizer.nextToken();
                }
                
                token = m_tokenizer.peekToken();
                if (token.type() == DefToken::Word)
                    classInfo.addAttributeDefinition(parseSpawnflags());
            }
            
            expect(DefToken::Newline, token = m_tokenizer.nextToken());
            
            Assets::AttributeDefinitionMap attributes;
            Assets::ModelDefinitionList models;
            StringList superClasses;
            parseProperties(attributes, models, superClasses);
            classInfo.addAttributeDefinitions(attributes);
            classInfo.addModelDefinitions(models);
            
            classInfo.setDescription(StringUtils::trim(parseDescription()));
            expect(DefToken::CDefinition, token = m_tokenizer.nextToken());
            
            if (classInfo.hasColor()) {
                classInfo.resolveBaseClasses(m_baseClasses, superClasses);
                if (classInfo.hasSize()) // point definition
                    return new Assets::PointEntityDefinition(classInfo.name(), classInfo.color(), classInfo.size(), classInfo.description(), classInfo.attributeList(), classInfo.models());
                return new Assets::BrushEntityDefinition(classInfo.name(), m_defaultEntityColor, classInfo.description(), classInfo.attributeList());
            }
            
            // base definition
            m_baseClasses[classInfo.name()] = classInfo;
            return parseDefinition();
        }
        
        Assets::AttributeDefinitionPtr DefParser::parseSpawnflags() {
            Assets::FlagsAttributeDefinition* definition = new Assets::FlagsAttributeDefinition(Model::AttributeNames::Spawnflags);
            size_t numOptions = 0;
            
            try {
                Token token = m_tokenizer.peekToken();
                while (token.type() == DefToken::Word) {
                    token = m_tokenizer.nextToken();
                    String name = token.data();
                    int value = 1 << numOptions++;
                    definition->addOption(value, name, false);
                    token = m_tokenizer.peekToken();
                }
            } catch (...) {
                delete definition;
                throw;
            }
            
            return Assets::AttributeDefinitionPtr(definition);
        }
        
        void DefParser::parseProperties(Assets::AttributeDefinitionMap& attributes, Assets::ModelDefinitionList& modelDefinitions, StringList& superClasses) {
            Token token = m_tokenizer.peekToken();
            if (token.type() == DefToken::OBrace) {
                token = m_tokenizer.nextToken();
                while (parseAttribute(attributes, modelDefinitions, superClasses));
            }
        }
        
        bool DefParser::parseAttribute(Assets::AttributeDefinitionMap& attributes, Assets::ModelDefinitionList& modelDefinitions, StringList& superClasses) {
            Token token;
            expect(DefToken::Word | DefToken::CBrace, token = nextTokenIgnoringNewlines());
            if (token.type() != DefToken::Word)
                return false;
            
            String typeName = token.data();
            if (typeName == "choice") {
                expect(DefToken::QuotedString, token = m_tokenizer.nextToken());
                const String attributeName = token.data();
                
                Assets::ChoiceAttributeOption::List options;
                expect(DefToken::OParenthesis, token = nextTokenIgnoringNewlines());
                token = nextTokenIgnoringNewlines();
                while (token.type() == DefToken::OParenthesis) {
                    expect(DefToken::Integer, token = nextTokenIgnoringNewlines());
                    const String name = token.data();
                    expect(DefToken::Comma, token = nextTokenIgnoringNewlines());
                    expect(DefToken::QuotedString, token = nextTokenIgnoringNewlines());
                    const String value = token.data();
                    options.push_back(Assets::ChoiceAttributeOption(name, value));
                    
                    expect(DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                    token = nextTokenIgnoringNewlines();
                }
                
                expect(DefToken::CParenthesis, token);
                attributes[attributeName] = Assets::AttributeDefinitionPtr(new Assets::ChoiceAttributeDefinition(attributeName, "", "", options));
            } else if (typeName == "model") {
                expect(DefToken::OParenthesis, token = nextTokenIgnoringNewlines());
                expect(DefToken::QuotedString | DefToken::Word | DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                if (token.type() == DefToken::QuotedString) {
                    const String modelPath = token.data();
                    std::vector<size_t> indices;
                    
                    expect(DefToken::Integer | DefToken::Word | DefToken::Comma | DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                    if (token.type() == DefToken::Integer) {
                        indices.push_back(token.toInteger<size_t>());
                        expect(DefToken::Integer | DefToken::Word | DefToken::Comma | DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                        if (token.type() == DefToken::Integer) {
                            indices.push_back(token.toInteger<size_t>());
                            expect(DefToken::Word | DefToken::Comma | DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                        }
                    }
                    
                    size_t skinIndex = 0;
                    size_t frameIndex = 0;
                    if (!indices.empty()) {
                        skinIndex = indices[0];
                        if (indices.size() > 1)
                            frameIndex = indices[1];
                    }
                    
                    if (token.type() == DefToken::Word) { // parse attribute or flag
                        const String attributeKey = token.data();
                        
                        expect(DefToken::Equality, token = nextTokenIgnoringNewlines());
                        expect(DefToken::QuotedString | DefToken::Integer, token = nextTokenIgnoringNewlines());
                        if (token.type() == DefToken::QuotedString) {
                            const String attributeValue = token.data();
                            modelDefinitions.push_back(Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(modelPath,
                                                                                                                    skinIndex,
                                                                                                                    frameIndex,
                                                                                                                    attributeKey, attributeValue)));
                        } else {
                            const int flagValue = token.toInteger<int>();
                            modelDefinitions.push_back(Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(modelPath,
                                                                                                                    skinIndex,
                                                                                                                    frameIndex,
                                                                                                                    attributeKey, flagValue)));
                        }
                        expect(DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                    } else {
                        modelDefinitions.push_back(Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(modelPath,
                                                                                                                skinIndex,
                                                                                                                frameIndex)));
                    }
                } else if (token.type() == DefToken::Word) {
                    String pathKey, skinKey, frameKey;
                    
                    if (!StringUtils::caseInsensitiveEqual("pathKey", token.data()))
                        throw ParserException(token.line(), token.column(), "Expected 'pathKey', but found '" + token.data() + "'");
                    
                    expect(DefToken::Equality, token = m_tokenizer.nextToken());
                    expect(DefToken::QuotedString, token = m_tokenizer.nextToken());
                    pathKey = token.data();
                    
                    expect(DefToken::Word | DefToken::Comma | DefToken::CParenthesis, token = m_tokenizer.nextToken());
                    if (token.type() == DefToken::Word) {
                        if (!StringUtils::caseInsensitiveEqual("skinKey", token.data()))
                            throw ParserException(token.line(), token.column(), "Expected 'skinKey', but found '" + token.data() + "'");
                        
                        expect(DefToken::Equality, token = m_tokenizer.nextToken());
                        expect(DefToken::QuotedString, token = m_tokenizer.nextToken());
                        skinKey = token.data();
                        
                        expect(DefToken::Word | DefToken::Comma | DefToken::CParenthesis, token = m_tokenizer.nextToken());
                        if (token.type() == DefToken::Word) {
                            if (!StringUtils::caseInsensitiveEqual("frameKey", token.data()))
                                throw ParserException(token.line(), token.column(), "Expected 'frameKey', but found '" + token.data() + "'");
                            
                            expect(DefToken::Equality, token = m_tokenizer.nextToken());
                            expect(DefToken::QuotedString, token = m_tokenizer.nextToken());
                            frameKey = token.data();
                        } else {
                            m_tokenizer.pushToken(token);
                        }
                    } else {
                        m_tokenizer.pushToken(token);
                    }
                    
                    expect(DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                    modelDefinitions.push_back(Assets::ModelDefinitionPtr(new Assets::DynamicModelDefinition(pathKey, skinKey, frameKey)));
                }
            } else if (typeName == "default") {
                expect(DefToken::OParenthesis, token = nextTokenIgnoringNewlines());
                expect(DefToken::QuotedString, token = nextTokenIgnoringNewlines());
                const String attributeName = token.data();
                expect(DefToken::Comma, token = nextTokenIgnoringNewlines());
                expect(DefToken::QuotedString, token = nextTokenIgnoringNewlines());
                const String attributeValue = token.data();
                expect(DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                
                // ignore these attributes
            } else if (typeName == "base") {
                expect(DefToken::OParenthesis, token = nextTokenIgnoringNewlines());
                expect(DefToken::QuotedString, token = nextTokenIgnoringNewlines());
                const String basename = token.data();
                expect(DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                
                superClasses.push_back(basename);
            }
            
            expect(DefToken::Semicolon, token = nextTokenIgnoringNewlines());
            return true;
        }
        
        String DefParser::parseDescription() {
            Token token = m_tokenizer.peekToken();
            if (token.type() == DefToken::CDefinition)
                return "";
            return m_tokenizer.readRemainder(DefToken::CDefinition);
        }
        
        Vec3 DefParser::parseVector() {
            Token token;
            Vec3 vec;
            for (size_t i = 0; i < 3; i++) {
                expect(DefToken::Integer | DefToken::Decimal, token = m_tokenizer.nextToken());
                vec[i] = token.toFloat<double>();
            }
            return vec;
        }
        
        BBox3 DefParser::parseBounds() {
            BBox3 bounds;
            Token token;
            expect(DefToken::OParenthesis, token = m_tokenizer.nextToken());
            bounds.min = parseVector();
            expect(DefToken::CParenthesis, token = m_tokenizer.nextToken());
            expect(DefToken::OParenthesis, token = m_tokenizer.nextToken());
            bounds.max = parseVector();
            expect(DefToken::CParenthesis, token = m_tokenizer.nextToken());
            return bounds;
        }
        
        Color DefParser::parseColor() {
            Color color;
            Token token;
            expect(DefToken::OParenthesis, token = m_tokenizer.nextToken());
            for (size_t i = 0; i < 3; i++) {
                expect(DefToken::Decimal | DefToken::Integer, token = m_tokenizer.nextToken());
                color[i] = token.toFloat<float>();
                if (color[i] > 1.0f)
                    color[i] /= 255.0f;
            }
            expect(DefToken::CParenthesis, token = m_tokenizer.nextToken());
            color[3] = 1.0f;
            return color;
        }
        
        DefParser::Token DefParser::nextTokenIgnoringNewlines() {
            Token token = m_tokenizer.nextToken();
            while (token.type() == DefToken::Newline)
                token = m_tokenizer.nextToken();
            return token;
        }
    }
}
