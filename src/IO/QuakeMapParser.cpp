/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "QuakeMapParser.h"

#include "Exceptions.h"
#include "Logger.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceTypes.h"
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/QuakeEntityRotator.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"

namespace TrenchBroom {
    namespace IO {
        QuakeMapTokenizer::QuakeMapTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end) {}
        
        QuakeMapTokenizer::QuakeMapTokenizer(const String& str) :
        Tokenizer(str) {}

        QuakeMapTokenizer::Token QuakeMapTokenizer::emitToken() {
            while (!eof()) {
                size_t startLine = line();
                size_t startColumn = column();
                const char* c = nextChar();
                switch (*c) {
                    case '/':
                        if (peekChar() == '/')
                            discardUntil("\n\r");
                        break;
                    case '{':
                        return Token(QuakeMapToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                    case '}':
                        return Token(QuakeMapToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                    case '(':
                        return Token(QuakeMapToken::OParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case ')':
                        return Token(QuakeMapToken::CParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case '[':
                        return Token(QuakeMapToken::OBracket, c, c+1, offset(c), startLine, startColumn);
                    case ']':
                        return Token(QuakeMapToken::CBracket, c, c+1, offset(c), startLine, startColumn);
                    case '"': { // quoted string
                        const char* begin = nextChar();
                        const char* end = readQuotedString(begin);
                        return Token(QuakeMapToken::String, begin, end, offset(begin), startLine, startColumn);
                    }
                    default: { // whitespace, integer, decimal or word
                        if (isWhitespace(*c)) {
                            discardWhile(Whitespace);
                        } else {
                            const char* begin = c;
                            const char* end = readInteger(begin, Whitespace);
                            if (end > begin)
                                return Token(QuakeMapToken::Integer, begin, end, offset(begin), startLine, startColumn);
                            
                            end = readDecimal(begin, Whitespace);
                            if (end > begin)
                                return Token(QuakeMapToken::Decimal, begin, end, offset(begin), startLine, startColumn);
                            
                            end = readString(begin, Whitespace);
                            if (end == begin)
                                throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                            return Token(QuakeMapToken::String, begin, end, offset(begin), startLine, startColumn);
                        }
                    }
                }
            }
            return Token(QuakeMapToken::Eof, NULL, NULL, length(), line(), column());
        }

        QuakeMapParser::PlaneWeightOrder::PlaneWeightOrder(const bool deterministic) :
        m_deterministic(deterministic) {}

        QuakeMapParser::FaceWeightOrder::FaceWeightOrder(const PlaneWeightOrder& planeOrder) :
        m_planeOrder(planeOrder) {}

        bool QuakeMapParser::FaceWeightOrder::operator()(const Model::BrushFace* lhs, const Model::BrushFace* rhs) const  {
            return m_planeOrder(lhs->boundary(), rhs->boundary());
        }

        QuakeMapParser::QuakeMapParser(const char* begin, const char* end, Logger* logger) :
        m_logger(logger),
        m_tokenizer(QuakeMapTokenizer(begin, end)),
        m_format(Model::MFUnknown) {}
                    
        QuakeMapParser::QuakeMapParser(const String& str, Logger* logger) :
        m_logger(logger),
        m_tokenizer(QuakeMapTokenizer(str)),
        m_format(Model::MFUnknown) {}
        
        String QuakeMapParser::tokenName(const QuakeMapToken::Type typeMask) const {
            StringList names;
            if ((typeMask & QuakeMapToken::Integer) != 0)
                names.push_back("integer number");
            if ((typeMask & QuakeMapToken::Decimal) != 0)
                names.push_back("decimal number");
            if ((typeMask & QuakeMapToken::String) != 0)
                names.push_back("string");
            if ((typeMask & QuakeMapToken::OParenthesis) != 0)
                names.push_back("opening parenthesis");
            if ((typeMask & QuakeMapToken::CParenthesis) != 0)
                names.push_back("closing parenthesis");
            if ((typeMask & QuakeMapToken::OBrace) != 0)
                names.push_back("opening brace");
            if ((typeMask & QuakeMapToken::CBrace) != 0)
                names.push_back("closing brace");
            if ((typeMask & QuakeMapToken::OBracket) != 0)
                names.push_back("opening bracket");
            if ((typeMask & QuakeMapToken::CBracket) != 0)
                names.push_back("closing bracket");
            if ((typeMask & QuakeMapToken::Comment) != 0)
                names.push_back("comment");
            
            if (names.empty())
                return "unknown token type";
            if (names.size() == 1)
                return names[0];
            
            StringStream str;
            str << names[0];
            for (size_t i = 1; i < names.size() - 1; i++)
                str << ", " << names[i];
            if (names.size() > 2)
                str << ",";
            str << " or " << names[names.size() - 1];
            return str.str();
        }

        Model::Map* QuakeMapParser::doParseMap(const BBox3& worldBounds) {
            m_format = detectFormat();
            m_tokenizer.reset();
            
            Model::Map* map = new Model::Map(m_format);
            try {
                Model::Entity* entity = parseEntity(worldBounds);
                while (entity != NULL) {
                    map->addEntity(entity);
                    entity = parseEntity(worldBounds);
                }
                return map;
            } catch (...) {
                delete map;
                throw;
            }
        }

        Model::MapFormat QuakeMapParser::detectFormat() {
            // iterate over all entities
            while (true) {
                Token token = m_tokenizer.nextToken();
                while (token.type() != QuakeMapToken::OBrace && token.type() != QuakeMapToken::Eof)
                    token = m_tokenizer.nextToken();
                if (token.type() == QuakeMapToken::Eof)
                    return Model::MFQuake;
                // find a brush
                token = m_tokenizer.nextToken();
                while (token.type() != QuakeMapToken::OBrace && token.type() != QuakeMapToken::CBrace && token.type() != QuakeMapToken::Eof)
                    token = m_tokenizer.nextToken();
                if (token.type() == QuakeMapToken::Eof)
                    return Model::MFQuake;
                
                // we have a brush, now parse the first face
                if (token.type() == QuakeMapToken::OBrace) {
                    for (size_t i = 0; i < 3; ++i) {
                        expect(QuakeMapToken::OParenthesis, token = m_tokenizer.nextToken());
                        parseVector();
                        expect(QuakeMapToken::CParenthesis, token = m_tokenizer.nextToken());
                    }
                    
                    expect(QuakeMapToken::String, token = m_tokenizer.nextToken()); // texture name
                    expect(QuakeMapToken::Integer | QuakeMapToken::Decimal | QuakeMapToken::OBracket, token = m_tokenizer.nextToken());
                    if (token.type() == QuakeMapToken::OBracket)
                        return Model::MFValve;
                    expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // y offset
                    expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // rotation
                    expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // x scale
                    expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // y scale
                    expect(QuakeMapToken::Integer | QuakeMapToken::Decimal | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, token = m_tokenizer.nextToken());
                    if (token.type() == QuakeMapToken::OParenthesis || token.type() == QuakeMapToken::CBrace)
                        return Model::MFQuake;
                    expect(QuakeMapToken::Integer | QuakeMapToken::Decimal | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, token); // unknown Hexen 2 flag or Quake 2 surface contents
                    if (token.type() == QuakeMapToken::OParenthesis || token.type() == QuakeMapToken::CBrace)
                        return Model::MFHexen2;
                    return Model::MFQuake2;
                }
            }
            return Model::MFQuake;
        }

        Model::Entity* QuakeMapParser::parseEntity(const BBox3& worldBounds) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                return NULL;
            
            expect(QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
            if (token.type() == QuakeMapToken::CBrace)
                return NULL;
            
            Model::Entity* entity = new Model::ConfigurableEntity<Model::QuakeEntityRotationPolicy>();
            const size_t firstLine = token.line();
            
            try {
                while ((token = m_tokenizer.nextToken()).type() != QuakeMapToken::Eof) {
                    switch (token.type()) {
                        case QuakeMapToken::String: {
                            String key = token.data();
                            expect(QuakeMapToken::String, token = m_tokenizer.nextToken());
                            String value = token.data();
                            entity->addOrUpdateProperty(key, value);
                            break;
                        }
                        case QuakeMapToken::OBrace: {
                            m_tokenizer.pushToken(token);
                            bool moreBrushes = true;
                            while (moreBrushes) {
                                Model::Brush* brush = parseBrush(worldBounds);
                                if (brush != NULL)
                                    entity->addBrush(brush);
                                expect(QuakeMapToken::OBrace | QuakeMapToken::CBrace, token = m_tokenizer.nextToken());
                                moreBrushes = (token.type() == QuakeMapToken::OBrace);
                                m_tokenizer.pushToken(token);
                            }
                            break;
                        }
                        case QuakeMapToken::CBrace: {
                            entity->setFilePosition(firstLine, token.line() - firstLine);
                            return entity;
                        }
                        default:
                            expect(QuakeMapToken::String | QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
                    }
                }
            } catch (...) {
                delete entity;
                throw;
            }
            
            return entity;
        }
        
        Model::Brush* QuakeMapParser::parseBrush(const BBox3& worldBounds) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                return NULL;
            
            expect(QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
            if (token.type() == QuakeMapToken::CBrace)
                return NULL;
            
            const size_t firstLine = token.line();
            Model::BrushFaceList faces;
            
            while ((token = m_tokenizer.nextToken()).type() != QuakeMapToken::Eof) {
                switch (token.type()) {
                    case QuakeMapToken::OParenthesis: {
                        m_tokenizer.pushToken(token);
                        Model::BrushFace* face = parseFace(worldBounds);
                        if (face != NULL)
                            faces.push_back(face);
                        break;
                    }
                    case QuakeMapToken::CBrace: {
                        return createBrush(worldBounds, faces, firstLine, token.line() - firstLine);
                    }
                    default: {
                        expect(QuakeMapToken::OParenthesis | QuakeMapToken::CParenthesis, token);
                    }
                }
            }
            
            return NULL;
        }
        
        Model::BrushFace* QuakeMapParser::parseFace(const BBox3& worldBounds) {
            float xOffset, yOffset, rotation, xScale, yScale, surfaceValue;
            size_t surfaceContents, surfaceFlags;
            Vec3 texAxisX, texAxisY;
            Token token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                return NULL;
            
            expect(QuakeMapToken::OParenthesis, token);
            const Vec3 p1 = parseVector().corrected();
            expect(QuakeMapToken::CParenthesis, token = m_tokenizer.nextToken());
            expect(QuakeMapToken::OParenthesis, token = m_tokenizer.nextToken());
            const Vec3 p2 = parseVector().corrected();
            expect(QuakeMapToken::CParenthesis, token = m_tokenizer.nextToken());
            expect(QuakeMapToken::OParenthesis, token = m_tokenizer.nextToken());
            const Vec3 p3 = parseVector().corrected();
            expect(QuakeMapToken::CParenthesis, token = m_tokenizer.nextToken());
            
            expect(QuakeMapToken::String, token = m_tokenizer.nextToken());
            String textureName = token.data();
            if (textureName == Model::BrushFace::NoTextureName)
                textureName = "";
            
            const Vec3 normal = crossed(p3 - p1, p2 - p1).normalized();
            if (normal.null())
                return NULL;
            
            if (m_format == Model::MFValve) {
                expect(QuakeMapToken::OBracket, m_tokenizer.nextToken());
                texAxisX = parseVector();
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                xOffset = token.toFloat<float>();
                expect(QuakeMapToken::CBracket, m_tokenizer.nextToken());
                
                expect(QuakeMapToken::OBracket, m_tokenizer.nextToken());
                texAxisY = parseVector();
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                yOffset = token.toFloat<float>();
                expect(QuakeMapToken::CBracket, m_tokenizer.nextToken());
            } else {
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                xOffset = token.toFloat<float>();
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                yOffset = token.toFloat<float>();
            }

            expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
            rotation = token.toFloat<float>();
            expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
            xScale = token.toFloat<float>();
            expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
            yScale = token.toFloat<float>();
            
            Model::BrushFace* face = NULL;
            if (m_format == Model::MFValve)
                face = new Model::ValveBrushFace(p1, p2, p3, texAxisX, texAxisY, normal, rotation, textureName);
            else
                face = new Model::QuakeBrushFace(p1, p2, p3, textureName);

            if (m_format == Model::MFQuake2) {
                expect(QuakeMapToken::Integer, token = m_tokenizer.nextToken());
                surfaceContents = token.toInteger<size_t>();
                expect(QuakeMapToken::Integer, token = m_tokenizer.nextToken());
                surfaceFlags = token.toInteger<size_t>();
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                surfaceValue = token.toFloat<float>();
            } else if (m_format == Model::MFHexen2) {
                // noone seems to know what the extra face attribute in Hexen 2 maps does, so we discard it
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                surfaceContents = surfaceFlags = 0;
                surfaceValue = 0.0f;
            } else {
                surfaceContents = surfaceFlags = 0;
                surfaceValue = 0.0f;
            }
            
            face->setXOffset(xOffset);
            face->setYOffset(yOffset);
            face->setRotation(rotation);
            face->setXScale(xScale);
            face->setYScale(yScale);
            face->setSurfaceContents(surfaceContents);
            face->setSurfaceFlags(surfaceFlags);
            face->setSurfaceValue(surfaceValue);
            face->setFilePosition(token.line(), 1);
            
            return face;
        }
        
        Vec3 QuakeMapParser::parseVector() {
            Token token;
            Vec3 vec;
            
            for (size_t i = 0; i < 3; i++) {
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                vec[i] = token.toFloat<double>();
            }
            return vec;
        }

        Model::Brush* QuakeMapParser::createBrush(const BBox3& worldBounds, const Model::BrushFaceList faces, const size_t firstLine, const size_t lineCount) const {
            Model::Brush* brush = NULL;
            try {
                // sort the faces by the weight of their plane normals like QBSP does
                Model::BrushFaceList sortedFaces = faces;
                std::sort(sortedFaces.begin(), sortedFaces.end(), FaceWeightOrder(PlaneWeightOrder(true)));
                std::sort(sortedFaces.begin(), sortedFaces.end(), FaceWeightOrder(PlaneWeightOrder(false)));

                brush = new Model::Brush(worldBounds, sortedFaces);
                brush->setFilePosition(firstLine, lineCount);
            } catch (GeometryException& e) {
                if (m_logger != NULL)
                    m_logger->error("Error parsing brush at line %u: %s", firstLine, e.what());
                delete brush;
                brush = NULL;
            }
            return brush;
        }
    }
}
