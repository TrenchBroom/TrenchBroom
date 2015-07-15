/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "Model/Entity.h"
#include "Utility/List.h"

#include <sstream>

using namespace TrenchBroom::IO::DefTokenType;

namespace TrenchBroom {
    namespace IO {
        Token DefTokenEmitter::doEmit(Tokenizer& tokenizer) {
            while (!tokenizer.eof()) {
                size_t line = tokenizer.line();
                size_t column = tokenizer.column();
                const char* c = tokenizer.nextChar();
                switch (*c) {
                    case '/': {
                        const char* begin = c;
                        if (tokenizer.peekChar() == '*') {
                            // eat all chars immediately after the '*' because it's often followed by QUAKE
                            while (!isWhitespace(*tokenizer.nextChar()) && !tokenizer.eof());
                            return Token(ODefinition, begin, c, tokenizer.offset(begin), line, column);
                        } else if (tokenizer.peekChar() == '/') {
                            // eat everything up to and including the next newline
                            while (*tokenizer.nextChar() != '\n');
                            break;
                        }
                        // accept standalone / character in the description - used to be an error
                        return Token(Word, c, c + 1, tokenizer.offset(c), line, column);
                    }
                    case '*': {
                        const char* begin = c;
                        if (tokenizer.peekChar() == '/') {
                            tokenizer.nextChar();
                            return Token(CDefinition, begin, c, tokenizer.offset(begin), line, column);
                        }
                        // accept standalone * character in the description - used to be an error
                        return Token(Word, c, c + 1, tokenizer.offset(c), line, column);
                    }
                    case '(':
                        return Token(OParenthesis, c, c + 1, tokenizer.offset(c), line, column);
                    case ')':
                        return Token(CParenthesis, c, c + 1, tokenizer.offset(c), line, column);
                    case '{':
                        return Token(OBrace, c, c + 1, tokenizer.offset(c), line, column);
                    case '}':
                        return Token(CBrace, c, c + 1, tokenizer.offset(c), line, column);
                    case '=':
                        return Token(Equality, c, c + 1, tokenizer.offset(c), line, column);
                    case ';':
                        return Token(Semicolon, c, c + 1, tokenizer.offset(c), line, column);
                    case '?':
                        return Token(Question, c, c + 1, tokenizer.offset(c), line, column);
                    case '\r':
                        if (tokenizer.peekChar() == '\n') {
                            tokenizer.nextChar();
                        }
                    case '\n':
                        return Token(Newline, c, c + 1, tokenizer.offset(c), line, column);
                    case ',':
                        return Token(Comma, c, c + 1, tokenizer.offset(c), line, column);
                    case ' ':
                    case '\t':
                        break;
                    case '"': { // quoted string
                        const char* begin = c;
                        const char* end;
                        tokenizer.quotedString(begin, end);
                        return Token(QuotedString, begin, end, tokenizer.offset(begin), line, column);
                    }
                    default: { // integer, decimal or word
                        const char* begin = c;

                        // try to read a number
                        if (*c == '-' || isDigit(*c)) {
                            while (isDigit(*(c = tokenizer.nextChar())));
                            if (isDelimiter(*c)) {
                                if (!tokenizer.eof())
                                    tokenizer.pushChar();
                                return Token(Integer, begin, c, tokenizer.offset(begin), line, column);
                            }
                        }
                        
                        // try to read a decimal (may start with '.')
                        if (*c == '.') {
                            while (isDigit(*(c = tokenizer.nextChar())));
                            if (isDelimiter(*c)) {
                                if (!tokenizer.eof())
                                    tokenizer.pushChar();
                                return Token(Decimal, begin, c, tokenizer.offset(begin), line, column);
                            }
                        }
                        
                        // read a word
                        while (!tokenizer.eof() && !isDelimiter(*(c = tokenizer.nextChar())));
                        if (!tokenizer.eof())
                            tokenizer.pushChar();
                        return Token(Word, begin, c, tokenizer.offset(begin), line, column);
                    }
                }
            }
            
            return Token(Eof, NULL, NULL, 0, tokenizer.line(), tokenizer.column());
        }

