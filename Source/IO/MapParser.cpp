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
        Token MapTokenEmitter::doEmit(Tokenizer& tokenizer) {
            while (!tokenizer.eof()) {
                size_t line = tokenizer.line();
                size_t column = tokenizer.column();
                const char* c = tokenizer.nextChar();
                switch (*c) {
                    case '/':
                        if (tokenizer.peekChar() == '/') {
                            tokenizer.nextChar();
                            if (tokenizer.peekChar() == '/') {
                                tokenizer.nextChar(); // it's a TB comment
                            } else {
                                // eat everything up to and including the next newline
                                while (*tokenizer.nextChar() != '\n');
                            }
                        }
                        break;
                    case '{':
                        return Token(TokenType::OBrace, c, c + 1, tokenizer.offset(c), line, column);
                    case '}':
                        return Token(TokenType::CBrace, c, c + 1, tokenizer.offset(c), line, column);
                    case '(':
                        return Token(TokenType::OParenthesis, c, c + 1, tokenizer.offset(c), line, column);
                    case ')':
                        return Token(TokenType::CParenthesis, c, c + 1, tokenizer.offset(c), line, column);
                    case '[':
                        return Token(TokenType::OBracket, c, c + 1, tokenizer.offset(c), line, column);
                    case ']':
                        return Token(TokenType::CBracket, c, c + 1, tokenizer.offset(c), line, column);
                    case '"': { // quoted string
                        const char* begin = c;
                        const char* end;
                        tokenizer.quotedString(begin, end);
                        return Token(TokenType::String, begin, end, tokenizer.offset(begin), line, column);
                    }
                    default: { // whitespace, integer, decimal or word
                        if (isWhitespace(*c))
                            break;
                        
                        const char* begin = c;

                        // try to read a number
                        if (*c == '-' || isDigit(*c)) {
                            while (isDigit(*(c = tokenizer.nextChar())));
                            if (isDelimiter(*c)) {
                                if (!tokenizer.eof())
                                    tokenizer.pushChar();
                                return Token(TokenType::Integer, begin, c, tokenizer.offset(begin), line, column);
                            }
                        }
                        
                        // try to read a decimal (may start with '.')
                        if (*c == '.') {
                            while (isDigit(*(c = tokenizer.nextChar())));
                            if (isDelimiter(*c)) {
                                if (!tokenizer.eof())
                                    tokenizer.pushChar();
                                return Token(TokenType::Decimal, begin, c, tokenizer.offset(begin), line, column);
                            }
                        }
                        
                        // try to read decimal in scientific notation
                        if (*c == 'e') {
                            c = tokenizer.nextChar();
                            if (isDigit(*c) || *c == '+' || *c == '-') {
                                while (isDigit(*(c = tokenizer.nextChar())));
                                if (isDelimiter(*c)) {
                                    if (!tokenizer.eof())
                                        tokenizer.pushChar();
                                    return Token(TokenType::Decimal, begin, c, tokenizer.offset(begin), line, column);
                                }
                            }
                        }
                        
                        // read a word
                        while (!tokenizer.eof() && !isDelimiter(*(c = tokenizer.nextChar())));
                        if (!tokenizer.eof())
                            tokenizer.pushChar();
                        return Token(TokenType::String, begin, c, tokenizer.offset(begin), line, column);
                    }
                }
            }
            return Token(TokenType::Eof, NULL, NULL, 0, tokenizer.line(), tokenizer.column());
        }
        
        MapParser::MapParser(const char* begin, const char* end, Utility::Console& console) :
        m_console(console),
        m_tokenizer(begin, end),
        m_format(Undefined),
        m_size(static_cast<size_t>(end - begin)) {
            assert(end >= begin);
        }

        MapParser::MapParser(const String& str, Utility::Console& console) :
        m_console(console),
        m_tokenizer(str.c_str(), str.c_str() + str.size()),
        m_format(Undefined),
        m_size(str.size()) {}

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
            
            if (!m_staleBrushes.empty()) {
                StringStream stream;
                stream << "Found brushes with invalid or missing geometry at lines ";
                for (size_t i = 0; i < m_staleBrushes.size(); i++) {
                    const Model::Brush* brush = m_staleBrushes[i];
                    stream << brush->fileLine();
                    if (i < m_staleBrushes.size() - 1)
                        stream << ", ";
                }
                m_console.warn(stream.str());
            }
        }
        
        Model::Entity* MapParser::parseEntity(const BBox& worldBounds, Utility::ProgressIndicator* indicator) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == TokenType::Eof)
                return NULL;
            
            expect(TokenType::OBrace | TokenType::CBrace, token);
            if (token.type() == TokenType::CBrace)
                return NULL;
            
            Model::Entity* entity = new Model::Entity(worldBounds);
            size_t firstLine = token.line();
            
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
                        entity->setFilePosition(firstLine, token.line() - firstLine);
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
            
            const size_t firstLine = token.line();
            Model::FaceList faces;
            
            while ((token = m_tokenizer.nextToken()).type() != TokenType::Eof) {
                switch (token.type()) {
                    case TokenType::OParenthesis: {
                        m_tokenizer.pushToken(token);
                        Model::Face* face = parseFace(worldBounds);
                        if (face != NULL)
                            faces.push_back(face);
                        break;
                    }
                    case TokenType::CBrace: {
                        if (indicator != NULL) indicator->update(static_cast<int>(token.position()));
                        
                        Model::Brush* brush = new Model::Brush(worldBounds);

                        // sort the faces by the weight of their plane normals like QBSP does
                        Model::FaceList sortedFaces = faces;
                        std::sort(sortedFaces.begin(), sortedFaces.end(), Model::Face::WeightOrder(Plane::WeightOrder(true)));
                        std::sort(sortedFaces.begin(), sortedFaces.end(), Model::Face::WeightOrder(Plane::WeightOrder(false)));
                        
                        Model::FaceList::iterator faceIt = sortedFaces.begin();
                        Model::FaceList::iterator faceEnd = sortedFaces.end();
                        while (faceIt != faceEnd) {
                            Model::Face* face = *faceIt++;
                            if (!brush->addFace(face)) {
                                m_console.warn("Skipping malformed brush at line %i", firstLine);
                                delete brush;
                                brush = NULL;
                                break;
                            }
                        }
                        
                        // if something went wrong, we must delete all faces that have not been added to the brush yet
                        if (faceIt != faceEnd)
                            Utility::deleteAll(sortedFaces, faceIt);
                        
                        if (brush != NULL) {
                            brush->setFilePosition(firstLine, token.line() - firstLine);
                            if (!brush->closed())
                                m_console.warn("Non-closed brush at line %i", firstLine);
                        }
                        return brush;
                    }
                    default: {
                        Utility::deleteAll(faces);
                        throw MapParserException(token, TokenType::OParenthesis | TokenType::CParenthesis);
                    }
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
