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
#include "Model/Texture.h"
#include "Utility/Console.h"
#include "Utility/ProgressIndicator.h"

namespace TrenchBroom {
    namespace IO {
        MapTokenizer::TokenPtr MapTokenizer::nextToken() {
            m_buffer.str(String());
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
								if (peekChar() == '\n')
									nextChar();
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
                                m_buffer << c;
                                m_startLine = m_line;
                                m_startColumn = m_column;
                                break;
                            default:
                                m_state = TokenizerState::String;
                                m_buffer << c;
                                m_startLine = m_line;
                                m_startColumn = m_column;
                                break;
                        }
                        break;
                    case TokenizerState::QString:
                        switch (c) {
                            case '"': {
                                m_state = TokenizerState::Default;
                                return token(TokenType::String, m_buffer.str(), m_startLine, m_startColumn);
                            }
                            default:
                                m_buffer << c;
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
								if (peekChar() == '\n')
									nextChar();
                            case '\n':
                            case '\t':
                            case ' ': {
                                m_state = comment ? TokenizerState::Comment : TokenizerState::Default;
                                return token(TokenType::String, m_buffer.str(), m_startLine, m_startColumn);
                            }
                            default:
                                m_buffer << c;
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
								if (peekChar() == '\n')
									nextChar();
                            case '\n':
                            case '\t':
                            case ' ': {
                                unsigned int previousState = m_state;
                                m_state = comment ? TokenizerState::Comment : TokenizerState::Default;
                                if (previousState == TokenizerState::Integer)
                                    return token(TokenType::Integer, m_buffer.str(), m_startLine, m_startColumn);
                                return token(TokenType::Decimal, m_buffer.str(), m_startLine, m_startColumn);
                            }
                            default:
                                if ((c < '0' || c > '9') && (c != '.'))
                                    m_state = TokenizerState::String;
                                m_buffer << c;
                                break;
                        }
                        break;
                    }
                    case TokenizerState::Comment:
                        switch (c) {
							case '\r':
								if (peekChar() == '\n')
									nextChar();
                            case '\n': {
                                m_state = TokenizerState::Default;
                                return token(TokenType::Comment, m_buffer.str(), m_startLine, m_startColumn);
                            }
                            default:
                                m_buffer << c;
                                break;
                        }
                        break;
                    default:
                        break;
                }
            }
            
            return MapTokenizer::TokenPtr(NULL);
        }
        
        MapTokenizer::TokenPtr MapTokenizer::peekToken() {
            unsigned int oldState = m_state;
            size_t oldLine = m_line;
            size_t oldColumn = m_column;
            size_t oldStartLine = m_startLine;
            size_t oldStartColumn = m_startColumn;
            std::ios::pos_type oldPosition = m_stream.tellg();
            
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

        MapParser::MapParser(std::istream& stream, Utility::Console& console) :
        m_tokenizer(stream),
        m_console(console),
        m_format(Undefined) {
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
                m_console.error(e.what());
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
            
            Model::Entity* entity = new Model::Entity(worldBounds);
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
            
            while ((token = nextToken()).get() != NULL) {
                switch (token->type()) {
                    case TokenType::OParenthesis: {
                        pushToken(token);
                        Model::Face* face = parseFace(worldBounds);
                        if (face == NULL) {
                            m_console.warn("Skipping malformed face at line %i", token->line());
                        } else if (brush != NULL) {
                            if (!brush->addFace(face)) {
                                m_console.warn("Skipping malformed brush at line %i", brush->filePosition());
                                delete brush;
                                brush = NULL;
                            }
                        } else {
                            delete face;
                        }
                        break;
                    }
                    case TokenType::CBrace:
                        if (indicator != NULL) indicator->update(static_cast<float>(token->position()));
                        if (brush != NULL && !brush->closed()) {
                            m_console.warn("Skipping non-closed brush at line %i", brush->filePosition());
                            delete brush;
                            brush = NULL;
                        }
                        return brush;
                    default:
                        delete brush;
                        throw MapParserException(token.get(), TokenType::OParenthesis | TokenType::CParenthesis);
                }
            }
            
            return NULL;
        }
        
        Model::Face* MapParser::parseFace(const BBox& worldBounds) {
            Vec3f p1, p2, p3;
            float xOffset, yOffset, rotation, xScale, yScale;
            MapTokenizer::TokenPtr token = nextToken();
            if (token.get() == NULL)
                return NULL;
            
            expect(TokenType::OParenthesis, token.get());
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            p1.x = token->toFloat();
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            p1.y = token->toFloat();
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            p1.z = token->toFloat();
            expect(TokenType::CParenthesis, (token = nextToken()).get());
            expect(TokenType::OParenthesis, (token = nextToken()).get());
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            p2.x = token->toFloat();
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            p2.y = token->toFloat();
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            p2.z = token->toFloat();
            expect(TokenType::CParenthesis, (token = nextToken()).get());
            expect(TokenType::OParenthesis, (token = nextToken()).get());
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            p3.x = token->toFloat();
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            p3.y = token->toFloat();
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            p3.z = token->toFloat();
            expect(TokenType::CParenthesis, (token = nextToken()).get());
            
            expect(TokenType::String, (token = nextToken()).get());
            String textureName = token->data();
            
            token = nextToken();
            if (m_format == Undefined) {
                expect(TokenType::Integer | TokenType::Decimal | TokenType::OBracket, token.get());
                m_format = token->type() == TokenType::OBracket ? Valve : Standard;
                if (m_format == Valve)
                    m_console.warn("Loading unsupported map Valve 220 map format");
            }
            
            if (m_format == Standard) {
                expect(TokenType::Integer | TokenType::Decimal, token.get());
                bool dec = token->type() == TokenType::Decimal;
                xOffset = token->toFloat();
                expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
                yOffset = token->toFloat();
                if (dec || token->type() == TokenType::Decimal)
                    m_console.warn("Rounding fractional texture offset in line %i", token->line());
            } else { // Valve 220 format
                expect(TokenType::OBracket, token.get());
                expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get()); // X texture axis x
                expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get()); // X texture axis y
                expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get()); // X texture axis z
                expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get()); // X texture axis offset
                xOffset = token->toFloat();
                expect(TokenType::CBracket, (token = nextToken()).get());
                expect(TokenType::OBracket, (token = nextToken()).get());
                expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get()); // Y texture axis x
                expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get()); // Y texture axis y
                expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get()); // Y texture axis z
                expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get()); // Y texture axis offset
                yOffset = token->toFloat();
                expect(TokenType::CBracket, (token = nextToken()).get());
            }
            
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            rotation = token->toFloat();
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            xScale = token->toFloat();
            expect(TokenType::Integer | TokenType::Decimal, (token = nextToken()).get());
            yScale = token->toFloat();
            
            if (((p3 - p1).crossed(p2 - p1)).null()) {
                m_console.warn("Skipping invalid face in line %i", token->line());
                return NULL;
            }
            
            if (textureName == Model::Texture::Empty)
                textureName = "";
            
            Model::Face* face = new Model::Face(worldBounds, p1, p2, p3, textureName);
            face->setXOffset(xOffset);
            face->setYOffset(yOffset);
            face->setRotation(rotation);
            face->setXScale(xScale);
            face->setYScale(yScale);
            face->setFilePosition(token->line());
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
                m_tokenizer.reset();
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
                m_tokenizer.reset();
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
                m_tokenizer.reset();
                return false;
            }
        }
    }
}
