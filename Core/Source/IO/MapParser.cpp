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

#include "MapParser.h"
#include <assert.h>
#include <cmath>
#include "Controller/ProgressIndicator.h"
#include "Utilities/Console.h"

namespace TrenchBroom {
    namespace IO {
        char MapTokenizer::nextChar() {
            if (m_state == TB_TS_EOF)
                return 0;
            
            if (m_index == m_chars.size()) {
                m_state = TB_TS_EOF;
                return 0;
            }
            
            char c = m_chars[m_index++];
            if (c == '\n') {
                m_line++;
                m_column = 0;
            } else {
                m_column++;
            }
            
            return c;
        }
        
        char MapTokenizer::peekChar() {
            if (m_state == TB_TS_EOF || m_index == m_chars.size())
                return 0;
            
            char c = m_chars[m_index];
            return c;
        }
        
        MapToken* MapTokenizer::token(ETokenType type, char* data, unsigned int index, unsigned int line, unsigned int column) {
            m_token.type = type;
            if (data != NULL) m_token.data = std::string(data, index);
            else m_token.data.clear();
            m_token.line = line;
            m_token.column = column;
            m_token.charsRead = m_index;
            return &m_token;
        }
        
        MapTokenizer::MapTokenizer(std::istream& stream) : m_state(TB_TS_DEF), m_line(1), m_column(1) {
            std::istreambuf_iterator<char> begin(stream), end;
            m_chars = std::vector<char>(begin, end);
            m_index = 0;
        }
        
