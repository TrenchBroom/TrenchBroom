/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "SetBool.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/Map.h"
#include "Model/QuakeEntityRotationPolicy.h"
#include "Model/ParallelTexCoordSystem.h"
#include "Model/ParaxialTexCoordSystem.h"

namespace TrenchBroom {
    namespace IO {
        QuakeMapTokenizer::QuakeMapTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end),
        m_skipEol(true) {}
        
        QuakeMapTokenizer::QuakeMapTokenizer(const String& str) :
        Tokenizer(str),
        m_skipEol(true) {}

        void QuakeMapTokenizer::setSkipEol(bool skipEol) {
            m_skipEol = skipEol;
        }

        QuakeMapTokenizer::Token QuakeMapTokenizer::emitToken() {
            while (!eof()) {
                size_t startLine = line();
                size_t startColumn = column();
                const char* c = curPos();
                switch (*c) {
                    case '/':
                        advance();
                        if (curChar() == '/') {
                            advance();
                            if (curChar() == '/') {
                                advance();
                                return Token(QuakeMapToken::Comment, c, c+3, offset(c), startLine, startColumn);
                            }
                            discardUntil("\n\r");
                        }
                        break;
                    case '{':
                        advance();
                        return Token(QuakeMapToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        return Token(QuakeMapToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                    case '(':
                        advance();
                        return Token(QuakeMapToken::OParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case ')':
                        advance();
                        return Token(QuakeMapToken::CParenthesis, c, c+1, offset(c), startLine, startColumn);
                    case '[':
                        advance();
                        return Token(QuakeMapToken::OBracket, c, c+1, offset(c), startLine, startColumn);
                    case ']':
                        advance();
                        return Token(QuakeMapToken::CBracket, c, c+1, offset(c), startLine, startColumn);
                    case '"': { // quoted string
                        advance();
                        c = curPos();
                        const char* e = readQuotedString();
                        return Token(QuakeMapToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    case '\n':
                    case '\r':
                        if (!m_skipEol) {
                            advance();
                            return Token(QuakeMapToken::Eol, c, c+1, offset(c), startLine, startColumn);
                        }
                    case ' ':
                    case '\t':
                        discardWhile(Whitespace);
                        break;
                    default: { // whitespace, integer, decimal or word
                        const char* e = readInteger(Whitespace);
                        if (e != NULL)
                            return Token(QuakeMapToken::Integer, c, e, offset(c), startLine, startColumn);
                        
                        e = readDecimal(Whitespace);
                        if (e != NULL)
                            return Token(QuakeMapToken::Decimal, c, e, offset(c), startLine, startColumn);
                        
                        e = readString(Whitespace);
                        if (e == NULL)
                            throw ParserException(startLine, startColumn, "Unexpected character: " + String(c, 1));
                        return Token(QuakeMapToken::String, c, e, offset(c), startLine, startColumn);
                    }
                }
            }
            return Token(QuakeMapToken::Eof, NULL, NULL, length(), line(), column());
        }

        QuakeMapParser::PlaneWeightOrder::PlaneWeightOrder(const bool deterministic) :
        m_deterministic(deterministic) {}

        QuakeMapParser::FaceWeightOrder::FaceWeightOrder(const PlaneWeightOrder& planeOrder) :
        m_planeOrder(planeOrder) {}

        QuakeMapParser::ExtraProperty::ExtraProperty(Type type, const String& name, const String& value, const size_t line, const size_t column) :
        m_type(type),
        m_name(name),
        m_value(value),
        m_line(line),
        m_column(column) {}
        
        QuakeMapParser::ExtraProperty::Type QuakeMapParser::ExtraProperty::type() const {
            return m_type;
        }
        
        const String& QuakeMapParser::ExtraProperty::name() const {
            return m_name;
        }
        
        const String& QuakeMapParser::ExtraProperty::strValue() const {
            return m_value;
        }

        void QuakeMapParser::ExtraProperty::assertType(const Type expected) const {
            if (expected != m_type)
                throw ParserException(m_line, m_column, "Invalid extra property type");
        }

        bool QuakeMapParser::FaceWeightOrder::operator()(const Model::BrushFace* lhs, const Model::BrushFace* rhs) const  {
            return m_planeOrder(lhs->boundary(), rhs->boundary());
        }

        QuakeMapParser::QuakeMapParser(const char* begin, const char* end, Logger* logger) :
        m_logger(logger),
        m_tokenizer(QuakeMapTokenizer(begin, end)),
        m_format(Model::MapFormat::Unknown) {}
                    
        QuakeMapParser::QuakeMapParser(const String& str, Logger* logger) :
        m_logger(logger),
        m_tokenizer(QuakeMapTokenizer(str)),
        m_format(Model::MapFormat::Unknown) {}
        
        QuakeMapParser::TokenNameMap QuakeMapParser::tokenNames() const {
            using namespace QuakeMapToken;
            
            TokenNameMap names;
            names[Integer]      = "integer";
            names[Decimal]      = "decimal";
            names[String]       = "string";
            names[OParenthesis] = "'('";
            names[CParenthesis] = "')'";
            names[OBrace]       = "'{'";
            names[CBrace]       = "'}'";
            names[OBracket]     = "'['";
            names[CBracket]     = "']'";
            names[Comment]      = "comment";
            names[Eof]          = "end of file";
            return names;
        }

        Model::Map* QuakeMapParser::doParseMap(const BBox3& worldBounds) {
            setFormat(detectFormat());
            
            Model::Map* map = m_factory.createMap();
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

        Model::EntityList QuakeMapParser::doParseEntities(const BBox3& worldBounds, const Model::MapFormat::Type format) {
            Model::EntityList entities;
            try {
                setFormat(format);
                Model::Entity* entity = parseEntity(worldBounds);
                while (entity != NULL) {
                    entities.push_back(entity);
                    entity = parseEntity(worldBounds);
                }
            } catch (...) {
                VectorUtils::clearAndDelete(entities);
            }
            return entities;
        }
        
        Model::BrushList QuakeMapParser::doParseBrushes(const BBox3& worldBounds, const Model::MapFormat::Type format) {
            Model::BrushList brushes;
            try {
                setFormat(format);
                Model::Brush* brush = parseBrush(worldBounds);
                while (brush != NULL) {
                    brushes.push_back(brush);
                    brush = parseBrush(worldBounds);
                }
            } catch (...) {
                VectorUtils::clearAndDelete(brushes);
            }
            return brushes;
        }
        
        Model::BrushFaceList QuakeMapParser::doParseFaces(const BBox3& worldBounds, const Model::MapFormat::Type format) {
            Model::BrushFaceList faces;
            try {
                setFormat(format);
                Model::BrushFace* face = parseFace(worldBounds);
                while (face != NULL) {
                    faces.push_back(face);
                    face = parseFace(worldBounds);
                }
            } catch (...) {
                VectorUtils::clearAndDelete(faces);
            }
            return faces;
        }

        void QuakeMapParser::setFormat(Model::MapFormat::Type format) {
            assert(format != Model::MapFormat::Unknown);
            m_format = format;
            m_factory = Model::ModelFactory(format);
        }

        Model::MapFormat::Type QuakeMapParser::detectFormat() {
            Model::MapFormat::Type format = Model::MapFormat::Unknown;
            
            // try to find an opening parenthesis
            Token token = m_tokenizer.nextToken();
            while (token.type() != QuakeMapToken::OParenthesis &&
                   token.type() != QuakeMapToken::Eof)
                token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                format = Model::MapFormat::Quake;
            
            if (format == Model::MapFormat::Unknown) {
                m_tokenizer.pushToken(token);
                for (size_t i = 0; i < 3; ++i) {
                    expect(QuakeMapToken::OParenthesis, token = m_tokenizer.nextToken());
                    parseVector();
                    expect(QuakeMapToken::CParenthesis, token = m_tokenizer.nextToken());
                }
                
                expect(QuakeMapToken::String, token = m_tokenizer.nextToken()); // texture name
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal | QuakeMapToken::OBracket, token = m_tokenizer.nextToken());
                if (token.type() == QuakeMapToken::OBracket)
                    format = Model::MapFormat::Valve;
            }

            if (format == Model::MapFormat::Unknown) {
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // y offset
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // rotation
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // x scale
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken()); // y scale
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, token = m_tokenizer.nextToken());
                if (token.type() == QuakeMapToken::OParenthesis || token.type() == QuakeMapToken::CBrace)
                    format = Model::MapFormat::Quake;
            }

            if (format == Model::MapFormat::Unknown) {
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal | QuakeMapToken::OParenthesis | QuakeMapToken::CBrace, token = m_tokenizer.nextToken()); // unknown Hexen 2 flag or Quake 2 surface contents
                if (token.type() == QuakeMapToken::OParenthesis || token.type() == QuakeMapToken::CBrace)
                    format = Model::MapFormat::Hexen2;
            }
            
            if (format == Model::MapFormat::Unknown)
                format = Model::MapFormat::Quake2;
            
            m_tokenizer.reset();
            return format;
        }

        Model::Entity* QuakeMapParser::parseEntity(const BBox3& worldBounds) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                return NULL;
            
            expect(QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
            if (token.type() == QuakeMapToken::CBrace)
                return NULL;
            
            ExtraProperties extraProperties;
            Model::Entity* entity = m_factory.createEntity();
            const size_t firstLine = token.line();
            
            try {
                while ((token = m_tokenizer.nextToken()).type() != QuakeMapToken::Eof) {
                    switch (token.type()) {
                        case QuakeMapToken::Comment: {
                            parseExtraProperties(extraProperties);
                            break;
                        }
                        case QuakeMapToken::String:
                            m_tokenizer.pushToken(token);
                            parseEntityProperty(entity);
                            break;
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
                            setExtraObjectProperties(entity, extraProperties);
                            return entity;
                        }
                        default:
                            expect(QuakeMapToken::Comment | QuakeMapToken::String | QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
                    }
                }
            } catch (...) {
                delete entity;
                throw;
            }
            
            return entity;
        }
        
        void QuakeMapParser::parseEntityProperty(Model::Entity* entity) {
            Token token = m_tokenizer.nextToken();
            assert(token.type() == QuakeMapToken::String);
            const String key = token.data();
            
            expect(QuakeMapToken::String, token = m_tokenizer.nextToken());
            const String value = token.data();
            entity->addOrUpdateProperty(key, value);
        }

        Model::Brush* QuakeMapParser::parseBrush(const BBox3& worldBounds) {
            Token token = m_tokenizer.nextToken();
            if (token.type() == QuakeMapToken::Eof)
                return NULL;
            
            expect(QuakeMapToken::OBrace | QuakeMapToken::CBrace, token);
            if (token.type() == QuakeMapToken::CBrace)
                return NULL;

            ExtraProperties extraProperties;
            const size_t firstLine = token.line();
            
            Model::BrushFaceList faces;
            while ((token = m_tokenizer.nextToken()).type() != QuakeMapToken::Eof) {
                switch (token.type()) {
                    case QuakeMapToken::Comment: {
                        parseExtraProperties(extraProperties);
                        break;
                    }
                    case QuakeMapToken::OParenthesis: {
                        m_tokenizer.pushToken(token);
                        Model::BrushFace* face = parseFace(worldBounds);
                        if (face != NULL)
                            faces.push_back(face);
                        break;
                    }
                    case QuakeMapToken::CBrace: {
                        return createBrush(worldBounds, faces, extraProperties, firstLine, token.line() - firstLine);
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
            int surfaceContents = 0;
            int  surfaceFlags = 0;
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
            
            if (m_format == Model::MapFormat::Valve) {
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
            
            Model::BrushFace* face = m_factory.createFaceWithAxes(p1, p2, p3, textureName, texAxisX, texAxisY);

            if (m_format == Model::MapFormat::Quake2) {
                expect(QuakeMapToken::Integer, token = m_tokenizer.nextToken());
                surfaceContents = token.toInteger<int>();
                expect(QuakeMapToken::Integer, token = m_tokenizer.nextToken());
                surfaceFlags = token.toInteger<int>();
                expect(QuakeMapToken::Integer | QuakeMapToken::Decimal, token = m_tokenizer.nextToken());
                surfaceValue = token.toFloat<float>();
            } else if (m_format == Model::MapFormat::Hexen2) {
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

        void QuakeMapParser::parseExtraProperties(ExtraProperties& properties) {
            const SetBoolFun<QuakeMapTokenizer> parseEof(&m_tokenizer, &QuakeMapTokenizer::setSkipEol, false);
            Token token = m_tokenizer.nextToken();
            expect(QuakeMapToken::String | QuakeMapToken::Eol | QuakeMapToken::Eof, token);
            while (token.type() != QuakeMapToken::Eol && token.type() != QuakeMapToken::Eof) {
                const String name = token.data();
                expect(QuakeMapToken::String | QuakeMapToken::Integer, token = m_tokenizer.nextToken());
                const String value = token.data();
                const ExtraProperty::Type type = token.type() == QuakeMapToken::String ? ExtraProperty::Type_String : ExtraProperty::Type_Integer;
                properties.insert(std::make_pair(name, ExtraProperty(type, name, value, token.line(), token.column())));
                expect(QuakeMapToken::String | QuakeMapToken::Eol | QuakeMapToken::Eof, token = m_tokenizer.nextToken());
            }
        }

        void QuakeMapParser::setExtraObjectProperties(Model::Object* object, const ExtraProperties properties) const {
            ExtraProperties::const_iterator it;
            it = properties.find("hideIssues");
            if (it != properties.end()) {
                const ExtraProperty& property = it->second;
                property.assertType(ExtraProperty::Type_Integer);
                const Model::IssueType mask = property.intValue<Model::IssueType>();
                object->setHiddenIssues(mask);
            }
        }

        Model::Brush* QuakeMapParser::createBrush(const BBox3& worldBounds, const Model::BrushFaceList faces, const ExtraProperties& extraProperties, const size_t firstLine, const size_t lineCount) const {
            Model::Brush* brush = NULL;
            try {
                // sort the faces by the weight of their plane normals like QBSP does
                Model::BrushFaceList sortedFaces = faces;
                std::sort(sortedFaces.begin(), sortedFaces.end(), FaceWeightOrder(PlaneWeightOrder(true)));
                std::sort(sortedFaces.begin(), sortedFaces.end(), FaceWeightOrder(PlaneWeightOrder(false)));

                brush = m_factory.createBrush(worldBounds, sortedFaces);
                brush->setFilePosition(firstLine, lineCount);
                setExtraObjectProperties(brush, extraProperties);
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