        String DefParser::typeNames(unsigned int types) {
            StringList names;
            if ((types & Integer) != 0)
                names.push_back("integer number");
            if ((types & Decimal) != 0)
                names.push_back("decimal number");
            if ((types & QuotedString) != 0)
                names.push_back("string");
            if ((types & OParenthesis) != 0)
                names.push_back("opening parenthesis");
            if ((types & CParenthesis) != 0)
                names.push_back("closing parenthesis");
            if ((types & OBrace) != 0)
                names.push_back("opening brace");
            if ((types & CBrace) != 0)
                names.push_back("closing brace");
            if ((types & Word) != 0)
                names.push_back("word");
            if ((types & Question) != 0)
                names.push_back("question mark");
            if ((types & ODefinition) != 0)
                names.push_back("definition start ('/*')");
            if ((types & CDefinition) != 0)
                names.push_back("definition end ('*/')");
            if ((types & Semicolon) != 0)
                names.push_back("semicolon");
            if ((types & Newline) != 0)
                names.push_back("newline");
            if ((types & Comma) != 0)
                names.push_back("comma");
            if ((types & Equality) != 0)
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
        
        Token DefParser::nextTokenIgnoringNewlines() {
            Token token = m_tokenizer.nextToken();
            while (token.type() == Newline)
                token = m_tokenizer.nextToken();
            return token;
        }
        
        Color DefParser::parseColor() {
            Color color;
            Token token;
            
            expect(OParenthesis, token = m_tokenizer.nextToken());
            for (size_t i = 0; i < 3; i++) {
                expect(Decimal | Integer, token = m_tokenizer.nextToken());
                color[i] = token.toFloat();
            }
            expect(CParenthesis, token = m_tokenizer.nextToken());
            color[3] = 1.0f;
            return color;
        }

        Vec3f DefParser::parseVector() {
            Vec3f vec;
            Token token;

            for (size_t i = 0; i < 3; i++) {
                expect(Integer | Decimal, token = m_tokenizer.nextToken());
                vec[i] = token.toFloat();
            }
            return vec;
        }
        
        BBoxf DefParser::parseBounds() {
            BBoxf bounds;
            Token token;
            
            expect(OParenthesis, token = m_tokenizer.nextToken());
            bounds.min = parseVector();
            expect(CParenthesis, token = m_tokenizer.nextToken());
            expect(OParenthesis, token = m_tokenizer.nextToken());
            bounds.max = parseVector();
            expect(CParenthesis, token = m_tokenizer.nextToken());
            return bounds;
        }

        Model::FlagsPropertyDefinition::Ptr DefParser::parseFlags() {
            Model::FlagsPropertyDefinition* definition = new Model::FlagsPropertyDefinition(Model::Entity::SpawnFlagsKey, "");
            size_t numOptions = 0;
            
            Token token = m_tokenizer.peekToken();
            while (token.type() == Word) {
                token = m_tokenizer.nextToken();
                String name = token.data();
                int value = 1 << numOptions++;
                definition->addOption(value, name, false);
                token = m_tokenizer.peekToken();
            }
            
            return Model::PropertyDefinition::Ptr(definition);
        }
        
        bool DefParser::parseProperty(Model::PropertyDefinition::Map& properties, Model::ModelDefinition::List& modelDefinitions, StringList& baseClasses) {
            Token token;
            expect(Word | CBrace, token = nextTokenIgnoringNewlines());
            if (token.type() != Word)
                return false;

            String typeName = token.data();
            if (typeName == "choice") {
                expect(QuotedString, token = m_tokenizer.nextToken());
                String propertyName = token.data();

                Model::ChoicePropertyOption::List options;
                expect(OParenthesis, token = nextTokenIgnoringNewlines());
                token = nextTokenIgnoringNewlines();
                while (token.type() == OParenthesis) {
                    expect(Integer, token = nextTokenIgnoringNewlines());
                    String key = token.data();
                    expect(Comma, token = nextTokenIgnoringNewlines());
                    expect(QuotedString, token = nextTokenIgnoringNewlines());
                    String value = token.data();
                    options.push_back(Model::ChoicePropertyOption(key, value));

                    expect(CParenthesis, token = nextTokenIgnoringNewlines());
                    token = nextTokenIgnoringNewlines();
                }
                
                expect(CParenthesis, token);
                properties[propertyName] = Model::PropertyDefinition::Ptr(new Model::ChoicePropertyDefinition(propertyName, "", 0));
            } else if (typeName == "model") {
                Model::ModelDefinition* modelDefinition = NULL;
                
                expect(OParenthesis, token = nextTokenIgnoringNewlines());
                expect(QuotedString , token = nextTokenIgnoringNewlines());
                
                const String modelPath = token.data();

                std::vector<int> indices;
                
                expect(Integer | Word | Comma | CParenthesis, token = nextTokenIgnoringNewlines());
                if (token.type() == Integer) {
                    indices.push_back(token.toInteger());
                    expect(Integer | Word | Comma | CParenthesis, token = nextTokenIgnoringNewlines());
                    if (token.type() == Integer) {
                        indices.push_back(token.toInteger());
                        expect(Word | Comma | CParenthesis, token = nextTokenIgnoringNewlines());
                    }
                }
                
                unsigned int skinIndex = 0;
                unsigned int frameIndex = 0;
                if (!indices.empty()) {
                    skinIndex = static_cast<unsigned int>(indices[0]);
                    if (indices.size() > 1)
                        frameIndex = static_cast<unsigned int>(indices[1]);
                }

                if (token.type() == Word) { // parse property or flag
                    String propertyKey = token.data();
                    
                    expect(Equality, token = nextTokenIgnoringNewlines());
                    expect(QuotedString | Integer, token = nextTokenIgnoringNewlines());
                    if (token.type() == QuotedString) {
                        String propertyValue = token.data();
                        modelDefinition = new Model::ModelDefinition(modelPath,
                                                                     static_cast<unsigned int>(skinIndex),
                                                                     static_cast<unsigned int>(frameIndex),
                                                                     propertyKey, propertyValue);
                    } else {
                        int flagValue = token.toInteger();
                        modelDefinition = new Model::ModelDefinition(modelPath,
                                                                     static_cast<unsigned int>(skinIndex),
                                                                     static_cast<unsigned int>(frameIndex),
                                                                     propertyKey, flagValue);
                    }
                    
                    expect(CParenthesis, token = nextTokenIgnoringNewlines());
                } else {
                    modelDefinition = new Model::ModelDefinition(modelPath,
                                                                 static_cast<unsigned int>(skinIndex),
                                                                 static_cast<unsigned int>(frameIndex));
                }
                
                modelDefinitions.push_back(Model::ModelDefinition::Ptr(modelDefinition));
            } else if (typeName == "default") {
                expect(OParenthesis, token = nextTokenIgnoringNewlines());
                expect(QuotedString, token = nextTokenIgnoringNewlines());
                String propertyName = token.data();
                expect(Comma, token = nextTokenIgnoringNewlines());
                expect(QuotedString, token = nextTokenIgnoringNewlines());
                String propertyValue = token.data();
                expect(CParenthesis, token = nextTokenIgnoringNewlines());

                // ignore these properties
            } else if (typeName == "base") {
                expect(OParenthesis, token = nextTokenIgnoringNewlines());
                expect(QuotedString, token = nextTokenIgnoringNewlines());
                String basename = token.data();
                expect(CParenthesis, token = nextTokenIgnoringNewlines());
               
                baseClasses.push_back(basename);
            }

            expect(Semicolon, token = nextTokenIgnoringNewlines());
            return true;
        }
        
        void DefParser::parseProperties(Model::PropertyDefinition::Map& properties, Model::ModelDefinition::List& modelDefinitions, StringList& baseClasses) {
            Token token = m_tokenizer.peekToken();
            if (token.type() == OBrace) {
                token = m_tokenizer.nextToken();
                while (parseProperty(properties, modelDefinitions, baseClasses));
            }
        }
        
        String DefParser::parseDescription() {
            Token token = m_tokenizer.peekToken();
            if (token.type() == CDefinition)
                return "";
            return m_tokenizer.remainder(CDefinition);
        }

        Model::EntityDefinition* DefParser::nextDefinition() {
            Token token = m_tokenizer.nextToken();
            while (token.type() != Eof && token.type() != ODefinition)
                token = m_tokenizer.nextToken();
            if (token.type() == Eof)
                return NULL;

            expect(ODefinition, token);
            
            StringList baseClasses;
            ClassInfo classInfo;
            
            token = m_tokenizer.nextToken();
            expect(Word, token);
            classInfo.name = token.data();

            token = m_tokenizer.peekToken();
            expect(OParenthesis | Newline, token);
            if (token.type() == OParenthesis) {
                classInfo.setColor(parseColor());
                
                token = m_tokenizer.peekToken();
                expect(OParenthesis | Question, token);
                if (token.type() == OParenthesis) {
                    classInfo.setSize(parseBounds());
                } else {
                    m_tokenizer.nextToken();
                }
                
                token = m_tokenizer.peekToken();
                if (token.type() == Word) {
                    Model::FlagsPropertyDefinition::Ptr spawnflags = parseFlags();
                    classInfo.properties[spawnflags->name()] = spawnflags;
                }
            }

            expect(Newline, token = m_tokenizer.nextToken());
            parseProperties(classInfo.properties, classInfo.models, baseClasses);
            
            classInfo.setDescription(parseDescription());
            expect(CDefinition, token = m_tokenizer.nextToken());

            Model::EntityDefinition* definition = NULL;
            
            if (classInfo.hasColor) {
                ClassInfo::resolveBaseClasses(m_baseClasses, baseClasses, classInfo);
                if (classInfo.hasSize) { // point definition
                    definition = new Model::PointEntityDefinition(classInfo.name, classInfo.color, classInfo.size, classInfo.description, classInfo.propertyList(), classInfo.models);
                } else {
                    definition = new Model::BrushEntityDefinition(classInfo.name, classInfo.color, classInfo.description, classInfo.propertyList());
                }
            } else { // base definition
                m_baseClasses[classInfo.name] = classInfo;
                definition = nextDefinition();
            }
            
            return definition;
        }
    }
}
