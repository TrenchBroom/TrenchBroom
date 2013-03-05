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
#include "Utility/List.h"
#include "Utility/ProgressIndicator.h"

namespace TrenchBroom {
    namespace IO {
        Token MapTokenEmitter::doEmit(Tokenizer& tokenizer, size_t line, size_t column) {
            const size_t startPosition = tokenizer.position();
            while (!tokenizer.eof()) {
                char c = tokenizer.nextChar();
                switch (c) {
                    case '/':
                        if (tokenizer.peekChar() == '/') {
                            // eat everything up to and including the next newline
                            while (tokenizer.nextChar() != '\n');
                        }
                        break;
                    case '{':
                        return Token(TokenType::OBrace, "", startPosition, tokenizer.position() - startPosition, line, column);
                    case '}':
                        return Token(TokenType::CBrace, "", startPosition, tokenizer.position() - startPosition, line, column);
                    case '(':
                        return Token(TokenType::OParenthesis, "", startPosition, tokenizer.position() - startPosition, line, column);
                    case ')':
                        return Token(TokenType::CParenthesis, "", startPosition, tokenizer.position() - startPosition, line, column);
                    case '[':
                        return Token(TokenType::OBracket, "", startPosition, tokenizer.position() - startPosition, line, column);
                    case ']':
                        return Token(TokenType::CBracket, "", startPosition, tokenizer.position() - startPosition, line, column);
                    case '"': // quoted string
                        m_buffer.str(String());
                        while (!tokenizer.eof() && (c = tokenizer.nextChar()) != '"')
                            m_buffer << c;
                        return Token(TokenType::String, m_buffer.str(), startPosition, tokenizer.position() - startPosition, line, column);
                    default: // whitespace, integer, decimal or word
                        if (isWhitespace(c))
                            break;
                        
                        // clear the buffer
                        m_buffer.str(String());
                        
                        // try to read a number
                        if (c == '-' || isDigit(c)) {
                            m_buffer << c;
                            while (isDigit((c = tokenizer.nextChar())))
                                m_buffer << c;
                            if (isDelimiter(c)) {
                                if (!tokenizer.eof())
                                    tokenizer.pushChar();
                                return Token(TokenType::Integer, m_buffer.str(), startPosition, tokenizer.position() - startPosition, line, column);
                            }
                        }
                        
                        // try to read a decimal (may start with '.')
                        if (c == '.') {
                            m_buffer << c;
                            while (isDigit((c = tokenizer.nextChar())))
                                m_buffer << c;
                            if (isDelimiter(c)) {
                                if (!tokenizer.eof())
                                    tokenizer.pushChar();
                                return Token(TokenType::Decimal, m_buffer.str(), startPosition, tokenizer.position() - startPosition, line, column);
                            }
                        }
                        
                        // read a word
                        m_buffer << c;
                        while (!tokenizer.eof() && !isDelimiter(c = tokenizer.nextChar()))
                            m_buffer << c;
                        if (!tokenizer.eof())
                            tokenizer.pushChar();
                        return Token(TokenType::String, m_buffer.str(), startPosition, tokenizer.position() - startPosition, line, column);
                }
            }
            return Token(TokenType::Eof, "", startPosition, tokenizer.position() - startPosition, line, column);
        }

        MapParser::MapParser(std::istream& stream, Utility::Console& console) :
        m_console(console),
        m_tokenizer(stream),
        m_format(Undefined) {
            std::streamoff cur = stream.tellg();
            stream.seekg(0, std::ios::end);
            m_size = static_cast<size_t>(stream.tellg() - cur);
            stream.seekg(cur, std::ios::beg);
        }

        void MapParser::parseMap(Model::Map& map, Utility::ProgressIndicator* indicator) {
            Model::Entity* entity = NULL;
            
            if (indicator != NULL) indicator->reset(static_cast<int>(m_size));
            try {
                while ((entity = parseEntity(map.worldBounds(), indicator)) != NULL)
                    map.addEntity(*entity);
            } catch (MapParserException e) {
                m_console.error(e.what());
            }
            if (indicator != NULL)
                indicator->update(static_cast<int>(m_size));
        }

        Model::Entity* MapParser::parseEntity(const BBox& worldBounds, Utility::ProgressIndicator* indicator) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == TokenType::Eof)
                return NULL;
            
            expect(TokenType::OBrace | TokenType::CBrace, token);
            if (token.type() == TokenType::CBrace)
                return NULL;
            
            Model::Entity* entity = new Model::Entity(worldBounds);
            entity->setFilePosition(token.line());
            
            while ((token = m_tokenizer.nextToken()).type() != TokenType::Eof) {
                switch (token.type()) {
                    case TokenType::String: {
                        String key = token.data();
                        expect(TokenType::String, token = m_tokenizer.nextToken());
                        String value = token.data();
                        entity->setProperty(key, value);
                        break;
                    }
                    case TokenType::OBrace: {
                        m_tokenizer.pushToken(token);
                        bool moreBrushes = true;
                        while (moreBrushes) {
                            Model::Brush* brush = parseBrush(worldBounds, indicator);
                            if (brush != NULL)
                                entity->addBrush(*brush);
                            expect(TokenType::OBrace | TokenType::CBrace, token = m_tokenizer.nextToken());
                            moreBrushes = (token.type() == TokenType::OBrace);
                            m_tokenizer.pushToken(token);
                        }
                        break;
                    }
                    case TokenType::CBrace: {
                        if (indicator != NULL)
                            indicator->update(static_cast<int>(token.position()));
                        return entity;
                    }
                    default:
                        delete entity;
                        throw MapParserException(token, TokenType::String | TokenType::OBrace | TokenType::CBrace);
                }
            }
            
