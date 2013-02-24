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

namespace TrenchBroom {
    namespace IO {
        Token DefTokenEmitter::doEmit(Tokenizer& tokenizer) {
            const size_t position = tokenizer.position();
            const size_t line = tokenizer.line();
            const size_t column = tokenizer.column();
            
            m_buffer.str(String());
            while (!tokenizer.eof()) {
                char c = tokenizer.nextChar();
                switch (m_state) {
                    case TokenizerState::Outside:
                        switch (c) {
                            case '/':
                                if (tokenizer.peekChar() == '*') {
                                    m_state = TokenizerState::Inside;
                                    while (c != ' ')
                                        c = tokenizer.nextChar();
                                    return Token(TokenType::ODefinition, "", position, tokenizer.position() - position, line, column);
                                } else if (tokenizer.peekChar() == '/') {
                                    m_state = TokenizerState::Comment;
                                }
                                break;
                            default:
                                break;
                        }
                        break;
                    case TokenizerState::Inside:
                        switch (c) {
                            case '*':
                                if (tokenizer.peekChar() == '/') {
                                    tokenizer.nextChar();
                                    m_state = TokenizerState::Outside;
                                    return Token(TokenType::CDefinition, "", position, tokenizer.position() - position, line, column);
                                }
                                break;
                            case '(':
                                return Token(TokenType::OParenthesis, "", position, tokenizer.position() - position, line, column);
                            case ')':
                                return Token(TokenType::CParenthesis, "", position, tokenizer.position() - position, line, column);
                            case '{':
                                return Token(TokenType::OBrace, "", position, tokenizer.position() - position, line, column);
                            case '}':
                                return Token(TokenType::CBrace, "", position, tokenizer.position() - position, line, column);
                            case '=':
                                return Token(TokenType::Equality, "", position, tokenizer.position() - position, line, column);
                            case ';':
                                return Token(TokenType::Semicolon, "", position, tokenizer.position() - position, line, column);
                            case '?':
                                return Token(TokenType::Question, "", position, tokenizer.position() - position, line, column);
							case '\r':
								if (tokenizer.peekChar() == '\n')
									tokenizer.nextChar();
                            case '\n':
                                return Token(TokenType::Newline, "", position, tokenizer.position() - position, line, column);
                            case ',':
                                return Token(TokenType::Comma, "", position, tokenizer.position() - position, line, column);
                            case ' ':
                            case '\t':
                                break;
                            case '-':
                            case '0':
                            case '1':
                            case '2':
                            case '3':
                            case '4':
                            case '5':
                            case '6':
                            case '7':
                            case '8':
                            case '9':
                                m_state = TokenizerState::Integer;
                                m_buffer.str(String());
                                m_buffer << c;
                                break;
                            case '.':
                                m_state = TokenizerState::Decimal;
                                m_buffer.str(String());
                                m_buffer << '0' << c;
                                break;
                            case '"':
                                m_state = TokenizerState::String;
                                m_buffer.str(String());
                                break;
                            default:
                                m_state = TokenizerState::Word;
                                m_buffer.str(String());
                                m_buffer << c;
                                break;
                        }
                        break;
                    case TokenizerState::Comment:
                        if (c == '\n')
                            m_state = TokenizerState::Outside;
                        break;
                    case TokenizerState::Word:
                        switch (c) {
                            case '/':
                                if (tokenizer.peekChar() == '*') {
                                    tokenizer.pushChar();
                                } else {
                                    m_buffer << c;
                                    break;
                                }
                            case '(':
                            case ' ':
							case '\r':
								if (tokenizer.peekChar() == '\n')
									tokenizer.nextChar();
                            case '\n':
                            case '\t':
                                m_state = TokenizerState::Inside;
                                tokenizer.pushChar();
                                return Token(TokenType::Word, m_buffer.str(), position, tokenizer.position() - position, line, column);
                            default:
                                m_buffer << c;
                                break;
                        }
                        break;
                    case TokenizerState::String:
                        if (c == '"') {
                            m_state = TokenizerState::Inside;
                            return Token(TokenType::String, m_buffer.str(), position, tokenizer.position() - position, line, column);
                        } else {
                            m_buffer << c;
                        }
                        break;
                    case TokenizerState::Integer:
                        if (c == '.')
                            m_state = TokenizerState::Decimal;
                    case TokenizerState::Decimal: {
                        switch (c) {
                            case '0':
                            case '1':
                            case '2':
                            case '3':
                            case '4':
                            case '5':
                            case '6':
                            case '7':
                            case '8':
                            case '9':
                            case '.':
                                m_buffer << c;
                                break;
                            case ')':
                            case '\t':
                            case ',':
                            case ' ': {
                                if (m_state == TokenizerState::Integer) {
                                    tokenizer.pushChar();
                                    m_state = TokenizerState::Inside;
                                    return Token(TokenType::Integer, m_buffer.str(), position, tokenizer.position() - position, line, column);
                                } else {
                                    tokenizer.pushChar();
                                    m_state = TokenizerState::Inside;
                                    return Token(TokenType::Decimal, m_buffer.str(), position, tokenizer.position() - position, line, column);
                                }
                                break;
                            }
                            default:
                                m_state = TokenizerState::Word;
                                break;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
            
            return Token(TokenType::Eof, "", position, tokenizer.position() - position, line, column);
        }

        DefTokenEmitter::DefTokenEmitter() :
        m_state(TokenizerState::Outside) {}
        
        String DefParser::typeNames(unsigned int types) {
            std::vector<std::string> names;
            if ((types & TokenType::Integer) != 0)
                names.push_back("integer number");
            if ((types & TokenType::Decimal) != 0)
                names.push_back("decimal number");
            if ((types & TokenType::String) != 0)
                names.push_back("string");
            if ((types & TokenType::OParenthesis) != 0)
                names.push_back("opening parenthesis");
            if ((types & TokenType::CParenthesis) != 0)
                names.push_back("closing parenthesis");
            if ((types & TokenType::OBrace) != 0)
                names.push_back("opening brace");
            if ((types & TokenType::CBrace) != 0)
                names.push_back("closing brace");
            if ((types & TokenType::Word) != 0)
                names.push_back("word");
            if ((types & TokenType::Question) != 0)
                names.push_back("question mark");
            if ((types & TokenType::ODefinition) != 0)
                names.push_back("definition start ('/*')");
            if ((types & TokenType::CDefinition) != 0)
                names.push_back("definition end ('*/')");
            if ((types & TokenType::Semicolon) != 0)
                names.push_back("semicolon");
            if ((types & TokenType::Newline) != 0)
                names.push_back("newline");
            if ((types & TokenType::Comma) != 0)
                names.push_back("comma");
            if ((types & TokenType::Equality) != 0)
                names.push_back("equality sign");
            
            if (names.empty())
                return "unknown token type";
            if (names.size() == 1)
                return names[0];
            
            std::stringstream str;
            str << names[0];
            for (unsigned int i = 1; i < names.size() - 1; i++)
                str << ", " << names[i];
            str << ", or " << names[names.size() - 1];
            return str.str();
        }
        
        Token DefParser::nextTokenIgnoringNewlines() {
            Token token = m_tokenizer.nextToken();
            while (token.type() == TokenType::Newline)
                token = m_tokenizer.nextToken();
            return token;
        }
        
        Color DefParser::parseColor() {
            Color color;
            Token token;
            
            expect(TokenType::OParenthesis, token = m_tokenizer.nextToken());
            expect(TokenType::Decimal | TokenType::Integer, token = m_tokenizer.nextToken());
            color.x = token.toFloat();
            expect(TokenType::Decimal | TokenType::Integer, token = m_tokenizer.nextToken());
            color.y = token.toFloat();
            expect(TokenType::Decimal | TokenType::Integer, token = m_tokenizer.nextToken());
            color.z = token.toFloat();
            expect(TokenType::CParenthesis, token = m_tokenizer.nextToken());
            color.w = 1.0f;
            return color;
        }

        BBox DefParser::parseBounds() {
            BBox bounds;
            Token token;
            
            expect(TokenType::OParenthesis, token = m_tokenizer.nextToken());
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            bounds.min.x = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            bounds.min.y = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            bounds.min.z = token.toFloat();
            expect(TokenType::CParenthesis, token = m_tokenizer.nextToken());
            expect(TokenType::OParenthesis, token = m_tokenizer.nextToken());
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            bounds.max.x = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            bounds.max.y = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            bounds.max.z = token.toFloat();
            expect(TokenType::CParenthesis, token = m_tokenizer.nextToken());
            return bounds;
        }

        Model::FlagsPropertyDefinition* DefParser::parseFlags() {
            Model::FlagsPropertyDefinition* definition = new Model::FlagsPropertyDefinition(Model::Entity::SpawnFlagsKey, "");
            size_t numOptions = 0;
            
            Token token = m_tokenizer.peekToken();
            while (token.type() == TokenType::Word) {
                token = m_tokenizer.nextToken();
                String name = token.data();
                int value = 1 << numOptions++;
                definition->addOption(value, name, false);
                token = m_tokenizer.peekToken();
            }
            
            return definition;
        }
        
        bool DefParser::parseProperty(Model::PropertyDefinition::List& properties, Model::ModelDefinition::List& modelDefinitions, StringList& baseClasses) {
            Token token;
            expect(TokenType::Word | TokenType::CBrace, token = nextTokenIgnoringNewlines());
            if (token.type() != TokenType::Word)
                return false;

            String typeName = token.data();
            if (typeName == "choice") {
                expect(TokenType::String, token = m_tokenizer.nextToken());
                String propertyName = token.data();

                Model::ChoicePropertyOption::List options;
                expect(TokenType::OParenthesis, token = nextTokenIgnoringNewlines());
                token = nextTokenIgnoringNewlines();
                while (token.type() == TokenType::OParenthesis) {
                    expect(TokenType::Integer, token = nextTokenIgnoringNewlines());
                    int key = token.toInteger();
                    expect(TokenType::Comma, token = nextTokenIgnoringNewlines());
                    expect(TokenType::String, token = nextTokenIgnoringNewlines());
                    String value = token.data();
                    options.push_back(Model::ChoicePropertyOption(key, value));

                    expect(TokenType::CParenthesis, token = nextTokenIgnoringNewlines());
                    token = nextTokenIgnoringNewlines();
                }
                
                expect(TokenType::CParenthesis, token);
                properties.push_back(new Model::ChoicePropertyDefinition(propertyName, "", 0));
            } else if (typeName == "model") {
                Model::ModelDefinition* modelDefinition = NULL;
                
                expect(TokenType::OParenthesis, token = nextTokenIgnoringNewlines());
                expect(TokenType::String , token = nextTokenIgnoringNewlines());
                
                const String modelPath = token.data();

                expect(TokenType::Integer, token = nextTokenIgnoringNewlines());
                int skinIndex = token.toInteger();
                assert(skinIndex >= 0);
                
                expect(TokenType::Integer, token = nextTokenIgnoringNewlines());
                int frameIndex = token.toInteger();
                assert(frameIndex >= 0);
                
                expect(TokenType::Word | TokenType::CParenthesis, token = nextTokenIgnoringNewlines());
                if (token.type() == TokenType::Word) { // parse property or flag
                    String propertyKey = token.data();
                    
                    expect(TokenType::Equality, token = nextTokenIgnoringNewlines());
                    expect(TokenType::String | TokenType::Integer, token = nextTokenIgnoringNewlines());
                    if (token.type() == TokenType::String) {
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
                    
                    expect(TokenType::CParenthesis, token = nextTokenIgnoringNewlines());
                } else {
                    modelDefinition = new Model::ModelDefinition(modelPath,
                                                                 static_cast<unsigned int>(skinIndex),
                                                                 static_cast<unsigned int>(frameIndex));
                }
                
                modelDefinitions.push_back(modelDefinition);
            } else if (typeName == "default") {
                expect(TokenType::OParenthesis, token = nextTokenIgnoringNewlines());
                expect(TokenType::String, token = nextTokenIgnoringNewlines());
                String propertyName = token.data();
                expect(TokenType::Comma, token = nextTokenIgnoringNewlines());
                expect(TokenType::String, token = nextTokenIgnoringNewlines());
                String propertyValue = token.data();
                expect(TokenType::CParenthesis, token = nextTokenIgnoringNewlines());

                // ignore these propertiesu
            } else if (typeName == "base") {
                expect(TokenType::OParenthesis, token = nextTokenIgnoringNewlines());
                expect(TokenType::String, token = nextTokenIgnoringNewlines());
                String basename = token.data();
                expect(TokenType::CParenthesis, token = nextTokenIgnoringNewlines());
               
                baseClasses.push_back(basename);
            }

            expect(TokenType::Semicolon, token = nextTokenIgnoringNewlines());
            return true;
        }
        
        void DefParser::parseProperties(Model::PropertyDefinition::List& properties, Model::ModelDefinition::List& modelDefinitions, StringList& baseClasses) {
            Token token = m_tokenizer.peekToken();
            if (token.type() == TokenType::OBrace) {
                token = m_tokenizer.nextToken();
                while (parseProperty(properties, modelDefinitions, baseClasses));
            }
        }
        
        String DefParser::parseDescription() {
            Token token = m_tokenizer.peekToken();
            if (token.type() == TokenType::CDefinition)
                return "";
            return m_tokenizer.remainder(TokenType::CDefinition);
        }

        DefParser::~DefParser() {
            BasePropertiesMap::iterator it, end;
            for (it = m_baseProperties.begin(), end = m_baseProperties.end(); it != end; ++it)
                Utility::deleteAll(it->second);
            m_baseProperties.clear();
        }

        Model::EntityDefinition* DefParser::nextDefinition() {
            Token token = m_tokenizer.nextToken();
            if (token.type() == TokenType::Eof)
                return NULL;

            expect(TokenType::ODefinition, token);
            String name;
            bool hasColor = false;
            bool hasBounds = false;
            Color color;
            BBox bounds;
            Model::FlagsPropertyDefinition* spawnflags = NULL;
            Model::PropertyDefinition::List propertyDefinitions;
            Model::ModelDefinition::List modelDefinitions;
            StringList baseClasses;
            String description;

            token = m_tokenizer.nextToken();
            expect(TokenType::Word, token);
            name = token.data();

            token = m_tokenizer.peekToken();
            expect(TokenType::OParenthesis | TokenType::Newline, token);
            if (token.type() == TokenType::OParenthesis) {
                hasColor = true;
                color = parseColor();
                
                token = m_tokenizer.peekToken();
                expect(TokenType::OParenthesis | TokenType::Question, token);
                if (token.type() == TokenType::OParenthesis) {
                    hasBounds = true;
                    bounds = parseBounds();
                } else {
                    m_tokenizer.nextToken();
                }
                
                token = m_tokenizer.peekToken();
                if (token.type() == TokenType::Word)
                    spawnflags = parseFlags();
            }

            expect(TokenType::Newline, token = m_tokenizer.nextToken());
            parseProperties(propertyDefinitions, modelDefinitions, baseClasses);
            description = parseDescription();
            expect(TokenType::CDefinition, token = m_tokenizer.nextToken());

            Model::EntityDefinition* definition = NULL;
            
            if (hasColor) {
                // TODO: if we handle properties, we must add the base properties here!
                if (hasBounds) { // point definition
                    definition = new Model::PointEntityDefinition(name, color, spawnflags, bounds, description, propertyDefinitions, modelDefinitions);
                } else {
                    definition = new Model::BrushEntityDefinition(name, color, spawnflags, description, Model::PropertyDefinition::List());
                }
            } else { // base definition
                m_baseProperties[name] = propertyDefinitions;
                definition = nextDefinition();
            }
            
            return definition;
        }
    }
}
