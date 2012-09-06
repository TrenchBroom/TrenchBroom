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

#include "StandardDefinitionParser.h"

#include <sstream>

namespace TrenchBroom {
    namespace IO {
        StandardDefinitionTokenizer::TokenPtr StandardDefinitionTokenizer::nextToken() {
            m_buffer.str(String());
            while (!eof()) {
                char c = nextChar();
                switch (m_state) {
                    case TokenizerState::Outside:
                        switch (c) {
                            case '/':
                                if (peekChar() == '*') {
                                    m_state = TokenizerState::Inside;
                                    while (c != ' ')
                                        c = nextChar();
                                    return token(TokenType::ODefinition, "");
                                } else if (peekChar() == '/') {
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
                                if (peekChar() == '/') {
                                    nextChar();
                                    m_state = TokenizerState::Outside;
                                    return token(TokenType::CDefinition, "");
                                }
                                break;
                            case '(':
                                return token(TokenType::OParenthesis, "");
                            case ')':
                                return token(TokenType::CParenthesis, "");
                            case '{':
                                return token(TokenType::OBrace, "");
                            case '}':
                                return token(TokenType::CBrace, "");
                            case ';':
                                return token(TokenType::Semicolon, "");
                            case '?':
                                return token(TokenType::Question, "");
							case '\r':
								if (peekChar() == '\n')
									nextChar();
                            case '\n':
                                return token(TokenType::Newline, "");
                            case ',':
                                return token(TokenType::Comma, "");
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
                                if (peekChar() == '*') {
                                    pushChar();
                                } else {
                                    m_buffer << c;
                                    break;
                                }
                            case '(':
                            case ' ':
							case '\r':
								if (peekChar() == '\n')
									nextChar();
                            case '\n':
                            case '\t':
                                m_state = TokenizerState::Inside;
                                pushChar();
                                return token(TokenType::Word, m_buffer.str());
                            default:
                                m_buffer << c;
                                break;
                        }
                        break;
                    case TokenizerState::String:
                        if (c == '"') {
                            m_state = TokenizerState::Inside;
                            return token(TokenType::String, m_buffer.str());
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
                                    pushChar();
                                    m_state = TokenizerState::Inside;
                                    return token(TokenType::Integer, m_buffer.str());
                                } else {
                                    pushChar();
                                    m_state = TokenizerState::Inside;
                                    return token(TokenType::Decimal, m_buffer.str());
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
            
            return TokenPtr(NULL);
        }
        
        StandardDefinitionTokenizer::TokenPtr StandardDefinitionTokenizer::peekToken() {
            unsigned int oldState = m_state;
            size_t oldLine = m_line;
            size_t oldColumn = m_column;
            std::ios::pos_type oldPosition = m_stream.tellg();
            
            StandardDefinitionTokenizer::TokenPtr token = nextToken();
            m_state = oldState;
            m_line = oldLine;
            m_column = oldColumn;
            m_stream.seekg(oldPosition, std::ios::beg);
            
            assert(oldPosition == m_stream.tellg());
            
            return token;
        }

        String StandardDefinitionTokenizer::remainder() {
            assert(m_state == TokenizerState::Inside);
            
            m_buffer.str(String());
            char c = nextChar();
            while (m_state != TokenizerState::Eof && c != '*' && peekChar() != '/') {
                m_buffer << c;
                c = nextChar();
            }
            pushChar();
            return m_buffer.str();
        }
        
        String StandardDefinitionParser::typeNames(unsigned int types) {
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
        
        StandardDefinitionTokenizer::TokenPtr StandardDefinitionParser::nextTokenIgnoringNewlines() {
            StandardDefinitionTokenizer::TokenPtr token = m_tokenizer.nextToken();
            while (token->type() == TokenType::Newline)
                token = m_tokenizer.nextToken();
            return token;
        }
        
        Color StandardDefinitionParser::parseColor() {
            Color color;
            StandardDefinitionTokenizer::TokenPtr token(NULL);
            
            expect(TokenType::OParenthesis, (token = m_tokenizer.nextToken()).get());
            expect(TokenType::Decimal, (token = m_tokenizer.nextToken()).get());
            color.x = token->toFloat();
            expect(TokenType::Decimal, (token = m_tokenizer.nextToken()).get());
            color.y = token->toFloat();
            expect(TokenType::Decimal, (token = m_tokenizer.nextToken()).get());
            color.z = token->toFloat();
            expect(TokenType::CParenthesis, (token = m_tokenizer.nextToken()).get());
            color.w = 1.0f;
            return color;
        }

        BBox StandardDefinitionParser::parseBounds() {
            BBox bounds;
            StandardDefinitionTokenizer::TokenPtr token(NULL);
            
            expect(TokenType::OParenthesis, (token = m_tokenizer.nextToken()).get());
            expect(TokenType::Integer, (token = m_tokenizer.nextToken()).get());
            bounds.min.x = token->toFloat();
            expect(TokenType::Integer, (token = m_tokenizer.nextToken()).get());
            bounds.min.y = token->toFloat();
            expect(TokenType::Integer, (token = m_tokenizer.nextToken()).get());
            bounds.min.z = token->toFloat();
            expect(TokenType::CParenthesis, (token = m_tokenizer.nextToken()).get());
            expect(TokenType::OParenthesis, (token = m_tokenizer.nextToken()).get());
            expect(TokenType::Integer, (token = m_tokenizer.nextToken()).get());
            bounds.max.x = token->toFloat();
            expect(TokenType::Integer, (token = m_tokenizer.nextToken()).get());
            bounds.max.y = token->toFloat();
            expect(TokenType::Integer, (token = m_tokenizer.nextToken()).get());
            bounds.max.z = token->toFloat();
            expect(TokenType::CParenthesis, (token = m_tokenizer.nextToken()).get());
            return bounds;
        }

        Model::SpawnflagList StandardDefinitionParser::parseFlags() {
            Model::SpawnflagList flags;
            StandardDefinitionTokenizer::TokenPtr token = m_tokenizer.peekToken();
            if (token->type() != TokenType::Word)
                return flags;
            
            while (token->type() == TokenType::Word) {
                token = m_tokenizer.nextToken();
                String name = token->data();
                unsigned int value = 1 << flags.size();
                
                flags.push_back(Model::Spawnflag(name, value));
                token = m_tokenizer.peekToken();
            }
            
            return flags;
        }
        
        bool StandardDefinitionParser::parseProperty(StandardProperty::List& properties) {
            StandardDefinitionTokenizer::TokenPtr token;
            expect(TokenType::Word | TokenType::CBrace, (token = nextTokenIgnoringNewlines()).get());
            if (token->type() != TokenType::Word)
                return false;

            String typeName = token->data();
            if (typeName == "choice") {
                expect(TokenType::String, (token = m_tokenizer.nextToken()).get());
                String propertyName = token->data();

                StandardChoiceArgument::List arguments;
                expect(TokenType::OParenthesis, (token = nextTokenIgnoringNewlines()).get());
                token = nextTokenIgnoringNewlines();
                while (token->type() == TokenType::OParenthesis) {
                    expect(TokenType::Integer, (token = nextTokenIgnoringNewlines()).get());
                    int key = token->toInteger();
                    expect(TokenType::Comma, (token = nextTokenIgnoringNewlines()).get());
                    expect(TokenType::String, (token = nextTokenIgnoringNewlines()).get());
                    String value = token->data();
                    arguments.push_back(StandardChoiceArgument(key, value));

                    expect(TokenType::CParenthesis, (token = nextTokenIgnoringNewlines()).get());
                    token = nextTokenIgnoringNewlines();
                }
                
                expect(TokenType::CParenthesis, token.get());
                properties.push_back(new StandardChoiceProperty(propertyName, arguments));
            } else if (typeName == "model") {
                expect(TokenType::OParenthesis, (token = nextTokenIgnoringNewlines()).get());
                expect(TokenType::String, (token = nextTokenIgnoringNewlines()).get());
                String modelPath = token->data();
                unsigned int skinIndex = 0;
                size_t lastColon = modelPath.find_last_of(':');
                if (lastColon > 0 && lastColon != std::string::npos) {
                    skinIndex = atoi(modelPath.c_str() + lastColon + 1);
                    modelPath = modelPath.substr(0, lastColon);
                }

                String flagName = "";
                expect(TokenType::Comma | TokenType::CParenthesis, (token = nextTokenIgnoringNewlines()).get());
                if (token->type() == TokenType::Comma) {
                    expect(TokenType::String, (token = nextTokenIgnoringNewlines()).get());
                    flagName = token->data();
                    expect(TokenType::CParenthesis, (token = nextTokenIgnoringNewlines()).get());
                }
                
                properties.push_back(new StandardModelProperty(modelPath, flagName, skinIndex));
            } else if (typeName == "default") {
                expect(TokenType::OParenthesis, (token = nextTokenIgnoringNewlines()).get());
                expect(TokenType::String, (token = nextTokenIgnoringNewlines()).get());
                String propertyName = token->data();
                expect(TokenType::Comma, (token = nextTokenIgnoringNewlines()).get());
                expect(TokenType::String, (token = nextTokenIgnoringNewlines()).get());
                String propertyValue = token->data();
                expect(TokenType::CParenthesis, (token = nextTokenIgnoringNewlines()).get());
                
                properties.push_back(new StandardDefaultProperty(propertyName, propertyValue));
            } else if (typeName == "base") {
                expect(TokenType::OParenthesis, (token = nextTokenIgnoringNewlines()).get());
                expect(TokenType::String, (token = nextTokenIgnoringNewlines()).get());
                String basename = token->data();
                expect(TokenType::CParenthesis, (token = nextTokenIgnoringNewlines()).get());
                
                properties.push_back(new StandardBaseProperty(StandardProperty::Base, basename));
            }

            expect(TokenType::Semicolon, (token = nextTokenIgnoringNewlines()).get());
            return true;
        }
        
        StandardProperty::List StandardDefinitionParser::parseProperties() {
            StandardProperty::List properties;
            StandardDefinitionTokenizer::TokenPtr token = m_tokenizer.peekToken();
            if (token->type() == TokenType::OBrace) {
                token = m_tokenizer.nextToken();
                while (parseProperty(properties));
            }
            return properties;
        }
        
        String StandardDefinitionParser::parseDescription() {
            StandardDefinitionTokenizer::TokenPtr token = m_tokenizer.peekToken();
            if (token->type() == TokenType::CDefinition)
                return "";
            return m_tokenizer.remainder();
        }

        StandardDefinitionParser::~StandardDefinitionParser() {
            BasePropertiesMap::iterator it, end;
            for (it = m_baseProperties.begin(), end = m_baseProperties.end(); it != end; ++it) {
                StandardProperty::List& properties = it->second;
                while (!properties.empty()) delete properties.back(), properties.pop_back();
            }
            m_baseProperties.clear();
        }

        Model::EntityDefinition* StandardDefinitionParser::nextDefinition() {
            StandardDefinitionTokenizer::TokenPtr token = m_tokenizer.nextToken();
            if (token.get() == NULL)
                return NULL;

            expect(TokenType::ODefinition, token.get());
            String name;
            bool hasColor = false;
            bool hasBounds = false;
            bool hasFlags = false;
            Color color;
            BBox bounds;
            Model::SpawnflagList spawnflags;
            StandardProperty::List standardProperties;
            Model::PropertyDefinition::List propertyDefinitions;
            std::string description;

            token = m_tokenizer.nextToken();
            expect(TokenType::Word, token.get());
            name = token->data();

            token = m_tokenizer.peekToken();
            expect(TokenType::OParenthesis | TokenType::Newline, token.get());
            if (token->type() == TokenType::OParenthesis) {
                hasColor = true;
                color = parseColor();
                
                token = m_tokenizer.peekToken();
                expect(TokenType::OParenthesis | TokenType::Question, token.get());
                if (token->type() == TokenType::OParenthesis) {
                    hasBounds = true;
                    bounds = parseBounds();
                } else {
                    m_tokenizer.nextToken();
                }
                
                token = m_tokenizer.peekToken();
                if (token->type() == TokenType::Word) {
                    hasFlags = true;
                    spawnflags = parseFlags();
                }
            }

            expect(TokenType::Newline, (token = m_tokenizer.nextToken()).get());
            standardProperties = parseProperties();
            description = parseDescription();
            expect(TokenType::CDefinition, (token = m_tokenizer.nextToken()).get());

            Model::EntityDefinition* definition = NULL;
            
            if (hasColor) {
                // TODO: if we handle properties, we must add the base properties here!
                if (hasBounds) { // point definition
                    // extract the model property
                    StandardModelProperty* modelProperty = NULL;
                    for (unsigned int i = 0; i < standardProperties.size(); i++) {
                        StandardProperty* standardProperty = standardProperties[i];
                        if (standardProperty->type() == StandardProperty::Model) {
                            modelProperty = static_cast<StandardModelProperty*>(standardProperty);
                            break;
                        }
                    }
                    if (modelProperty != NULL) {
                        Model::PointEntityModel model = Model::PointEntityModel(modelProperty->modelName(), modelProperty->flagName(), modelProperty->skinIndex());
                        definition =  new Model::PointEntityDefinition(name, color, spawnflags, bounds, description, Model::PropertyDefinition::List(), model);
                    } else {
                        definition = new Model::PointEntityDefinition(name, color, spawnflags, bounds, description, Model::PropertyDefinition::List());
                    }
                } else {
                    definition = new Model::BrushEntityDefinition(name, color, spawnflags, description, Model::PropertyDefinition::List());
                }
            } else { // base definition
                m_baseProperties[name] = standardProperties;
                standardProperties.clear(); // to prevent them from being deleted
                
                definition = nextDefinition();
            }
            
            // clean up
            while (!standardProperties.empty()) delete standardProperties.back(), standardProperties.pop_back();
            
            return definition;
        }
    }
}
