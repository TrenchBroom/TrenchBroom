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
#include "IO/ParserStatus.h"

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

        Assets::EntityDefinitionList DefParser::doParseDefinitions(ParserStatus& status) {
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
        
        Assets::EntityDefinition* DefParser::parseDefinition(ParserStatus& status) {
            Token token = m_tokenizer.nextToken();
            while (token.type() != DefToken::Eof && token.type() != DefToken::ODefinition)
                token = m_tokenizer.nextToken();
            if (token.type() == DefToken::Eof)
                return NULL;
            
            expect(status, DefToken::ODefinition, token);
            
            StringList baseClasses;
            EntityDefinitionClassInfo classInfo;
            
            token = m_tokenizer.nextToken();
            expect(status, DefToken::Word, token);
            classInfo.setName(token.data());
            
            token = m_tokenizer.peekToken();
            expect(status, DefToken::OParenthesis | DefToken::Newline, token);
            if (token.type() == DefToken::OParenthesis) {
                classInfo.setColor(parseColor(status));
                
                token = m_tokenizer.peekToken();
                expect(status, DefToken::OParenthesis | DefToken::Question, token);
                if (token.type() == DefToken::OParenthesis) {
                    classInfo.setSize(parseBounds(status));
                } else {
                    m_tokenizer.nextToken();
                }
                
                token = m_tokenizer.peekToken();
                if (token.type() == DefToken::Word)
                    classInfo.addAttributeDefinition(parseSpawnflags(status));
            }
            
            expect(status, DefToken::Newline, token = m_tokenizer.nextToken());
            
            Assets::AttributeDefinitionMap attributes;
            Assets::ModelDefinitionList models;
            StringList superClasses;
            parserAttributes(status, attributes, models, superClasses);
            classInfo.addAttributeDefinitions(attributes);
            classInfo.addModelDefinitions(models);
            
            classInfo.setDescription(StringUtils::trim(parseDescription()));
            expect(status, DefToken::CDefinition, token = m_tokenizer.nextToken());
            
            if (classInfo.hasColor()) {
                classInfo.resolveBaseClasses(m_baseClasses, superClasses);
                if (classInfo.hasSize()) // point definition
                    return new Assets::PointEntityDefinition(classInfo.name(), classInfo.color(), classInfo.size(), classInfo.description(), classInfo.attributeList(), classInfo.models());
                return new Assets::BrushEntityDefinition(classInfo.name(), m_defaultEntityColor, classInfo.description(), classInfo.attributeList());
            }
            
            // base definition
            m_baseClasses[classInfo.name()] = classInfo;
            return parseDefinition(status);
        }
        
        Assets::AttributeDefinitionPtr DefParser::parseSpawnflags(ParserStatus& status) {
            Assets::FlagsAttributeDefinition* definition = new Assets::FlagsAttributeDefinition(Model::AttributeNames::Spawnflags);
            size_t numOptions = 0;
            
            try {
                Token token = m_tokenizer.peekToken();
                while (token.type() == DefToken::Word) {
                    token = m_tokenizer.nextToken();
                    String name = token.data();
                    int value = 1 << numOptions++;
                    definition->addOption(value, name, "", false);
                    token = m_tokenizer.peekToken();
                }
            } catch (...) {
                delete definition;
                throw;
            }
            
            return Assets::AttributeDefinitionPtr(definition);
        }
        
        void DefParser::parserAttributes(ParserStatus& status, Assets::AttributeDefinitionMap& attributes, Assets::ModelDefinitionList& modelDefinitions, StringList& superClasses) {
            Token token = m_tokenizer.peekToken();
            if (token.type() == DefToken::OBrace) {
                token = m_tokenizer.nextToken();
                while (parseAttribute(status, attributes, modelDefinitions, superClasses));
            }
        }
        
        bool DefParser::parseAttribute(ParserStatus& status, Assets::AttributeDefinitionMap& attributes, Assets::ModelDefinitionList& modelDefinitions, StringList& superClasses) {
            Token token;
            expect(status, DefToken::Word | DefToken::CBrace, token = nextTokenIgnoringNewlines());
            if (token.type() != DefToken::Word)
                return false;

            String typeName = token.data();
            if (typeName == "default") {
                // ignore these attributes
                parseDefaultAttribute(status);
            } else if (typeName == "base") {
                parseBaseAttribute(status, superClasses);
            } else if (typeName == "choice") {
                parseChoiceAttribute(status, attributes);
            } else if (typeName == "model") {
                parseModelDefinitions(status, modelDefinitions);
            }
            
            expect(status, DefToken::Semicolon, token = nextTokenIgnoringNewlines());
            return true;
        }
        
        void DefParser::parseDefaultAttribute(ParserStatus& status) {
            Token token;
            expect(status, DefToken::OParenthesis, token = nextTokenIgnoringNewlines());
            expect(status, DefToken::QuotedString, token = nextTokenIgnoringNewlines());
            const String attributeName = token.data();
            expect(status, DefToken::Comma, token = nextTokenIgnoringNewlines());
            expect(status, DefToken::QuotedString, token = nextTokenIgnoringNewlines());
            const String attributeValue = token.data();
            expect(status, DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
        }

        void DefParser::parseBaseAttribute(ParserStatus& status, StringList& superClasses) {
            Token token;
            expect(status, DefToken::OParenthesis, token = nextTokenIgnoringNewlines());
            expect(status, DefToken::QuotedString, token = nextTokenIgnoringNewlines());
            const String basename = token.data();
            expect(status, DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
            
            superClasses.push_back(basename);
        }

        void DefParser::parseChoiceAttribute(ParserStatus& status, Assets::AttributeDefinitionMap& attributes) {
            Token token;
            expect(status, DefToken::QuotedString, token = m_tokenizer.nextToken());
            const String attributeName = token.data();
            
            Assets::ChoiceAttributeOption::List options;
            expect(status, DefToken::OParenthesis, token = nextTokenIgnoringNewlines());
            token = nextTokenIgnoringNewlines();
            while (token.type() == DefToken::OParenthesis) {
                expect(status, DefToken::Integer, token = nextTokenIgnoringNewlines());
                const String name = token.data();
                expect(status, DefToken::Comma, token = nextTokenIgnoringNewlines());
                expect(status, DefToken::QuotedString, token = nextTokenIgnoringNewlines());
                const String value = token.data();
                options.push_back(Assets::ChoiceAttributeOption(name, value));
                
                expect(status, DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                token = nextTokenIgnoringNewlines();
            }
            
            expect(status, DefToken::CParenthesis, token);
            
            attributes[attributeName] = Assets::AttributeDefinitionPtr(new Assets::ChoiceAttributeDefinition(attributeName, "", "", options));
        }

        void DefParser::parseModelDefinitions(ParserStatus& status, Assets::ModelDefinitionList& modelDefinitions) {
            Token token;
            expect(status, DefToken::OParenthesis, token = nextTokenIgnoringNewlines());
            expect(status, DefToken::QuotedString | DefToken::Word | DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
            if (token.type() == DefToken::QuotedString) {
                m_tokenizer.pushToken(token);
                parseStaticModelDefinition(status, modelDefinitions);
            } else if (token.type() == DefToken::Word) {
                m_tokenizer.pushToken(token);
                parseDynamicModelDefinition(status, modelDefinitions);
            }
        }

        void DefParser::parseStaticModelDefinition(ParserStatus& status, Assets::ModelDefinitionList& modelDefinitions) {
            Token token;
            expect(status, DefToken::QuotedString, token = nextTokenIgnoringNewlines());

            const String pathStr = token.data();
            const IO::Path path(!pathStr.empty() && pathStr[0] == ':' ? pathStr.substr(1) : pathStr);
            
            std::vector<size_t> indices;
            
            expect(status, DefToken::Integer | DefToken::Word | DefToken::Comma | DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
            if (token.type() == DefToken::Integer) {
                indices.push_back(token.toInteger<size_t>());
                expect(status, DefToken::Integer | DefToken::Word | DefToken::Comma | DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
                if (token.type() == DefToken::Integer) {
                    indices.push_back(token.toInteger<size_t>());
                    expect(status, DefToken::Word | DefToken::Comma | DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
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
                
                expect(status, DefToken::Equality, token = nextTokenIgnoringNewlines());
                expect(status, DefToken::QuotedString | DefToken::Integer, token = nextTokenIgnoringNewlines());
                if (token.type() == DefToken::QuotedString) {
                    const String attributeValue = token.data();
                    modelDefinitions.push_back(Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(path,
                                                                                                            skinIndex,
                                                                                                            frameIndex,
                                                                                                            attributeKey, attributeValue)));
                } else {
                    const int flagValue = token.toInteger<int>();
                    modelDefinitions.push_back(Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(path,
                                                                                                            skinIndex,
                                                                                                            frameIndex,
                                                                                                            attributeKey, flagValue)));
                }
                expect(status, DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
            } else {
                modelDefinitions.push_back(Assets::ModelDefinitionPtr(new Assets::StaticModelDefinition(path,
                                                                                                        skinIndex,
                                                                                                        frameIndex)));
            }
        }
        
        void DefParser::parseDynamicModelDefinition(ParserStatus& status, Assets::ModelDefinitionList& modelDefinitions) {
            Token token;
            expect(status, DefToken::Word, token = nextTokenIgnoringNewlines());

            String pathKey, skinKey, frameKey;
            
            if (!StringUtils::caseInsensitiveEqual("pathKey", token.data())) {
                const String msg("Expected 'pathKey', but found '" + token.data() + "'");
                status.error(token.line(), token.column(), msg);
                throw ParserException(token.line(), token.column(), msg);
            }
            
            expect(status, DefToken::Equality, token = m_tokenizer.nextToken());
            expect(status, DefToken::QuotedString, token = m_tokenizer.nextToken());
            pathKey = token.data();
            
            expect(status, DefToken::Word | DefToken::Comma | DefToken::CParenthesis, token = m_tokenizer.nextToken());
            if (token.type() == DefToken::Word) {
                if (!StringUtils::caseInsensitiveEqual("skinKey", token.data())) {
                    const String msg("Expected 'skinKey', but found '" + token.data() + "'");
                    status.error(token.line(), token.column(), msg);
                    throw ParserException(token.line(), token.column(), msg);
                }
                
                expect(status, DefToken::Equality, token = m_tokenizer.nextToken());
                expect(status, DefToken::QuotedString, token = m_tokenizer.nextToken());
                skinKey = token.data();
                
                expect(status, DefToken::Word | DefToken::Comma | DefToken::CParenthesis, token = m_tokenizer.nextToken());
                if (token.type() == DefToken::Word) {
                    if (!StringUtils::caseInsensitiveEqual("frameKey", token.data())) {
                        const String msg("Expected 'frameKey', but found '" + token.data() + "'");
                        status.error(token.line(), token.column(), msg);
                        throw ParserException(token.line(), token.column(), msg);
                    }
                    
                    expect(status, DefToken::Equality, token = m_tokenizer.nextToken());
                    expect(status, DefToken::QuotedString, token = m_tokenizer.nextToken());
                    frameKey = token.data();
                } else {
                    m_tokenizer.pushToken(token);
                }
            } else {
                m_tokenizer.pushToken(token);
            }
            
            expect(status, DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
            modelDefinitions.push_back(Assets::ModelDefinitionPtr(new Assets::DynamicModelDefinition(pathKey, skinKey, frameKey)));
        }

        String DefParser::parseDescription() {
            Token token = m_tokenizer.peekToken();
            if (token.type() == DefToken::CDefinition)
                return "";
            return m_tokenizer.readRemainder(DefToken::CDefinition);
        }
        
        Vec3 DefParser::parseVector(ParserStatus& status) {
            Token token;
            Vec3 vec;
            for (size_t i = 0; i < 3; i++) {
                expect(status, DefToken::Integer | DefToken::Decimal, token = m_tokenizer.nextToken());
                vec[i] = token.toFloat<double>();
            }
            return vec;
        }
        
        BBox3 DefParser::parseBounds(ParserStatus& status) {
            BBox3 bounds;
            Token token;
            expect(status, DefToken::OParenthesis, token = m_tokenizer.nextToken());
            bounds.min = parseVector(status);
            expect(status, DefToken::CParenthesis, token = m_tokenizer.nextToken());
            expect(status, DefToken::OParenthesis, token = m_tokenizer.nextToken());
            bounds.max = parseVector(status);
            expect(status, DefToken::CParenthesis, token = m_tokenizer.nextToken());
            return bounds;
        }
        
        Color DefParser::parseColor(ParserStatus& status) {
            Color color;
            Token token;
            expect(status, DefToken::OParenthesis, token = m_tokenizer.nextToken());
            for (size_t i = 0; i < 3; i++) {
                expect(status, DefToken::Decimal | DefToken::Integer, token = m_tokenizer.nextToken());
                color[i] = token.toFloat<float>();
                if (color[i] > 1.0f)
                    color[i] /= 255.0f;
            }
            expect(status, DefToken::CParenthesis, token = m_tokenizer.nextToken());
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
