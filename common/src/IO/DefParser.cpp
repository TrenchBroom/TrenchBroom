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

#include "DefParser.h"

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
        DefTokenizer::DefTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end) {}
        
        DefTokenizer::DefTokenizer(const String& str) :
        Tokenizer(str) {}
        
        const String DefTokenizer::WordDelims = " \t\n\r()[]{};,=";
        
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
                    }
                    case '*': {
                        if (lookAhead() == '/') {
                            advance();
                            return Token(DefToken::CDefinition, c, curPos(), offset(c), startLine, startColumn);
                        }
                        // fall through and try to read as word
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
                        advance();
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
                    default: { // integer, decimal or word
                        const char* e = readInteger(WordDelims);
                        if (e != NULL)
                            return Token(DefToken::Integer, c, e, offset(c), startLine, startColumn);
                        e = readDecimal(WordDelims);
                        if (e != NULL)
                            return Token(DefToken::Decimal, c, e, offset(c), startLine, startColumn);
                        e = readUntil(WordDelims);
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
            names[ODefinition]  = "'/*'";
            names[CDefinition]  = "'*/'";
            names[Semicolon]    = "';'";
            names[Newline]      = "newline";
            names[Comma]        = "','";
            names[Equality]     = "'='";
            names[Eof]          = "end of file";
            return names;
        }

        Assets::EntityDefinitionArray DefParser::doParseDefinitions(ParserStatus& status) {
            Assets::EntityDefinitionArray definitions;
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
            
            StringArray baseClasses;
            EntityDefinitionClassInfo classInfo;
            
            token = m_tokenizer.nextToken();
            expect(status, DefToken::Word, token);
            classInfo.setName(token.data());
            
            token = m_tokenizer.peekToken();
            expect(status, DefToken::OParenthesis | DefToken::Newline, token);
            if (token.type() == DefToken::OParenthesis) {
                classInfo.setColor(parseColor(status));
                
                token = m_tokenizer.peekToken();
                expect(status, DefToken::OParenthesis | DefToken::Word, token);
                if (token.type() == DefToken::OParenthesis) {
                    classInfo.setSize(parseBounds(status));
                } else if (token.data() == "?") {
                    m_tokenizer.nextToken();
                } else {
                    expect(status, "question mark: ?", token);
                }
                
                token = m_tokenizer.peekToken();
                if (token.type() == DefToken::Word)
                    classInfo.addAttributeDefinition(parseSpawnflags(status));
            }
            
            expect(status, DefToken::Newline, token = m_tokenizer.nextToken());
            
            Assets::AttributeDefinitionMap attributes;
            StringArray superClasses;
            parseAttributes(status, classInfo, superClasses);
            
            classInfo.setDescription(StringUtils::trim(parseDescription()));
            expect(status, DefToken::CDefinition, token = m_tokenizer.nextToken());
            
            if (classInfo.hasColor()) {
                classInfo.resolveBaseClasses(m_baseClasses, superClasses);
                if (classInfo.hasSize()) // point definition
                    return new Assets::PointEntityDefinition(classInfo.name(), classInfo.color(), classInfo.size(), classInfo.description(), classInfo.attributeArray(), classInfo.modelDefinition());
                return new Assets::BrushEntityDefinition(classInfo.name(), m_defaultEntityColor, classInfo.description(), classInfo.attributeArray());
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
        
        void DefParser::parseAttributes(ParserStatus& status, EntityDefinitionClassInfo& classInfo, StringArray& superClasses) {
            Token token = m_tokenizer.peekToken();
            if (token.type() == DefToken::OBrace) {
                token = m_tokenizer.nextToken();
                while (parseAttribute(status, classInfo, superClasses));
            }
        }
        
        bool DefParser::parseAttribute(ParserStatus& status, EntityDefinitionClassInfo& classInfo, StringArray& superClasses) {
            Token token;
            expect(status, DefToken::Word | DefToken::CBrace, token = nextTokenIgnoringNewlines());
            if (token.type() != DefToken::Word)
                return false;

            String typeName = token.data();
            if (typeName == "default") {
                // ignore these attributes
                parseDefaultAttribute(status);
            } else if (typeName == "base") {
                superClasses.push_back(parseBaseAttribute(status));
            } else if (typeName == "choice") {
                classInfo.addAttributeDefinition(parseChoiceAttribute(status));
            } else if (typeName == "model") {
                classInfo.setModelDefinition(parseModel(status));
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

        String DefParser::parseBaseAttribute(ParserStatus& status) {
            Token token;
            expect(status, DefToken::OParenthesis, token = nextTokenIgnoringNewlines());
            expect(status, DefToken::QuotedString, token = nextTokenIgnoringNewlines());
            const String basename = token.data();
            expect(status, DefToken::CParenthesis, token = nextTokenIgnoringNewlines());
            
            return basename;
        }

        Assets::AttributeDefinitionPtr DefParser::parseChoiceAttribute(ParserStatus& status) {
            Token token;
            expect(status, DefToken::QuotedString, token = m_tokenizer.nextToken());
            const String attributeName = token.data();
            
            Assets::ChoiceAttributeOption::Array options;
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
            
            return Assets::AttributeDefinitionPtr(new Assets::ChoiceAttributeDefinition(attributeName, "", "", options));
        }

        Assets::ModelDefinition DefParser::parseModel(ParserStatus& status) {
            expect(status, DefToken::OParenthesis, m_tokenizer.nextToken());
            
            const TokenizerState::Snapshot snapshot = m_tokenizer.snapshot();
            const size_t line = m_tokenizer.line();
            const size_t column = m_tokenizer.column();
            
            try {
                ELParser parser(m_tokenizer);
                EL::Expression expression = parser.parse();
                expect(status, DefToken::CParenthesis, m_tokenizer.nextToken());
                
                expression.optimize();
                return Assets::ModelDefinition(expression);
            } catch (const ParserException& e) {
                try {
                    m_tokenizer.restore(snapshot);
                    
                    LegacyModelDefinitionParser parser(m_tokenizer);
                    EL::Expression expression = parser.parse(status);
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
