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
        StandardDefinitionTokenizer::TokenPtr StandardDefinitionTokenizer::NextToken() {
            StringStream buffer;
            while (!Eof()) {
                char c = NextChar();
                switch (m_state) {
                    case TokenizerState::Outside:
                        switch (c) {
                            case '/':
                                if (PeekChar() == '*') {
                                    m_state = TokenizerState::Inside;
                                    while (c != ' ')
                                        NextChar();
                                    return MakeToken(TokenType::ODefinition, "");
                                } else if (PeekChar() == '/') {
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
                                if (PeekChar() == '/') {
                                    NextChar();
                                    m_state = TokenizerState::Outside;
                                    return MakeToken(TokenType::CDefinition, "");
                                }
                                break;
                            case '(':
                                return MakeToken(TokenType::OParenthesis, "");
                            case ')':
                                return MakeToken(TokenType::CParenthesis, "");
                            case '{':
                                return MakeToken(TokenType::OBrace, "");
                            case '}':
                                return MakeToken(TokenType::CBrace, "");
                            case ';':
                                return MakeToken(TokenType::Semicolon, "");
                            case '?':
                                return MakeToken(TokenType::Question, "");
                            case '\n':
                                return MakeToken(TokenType::Newline, "");
                            case ',':
                                return MakeToken(TokenType::Comma, "");
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
                                buffer.clear();
                                buffer << c;
                                break;
                            case '"':
                                m_state = TokenizerState::String;
                                buffer.clear();
                                break;
                            default:
                                m_state = TokenizerState::Word;
                                buffer.clear();
                                buffer << c;
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
                                if (PeekChar() == '*') {
                                    PushChar();
                                } else {
                                    buffer << c;
                                    break;
                                }
                            case '(':
                            case ' ':
                            case '\n':
                            case '\t':
                                m_state = TokenizerState::Inside;
                                PushChar();
                                return MakeToken(TokenType::Word, buffer.str());
                            default:
                                buffer << c;
                                break;
                        }
                        break;
                    case TokenizerState::String:
                        if (c == '"') {
                            m_state = TokenizerState::Inside;
                            return MakeToken(TokenType::String, buffer.str());
                        } else {
                            buffer << c;
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
                                buffer << c;
                                break;
                            case ')':
                            case '\t':
                            case ',':
                            case ' ': {
                                if (m_state == TokenizerState::Integer) {
                                    PushChar();
                                    m_state = TokenizerState::Inside;
                                    return MakeToken(TokenType::Integer, buffer.str());
                                } else {
                                    PushChar();
                                    m_state = TokenizerState::Inside;
                                    return MakeToken(TokenType::Decimal, buffer.str());
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
        
        StandardDefinitionTokenizer::TokenPtr StandardDefinitionTokenizer::PeekToken() {
            unsigned int oldState = m_state;
            size_t oldLine = m_line;
            size_t oldColumn = m_column;
            size_t oldPosition = m_stream.tellg();
            
            StandardDefinitionTokenizer::TokenPtr token = NextToken();
            m_state = oldState;
            m_line = oldLine;
            m_column = oldColumn;
            m_stream.seekg(oldPosition, std::ios::beg);
            
            return token;
        }

        String StandardDefinitionTokenizer::Remainder() {
            assert(m_state == TokenizerState::Inside);
            
            char c = NextChar();
            StringStream buffer;
            while (m_state != TokenizerState::Eof && c != '*' && PeekChar() != '/') {
                buffer << c;
                c = NextChar();
            }
            PushChar();
            return buffer.str();
        }
        
        String StandardDefinitionParser::TypeNames(unsigned int types) {
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
        
        StandardDefinitionTokenizer::TokenPtr StandardDefinitionParser::NextTokenIgnoringNewlines() {
            StandardDefinitionTokenizer::TokenPtr token = m_tokenizer.NextToken();
            while (token->Type() == TokenType::Newline)
                token = m_tokenizer.NextToken();
            return token;
        }
        
        Vec4f StandardDefinitionParser::ParseColor() {
            Vec4f color;
            StandardDefinitionTokenizer::TokenPtr token(NULL);
            
            Expect(TokenType::OBrace, (token = m_tokenizer.NextToken()).get());
            Expect(TokenType::Decimal, (token = m_tokenizer.NextToken()).get());
            color.x = token->ToFloat();
            Expect(TokenType::Decimal, (token = m_tokenizer.NextToken()).get());
            color.y = token->ToFloat();
            Expect(TokenType::Decimal, (token = m_tokenizer.NextToken()).get());
            color.z = token->ToFloat();
            Expect(TokenType::CBrace, (token = m_tokenizer.NextToken()).get());
            color.w = 1;
            return color;
        }

        BBox StandardDefinitionParser::ParseBounds() {
            BBox bounds;
            StandardDefinitionTokenizer::TokenPtr token(NULL);
            
            Expect(TokenType::OBrace, (token = m_tokenizer.NextToken()).get());
            Expect(TokenType::Integer, (token = m_tokenizer.NextToken()).get());
            bounds.min.x = token->ToFloat();
            Expect(TokenType::Integer, (token = m_tokenizer.NextToken()).get());
            bounds.min.y = token->ToFloat();
            Expect(TokenType::Integer, (token = m_tokenizer.NextToken()).get());
            bounds.min.z = token->ToFloat();
            Expect(TokenType::CBrace, (token = m_tokenizer.NextToken()).get());
            Expect(TokenType::OBrace, (token = m_tokenizer.NextToken()).get());
            Expect(TokenType::Integer, (token = m_tokenizer.NextToken()).get());
            bounds.max.x = token->ToFloat();
            Expect(TokenType::Integer, (token = m_tokenizer.NextToken()).get());
            bounds.max.y = token->ToFloat();
            Expect(TokenType::Integer, (token = m_tokenizer.NextToken()).get());
            bounds.max.z = token->ToFloat();
            Expect(TokenType::CBrace, (token = m_tokenizer.NextToken()).get());
            return bounds;
        }

        Model::Spawnflag::List StandardDefinitionParser::ParseFlags() {
            Model::Spawnflag::List flags;
            StandardDefinitionTokenizer::TokenPtr token = m_tokenizer.PeekToken();
            if (token->Type() != TokenType::Word)
                return flags;
            
            while (token->Type() == TokenType::Word) {
                token = m_tokenizer.NextToken();
                String name = token->Data();
                unsigned int value = 1 << flags.size();
                
                flags.push_back(Model::Spawnflag(name, value));
                token = m_tokenizer.PeekToken();
            }
            
            return flags;
        }
        
        bool StandardDefinitionParser::ParseProperty(StandardProperty::List& properties) {
            StandardDefinitionTokenizer::TokenPtr token;
            Expect(TokenType::Word | TokenType::CBrace, (token = NextTokenIgnoringNewlines()).get());
            if (token->Type() != TokenType::Word)
                return false;

            String typeName = token->Data();
            if (typeName == "choice") {
                Expect(TokenType::String, (token = m_tokenizer.NextToken()).get());
                String propertyName = token->Data();

                StandardChoiceArgument::List arguments;
                Expect(TokenType::OParenthesis, (token = NextTokenIgnoringNewlines()).get());
                token = NextTokenIgnoringNewlines();
                while (token->Type() == TokenType::OParenthesis) {
                    Expect(TokenType::Integer, (token = NextTokenIgnoringNewlines()).get());
                    int key = token->ToInteger();
                    Expect(TokenType::Comma, (token = NextTokenIgnoringNewlines()).get());
                    Expect(TokenType::String, (token = NextTokenIgnoringNewlines()).get());
                    String value = token->Data();
                    arguments.push_back(StandardChoiceArgument(key, value));

                    Expect(TokenType::CParenthesis, (token = NextTokenIgnoringNewlines()).get());
                    token = NextTokenIgnoringNewlines();
                }
                
                Expect(TokenType::CParenthesis, token.get());
                properties.push_back(StandardChoiceProperty(StandardProperty::PropertyType::Choice, propertyName, arguments));
            } else if (typeName == "model") {
                Expect(TokenType::OParenthesis, (token = NextTokenIgnoringNewlines()).get());
                Expect(TokenType::String, (token = NextTokenIgnoringNewlines()).get());
                String modelPath = token->Data();
                unsigned int skinIndex = 0;
                size_t lastColon = modelPath.find_last_of(':');
                if (lastColon > 0 && lastColon != std::string::npos) {
                    skinIndex = atoi(modelPath.c_str() + lastColon + 1);
                    modelPath = modelPath.substr(0, lastColon);
                }

                String flagName = "";
                Expect(TokenType::Comma | TokenType::CParenthesis, (token = NextTokenIgnoringNewlines()).get());
                if (token->Type() == TokenType::Comma) {
                    Expect(TokenType::String, (token = NextTokenIgnoringNewlines()).get());
                    flagName = token->Data();
                    Expect(TokenType::Comma, (token = NextTokenIgnoringNewlines()).get());
                }
                
                properties.push_back(StandardModelProperty(StandardProperty::PropertyType::Model, modelPath, flagName, skinIndex));
            } else if (typeName == "default") {
                Expect(TokenType::OParenthesis, (token = NextTokenIgnoringNewlines()).get());
                Expect(TokenType::String, (token = NextTokenIgnoringNewlines()).get());
                String propertyName = token->Data();
                Expect(TokenType::Comma, (token = NextTokenIgnoringNewlines()).get());
                Expect(TokenType::String, (token = NextTokenIgnoringNewlines()).get());
                String propertyValue = token->Data();
                Expect(TokenType::CParenthesis, (token = NextTokenIgnoringNewlines()).get());
                
                properties.push_back(StandardDefaultProperty(StandardProperty::PropertyType::Default, propertyName, propertyValue));
            } else if (typeName == "base") {
                Expect(TokenType::OParenthesis, (token = NextTokenIgnoringNewlines()).get());
                Expect(TokenType::String, (token = NextTokenIgnoringNewlines()).get());
                String basename = token->Data();
                Expect(TokenType::CParenthesis, (token = NextTokenIgnoringNewlines()).get());
                
                properties.push_back(StandardBaseProperty(StandardProperty::PropertyType::Base, basename));
            }

            Expect(TokenType::Semicolon, (token = NextTokenIgnoringNewlines()).get());
            return true;
        }
        
        StandardProperty::List StandardDefinitionParser::ParseProperties() {
            StandardProperty::List properties;
            StandardDefinitionTokenizer::TokenPtr token = m_tokenizer.PeekToken();
            if (token->Type() == TokenType::OBrace) {
                token = m_tokenizer.NextToken();
                while (ParseProperty(properties));
            }
            return properties;
        }
        
        String StandardDefinitionParser::ParseDescription() {
            StandardDefinitionTokenizer::TokenPtr token = m_tokenizer.PeekToken();
            if (token->Type() == TokenType::CDefinition)
                return "";
            return m_tokenizer.Remainder();
        }

        Model::EntityDefinition* StandardDefinitionParser::NextDefinition() {
            StandardDefinitionTokenizer::TokenPtr token = m_tokenizer.NextToken();
            if (token.get() == NULL)
                return NULL;

            Expect(TokenType::ODefinition, token.get());
            String name;
            bool hasColor = false;
            bool hasBounds = false;
            bool hasFlags = false;
            Vec4f color;
            BBox bounds;
            Model::Spawnflag::List spawnflags;
            StandardProperty::List standardProperties;
            Model::PropertyDefinition::List propertyDefinitions;
            std::string description;

            token = m_tokenizer.NextToken();
            Expect(TokenType::Word, token.get());
            name = token->Data();

            token = m_tokenizer.PeekToken();
            Expect(TokenType::OBrace | TokenType::Newline, token.get());
            if (token->Type() == TokenType::OBrace) {
                hasColor = true;
                color = ParseColor();
                
                token = m_tokenizer.PeekToken();
                Expect(TokenType::OBrace | TokenType::Question, token.get());
                if (token->Type() == TokenType::OBrace) {
                    hasBounds = true;
                    bounds = ParseBounds();
                } else {
                    m_tokenizer.NextToken();
                }
                
                token = m_tokenizer.PeekToken();
                if (token->Type() == TokenType::Word) {
                    hasFlags = true;
                    spawnflags = ParseFlags();
                }
            }

            Expect(TokenType::Newline, (token = m_tokenizer.NextToken()).get());
            standardProperties = ParseProperties();
            description = ParseDescription();
            Expect(TokenType::CDefinition, (token = m_tokenizer.NextToken()).get());

            if (hasColor) {
                if (hasBounds) { // point definition
                    // extract the model property
                    for (unsigned int i = 0; i < standardProperties.size(); i++) {
                        StandardProperty& standardProperty = standardProperties[i];
                        if (standardProperty.Type() == StandardProperty::PropertyType::Model) {
                            StandardModelProperty& modelProperty = static_cast<StandardModelProperty&>(standardProperty);
                            return new Model::PointEntityDefinition(name, color, spawnflags, bounds, description, Model::PropertyDefinition::List(), Model::PointEntityModel(modelProperty.ModelName(), modelProperty.FlagName(), modelProperty.SkinIndex()));
                        }
                    }
                    return new Model::PointEntityDefinition(name, color, spawnflags, bounds, description, Model::PropertyDefinition::List());
                } else {
                    return new Model::BrushEntityDefinition(name, color, spawnflags, description, Model::PropertyDefinition::List());
                }
            }
            
            return NULL;
        }
    }
}
