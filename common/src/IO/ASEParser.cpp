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

#include <functional>

namespace TrenchBroom {
    namespace IO {
        AseTokenizer::AseTokenizer(const char* begin, const char* end) :
        Tokenizer(begin, end, "", 0){ }

        AseTokenizer::AseTokenizer(const String& str) :
        Tokenizer(str, "", 0) {}

        const String AseTokenizer::WordDelims = " \t\n\r:";

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
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        discardWhile(Whitespace());
                        break;
                    default: {
                        const auto* e = readInteger(WordDelims);
                        if (e != nullptr) {
                            return Token(AseToken::Integer, c, e, offset(c), startLine, startColumn);
                        }

                        e = readDecimal(WordDelims);
                        if (e != nullptr) {
                            return Token(AseToken::Decimal, c, e, offset(c), startLine, startColumn);
                        }

                        // must be a keyword or argument name
                        e = readUntil(WordDelims);
                        if (e != nullptr) {
                            if (*e == ':') {
                                advance();
                                return Token(AseToken::ArgumentName, c, e, offset(c), startLine, startColumn);
                            } else {
                                return Token(AseToken::Keyword, c, e, offset(c), startLine, startColumn);
                            }
                        }
                        throw ParserException(startLine, startColumn, "Unexpected character: '" + String(c, 1) + "'");
                    }
                }
            }
            return Token(AseToken::Eof, nullptr, nullptr, length(), line(), column());
        }

        ASEParser::ASEParser(const char* begin, const char* end) :
        m_tokenizer(begin, end) {}

        ASEParser::ASEParser(const String& str) :
        m_tokenizer(str) {}

        Assets::EntityModel* ASEParser::doParseModel(Logger& logger) {
            parseAseFile(logger);
            return nullptr;
        }

        void ASEParser::parseAseFile(Logger& logger) {
            expectDirective("3DSMAX_ASCIIEXPORT");
            expect(AseToken::Integer, m_tokenizer.nextToken());

            skipDirective("COMMENT");

            parseScene(logger);
            parseMaterialList(logger);
            parseGeomObject(logger);
        }

        void ASEParser::parseScene(Logger& logger) {
            expectDirective("SCENE");
            parseBlock({});
        }

        void ASEParser::parseMaterialList(Logger& logger) {
            expectDirective("MATERIAL_LIST");
            parseBlock({
                { "MATERIAL_COUNT", std::bind(&ASEParser::parseMaterialListMaterialCount, this, std::ref(logger)) },
                { "MATERIAL", std::bind(&ASEParser::parseMaterialListMaterial, this, std::ref(logger)) }
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
                { "MAP_DIFFUSE", std::bind(&ASEParser::parseMaterialListMaterialMapDiffuse, this, std::ref(logger)) }
            });
        }

        void ASEParser::parseMaterialListMaterialMapDiffuse(Logger& logger) {
            expectDirective("MAP_DIFFUSE");

            parseBlock({
                { "BITMAP", std::bind(&ASEParser::parseMaterialListMaterialMapDiffuseBitmap, this, std::ref(logger)) }
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
                { "NODE_NAME", std::bind(&ASEParser::parseGeomObjectNodeName, this, std::ref(logger)) },
                { "MESH", std::bind(&ASEParser::parseGeomObjectMesh, this, std::ref(logger)) },
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
            std::vector<MeshFace> faces;
            std::vector<vm::vec2f> uv;

            parseBlock({
                { "MESH_NUMVERTEX", std::bind(&ASEParser::parseGeomObjectMeshNumVertex, this, std::ref(logger), std::ref(vertices)) },
                { "MESH_VERTEX_LIST", std::bind(&ASEParser::parseGeomObjectMeshVertexList, this, std::ref(logger), std::ref(vertices)) },
                { "MESH_NUMFACES", std::bind(&ASEParser::parseGeomObjectMeshNumFaces, this, std::ref(logger), std::ref(faces)) },
                { "MESH_FACE_LIST", std::bind(&ASEParser::parseGeomObjectMeshFaceList, this, std::ref(logger), std::ref(faces)) },
                { "MESH_NUMTVERTEX", std::bind(&ASEParser::parseGeomObjectMeshNumTVertex, this, std::ref(logger), std::ref(uv)) },
                { "MESH_TVERTLIST", std::bind(&ASEParser::parseGeomObjectMeshTVertexList, this, std::ref(logger), std::ref(uv)) },
                { "MESH_NUMTVFACES", std::bind(&ASEParser::parseGeomObjectMeshNumTVFaces, this, std::ref(logger), faces.size()) },
                { "MESH_TFACELIST", std::bind(&ASEParser::parseGeomObjectMeshTFaceList, this, std::ref(logger), std::ref(faces)) },
            });
        }

        void ASEParser::parseGeomObjectMeshNumVertex(Logger& logger, std::vector<vm::vec3f>& vertices) {
            expectDirective("MESH_NUMVERTEX");
            const auto vertexCount = parseSizeArgument();
            vertices.reserve(vertexCount);
        }

        void ASEParser::parseGeomObjectMeshVertexList(Logger& logger, std::vector<vm::vec3f>& vertices) {
            expectDirective("MESH_VERTEX_LIST");

            parseBlock({
                { "MESH_VERTEX", std::bind(&ASEParser::parseGeomObjectMeshVertex, this, std::ref(logger), std::ref(vertices)) },
            });
        }

        void ASEParser::parseGeomObjectMeshVertex(Logger& logger, std::vector<vm::vec3f>& vertices) {
            expectDirective("MESH_VERTEX");
            expectSizeArgument(vertices.size());
            vertices.emplace_back(parseVecArgument());
        }

        void ASEParser::parseGeomObjectMeshNumFaces(Logger& logger, std::vector<MeshFace>& faces) {
            expectDirective("MESH_NUMFACES");
            const auto faceCount = parseSizeArgument();
            faces.reserve(faceCount);
        }

        void ASEParser::parseGeomObjectMeshFaceList(Logger& logger, std::vector<MeshFace>& faces) {
            expectDirective("MESH_FACE_LIST");

            parseBlock({
                { "MESH_FACE", std::bind(&ASEParser::parseGeomObjectMeshFace, this, std::ref(logger), std::ref(faces)) },
            });
        }

        void ASEParser::parseGeomObjectMeshFace(Logger& logger, std::vector<MeshFace>& faces) {
            expectDirective("MESH_FACE");
            expectSizeArgument(faces.size());

            expectArgumentName("A");
            auto vertexIndexA = parseSizeArgument();

            expectArgumentName("B");
            auto vertexIndexB = parseSizeArgument();

            expectArgumentName("C");
            auto vertexIndexC = parseSizeArgument();

            // skip edges
            expectArgumentName("AB");
            parseSizeArgument();
            expectArgumentName("BC");
            parseSizeArgument();
            expectArgumentName("CA");
            parseSizeArgument();

            // skip other
            expectDirective("MESH_SMOOTHING");
            parseSizeArgument();

            expectDirective("MESH_MTLID");
            parseSizeArgument();

            faces.emplace_back(MeshFace {
                MeshFaceVertex{ vertexIndexA, 0 },
                MeshFaceVertex{ vertexIndexB, 0 },
                MeshFaceVertex{ vertexIndexC, 0 }
            });
        }

        void ASEParser::parseGeomObjectMeshNumTVertex(Logger& logger, std::vector<vm::vec2f>& uv) {
            expectDirective("MESH_NUMTVERTEX");
            const auto uvCount = parseSizeArgument();
            uv.reserve(uvCount);
        }

        void ASEParser::parseGeomObjectMeshTVertexList(Logger& logger, std::vector<vm::vec2f>& uv) {
            expectDirective("MESH_TVERTEXLIST");

            parseBlock({
                { "MESH_TVERT", std::bind(&ASEParser::parseGeomObjectMeshTVertex, this, std::ref(logger), std::ref(uv)) },
            });
        }

        void ASEParser::parseGeomObjectMeshTVertex(Logger& logger, std::vector<vm::vec2f>& uv) {
            expectDirective("MESH_VERTEX");
            expectSizeArgument(uv.size());
            uv.emplace_back(parseVecArgument());
        }

        void ASEParser::parseGeomObjectMeshNumTVFaces(Logger& logger, const size_t expected) {
            expectDirective("MESH_NUMTVFACES");
            const auto token = m_tokenizer.peekToken();
            const auto count = parseSizeArgument();
            if (count != expected) {
                logger.warn() << "Mismatching number of TVFACES at line " << token.line() << ": expected " << expected << ", but got " << count;
            }
        }

        void ASEParser::parseGeomObjectMeshTFaceList(Logger& logger, std::vector<MeshFace>& faces) {
            expectDirective("MESH_TFACELIST");

            parseBlock({
                { "MESH_TFACE", std::bind(&ASEParser::parseGeomObjectMeshTFace, this, std::ref(logger), std::ref(faces)) },
            });
        }

        void ASEParser::parseGeomObjectMeshTFace(Logger& logger, std::vector<MeshFace>& faces) {
            expectDirective("MESH_TFACE");
            const auto token = m_tokenizer.peekToken();
            const auto index = parseSizeArgument();
            if (index >= faces.size()) {
                throw ParserException(token.line(), token.column()) << "Invalid face index " << index;
            }

            for (size_t i = 0; i < 3; ++i) {
                faces[index][i].uvIndex = parseSizeArgument();
            }
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

        void ASEParser::skipDirective(const String& name) {
            auto token = expect(AseToken::Directive, m_tokenizer.peekToken());
            if (token.data() == name) {
                m_tokenizer.nextToken();

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
        }

        void ASEParser::skipDirective() {
            expect(AseToken::Directive, m_tokenizer.nextToken());

            // skip arguments
            while (!m_tokenizer.peekToken().hasType(AseToken::OBrace | AseToken::CBrace | AseToken::Directive)) {
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

        void ASEParser::expectArgumentName(const String& expected) {
            const auto token = expect(AseToken::ArgumentName, m_tokenizer.nextToken());
            const auto& actual = token.data();
            if (actual != expected) {
                throw ParserException(token.line(), token.column()) << "Expected argument name '" << expected << "', but got '" << actual << "'";
            }
        }

        void ASEParser::expectSizeArgument(const size_t expected) {
            const auto token = m_tokenizer.peekToken();
            const auto actual = parseSizeArgument();
            if (actual != expected) {
                throw ParserException(token.line(), token.column()) << "Expected value '" << expected << "', but got '" << actual << "'";
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

        vm::vec3f ASEParser::parseVecArgument() {
            return {
                expect(AseToken::Decimal, m_tokenizer.nextToken()).toFloat<float>(),
                expect(AseToken::Decimal, m_tokenizer.nextToken()).toFloat<float>(),
                expect(AseToken::Decimal, m_tokenizer.nextToken()).toFloat<float>()
            };
        }

        ASEParser::TokenNameMap ASEParser::tokenNames() const {
            TokenNameMap result;
            result[AseToken::Directive]    = "directive";
            result[AseToken::OBrace]       = "'{'";
            result[AseToken::CBrace]       = "'}'";
            result[AseToken::String]       = "quoted string";
            result[AseToken::Integer]      = "integer";
            result[AseToken::Decimal]      = "decimal";
            result[AseToken::Keyword]      = "keyword";
            result[AseToken::ArgumentName] = "argument name";
            result[AseToken::Eof]          = "end of file";
            return result;
        }
    }
}
