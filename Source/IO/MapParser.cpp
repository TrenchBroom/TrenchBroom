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

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Face.h"
#include "Model/Map.h"
#include "Utility/ProgressIndicator.h"

namespace TrenchBroom {
    namespace IO {
        MapTokenizer::TokenPtr MapTokenizer::nextToken() {
            StringStream buffer;
            while (!eof()) {
                char c = nextChar();
                switch (m_state) {
                    case TokenizerState::Default:
                        switch (c) {
                            case '/': {
                                char d = peekChar();
                                if (d == '/') {
                                    m_state = TokenizerState::Comment;
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
                                return token(TokenType::OBrace, "");
                            case '}':
                                return token(TokenType::CBrace, "");
                            case '(':
                                return token(TokenType::OParenthesis, "");
                            case ')':
                                return token(TokenType::CParenthesis, "");
                            case '[':
                                return token(TokenType::OBracket, "");
                            case ']':
                                return token(TokenType::CBracket, "");
                            case '"':
                                m_state = TokenizerState::QString;
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
                                m_state = TokenizerState::Integer;
                                buffer << c;
                                m_startLine = m_line;
                                m_startColumn = m_column;
                                break;
                            default:
                                m_state = TokenizerState::String;
                                buffer << c;
                                m_startLine = m_line;
                                m_startColumn = m_column;
                                break;
                        }
                        break;
                    case TokenizerState::QString:
                        switch (c) {
                            case '"': {
                                m_state = TokenizerState::Default;
                                return token(TokenType::String, buffer.str(), m_startLine, m_startColumn);
                            }
                            default:
                                buffer << c;
                                break;
                        }
                        break;
                    case TokenizerState::String: {
                        bool comment = false;
                        switch (c) {
                            case '/': {
                                if (peekChar() == '/') {
                                    comment = true;
                                    nextChar();
                                }
                            }
                            case '\r':
                            case '\n':
                            case '\t':
                            case ' ': {
                                m_state = comment ? TokenizerState::Comment : TokenizerState::Default;
                                return token(TokenType::String, buffer.str(), m_startLine, m_startColumn);
                            }
                            default:
                                buffer << c;
                                break;
                        }
                        break;
                    }
                    case TokenizerState::Integer:
                        if (c == '.')
                            m_state = TokenizerState::Decimal;
                    case TokenizerState::Decimal: {
                        bool comment = false;
                        switch (c) {
                            case '/':
                                if (peekChar() == '/') {
                                    comment = true;
                                    nextChar();
                                }
                            case '\r':
                            case '\n':
                            case '\t':
                            case ' ': {
                                unsigned int previousState = m_state;
                                m_state = comment ? TokenizerState::Comment : TokenizerState::Default;
                                if (previousState == TokenizerState::Integer)
                                    return token(TokenType::Integer, buffer.str(), m_startLine, m_startColumn);
                                return token(TokenType::Decimal, buffer.str(), m_startLine, m_startColumn);
                            }
                            default:
                                if ((c < '0' || c > '9') && (c != '.'))
                                    m_state = TokenizerState::String;
                                buffer << c;
                                break;
                        }
                        break;
                    }
                    case TokenizerState::Comment:
                        switch (c) {
                            case '\r':
                            case '\n': {
                                m_state = TokenizerState::Default;
                                return token(TokenType::Comment, buffer.str(), m_startLine, m_startColumn);
                            }
                            default:
                                buffer << c;
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }
            
            return MapTokenizer::TokenPtr();
        }
        
        MapTokenizer::TokenPtr MapTokenizer::peekToken() {
            unsigned int oldState = m_state;
            size_t oldLine = m_line;
            size_t oldColumn = m_column;
            size_t oldStartLine = m_startLine;
            size_t oldStartColumn = m_startColumn;
            size_t oldPosition = m_stream.tellg();
            
            MapTokenizer::TokenPtr token = nextToken();
            m_state = oldState;
            m_line = oldLine;
            m_column = oldColumn;
            m_startLine = oldStartLine;
            m_startColumn = oldStartColumn;
            m_stream.seekg(oldPosition, std::ios::beg);
            
            return token;
        }
        
        void MapTokenizer::reset() {
            m_state = TokenizerState::Default;
            m_line = 1;
            m_column = 1;
            m_startLine = 1;
            m_startColumn = 1;
            m_stream.seekg(0, std::ios::beg);
        }

        MapParser::MapParser(const String& path, std::istream& stream) :
        m_tokenizer(path, stream),
        m_format(MapFormat::Undefined) {
            std::streamoff cur = stream.tellg();
            stream.seekg(0, std::ios::end);
            m_size = static_cast<size_t>(stream.tellg() - cur);
            stream.seekg(cur, std::ios::beg);
        }

        void MapParser::parseMap(Model::Map& map, Utility::ProgressIndicator* indicator) {
            Model::Entity* entity = NULL;
            
            if (indicator != NULL) indicator->reset(static_cast<float>(m_size));
            try {
                while ((entity = parseEntity(map.worldBounds(), indicator)) != NULL)
                    map.addEntity(entity);
            } catch (MapParserException e) {
                // log(TB_LL_ERR, e.std::exception::what());
            }
            if (indicator != NULL)
                indicator->update(static_cast<float>(m_size));
        }

        Model::Entity* MapParser::parseEntity(const BBox& worldBounds, Utility::ProgressIndicator* indicator) {
            MapTokenizer::TokenPtr token = nextToken();
            if (token.get() == NULL)
                return NULL;
            
            expect(TokenType::OBrace | TokenType::CBrace, token.get());
            if (token->type() == TokenType::CBrace)
                return NULL;
            
            Model::Entity* entity = new Model::Entity();
            entity->setFilePosition(token->line());
            
            while ((token = nextToken()).get() != NULL) {
                switch (token->type()) {
                    case TokenType::String: {
                        String key = token->data();
                        token = nextToken();
                        expect(TokenType::String, token.get());
                        String value = token->data();
                        entity->setProperty(key, value);
                        break;
                    }
                    case TokenType::OBrace: {
                        pushToken(token);
                        bool moreBrushes = true;
                        while (moreBrushes) {
                            Model::Brush* brush = parseBrush(worldBounds, indicator);
                            if (brush != NULL)
                                entity->addBrush(brush);
                            token = nextToken();
                            expect(TokenType::OBrace | TokenType::CBrace, token.get());
                            moreBrushes = (token->type() == TokenType::OBrace);
                            pushToken(token);
                        }
                        break;
                    }
                    case TokenType::CBrace: {
                        if (indicator != NULL)
                            indicator->update(static_cast<float>(token->position()));
                        return entity;
                    }
                    default:
                        delete entity;
                        throw MapParserException(token.get(), TokenType::String | TokenType::OBrace | TokenType::CBrace);
                }
            }
            
            return entity;
        }
        
        Model::Brush* MapParser::parseBrush(const BBox& worldBounds, Utility::ProgressIndicator* indicator) {
            MapTokenizer::TokenPtr token = nextToken();
            if (token.get() == NULL)
                return NULL;
            
            expect(TokenType::OBrace | TokenType::CBrace, token.get());
            if (token->type() == TokenType::CBrace)
                return NULL;
            
            Model::Brush* brush = new Model::Brush(worldBounds);
            brush->setFilePosition(token->line());
            
            while ((token = nextToken()) != NULL) {
                switch (token->type) {
                    case TB_TT_B_O: {
                        pushToken(token);
                        Model::Face* face = parseFace(worldBounds);
                        if (face == NULL) {
                            log(TB_LL_WARN, "Skipping malformed face at line %i\n", token->line);
                        } else if (brush != NULL) {
                            if (!brush->addFace(face)) {
                                log(TB_LL_WARN, "Skipping malformed brush at line %i\n", brush->filePosition);
                                delete brush;
                                brush = NULL;
                            }
                        } else {
                            delete face;
                        }
                        break;
                    }
                    case TB_TT_CB_C:
                        if (indicator != NULL) indicator->update(static_cast<float>(token->charsRead));
                        if (brush != NULL && !brush->geometry->closed()) {
                            log(TB_LL_WARN, "Skipping non-closed brush at line %i\n", brush->filePosition);
                            delete brush;
                            brush = NULL;
                        }
                        return brush;
                    default:
                        delete brush;
                        throw MapParserException(*token, TB_TT_B_O | TB_TT_CB_C);
                }
            }
            
            return NULL;
        }
        
        Model::Face* MapParser::parseFace(const BBox& worldBounds) {
            Vec3f p1, p2, p3;
            float xOffset, yOffset, rotation, xScale, yScale;
            MapToken* token = nextToken();
            if (token == NULL) return NULL;
            
            expect(TB_TT_B_O, token);
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
            String textureName = token->data;
            
            token = nextToken();
            if (m_format == TB_MF_UNDEFINED) {
                expect(TB_TT_DEC | TB_TT_FRAC | TB_TT_SB_O, token);
                m_format = token->type == TB_TT_SB_O ? TB_MF_VALVE : TB_MF_STANDARD;
                if (m_format == TB_MF_VALVE)
                    log(TB_LL_WARN, "Loading unsupported map Valve 220 map format\n");
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
            
            if (((p3 - p1) % (p2 - p1)).null()) {
                log(TB_LL_WARN, "Skipping invalid face in line %i", token->line);
                return NULL;
            }
            
            if (textureName == Model::Assets::Texture::EMPTY)
                textureName = "";
            
            Model::Face* face = new Model::Face(worldBounds, p1, p2, p3, textureName);
            face->xOffset = xOffset;
            face->yOffset = yOffset;
            face->rotation = rotation;
            face->xScale = xScale;
            face->yScale = yScale;
            face->filePosition = token->line;
            return face;
        }
        
        bool MapParser::parseEntities(const BBox& worldBounds, Model::EntityList& entities) {
            size_t oldSize = entities.size();
            try {
                Model::Entity* entity = NULL;
                while ((entity = parseEntity(worldBounds, NULL)) != NULL) entities.push_back(entity);
                return !entities.empty();
            } catch (MapParserException e) {
                while (entities.size() > oldSize) delete entities.back(), entities.pop_back();
                m_tokenizer->reset();
                return false;
            }
        }
        
        bool MapParser::parseBrushes(const BBox& worldBounds, Model::BrushList& brushes) {
            size_t oldSize = brushes.size();
            try {
                Model::Brush* brush = NULL;
                while ((brush = parseBrush(worldBounds, NULL)) != NULL) brushes.push_back(brush);
                return !brushes.empty();
            } catch (MapParserException e) {
                while (brushes.size() > oldSize) delete brushes.back(), brushes.pop_back();
                m_tokenizer->reset();
                return false;
            }
        }
        
        bool MapParser::parseFaces(const BBox& worldBounds, Model::FaceList& faces) {
            size_t oldSize = faces.size();
            try {
                Model::Face* face = NULL;
                while ((face = parseFace(worldBounds)) != NULL) faces.push_back(face);
                return !faces.empty();
            } catch (MapParserException e) {
                while (faces.size() > oldSize) delete faces.back(), faces.pop_back();
                m_tokenizer->reset();
                return false;
            }
        }
    }
}
