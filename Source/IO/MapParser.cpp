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
            const size_t startPosition = tokenizer.position();
            while (!tokenizer.eof()) {
                char c = tokenizer.nextChar();
                switch (c) {
                    case '/':
                        if (tokenizer.peekChar() == '/') {
                            tokenizer.nextChar();
                            if (tokenizer.peekChar() == '/') {
                                tokenizer.nextChar(); // it's a TB comment
                            } else {
                                // eat everything up to and including the next newline
                                while (tokenizer.nextChar() != '\n');
                            }
                        }
                        break;
                    case '{':
                        return Token(TokenType::OBrace, "", startPosition, tokenizer.position() - startPosition, tokenizer.line(), tokenizer.column());
                    case '}':
                        return Token(TokenType::CBrace, "", startPosition, tokenizer.position() - startPosition, tokenizer.line(), tokenizer.column());
                    case '(':
                        return Token(TokenType::OParenthesis, "", startPosition, tokenizer.position() - startPosition, tokenizer.line(), tokenizer.column());
                    case ')':
                        return Token(TokenType::CParenthesis, "", startPosition, tokenizer.position() - startPosition, tokenizer.line(), tokenizer.column());
                    case '[':
                        return Token(TokenType::OBracket, "", startPosition, tokenizer.position() - startPosition, tokenizer.line(), tokenizer.column());
                    case ']':
                        return Token(TokenType::CBracket, "", startPosition, tokenizer.position() - startPosition, tokenizer.line(), tokenizer.column());
                    case '"': { // quoted string
                        m_bufferLen = 0;
                        size_t line = tokenizer.line();
                        size_t column = tokenizer.column();
                        while (!tokenizer.eof() && (c = tokenizer.nextChar()) != '"')
                            m_buffer[m_bufferLen++] = c;
                        return Token(TokenType::String, String(m_buffer, m_bufferLen), startPosition, tokenizer.position() - startPosition, line, column);
                    }
                    default: { // whitespace, integer, decimal or word
                        if (isWhitespace(c))
                            break;
                        
                        size_t line = tokenizer.line();
                        size_t column = tokenizer.column();

                        // clear the buffer
                        m_bufferLen = 0;
                        
                        // try to read a number
                        if (c == '-' || isDigit(c)) {
                            m_buffer[m_bufferLen++] = c;
                            while (isDigit((c = tokenizer.nextChar())))
                                m_buffer[m_bufferLen++] = c;
                            if (isDelimiter(c)) {
                                if (!tokenizer.eof())
                                    tokenizer.pushChar();
                                return Token(TokenType::Integer, String(m_buffer, m_bufferLen), startPosition, tokenizer.position() - startPosition, line, column);
                            }
                        }
                        
                        // try to read a decimal (may start with '.')
                        if (c == '.') {
                            m_buffer[m_bufferLen++] = c;
                            while (isDigit((c = tokenizer.nextChar())))
                                m_buffer[m_bufferLen++] = c;
                            if (isDelimiter(c)) {
                                if (!tokenizer.eof())
                                    tokenizer.pushChar();
                                return Token(TokenType::Decimal, String(m_buffer, m_bufferLen), startPosition, tokenizer.position() - startPosition, line, column);
                            }
                        }
                        
                        // try to read decimal in scientific notation
                        if (c == 'e') {
                            m_buffer[m_bufferLen++] = c;
                            c = tokenizer.nextChar();
                            if (isDigit(c) || c == '+' || c == '-') {
                                m_buffer[m_bufferLen++] = c;
                                while (isDigit((c = tokenizer.nextChar())))
                                    m_buffer[m_bufferLen++] = c;
                                if (isDelimiter(c)) {
                                    if (!tokenizer.eof())
                                        tokenizer.pushChar();
                                    return Token(TokenType::Decimal, String(m_buffer, m_bufferLen), startPosition, tokenizer.position() - startPosition, line, column);
                                }
                            }
                        }
                        
                        // read a word
                        m_buffer[m_bufferLen++] = c;
                        while (!tokenizer.eof() && !isDelimiter(c = tokenizer.nextChar()))
                            m_buffer[m_bufferLen++] = c;
                        if (!tokenizer.eof())
                            tokenizer.pushChar();
                        return Token(TokenType::String, String(m_buffer, m_bufferLen), startPosition, tokenizer.position() - startPosition, line, column);
                    }
                }
            }
            return Token(TokenType::Eof, "", startPosition, tokenizer.position() - startPosition, tokenizer.line(), tokenizer.column());
        }
        
        Model::BrushGeometry* MapParser::buildGeometry(const BBox& worldBounds, const Model::FaceList& faces) {
            // sort the faces by the weight of their plane normals like QBSP does
            Model::FaceList sortedFaces = faces;
            std::sort(sortedFaces.begin(), sortedFaces.end(), Model::Face::WeightOrder(Plane::WeightOrder(true)));
            std::sort(sortedFaces.begin(), sortedFaces.end(), Model::Face::WeightOrder(Plane::WeightOrder(false)));
            
            Model::FaceList droppedFaces;
            Model::BrushGeometry* geometry = new Model::BrushGeometry(worldBounds);
            
            Model::FaceList::iterator faceIt, faceEnd;
            for (faceIt = sortedFaces.begin(), faceEnd = sortedFaces.end(); faceIt != faceEnd; ++faceIt) {
                Model::Face* face = *faceIt;
                if (!geometry->addFace(*face, droppedFaces)) {
                    delete geometry;
                    return NULL;
                }
            }
            
            return geometry;
        }

        bool MapParser::parseGeometry(const BBox& worldBounds, const Model::FaceList& faces, Model::BrushGeometry*& geometry) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == TokenType::Eof)
                return false;
            
            expect(TokenType::String, token);
            if (token.data() != "VertexData")
                throw MapParserException(token, "expected VertexData");

            Model::VertexList vertices;
            Model::EdgeList edges;
            Model::SideList sides;
            
            for (size_t i = 0; i < faces.size(); i++) {
                Model::Side* side = new Model::Side();
                side->face = faces[i];
                sides.push_back(side);
            }
            
            try {
                expect(TokenType::OBrace, token = m_tokenizer.nextToken());
                expect(TokenType::CBrace | TokenType::OParenthesis, token = m_tokenizer.nextToken());
                while (token.type() != TokenType::CBrace) {
                    expect(TokenType::Decimal | TokenType::Integer, token = m_tokenizer.nextToken());
                    const float x = token.toFloat();
                    expect(TokenType::Decimal | TokenType::Integer, token = m_tokenizer.nextToken());
                    const float y = token.toFloat();
                    expect(TokenType::Decimal | TokenType::Integer, token = m_tokenizer.nextToken());
                    const float z = token.toFloat();
                    expect(TokenType::CParenthesis, token = m_tokenizer.nextToken());
                    expect(TokenType::CBrace | TokenType::OParenthesis, token = m_tokenizer.nextToken());
                    
                    vertices.push_back(new Model::Vertex(x, y, z));
                }
                
                expect(TokenType::OBrace, token = m_tokenizer.nextToken());
                expect(TokenType::CBrace | TokenType::OParenthesis, token = m_tokenizer.nextToken());
                while (token.type() != TokenType::CBrace) {
                    expect(TokenType::Integer, token = m_tokenizer.nextToken());
                    size_t startIndex = static_cast<size_t>(token.toInteger());
                    expect(TokenType::Integer, token = m_tokenizer.nextToken());
                    size_t endIndex = static_cast<size_t>(token.toInteger());
                    expect(TokenType::Integer, token = m_tokenizer.nextToken());
                    size_t leftIndex = static_cast<size_t>(token.toInteger());
                    expect(TokenType::Integer, token = m_tokenizer.nextToken());
                    size_t rightIndex = static_cast<size_t>(token.toInteger());
                    expect(TokenType::CParenthesis, token = m_tokenizer.nextToken());
                    expect(TokenType::CBrace | TokenType::OParenthesis, token = m_tokenizer.nextToken());
                    
                    assert(startIndex < vertices.size());
                    assert(endIndex < vertices.size());
                    assert(leftIndex < sides.size());
                    assert(rightIndex < sides.size());
                    
                    Model::Vertex* start = vertices[startIndex];
                    Model::Vertex* end = vertices[endIndex];
                    Model::Side* left = sides[leftIndex];
                    Model::Side* right = sides[rightIndex];
                    
                    edges.push_back(new Model::Edge(start, end, left, right));
                }
                
                expect(TokenType::OBrace, token = m_tokenizer.nextToken());
                expect(TokenType::CBrace | TokenType::OParenthesis, token = m_tokenizer.nextToken());
                
                size_t sideIndex = 0;
                while (token.type() != TokenType::CBrace) {
                    if (sideIndex < sides.size()) {
                        Model::Side* side = sides[sideIndex];
                        while (token.type() != TokenType::CParenthesis) {
                            expect(TokenType::Integer, token = m_tokenizer.nextToken());
                            size_t vertexIndex = static_cast<size_t>(token.toInteger());
                            expect(TokenType::Integer, token = m_tokenizer.nextToken());
                            size_t edgeIndex = static_cast<size_t>(token.toInteger());
                            
                            expect(TokenType::Integer | TokenType::CParenthesis, token = m_tokenizer.nextToken());
                            if (token.type() == TokenType::Integer)
                                m_tokenizer.pushToken(token);
                            
                            assert(vertexIndex < vertices.size());
                            assert(edgeIndex < edges.size());
                            
                            side->vertices.push_back(vertices[vertexIndex]);
                            side->edges.push_back(edges[edgeIndex]);
                        }
                    }
                    sideIndex++;
                    expect(TokenType::CBrace | TokenType::OParenthesis, token = m_tokenizer.nextToken());
                }
                
                // build a brush geometry from the faces to test against
                Model::BrushGeometry* testGeometry = buildGeometry(worldBounds, faces);
                if (testGeometry == NULL) {
                    deleteAll(vertices);
                    deleteAll(edges);
                    deleteAll(sides);
                    return false;
                }

                if (sideIndex > sides.size()) {
                    deleteAll(vertices);
                    deleteAll(edges);
                    deleteAll(sides);
                    geometry = testGeometry;
                    return false;
                }
                
                for (size_t i = 0; i < vertices.size(); i++) {
                    const Model::Vertex& loaded = *vertices[i];
                    bool foundMatch = false;
                    for (size_t j = 0; j < testGeometry->vertices.size() && !foundMatch; j++) {
                        const Model::Vertex& computed = *testGeometry->vertices[j];
                        if (loaded.position.equals(computed.position, 0.01f))
                            foundMatch = true;
                    }
                    if (!foundMatch) {
                        deleteAll(vertices);
                        deleteAll(edges);
                        deleteAll(sides);
                        geometry = testGeometry;
                        return false;
                    }
                }
                
                delete testGeometry;

                for (size_t i = 0; i < faces.size(); i++)
                    sides[i]->face = faces[i];
                geometry = new Model::BrushGeometry(vertices, edges, sides);
                return true;
            } catch (MapParserException e) {
                deleteAll(vertices);
                deleteAll(edges);
                deleteAll(sides);
                throw e;
            }
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
            
            if (!m_staleBrushes.empty()) {
                StringStream stream;
                stream << "Found brushes with invalid or missing geometry at lines ";
                for (size_t i = 0; i < m_staleBrushes.size(); i++) {
                    const Model::Brush* brush = m_staleBrushes[i];
                    stream << brush->filePosition();
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
            
            const size_t filePosition = token.line();
            Model::FaceList faces;
            Model::BrushGeometry* geometry = NULL;
            bool hasGeometry = false;
            
            while ((token = m_tokenizer.nextToken()).type() != TokenType::Eof) {
                switch (token.type()) {
                    case TokenType::OParenthesis: {
                        m_tokenizer.pushToken(token);
                        Model::Face* face = parseFace(worldBounds);
                        if (face != NULL)
                            faces.push_back(face);
                        break;
                    }
                    case TokenType::String: {
                        m_tokenizer.pushToken(token);
                        hasGeometry = parseGeometry(worldBounds, faces, geometry);
                        break;
                    }
                    case TokenType::CBrace: {
                        if (indicator != NULL) indicator->update(static_cast<int>(token.position()));
                        
                        Model::Brush* brush = NULL;
                        if (!hasGeometry)
                            geometry = buildGeometry(worldBounds, faces);
                        if (geometry == NULL) {
                            m_console.warn("Skipping malformed brush at line %i", filePosition);
                            Utility::deleteAll(faces);
                        } else {
                            brush = new Model::Brush(worldBounds, faces, geometry);
                            brush->setFilePosition(filePosition);
                            if (!hasGeometry)
                                m_staleBrushes.push_back(brush);
                        }
                        if (brush != NULL && !brush->closed())
                            m_console.warn("Non-closed brush at line %i", filePosition);
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
