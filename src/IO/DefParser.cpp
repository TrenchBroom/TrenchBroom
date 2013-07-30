/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DefParser.h"

#include "CollectionUtils.h"
#include "Exceptions.h"
#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"
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
                const char* c = nextChar();
                switch (*c) {
                    case '/': {
                        const char* begin = c;
                        if (peekChar() == '*') {
                            // eat all chars immediately after the '*' because it's often followed by QUAKE
                            while (!isWhitespace(*nextChar()) && !eof());
                            return Token(DefToken::ODefinition, begin, c, offset(begin), startLine, startColumn);
                        } else if (peekChar() == '/') {
                            discardUntil("\n\r");
                            break;
                        }
                        // fall through and try to read as word
                    }
                    case '*': {
                        const char* begin = c;
                        if (peekChar() == '/') {
                            nextChar();
                            return Token(DefToken::CDefinition, begin, c, offset(begin), startLine, startColumn);
                        }
                        // fall through and try to read as word
                    }
                    case '(':
                        return Token(DefToken::OParenthesis, c, c + 1, offset(c), startLine, startColumn);
                    case ')':
                        return Token(DefToken::CParenthesis, c, c + 1, offset(c), startLine, startColumn);
                    case '{':
                        return Token(DefToken::OBrace, c, c + 1, offset(c), startLine, startColumn);
                    case '}':
                        return Token(DefToken::CBrace, c, c + 1, offset(c), startLine, startColumn);
                    case '=':
                        return Token(DefToken::Equality, c, c + 1, offset(c), startLine, startColumn);
                    case ';':
                        return Token(DefToken::Semicolon, c, c + 1, offset(c), startLine, startColumn);
                    case '?':
                        return Token(DefToken::Question, c, c + 1, offset(c), startLine, startColumn);
                    case '\r':
                        if (peekChar() == '\n')
                            nextChar();
                    case '\n':
                        return Token(DefToken::Newline, c, c + 1, offset(c), startLine, startColumn);
                    case ',':
                        return Token(DefToken::Comma, c, c + 1, offset(c), startLine, startColumn);
                    case ' ':
                    case '\t':
                        break;
                    case '"': { // quoted string
                        const char* begin = nextChar();
                        const char* end = readQuotedString(begin);
                        return Token(DefToken::QuotedString, begin, end, offset(begin), startLine, startColumn);
                    }
                    default: { // integer, decimal or word
                        if (isWhitespace(*c)) {
                            discardWhile(Whitespace);
                        } else {
                            const char* begin = c;
                            const char* end = readInteger(begin, WordDelims);
                            if (end > begin)
                                return Token(DefToken::Integer, begin, end, offset(begin), startLine, startColumn);
                            
                            end = readDecimal(begin, WordDelims);
                            if (end > begin)
                                return Token(DefToken::Decimal, begin, end, offset(begin), startLine, startColumn);
                            
                            end = readString(begin, WordDelims);
                            if (end == begin)
                                throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                            return Token(DefToken::Word, begin, end, offset(begin), startLine, startColumn);
                        }
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
        
        String DefParser::tokenName(const DefToken::Type typeMask) const {
            StringList names;
            if ((typeMask & DefToken::Integer) != 0)
                names.push_back("integer number");
            if ((typeMask & DefToken::Decimal) != 0)
                names.push_back("decimal number");
            if ((typeMask & DefToken::QuotedString) != 0)
                names.push_back("string");
            if ((typeMask & DefToken::OParenthesis) != 0)
                names.push_back("opening parenthesis");
            if ((typeMask & DefToken::CParenthesis) != 0)
                names.push_back("closing parenthesis");
            if ((typeMask & DefToken::OBrace) != 0)
                names.push_back("opening brace");
            if ((typeMask & DefToken::CBrace) != 0)
                names.push_back("closing brace");
            if ((typeMask & DefToken::Word) != 0)
                names.push_back("word");
            if ((typeMask & DefToken::Question) != 0)
                names.push_back("question mark");
            if ((typeMask & DefToken::ODefinition) != 0)
                names.push_back("definition start ('/*')");
            if ((typeMask & DefToken::CDefinition) != 0)
                names.push_back("definition end ('*/')");
            if ((typeMask & DefToken::Semicolon) != 0)
                names.push_back("semicolon");
            if ((typeMask & DefToken::Newline) != 0)
                names.push_back("newline");
            if ((typeMask & DefToken::Comma) != 0)
                names.push_back("comma");
            if ((typeMask & DefToken::Equality) != 0)
                names.push_back("equality sign");
            
            if (names.empty())
                return "unknown token type";
            if (names.size() == 1)
                return names[0];
            
            StringStream str;
            str << names[0];
            for (unsigned int i = 1; i < names.size() - 1; i++)
                str << ", " << names[i];
            str << ", or " << names[names.size() - 1];
            return str.str();
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
                    classInfo.addPropertyDefinition(parseSpawnflags());
            }
            
            expect(DefToken::Newline, token = m_tokenizer.nextToken());
            
            Assets::PropertyDefinitionMap properties;
            Assets::ModelDefinitionList models;
            StringList superClasses;
            parseProperties(properties, models, superClasses);
            classInfo.addPropertyDefinitions(properties);
            classInfo.addModelDefinitions(models);
            
            classInfo.setDescription(StringUtils::trim(parseDescription()));
            expect(DefToken::CDefinition, token = m_tokenizer.nextToken());
            
            if (classInfo.hasColor()) {
                classInfo.resolveBaseClasses(m_baseClasses, superClasses);
                if (classInfo.hasSize()) // point definition
                    return new Assets::PointEntityDefinition(classInfo.name(), classInfo.color(), classInfo.size(), classInfo.description(), classInfo.propertyList(), classInfo.models());
                return new Assets::BrushEntityDefinition(classInfo.name(), m_defaultEntityColor, classInfo.description(), classInfo.propertyList());
            }
            
            // base definition
            m_baseClasses[classInfo.name()] = classInfo;
            return parseDefinition();
        }
        
        Assets::PropertyDefinitionPtr DefParser::parseSpawnflags() {
            Assets::FlagsPropertyDefinition* definition = new Assets::FlagsPropertyDefinition(Model::PropertyKeys::Spawnflags, "Spawnflags");
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
            
            return Assets::PropertyDefinitionPtr(definition);
        }
        
        void DefParser::parseProperties(Assets::PropertyDefinitionMap& properties, Assets::ModelDefinitionList& modelDefinitions, StringList& superClasses) {
            Token token = m_tokenizer.peekToken();
            if (token.type() == DefToken::OBrace) {
                token = m_tokenizer.nextToken();
                while (parseProperty(properties, modelDefinitions, superClasses));
            }
        }
        
        bool DefParser::parseProperty(Assets::PropertyDefinitionMap& properties, Assets::ModelDefinitionList& modelDefinitions, StringList& superClasses) {
            Token token;
            expect(DefToken::Word | DefToken::CBrace, token = nextTokenIgnoringNewlines());
            if (token.type() != DefToken::Word)
                return false;
            
            String typeName = token.data();
            if (typeName == "choice") {
                expect(DefToken::QuotedString, token = m_tokenizer.nextToken());
                const String propertyName = token.data();
                
                Assets::ChoicePropertyOption::List options;
                expect(DefToken::OParenthesis, token = nextTokenIgnoringNewlines());
                token = nextTokenIgnoringNewlines();
                while (token.type() == DefToken::OParenthesis) {
                    expect(DefToken::Integer, token = nextTokenIgnoringNewlines());
                    const String key = token.data();
                    expect(DefToken::Comma, token = nextTokenIgnoringNewlines());
                    expect(DefToken::QuotedString, token = nextTokenIgnoringNewlines());
                    const String value = token.data();
                    options.push_back(Assets::ChoicePropertyOption(key, value));
                    
                    expect(DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                    token = nextTokenIgnoringNewlines();
                }
                
                expect(DefToken::CParenthesis, token);
                properties[propertyName] = Assets::PropertyDefinitionPtr(new Assets::ChoicePropertyDefinition(propertyName, "", options));
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
                    
                    if (token.type() == DefToken::Word) { // parse property or flag
                        const String propertyKey = token.data();
                        
                        expect(DefToken::Equality, token = nextTokenIgnoringNewlines());
                        expect(DefToken::QuotedString | DefToken::Integer, token = nextTokenIgnoringNewlines());
                        if (token.type() == DefToken::QuotedString) {
                            const String propertyValue = token.data();
                            modelDefinitions.push_back(Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(modelPath,
                                                                                                                    skinIndex,
                                                                                                                    frameIndex,
                                                                                                                    propertyKey, propertyValue)));
                        } else {
                            const int flagValue = token.toInteger<int>();
                            modelDefinitions.push_back(Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(modelPath,
                                                                                                                    skinIndex,
                                                                                                                    frameIndex,
                                                                                                                    propertyKey, flagValue)));
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
                const String propertyName = token.data();
                expect(DefToken::Comma, token = nextTokenIgnoringNewlines());
                expect(DefToken::QuotedString, token = nextTokenIgnoringNewlines());
                const String propertyValue = token.data();
                expect(DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                
                // ignore these properties
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
