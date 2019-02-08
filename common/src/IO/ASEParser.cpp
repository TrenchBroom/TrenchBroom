/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "ASEParser.h"

#include "IO/Path.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace IO {
        AseTokenizer::AseTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end, "", 0){ }

        AseTokenizer::AseTokenizer(const String& str) :
        Tokenizer(str, "", 0) {}

        const String AseTokenizer::WordDelims = " \t\n\r";

        Tokenizer<unsigned int>::Token AseTokenizer::emitToken() {
            while (!eof()) {
                auto startLine = line();
                auto startColumn = column();
                const auto* c = curPos();

                switch (*c) {
                    case '*': {
                        advance();
                        c = curPos();
                        const auto* e = readUntil(WordDelims);
                        return Token(AseToken::Directive, c, e, offset(c), startLine, startColumn);
                    }
                    case '{':
                        advance();
                        return Token(AseToken::OBrace, c, c+1, offset(c), startLine, startColumn);
                    case '}':
                        advance();
                        return Token(AseToken::CBrace, c, c+1, offset(c), startLine, startColumn);
                    case '"': { // quoted string
                        advance();
                        c = curPos();
                        const auto* e = readQuotedString();
                        return Token(AseToken::String, c, e, offset(c), startLine, startColumn);
                    }
                    default: {
                        const auto* e = readInteger(WordDelims);
                        if (e != nullptr) {
                            return Token(AseToken::Integer, c, e, offset(c), startLine, startColumn);
                        }

                        e = readDecimal(WordDelims);
                        if (e != nullptr) {
                            return Token(AseToken::Decimal, c, e, offset(c), startLine, startColumn);
                        }

                        // must be a keyword
                        e = readUntil(WordDelims);
                        if (e != nullptr) {
                            return Token(AseToken::Keyword, c, e, offset(c), startLine, startColumn);
                        }
                        throw ParserException(startLine, startColumn, "Unexpected character: '" + String(c, 1) + "'");
                    }
                }
            }
            return Token(AseToken::Eof, nullptr, nullptr, length(), line(), column());
        }

        Assets::EntityModel* ASEParser::doParseModel(Logger& logger) {
            return nullptr;
        }

        void ASEParser::parseAseFile(Logger& logger) {
            expectDirective("3DSMAX_ASCIIEXPORT");
            expect(AseToken::Integer, m_tokenizer.nextToken());

            parseScene(logger);
            parseMaterialList(logger);
            parseGeomObject(logger);
        }

        void ASEParser::parseScene(Logger& logger) {
            expectDirective("Scene");
            parseBlock({});
        }

        void ASEParser::parseMaterialList(Logger& logger) {
            expectDirective("MATERIAL_LIST");
            parseBlock({
                { "MATERIAL_COUNT", std::bind(&ASEParser::parseMaterialListMaterialCount, this, logger) }
                { "MATERIAL", std::bind(&ASEParser::parseMaterialListMaterial, this, logger) }
            });
        }

        void ASEParser::parseMaterialListMaterialCount(Logger& logger) {
            expectDirective("MATERIAL_COUNT");
            parseSizeArgument();
        }

        void ASEParser::parseMaterialListMaterial(Logger& logger) {
            expectDirective("MATERIAL");
            parseSizeArgument();

            parseBlock({
                { "MAP_DIFFUSE", std::bind(&ASEParser::parseMaterialListMaterialMapDiffuse, this, logger) }
            });
        }

        void ASEParser::parseMaterialListMaterialMapDiffuse(Logger& logger) {
            expectDirective("MAP_DIFFUSE");

            parseBlock({
                { "BITMAP", std::bind(&ASEParser::parseMaterialListMaterialMapDiffuseBitmap, this, logger) }
            });
        }

        void ASEParser::parseMaterialListMaterialMapDiffuseBitmap(Logger& logger) {
            expectDirective("BITMAP");
            const auto token = expect(AseToken::String, m_tokenizer.nextToken());
            const auto path = Path(token.data());
        }

        void ASEParser::parseGeomObject(Logger& logger) {
            expectDirective("GEOMOBJECT");

            parseBlock({
                { "NODE_NAME", std::bind(&ASEParser::parseGeomObjectNodeName, this, logger) },
                { "MESH", std::bind(&ASEParser::parseGeomObjectMesh, this, logger) },
            });
        }

        void ASEParser::parseGeomObjectNodeName(Logger& logger) {
            expectDirective("NODE_NAME");
            const auto token = expect(AseToken::String, m_tokenizer.nextToken());
            const auto name = token.data();
        }

        void ASEParser::parseGeomObjectMesh(Logger& logger) {
            expectDirective("MESH");

            std::vector<vm::vec3f> vertices;

            parseBlock({
                { "MESH_NUMVERTEX", std::bind(&ASEParser::parseGeomObjectMeshNumVertex, this, logger, vertices) },
                { "MESH_VERTEX_LIST", std::bind(&ASEParser::parseGeomObjectMeshVertexList, this, logger, vertices) },
                { "MESH_NUMFACES", std::bind(&ASEParser::parseGeomObjectMeshNumFaces, this, logger) },
                { "MESH_FACE_LIST", std::bind(&ASEParser::parseGeomObjectMeshFaceList, this, logger) },
                { "MESH_NUMTVERTEX", std::bind(&ASEParser::parseGeomObjectMeshNumTVertex, this, logger) },
                { "MESH_TVERTLIST", std::bind(&ASEParser::parseGeomObjectMeshTVertexList, this, logger) },
                { "MESH_NUMTVFACES", std::bind(&ASEParser::parseGeomObjectMeshNumTVFaces, this, logger) },
                { "MESH_TFACELIST", std::bind(&ASEParser::parseGeomObjectMeshTFaceList, this, logger) },
            });
        }

        void ASEParser::parseGeomObjectMeshNumVertex(Logger& logger, std::vector<vm::vec3f>& vertices) {
            expectDirective("MESH_NUMVERTEX");
            const auto vertexCount = parseSizeArgument();
            vertices.reserve(vertexCount);
        }

        void ASEParser::parseGeomObjectMeshVertexList(Logger& logger, std::vector<vm::vec3f>& vertices) {

        }

        void ASEParser::parseGeomObjectMeshNumFaces(Logger& logger) {
            expectDirective("MESH_NUMFACES");
            const auto faceCount = parseSizeArgument();
        }

        void ASEParser::parseGeomObjectMeshFaceList(Logger& logger) {

        }

        void ASEParser::parseGeomObjectMeshNumTVertex(Logger& logger) {

        }

        void ASEParser::parseGeomObjectMeshTVertexList(Logger& logger) {

        }

        void ASEParser::parseGeomObjectMeshNumTVFaces(Logger& logger) {

        }

        void ASEParser::parseGeomObjectMeshTFaceList(Logger& logger) {

        }

        void ASEParser::parseBlock(const std::map<std::string, std::function<void(void)>>& handlers) {
            expect(AseToken::OBrace, m_tokenizer.nextToken());
            auto token = m_tokenizer.peekToken();
            while (token.hasType(AseToken::Directive)) {
                const auto it = handlers.find(token.data());
                if (it != std::end(handlers)) {
                    auto& handler = it->second;
                    handler();
                } else {
                    skipDirective();
                }
                token = m_tokenizer.peekToken();
            }
            expect(AseToken::CBrace, m_tokenizer.nextToken());
        }

        void ASEParser::expectDirective(const String& name) {
            auto token = expect(AseToken::Directive, m_tokenizer.nextToken());
            expect(name, token);
        }

        void ASEParser::skipDirective() {
            expect(AseToken::Directive, m_tokenizer.nextToken());

            // skip arguments
            while (!m_tokenizer.peekToken().hasType(AseToken::OBrace | AseToken::Directive)) {
                m_tokenizer.nextToken();
            }

            // skip block
            if (m_tokenizer.peekToken().hasType(AseToken::OBrace)) {
                expect(AseToken::OBrace, m_tokenizer.nextToken());
                while (!m_tokenizer.peekToken().hasType(AseToken::CBrace)) {
                    skipDirective();
                }
                expect(AseToken::CBrace, m_tokenizer.nextToken());
            }
        }

        void ASEParser::expectSizeArgument(const size_t expected) {
            const auto token = m_tokenizer.peekToken();
            const auto actual = parseSizeArgument();
            if (actual != expected) {
                throw ParserException(token.line(), token.column()) << "Expected value '" << expected << "', but got '" << actual + "'";
            }
        }

        size_t ASEParser::parseSizeArgument() {
            const auto token = expect(AseToken::Integer, m_tokenizer.nextToken());
            auto i = token.toInteger<int>();
            if (i < 0) {
                throw ParserException(token.line(), token.column()) << "Expected positive integer, but got '" << token.data() << "'";
            } else {
                return static_cast<size_t>(i);
            }
        }
    }
}