        MapToken* MapTokenizer::next() {
            char c;
            
            while ((c = nextChar()) != 0) {
                switch (m_state) {
                    case TB_TS_DEF:
                        switch (c) {
                            case '/': {
                                char d = peekChar();
                                if (d == '/') {
                                    m_state = TB_TS_COM;
                                    m_bufferIndex = 0;
                                    m_startLine = m_line;
                                    m_startColumn = m_column;
                                    nextChar();
                                    break;
                                }
                            }
                            case '\r':
                            case '\n':
                            case '\t':
                            case ' ':
                                break; // ignore whitespace in boundaries
                            case '{':
                                return token(TB_TT_CB_O, NULL, 0, m_line, m_column);
                            case '}':
                                return token(TB_TT_CB_C, NULL, 0, m_line, m_column);
                            case '(':
                                return token(TB_TT_B_O, NULL, 0, m_line, m_column);
                            case ')':
                                return token(TB_TT_B_C, NULL, 0, m_line, m_column);
                            case '[':
                                return token(TB_TT_SB_O, NULL, 0, m_line, m_column);
                            case ']':
                                return token(TB_TT_CB_C, NULL, 0, m_line, m_column);
                            case '"':
                                m_state = TB_TS_Q_STR;
                                m_bufferIndex = 0;
                                m_startLine = m_line;
                                m_startColumn = m_column;
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
                                m_bufferIndex = 0;
                                m_buffer[m_bufferIndex++] = c;
                                m_startLine = m_line;
                                m_startColumn = m_column;
                                break;
                            default:
                                m_state = TB_TS_STR;
                                m_bufferIndex = 0;
                                m_buffer[m_bufferIndex++] = c;
                                m_startLine = m_line;
                                m_startColumn = m_column;
                                break;
                        }
                        break;
                    case TB_TS_Q_STR:
                        switch (c) {
                            case '"': {
                                MapToken* tok = token(TB_TT_STR, m_buffer, m_bufferIndex, m_startLine, m_startColumn);
                                m_bufferIndex = 0;
                                m_state = TB_TS_DEF;
                                return tok;
                            }
                            default:
                                m_buffer[m_bufferIndex++] = c;
                                break;
                        }
                        break;
                    case TB_TS_STR: {
                        bool comment = false;
                        switch (c) {
                            case '/': {
                                if (peekChar() == '/') comment = true, nextChar();
                            }
                            case '\r':
                            case '\n':
                            case '\t':
                            case ' ': {
                                MapToken* tok = token(TB_TT_STR, m_buffer, m_bufferIndex, m_startLine, m_startColumn);
                                m_bufferIndex = 0;
                                m_state = comment ? TB_TS_COM : TB_TS_DEF;
                                return tok;
                            }
                            default:
                                m_buffer[m_bufferIndex++] = c;
                                break;
                        }
                        break;
                    }
                    case TB_TS_DEC:
                        if (c == '.') m_state = TB_TS_FRAC;
                    case TB_TS_FRAC: {
                        bool comment = false;
                        switch (c) {
                            case '/':
                                if (peekChar() == '/') comment = true, nextChar();
                            case '\r':
                            case '\n':
                            case '\t':
                            case ' ': {
                                MapToken* tok;
                                if (m_state == TB_TS_DEC) tok = token(TB_TT_DEC, m_buffer, m_bufferIndex, m_startLine, m_startColumn);
                                else tok = token(TB_TT_FRAC, m_buffer, m_bufferIndex, m_startLine, m_startColumn);
                                m_bufferIndex = 0;
                                m_state = comment ? TB_TS_COM : TB_TS_DEF;
                                return tok;
                            }
                            default:
                                if ((c < '0' || c > '9') && (c != '.'))
                                    m_state = TB_TS_STR;
                                m_buffer[m_bufferIndex++] = c;
                                break;
                        }
                        break;
                    }
                    case TB_TS_COM:
                        switch (c) {
                            case '\r':
                            case '\n': {
                                MapToken* tok = token(TB_TT_COM, m_buffer, m_bufferIndex, m_startLine, m_startColumn);
                                m_bufferIndex = 0;
                                m_state = TB_TS_DEF;
                                return tok;
                            }
                            default:
                                m_buffer[m_bufferIndex++] = c;
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }
            
            return NULL;
        }
        
        int MapTokenizer::size() {
            return (int)m_chars.size();
        }

        void MapParser::expect(int expectedType, const MapToken* actualToken) const {
            assert(actualToken != NULL);
            assert((actualToken->type & expectedType) != 0);
        }
        
        MapToken* MapParser::nextToken() {
            MapToken* token = NULL;
            if (!m_tokenStack.empty()) {
                token = m_tokenStack.back();
                m_tokenStack.pop_back();
            } else {
                token = m_tokenizer->next();
                while (token != NULL && token->type == TB_TT_COM)
                    token = m_tokenizer->next();
            }
            
            return token;
        }
        
        void MapParser::pushToken(MapToken* token) {
            m_tokenStack.push_back(token);
        }
        
        MapParser::MapParser(std::istream& stream) {
            std::streamoff cur = stream.tellg();
            stream.seekg(0, std::ios::end);
            m_size = static_cast<unsigned int>(stream.tellg());
            m_size -= static_cast<unsigned int>(cur);
            stream.seekg(cur, std::ios::beg);
            m_tokenizer = new MapTokenizer(stream);
        }
        
        MapParser::~MapParser() {
            delete m_tokenizer;
        }
        
        void MapParser::parseMap(Model::Map& map, Controller::ProgressIndicator* indicator) {
            m_format = TB_MF_UNDEFINED;
            Model::Entity* entity = NULL;
            
            if (indicator != NULL) indicator->reset(static_cast<float>(m_tokenizer->size()));
            while ((entity = parseEntity(map.worldBounds(), indicator)) != NULL) map.addEntity(entity);
            if (indicator != NULL) indicator->update(static_cast<float>(m_tokenizer->size()));
        }
        
        Model::Entity* MapParser::parseEntity(const BBox& worldBounds, Controller::ProgressIndicator* indicator) {
            MapToken* token = nextToken();
            if (token == NULL) return NULL;
            
            expect(TB_TT_CB_O | TB_TT_CB_C, token);
            if (token->type == TB_TT_CB_C) return NULL;
            
            Model::Entity* entity = new Model::Entity();
            entity->setFilePosition(token->line);
            
            while ((token = nextToken()) != NULL) {
                switch (token->type) {
                    case TB_TT_STR: {
                        std::string key = token->data;
                        token = nextToken();
                        expect(TB_TT_STR, token);
                        std::string value = token->data;
                        entity->setProperty(key, value);
                        break;
                    }
                    case TB_TT_CB_O: {
                        pushToken(token);
                        Model::Brush* brush = NULL;
                        while ((brush = parseBrush(worldBounds, indicator)) != NULL)
                            entity->addBrush(brush);
                    }
                    case TB_TT_CB_C: {
                        if (indicator != NULL) indicator->update(static_cast<float>(token->charsRead));
                        return entity;
                    }
                    default:
                        log(TB_LL_ERR, "Unexpected token type %i at line %i\n", token->type, token->line);
                        return NULL;
                }
            }
            
            return entity;
        }
        
        Model::Brush* MapParser::parseBrush(const BBox& worldBounds, Controller::ProgressIndicator* indicator) {
            MapToken* token;

            expect(TB_TT_CB_O | TB_TT_CB_C, token = nextToken());
            if (token->type == TB_TT_CB_C) return NULL;
            
            Model::Brush* brush = new Model::Brush(worldBounds);
            brush->filePosition = token->line;
            
            while ((token = nextToken()) != NULL) {
                switch (token->type) {
                    case TB_TT_B_O: {
                        pushToken(token);
                        Model::Face* face = parseFace(worldBounds);
                        if (face != NULL) brush->addFace(face);
                        break;
                    }
                    case TB_TT_CB_C:
                        if (indicator != NULL) indicator->update(static_cast<float>(token->charsRead));
                        return brush;
                    default:
                        log(TB_LL_ERR, "Unexpected token type %i at line %i\n", token->type, token->line);
                        return NULL;
                }
            }
            return NULL;
        }
        
        Model::Face* MapParser::parseFace(const BBox& worldBounds) {
            Vec3f p1, p2, p3;
            float xOffset, yOffset, rotation, xScale, yScale;
            MapToken* token;
            
            expect(TB_TT_B_O, token = nextToken());
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            p1.x = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            p1.y = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            p1.z = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_B_C, token = nextToken());
            expect(TB_TT_B_O, token = nextToken());
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            p2.x = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            p2.y = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            p2.z = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_B_C, token = nextToken());
            expect(TB_TT_B_O, token = nextToken());
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            p3.x = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            p3.y = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            p3.z = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_B_C, token = nextToken());
            
