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

#include "AseParser.h"

#include "Assets/EntityModel.h"
#include "Assets/Texture.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"
#include "IO/Quake3ShaderTextureReader.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"

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

        AseParser::AseParser(const String& name, const char* begin, const char* end, const FileSystem& fs) :
        m_name(name),
        m_tokenizer(begin, end),
        m_fs(fs) {}

        std::unique_ptr<Assets::EntityModel> AseParser::doInitializeModel(Logger& logger) {
            Scene scene;
            parseAseFile(logger, scene);
            return buildModel(logger, scene);
        }

        void AseParser::parseAseFile(Logger& logger, Scene& scene) {
            expectDirective("3DSMAX_ASCIIEXPORT");
            expect(AseToken::Integer, m_tokenizer.nextToken());

            skipDirective("COMMENT");

            parseScene(logger);
            parseMaterialList(logger, scene.materialPaths);

            while (!m_tokenizer.peekToken().hasType(AseToken::Eof)) {
                GeomObject geomObject;
                parseGeomObject(logger, geomObject, scene.materialPaths);
                scene.geomObjects.emplace_back(std::move(geomObject));
            }
        }

        void AseParser::parseScene(Logger& logger) {
            expectDirective("SCENE");
            parseBlock({});
        }

        void AseParser::parseMaterialList(Logger& logger, Path::List& paths) {
            expectDirective("MATERIAL_LIST");

            parseBlock({
                { "MATERIAL_COUNT", std::bind(&AseParser::parseMaterialListMaterialCount, this, std::ref(logger), std::ref(paths)) },
                { "MATERIAL", std::bind(&AseParser::parseMaterialListMaterial, this, std::ref(logger), std::ref(paths)) }
            });
        }

        void AseParser::parseMaterialListMaterialCount(Logger& logger, Path::List& paths) {
            expectDirective("MATERIAL_COUNT");
            parseSizeArgument();
        }

        void AseParser::parseMaterialListMaterial(Logger& logger, Path::List& paths) {
            expectDirective("MATERIAL");
            const auto count = parseSizeArgument();
            paths.reserve(count);

            parseBlock({
                { "MAP_DIFFUSE", std::bind(&AseParser::parseMaterialListMaterialMapDiffuse, this, std::ref(logger), std::ref(paths)) }
            });
        }

        void AseParser::parseMaterialListMaterialMapDiffuse(Logger& logger, Path::List& paths) {
            expectDirective("MAP_DIFFUSE");

            parseBlock({
                { "BITMAP", std::bind(&AseParser::parseMaterialListMaterialMapDiffuseBitmap, this, std::ref(logger), std::ref(paths)) }
            });
        }

        void AseParser::parseMaterialListMaterialMapDiffuseBitmap(Logger& logger, Path::List& paths) {
            expectDirective("BITMAP");
            const auto token = expect(AseToken::String, m_tokenizer.nextToken());
            paths.emplace_back(Path(token.data()));
        }

        void AseParser::parseGeomObject(Logger& logger, GeomObject& geomObject, const Path::List& materialPaths) {
            expectDirective("GEOMOBJECT");

            parseBlock({
                { "NODE_NAME", std::bind(&AseParser::parseGeomObjectNodeName, this, std::ref(logger), std::ref(geomObject)) },
                { "MATERIAL_REF", std::bind(&AseParser::parseGeomObjectMaterialRef, this, std::ref(logger), std::ref(geomObject), materialPaths.size()) },
                { "MESH", std::bind(&AseParser::parseGeomObjectMesh, this, std::ref(logger), std::ref(geomObject.mesh)) }
            });
        }

        void AseParser::parseGeomObjectNodeName(Logger& logger, GeomObject& geomObject) {
            expectDirective("NODE_NAME");
            const auto token = expect(AseToken::String, m_tokenizer.nextToken());
            geomObject.name = token.data();
        }

        void AseParser::parseGeomObjectMaterialRef(Logger& logger, GeomObject& geomObject, const size_t materialCount) {
            expectDirective("MATERIAL_REF");
            const auto token = m_tokenizer.peekToken();
            const size_t materialIndex = parseSizeArgument();
            if (materialIndex >= materialCount) {
                logger.warn() << "Line " << token.line() << ": Material index " << materialIndex << " is out of bounds, assuming " << materialCount - 1;

            }
            geomObject.materialIndex = std::min(materialIndex, materialCount - 1);
        }

        void AseParser::parseGeomObjectMesh(Logger& logger, Mesh& mesh) {
            expectDirective("MESH");

            parseBlock({
                { "MESH_NUMVERTEX", std::bind(&AseParser::parseGeomObjectMeshNumVertex, this, std::ref(logger), std::ref(mesh.vertices)) },
                { "MESH_VERTEX_LIST", std::bind(&AseParser::parseGeomObjectMeshVertexList, this, std::ref(logger), std::ref(mesh.vertices)) },
                { "MESH_NUMFACES", std::bind(&AseParser::parseGeomObjectMeshNumFaces, this, std::ref(logger), std::ref(mesh.faces)) },
                { "MESH_FACE_LIST", std::bind(&AseParser::parseGeomObjectMeshFaceList, this, std::ref(logger), std::ref(mesh.faces)) },
                { "MESH_NUMTVERTEX", std::bind(&AseParser::parseGeomObjectMeshNumTVertex, this, std::ref(logger), std::ref(mesh.uv)) },
                { "MESH_TVERTLIST", std::bind(&AseParser::parseGeomObjectMeshTVertexList, this, std::ref(logger), std::ref(mesh.uv)) },
                { "MESH_TFACELIST", std::bind(&AseParser::parseGeomObjectMeshTFaceList, this, std::ref(logger), std::ref(mesh.faces)) },
            });
        }

        void AseParser::parseGeomObjectMeshNumVertex(Logger& logger, std::vector<vm::vec3f>& vertices) {
            expectDirective("MESH_NUMVERTEX");
            const auto vertexCount = parseSizeArgument();
            vertices.reserve(vertexCount);
        }

        void AseParser::parseGeomObjectMeshVertexList(Logger& logger, std::vector<vm::vec3f>& vertices) {
            expectDirective("MESH_VERTEX_LIST");

            parseBlock({
                { "MESH_VERTEX", std::bind(&AseParser::parseGeomObjectMeshVertex, this, std::ref(logger), std::ref(vertices)) },
            });
        }

        void AseParser::parseGeomObjectMeshVertex(Logger& logger, std::vector<vm::vec3f>& vertices) {
            expectDirective("MESH_VERTEX");
            expectSizeArgument(vertices.size());
            vertices.emplace_back(parseVecArgument());
        }

        void AseParser::parseGeomObjectMeshNumFaces(Logger& logger, std::vector<MeshFace>& faces) {
            expectDirective("MESH_NUMFACES");
            const auto faceCount = parseSizeArgument();
            faces.reserve(faceCount);
        }

        void AseParser::parseGeomObjectMeshFaceList(Logger& logger, std::vector<MeshFace>& faces) {
            expectDirective("MESH_FACE_LIST");

            parseBlock({
                { "MESH_FACE", std::bind(&AseParser::parseGeomObjectMeshFace, this, std::ref(logger), std::ref(faces)) },
            });
        }

        void AseParser::parseGeomObjectMeshFace(Logger& logger, std::vector<MeshFace>& faces) {
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

        void AseParser::parseGeomObjectMeshNumTVertex(Logger& logger, std::vector<vm::vec2f>& uv) {
            expectDirective("MESH_NUMTVERTEX");
            const auto uvCount = parseSizeArgument();
            uv.reserve(uvCount);
        }

        void AseParser::parseGeomObjectMeshTVertexList(Logger& logger, std::vector<vm::vec2f>& uv) {
            expectDirective("MESH_TVERTLIST");

            parseBlock({
                { "MESH_TVERT", std::bind(&AseParser::parseGeomObjectMeshTVertex, this, std::ref(logger), std::ref(uv)) },
            });
        }

        void AseParser::parseGeomObjectMeshTVertex(Logger& logger, std::vector<vm::vec2f>& uv) {
            expectDirective("MESH_TVERT");
            expectSizeArgument(uv.size());
            const auto tmp = parseVecArgument();
            uv.emplace_back(tmp.x(), 1.0f - tmp.y());
        }

        void AseParser::parseGeomObjectMeshTFaceList(Logger& logger, std::vector<MeshFace>& faces) {
            expectDirective("MESH_TFACELIST");

            parseBlock({
                { "MESH_TFACE", std::bind(&AseParser::parseGeomObjectMeshTFace, this, std::ref(logger), std::ref(faces)) },
            });
        }

        void AseParser::parseGeomObjectMeshTFace(Logger& logger, std::vector<MeshFace>& faces) {
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

        void AseParser::parseBlock(const std::map<std::string, std::function<void(void)>>& handlers) {
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

        void AseParser::expectDirective(const String& name) {
            auto token = expect(AseToken::Directive, m_tokenizer.nextToken());
            expect(name, token);
        }

        void AseParser::skipDirective(const String& name) {
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

        void AseParser::skipDirective() {
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

        void AseParser::expectArgumentName(const String& expected) {
            const auto token = expect(AseToken::ArgumentName, m_tokenizer.nextToken());
            const auto& actual = token.data();
            if (actual != expected) {
                throw ParserException(token.line(), token.column()) << "Expected argument name '" << expected << "', but got '" << actual << "'";
            }
        }

        void AseParser::expectSizeArgument(const size_t expected) {
            const auto token = m_tokenizer.peekToken();
            const auto actual = parseSizeArgument();
            if (actual != expected) {
                throw ParserException(token.line(), token.column()) << "Expected value '" << expected << "', but got '" << actual << "'";
            }
        }

        size_t AseParser::parseSizeArgument() {
            const auto token = expect(AseToken::Integer, m_tokenizer.nextToken());
            auto i = token.toInteger<int>();
            if (i < 0) {
                throw ParserException(token.line(), token.column()) << "Expected positive integer, but got '" << token.data() << "'";
            } else {
                return static_cast<size_t>(i);
            }
        }

        vm::vec3f AseParser::parseVecArgument() {
            return {
                expect(AseToken::Decimal, m_tokenizer.nextToken()).toFloat<float>(),
                expect(AseToken::Decimal, m_tokenizer.nextToken()).toFloat<float>(),
                expect(AseToken::Decimal, m_tokenizer.nextToken()).toFloat<float>()
            };
        }

        AseParser::TokenNameMap AseParser::tokenNames() const {
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

        std::unique_ptr<Assets::EntityModel> AseParser::buildModel(Logger& logger, const Scene& scene) const {
            using Vertex = Assets::EntityModel::Vertex;

            auto model = std::make_unique<Assets::EntityModel>(m_name);
            model->addFrames(1);
            auto& surface = model->addSurface(m_name);

            Assets::TextureList textures;
            textures.resize(scene.materialPaths.size());

            // Load the textures
            for (size_t i = 0; i < scene.materialPaths.size(); ++i) {
                auto path = scene.materialPaths[i];
                try {
                    auto texture = loadTexture(logger, path);
                    textures[i] = texture.get();
                    if (texture != nullptr) {
                        surface.addSkin(texture.release());
                    }
                } catch (const std::exception& e) {
                    logger.error() << "Failed to load texture '" << path << "': " << e.what();
                    textures[i] = nullptr;
                }
            }

            // Count vertices and build bounds
            auto bounds = vm::bbox3f::builder();
            size_t totalVertexCount = 0;
            Renderer::TexturedIndexRangeMap::Size size;
            for (const auto& geomObject : scene.geomObjects) {
                const auto& mesh = geomObject.mesh;
                bounds.add(std::begin(mesh.vertices), std::end(mesh.vertices));

                const auto textureIndex = geomObject.materialIndex;
                auto* texture = textures[textureIndex];

                const auto vertexCount = mesh.faces.size() * 3;
                size.inc(texture, GL_TRIANGLES, vertexCount);
                totalVertexCount += vertexCount;
            }

            auto& frame = model->loadFrame(0, m_name, bounds.bounds());

            // Collect vertex data
            Renderer::TexturedIndexRangeMapBuilder<Vertex::Spec> builder(totalVertexCount, size);
            for (const auto& geomObject : scene.geomObjects) {
                const auto& mesh = geomObject.mesh;

                const auto textureIndex = geomObject.materialIndex;
                auto* texture = textures[textureIndex];

                for (const auto& face : mesh.faces) {
                    builder.addTriangle(
                        texture,
                        Vertex(mesh.vertices[face[2].vertexIndex], mesh.uv[face[2].uvIndex]),
                        Vertex(mesh.vertices[face[1].vertexIndex], mesh.uv[face[1].uvIndex]),
                        Vertex(mesh.vertices[face[0].vertexIndex], mesh.uv[face[0].uvIndex]));
                }

            }
            surface.addTexturedMesh(frame, builder.vertices(), builder.indices());

            return model;
        }

        std::unique_ptr<Assets::Texture> AseParser::loadTexture(Logger& logger, const Path& path) const {
            const auto actualPath = fixTexturePath(logger, path);
            if (!actualPath.isEmpty()) {
                logger.debug() << "Loading texture from '" << actualPath << "'";
                const auto file = m_fs.fileExists(actualPath.deleteExtension()) ? m_fs.openFile(actualPath.deleteExtension()) : m_fs.openFile(actualPath);

                Quake3ShaderTextureReader reader(TextureReader::PathSuffixNameStrategy(2, true), m_fs);
                return std::unique_ptr<Assets::Texture>(reader.readTexture(file));
            } else {
                return nullptr;
            }
        }

        Path AseParser::fixTexturePath(Logger& logger, Path path) const {
            if (!path.isAbsolute()) {
                // usually the paths appear to be relative to the map file, but this will just yield a valid path if we kick off the ".." parts
                while (!path.isEmpty() && path.firstComponent() == Path("..")) {
                    path = path.deleteFirstComponent();
                }
            }
            return path;
        }
    }
}
