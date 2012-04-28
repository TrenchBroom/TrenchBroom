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

#include "EntityDefinitionParser.h"
#include <cassert>
#include <cstdlib>

#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace IO {
        bool EntityDefinitionTokenizer::nextChar() {
            if (m_state == TB_TS_EOF)
                return false;

            if (m_stream.eof()) {
                m_state = TB_TS_EOF;
                return false;
            }

            m_stream.get(m_char);
            if (m_char == '\n') {
                m_line++;
                m_column = 0;
            } else {
                m_column++;
            }

            return true;
        }

        void EntityDefinitionTokenizer::pushChar() {
            if (m_state == TB_TS_EOF)
                m_state = TB_TS_OUTDEF;

            m_stream.seekg(-1, ios::cur);
            m_stream.get(m_char);
            if (m_char == '\n') {
                m_line--;
                m_column = 0;
                char c;
                streamoff pos = m_stream.tellg();
                for (int i = 0; i < pos; i++) {
                    m_stream.seekg(-i, ios::cur);
                    m_stream.get(c);
                    if (c == '\n')
                        break;
                    m_column++;
                }
                m_stream.seekg(pos, ios::beg);
            } else {
                m_column--;
            }
            m_stream.seekg(-1, ios::cur);
        }

        char EntityDefinitionTokenizer::peekChar() {
            char c;
            m_stream.get(c);
            m_stream.seekg(-1, ios::cur);
            return c;
        }

        EntityDefinitionToken* EntityDefinitionTokenizer::token(ETokenType type, string* data) {
            m_token.type = type;
            if (data == NULL)
                m_token.data.clear();
            else
                m_token.data = *data;
            m_token.line = m_line;
            m_token.column = m_column;
            m_token.charsRead = static_cast<int>(m_stream.tellg());
            return &m_token;
        }

        EntityDefinitionTokenizer::EntityDefinitionTokenizer(istream& stream) : m_stream(stream), m_state(TB_TS_OUTDEF), m_line(1), m_column(0) {}

        EntityDefinitionToken* EntityDefinitionTokenizer::next() {
            string buffer;
            while (nextChar()) {
                switch (m_state) {
                    case TB_TS_OUTDEF:
                        switch (m_char) {
                            case '/':
                                if (peekChar() == '*') {
                                    m_state = TB_TS_INDEF;
                                    while (m_char != ' ')
                                        nextChar();
                                    return token(TB_TT_ED_O, NULL);
                                } else if (peekChar() == '/') {
                                    m_state = TB_TS_COM;
                                }
                                break;
                            default:
                                break;
                        }
                        break;
                    case TB_TS_INDEF:
                        switch (m_char) {
                            case '*':
                                if (peekChar() == '/') {
                                    nextChar();
                                    m_state = TB_TS_OUTDEF;
                                    return token(TB_TT_ED_C, NULL);
                                }
                                break;
                            case '(':
                                return token(TB_TT_B_O, NULL);
                            case ')':
                                return token(TB_TT_B_C, NULL);
                            case '{':
                                return token(TB_TT_CB_O, NULL);
                            case '}':
                                return token(TB_TT_CB_C, NULL);
                            case ';':
                                return token(TB_TT_SC, NULL);
                            case '?':
                                return token(TB_TT_QM, NULL);
                            case '\n':
                                return token(TB_TT_NL, NULL);
                            case ',':
                                return token(TB_TT_C, NULL);
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
                                m_state = TB_TS_DEC;
                                buffer.clear();
                                buffer += m_char;
                                break;
                            case '"':
                                m_state = TB_TS_Q_STR;
                                buffer.clear();
                                break;
                            default:
                                m_state = TB_TS_WORD;
                                buffer.clear();
                                buffer += m_char;
                                break;
                        }
                        break;
                    case TB_TS_COM:
                        if (m_char == '\n')
                            m_state = TB_TS_OUTDEF;
                        break;
                    case TB_TS_WORD:
                        switch (m_char) {
                            case '/':
                                if (peekChar() == '*') {
                                    pushChar();
                                } else {
                                    buffer += m_char;
                                    break;
                                }
                            case '(':
                            case ' ':
                            case '\n':
                            case '\t':
                                m_state = TB_TS_INDEF;
                                pushChar();
                                return token(TB_TT_WORD, &buffer);
                            default:
                                buffer += m_char;
                                break;
                        }
                        break;
                    case TB_TS_Q_STR:
                        if (m_char == '"') {
                            m_state = TB_TS_INDEF;
                            return token(TB_TT_STR, &buffer);
                        } else {
                            buffer += m_char;
                        }
                        break;
                    case TB_TS_DEC:
                        if (m_char == '.')
                            m_state = TB_TS_FRAC;
                    case TB_TS_FRAC: {
                        switch (m_char) {
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
                                buffer += m_char;
                                break;
                            case ')':
                            case '\t':
                            case ',':
                            case ' ': {
                                if (m_state == TB_TS_DEC) {
                                    pushChar();
                                    m_state = TB_TS_INDEF;
                                    return token(TB_TT_DEC, &buffer);
                                } else {
                                    pushChar();
                                    m_state = TB_TS_INDEF;
                                    return token(TB_TT_FRAC, &buffer);
                                }
                                break;
                            }
                            default:
                                m_state = TB_TS_WORD;
                                break;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            return NULL;
        }

        EntityDefinitionToken* EntityDefinitionTokenizer::peek() {
            int oldLine = m_line;
            int oldColumn = m_column;
            streamoff oldPos = m_stream.tellg();
            char oldChar = m_char;
            ETokenizerState oldState = m_state;

            next();

            m_line = oldLine;
            m_column = oldColumn;
            m_stream.seekg(oldPos, ios::beg);
            m_char = oldChar;
            m_state = oldState;

            return &m_token;
        }

        string EntityDefinitionTokenizer::remainder() {
            assert(m_state == TB_TS_INDEF);

            nextChar();
            string buffer;
            while (m_state != TB_TS_EOF && m_char != '*' && peekChar() != '/') {
                buffer += m_char;
                nextChar();
            }
            pushChar();
            return buffer;
        }

        void EntityDefinitionParser::expect(int expectedType, const EntityDefinitionToken* actualToken) const {
            assert(actualToken != NULL);
            assert((actualToken->type & expectedType) != 0);
        }

        EntityDefinitionToken* EntityDefinitionParser::nextTokenIgnoringNewlines() {
            EntityDefinitionToken* token = m_tokenizer->next();
            while (token->type == TB_TT_NL)
                token = m_tokenizer->next();
            return token;
        }

        Vec4f EntityDefinitionParser::parseColor() {
            Vec4f color;
            EntityDefinitionToken* token = NULL;

            expect(TB_TT_B_O, token = m_tokenizer->next());
            expect(TB_TT_FRAC, token = m_tokenizer->next());
            color.x = atof(token->data.c_str());
            expect(TB_TT_FRAC, token = m_tokenizer->next());
            color.y = atof(token->data.c_str());
            expect(TB_TT_FRAC, token = m_tokenizer->next());
            color.z = atof(token->data.c_str());
            expect(TB_TT_B_C, token = m_tokenizer->next());
            color.w = 1;
            return color;
        }

        BBox EntityDefinitionParser::parseBounds() {
            BBox bounds;
            EntityDefinitionToken* token = NULL;

            expect(TB_TT_B_O, token = m_tokenizer->next());
            expect(TB_TT_DEC, token = m_tokenizer->next());
            bounds.min.x = atof(token->data.c_str());
            expect(TB_TT_DEC, token = m_tokenizer->next());
            bounds.min.y = atof(token->data.c_str());
            expect(TB_TT_DEC, token = m_tokenizer->next());
            bounds.min.z = atof(token->data.c_str());
            expect(TB_TT_B_C, token = m_tokenizer->next());
            expect(TB_TT_B_O, token = m_tokenizer->next());
            expect(TB_TT_DEC, token = m_tokenizer->next());
            bounds.max.x = atof(token->data.c_str());
            expect(TB_TT_DEC, token = m_tokenizer->next());
            bounds.max.y = atof(token->data.c_str());
            expect(TB_TT_DEC, token = m_tokenizer->next());
            bounds.max.z = atof(token->data.c_str());
            expect(TB_TT_B_C, token = m_tokenizer->next());
            return bounds;
        }

        map<string, Model::SpawnFlag> EntityDefinitionParser::parseFlags() {
            map<string, Model::SpawnFlag> flags;
            EntityDefinitionToken* token = m_tokenizer->peek();
            if (token->type != TB_TT_WORD)
                return flags;

            while (token->type == TB_TT_WORD) {
                token = m_tokenizer->next();
                string name = token->data;
                int value = 1 << flags.size();
                Model::SpawnFlag flag(name, value);
                flags[name] = flag;
                token = m_tokenizer->peek();
            }

            return flags;
        }

        vector<Model::Property*> EntityDefinitionParser::parseProperties() {
            vector<Model::Property*> properties;
            EntityDefinitionToken* token = m_tokenizer->peek();
            if (token->type == TB_TT_CB_O) {
                token = m_tokenizer->next();
                Model::Property* property;
                while ((property = parseProperty()) != NULL)
                    properties.push_back(property);
                expect(TB_TT_CB_C, token);
            }
            return properties;
        }

        Model::Property* EntityDefinitionParser::parseProperty() {
            EntityDefinitionToken* token = nextTokenIgnoringNewlines();
            if (token->type != TB_TT_WORD)
                return NULL;

            Model::Property* property = NULL;
            string type = token->data;
            if (type == "choice") {
                expect(TB_TT_STR, token = m_tokenizer->next());
                string name = token->data;

                vector<Model::ChoiceArgument> arguments;
                expect(TB_TT_B_O, token = nextTokenIgnoringNewlines());
                token = nextTokenIgnoringNewlines();
                while (token->type == TB_TT_B_O) {
                    expect(TB_TT_DEC, token = nextTokenIgnoringNewlines());
                    int key = atoi(token->data.c_str());
                    expect(TB_TT_C, token = nextTokenIgnoringNewlines());
                    expect(TB_TT_STR, token = nextTokenIgnoringNewlines());
                    string value = token->data;

                    Model::ChoiceArgument argument(key, value);
                    arguments.push_back(argument);

                    expect(TB_TT_B_C, token = nextTokenIgnoringNewlines());
                    token = nextTokenIgnoringNewlines();
                }
                expect(TB_TT_B_C, token);
                property = new Model::ChoiceProperty(name, arguments);
            } else if (type == "model") {
                expect(TB_TT_B_O, token = nextTokenIgnoringNewlines());
                expect(TB_TT_STR, nextTokenIgnoringNewlines());
                string modelPath = token->data;
                int skinIndex = 0;
                unsigned long lastColon = modelPath.find_last_of(':');
                if (lastColon > 0 && lastColon != string::npos) {
                    skinIndex = atoi(modelPath.c_str() + lastColon + 1);
                    modelPath = modelPath.substr(0, lastColon);
                }

                expect(TB_TT_C | TB_TT_B_C, token = nextTokenIgnoringNewlines());
                if (token->type == TB_TT_C) {
                    expect(TB_TT_STR, token = nextTokenIgnoringNewlines());
                    string flagName = token->data;
                    property = new Model::ModelProperty(flagName, modelPath, skinIndex);
                    expect(TB_TT_B_C, token = nextTokenIgnoringNewlines());
                } else {
                    property = new Model::ModelProperty(modelPath, skinIndex);
                }
            } else if (type == "default") {
                expect(TB_TT_B_O, token = nextTokenIgnoringNewlines());
                expect(TB_TT_STR, token = nextTokenIgnoringNewlines());
                string name = token->data;
                expect(TB_TT_C, token = nextTokenIgnoringNewlines());
                expect(TB_TT_STR, token = nextTokenIgnoringNewlines());
                string value = token->data;
                property = new Model::DefaultProperty(name, value);
                expect(TB_TT_B_C, token = nextTokenIgnoringNewlines());
            } else if (type == "base") {
                expect(TB_TT_B_O, token = nextTokenIgnoringNewlines());
                expect(TB_TT_STR, token = nextTokenIgnoringNewlines());
                string baseName = token->data;
                property = new Model::BaseProperty(baseName);
                expect(TB_TT_B_C, token = nextTokenIgnoringNewlines());
            }

            expect(TB_TT_SC, token = nextTokenIgnoringNewlines());
            return property;
        }

        string EntityDefinitionParser::parseDescription() {
            EntityDefinitionToken* token = m_tokenizer->peek();
            if (token->type == TB_TT_ED_C)
                return "";
            return m_tokenizer->remainder();
        }

        EntityDefinitionParser::EntityDefinitionParser(string path) {
            m_stream.open(path.c_str());
			assert(m_stream.is_open());

            m_tokenizer = new EntityDefinitionTokenizer(m_stream);
        }

        EntityDefinitionParser::~EntityDefinitionParser() {
            if (m_stream.is_open())
                m_stream.close();
            if (m_tokenizer != NULL)
                delete m_tokenizer;
        }

        Model::EntityDefinition* EntityDefinitionParser::nextDefinition() {
            EntityDefinitionToken* token = m_tokenizer->next();
            if (token == NULL)
                return NULL;

            expect(TB_TT_ED_O, token);
            string name;
            bool hasColor = false;
            bool hasBounds = false;
            bool hasFlags = false;
            Vec4f color;
            BBox bounds;
            map<string, Model::SpawnFlag> flags;
            vector<Model::Property*> properties;
            string description;

            token = m_tokenizer->next();
            expect(TB_TT_WORD, token);
            name = token->data;

            token = m_tokenizer->peek();
            expect(TB_TT_B_O | TB_TT_NL, token);
            if (token->type == TB_TT_B_O) {
                hasColor = true;
                color = parseColor();

                token = m_tokenizer->peek();
                expect(TB_TT_B_O | TB_TT_QM, token);
                if (token->type == TB_TT_B_O) {
                    hasBounds = true;
                    bounds = parseBounds();
                } else {
                    m_tokenizer->next();
                }

                token = m_tokenizer->peek();
                if (token->type == TB_TT_WORD) {
                    hasFlags = true;
                    flags = parseFlags();
                }
            }

            expect(TB_TT_NL, token = m_tokenizer->next());
            properties = parseProperties();
            description = parseDescription();
            expect(TB_TT_ED_C, token = m_tokenizer->next());

            Model::EntityDefinition* definition = NULL;
            if (!hasColor)
                definition = Model::EntityDefinition::baseDefinition(name, flags, properties);
            else if (hasBounds)
                definition = Model::EntityDefinition::pointDefinition(name, color, bounds, flags, properties, description);
            else
                definition = Model::EntityDefinition::brushDefinition(name, color, flags, properties, description);
            return definition;
        }
    }
}