            expect(TB_TT_STR, token = nextToken());
            std::string textureName = token->data;
            
            token = nextToken();
            if (m_format == TB_MF_UNDEFINED) {
                expect(TB_TT_DEC | TB_TT_FRAC | TB_TT_SB_O, token);
                m_format = token->type == TB_TT_DEC ? TB_MF_STANDARD : TB_MF_VALVE;
                if (m_format == TB_MF_VALVE) 
                    log(TB_LL_WARN, "Loading unsupported map Valve 220 map format");
            }
            
            if (m_format == TB_MF_STANDARD) {
                expect(TB_TT_DEC | TB_TT_FRAC, token);
                bool frac = token->type == TB_TT_FRAC;
                xOffset = static_cast<float>(atof(token->data.c_str()));
                expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
                yOffset = static_cast<float>(atof(token->data.c_str()));
                if (frac || token->type == TB_TT_FRAC)
                    log(TB_LL_WARN, "Rounding fractional texture offset in line %i", token->line);
            } else { // Valve 220 format
                expect(TB_TT_SB_O, token);
                expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken()); // X texture axis x
                expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken()); // X texture axis y
                expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken()); // X texture axis z
                expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken()); // X texture axis offset
                xOffset = static_cast<float>(atof(token->data.c_str()));
                expect(TB_TT_SB_C, token = nextToken());
                expect(TB_TT_SB_O, token = nextToken());
                expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken()); // Y texture axis x
                expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken()); // Y texture axis y
                expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken()); // Y texture axis z
                expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken()); // Y texture axis offset
                yOffset = static_cast<float>(atof(token->data.c_str()));
                expect(TB_TT_SB_C, token = nextToken());
            }
            
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            rotation = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            xScale = static_cast<float>(atof(token->data.c_str()));
            expect(TB_TT_DEC | TB_TT_FRAC, token = nextToken());
            yScale = static_cast<float>(atof(token->data.c_str()));
            
            if (((p3 - p1) % (p2 - p1)).equals(Null3f)) {
                log(TB_LL_WARN, "Skipping invalid face in line %i", token->line);
                return NULL;
            }
            
            Model::Face* face = new Model::Face(worldBounds, p1, p2, p3, textureName);
            face->xOffset = xOffset;
            face->yOffset = yOffset;
            face->rotation = rotation;
            face->xScale = xScale;
            face->yScale = yScale;
            face->filePosition = token->line;
            return face;
        }
    }
}