            return entity;
        }
        
        Model::Brush* MapParser::parseBrush(const BBox& worldBounds, Utility::ProgressIndicator* indicator) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == TokenType::Eof)
                return NULL;
            
            expect(TokenType::OBrace | TokenType::CBrace, token);
            if (token.type() == TokenType::CBrace)
                return NULL;
            
            Model::Brush* brush = new Model::Brush(worldBounds);
            brush->setFilePosition(token.line());
            
            while ((token = m_tokenizer.nextToken()).type() != TokenType::Eof) {
                switch (token.type()) {
                    case TokenType::OParenthesis: {
                        m_tokenizer.pushToken(token);
                        Model::Face* face = parseFace(worldBounds);
                        if (face != NULL && brush != NULL) {
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
                        if (indicator != NULL) indicator->update(static_cast<int>(token.position()));
                        if (brush != NULL && !brush->closed()) {
                            m_console.warn("Non-closed brush at line %i", brush->filePosition());
                            // delete brush;
                            // brush = NULL;
                        }

                        /*
                        if (brush != NULL)
                            brush->snap(0);
                         */
                        return brush;
                    default:
                        delete brush;
                        throw MapParserException(token, TokenType::OParenthesis | TokenType::CParenthesis);
                }
            }
            
            return NULL;
        }
        
        Model::Face* MapParser::parseFace(const BBox& worldBounds) {
            Vec3f p1, p2, p3;
            float xOffset, yOffset, rotation, xScale, yScale;
            Token token = m_tokenizer.nextToken();
            if (token.type() == TokenType::Eof)
                return NULL;
            
            expect(TokenType::OParenthesis, token);
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            p1.x = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            p1.y = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            p1.z = token.toFloat();
            expect(TokenType::CParenthesis, token = m_tokenizer.nextToken());
            expect(TokenType::OParenthesis, token = m_tokenizer.nextToken());
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            p2.x = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            p2.y = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            p2.z = token.toFloat();
            expect(TokenType::CParenthesis, token = m_tokenizer.nextToken());
            expect(TokenType::OParenthesis, token = m_tokenizer.nextToken());
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            p3.x = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            p3.y = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            p3.z = token.toFloat();
            expect(TokenType::CParenthesis, token = m_tokenizer.nextToken());
            
            expect(TokenType::String, token = m_tokenizer.nextToken());
            String textureName = token.data();
            
            token = m_tokenizer.nextToken();
            if (m_format == Undefined) {
                expect(TokenType::Integer | TokenType::Decimal | TokenType::OBracket, token);
                m_format = token.type() == TokenType::OBracket ? Valve : Standard;
                if (m_format == Valve)
                    m_console.warn("Loading unsupported map Valve 220 map format");
            }
            
            if (m_format == Standard) {
                expect(TokenType::Integer | TokenType::Decimal, token);
                xOffset = token.toFloat();
                expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
                yOffset = token.toFloat();
            } else { // Valve 220 format
                expect(TokenType::OBracket, token);
                expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken()); // X texture axis x
                expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken()); // X texture axis y
                expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken()); // X texture axis z
                expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken()); // X texture axis offset
                xOffset = token.toFloat();
                expect(TokenType::CBracket, token = m_tokenizer.nextToken());
                expect(TokenType::OBracket, token = m_tokenizer.nextToken());
                expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken()); // Y texture axis x
                expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken()); // Y texture axis y
                expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken()); // Y texture axis z
                expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken()); // Y texture axis offset
                yOffset = token.toFloat();
                expect(TokenType::CBracket, token = m_tokenizer.nextToken());
            }
            
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            rotation = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            xScale = token.toFloat();
            expect(TokenType::Integer | TokenType::Decimal, token = m_tokenizer.nextToken());
            yScale = token.toFloat();
            
            if (((p3 - p1).crossed(p2 - p1)).null()) {
                m_console.warn("Skipping face with colinear points in line %i", token.line());
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
            face->setFilePosition(token.line());
            return face;
        }
        
        bool MapParser::parseEntities(const BBox& worldBounds, Model::EntityList& entities) {
            size_t oldSize = entities.size();
            try {
                Model::Entity* entity = NULL;
                while ((entity = parseEntity(worldBounds, NULL)) != NULL)
                    entities.push_back(entity);
                return !entities.empty();
            } catch (MapParserException e) {
                Utility::deleteAll(entities, oldSize);
                m_tokenizer.reset();
                return false;
            }
        }
        
        bool MapParser::parseBrushes(const BBox& worldBounds, Model::BrushList& brushes) {
            size_t oldSize = brushes.size();
            try {
                Model::Brush* brush = NULL;
                while ((brush = parseBrush(worldBounds, NULL)) != NULL)
                    brushes.push_back(brush);
                return !brushes.empty();
            } catch (MapParserException e) {
                Utility::deleteAll(brushes, oldSize);
                m_tokenizer.reset();
                return false;
            }
        }
        
        bool MapParser::parseFaces(const BBox& worldBounds, Model::FaceList& faces) {
            size_t oldSize = faces.size();
            try {
                Model::Face* face = NULL;
                while ((face = parseFace(worldBounds)) != NULL)
                    faces.push_back(face);
                return !faces.empty();
            } catch (MapParserException e) {
                Utility::deleteAll(faces, oldSize);
                m_tokenizer.reset();
                return false;
            }
        }
    }
}
