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

#ifndef TRENCHBROOM_ASEPARSER_H
#define TRENCHBROOM_ASEPARSER_H

#include "IO/EntityModelParser.h"
#include "IO/Parser.h"
#include "IO/Tokenizer.h"

#include <vecmath/forward.h>

#include <array>
#include <functional>
#include <string>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class EntityModel;
        class Texture;
    }

    namespace IO {
        class FileSystem;
        class Path;

        namespace AseToken {
            typedef unsigned int Type;
            static const Type Directive         = 1 <<  0; // Any directive, i.e. *SCENE
            static const Type OBrace            = 1 <<  1; // opening brace: {
            static const Type CBrace            = 1 <<  2; // closing brace: }
            static const Type String            = 1 <<  3; // quoted string: "..."
            static const Type Integer           = 1 <<  4; // integer number
            static const Type Decimal           = 1 <<  5; // decimal number
            static const Type Keyword           = 1 <<  6; // keyword: Filter etc.
            static const Type ArgumentName      = 1 <<  7; // argument name: A:, B: etc.
            static const Type Colon             = 1 <<  8; // colon: :
            static const Type Eof               = 1 << 12; // end of file
        }

        class AseTokenizer : public Tokenizer<AseToken::Type> {
        private:
            static const std::string WordDelims;
        public:
            AseTokenizer(const char* begin, const char* end);
            explicit AseTokenizer(const std::string& str);
        private:
            Token emitToken() override;
        };

        class AseParser : public EntityModelParser, private Parser<AseToken::Type> {
        private:
            using Token = AseTokenizer::Token;

            struct MeshFaceVertex {
                size_t vertexIndex;
                size_t uvIndex;
            };

            using MeshFace = std::array<MeshFaceVertex, 3>;

            struct Mesh {
                std::vector<vm::vec3f> vertices;
                std::vector<vm::vec2f> uv;
                std::vector<MeshFace> faces;
            };

            struct GeomObject {
                std::string name;
                Mesh mesh;
                size_t materialIndex;
            };

            struct Scene {
                std::vector<Path> materialPaths;
                std::vector<GeomObject> geomObjects;
            };

            std::string m_name;
            AseTokenizer m_tokenizer;
            const FileSystem& m_fs;
        public:
            /**
             * Creates a new parser for ASE models.
             *
             * @param name the name of the model
             * @param begin the start of the text to parse
             * @param end the end of the text to parse
             * @param fs the file system used to load texture files
             */
            AseParser(const std::string& name, const char* begin, const char* end, const FileSystem& fs);
        private:
            std::unique_ptr<Assets::EntityModel> doInitializeModel(Logger& logger) override;
        private: // parsing
            void parseAseFile(Logger& logger, Scene& scene);

            // SCENE
            void parseScene(Logger& logger);

            // MATERIALS
            void parseMaterialList(Logger& logger, std::vector<Path>& paths);
            void parseMaterialListMaterialCount(Logger& logger, std::vector<Path>& paths);
            void parseMaterialListMaterial(Logger& logger, std::vector<Path>& paths);
            void parseMaterialListMaterialMapDiffuse(Logger& logger, Path& path);
            void parseMaterialListMaterialMapDiffuseBitmap(Logger& logger, Path& path);

            void parseGeomObject(Logger& logger, GeomObject& geomObject, const std::vector<Path>& materialPaths);
            void parseGeomObjectNodeName(Logger& logger, GeomObject& geomObject);
            void parseGeomObjectMaterialRef(Logger& logger, GeomObject& geomObject, size_t materialCount);
            void parseGeomObjectMesh(Logger& logger, Mesh& mesh);
            void parseGeomObjectMeshNumVertex(Logger& logger, std::vector<vm::vec3f>& vertices);
            void parseGeomObjectMeshVertexList(Logger& logger, std::vector<vm::vec3f>& vertices);
            void parseGeomObjectMeshVertex(Logger& logger, std::vector<vm::vec3f>& vertices);
            void parseGeomObjectMeshNumFaces(Logger& logger, std::vector<MeshFace>& faces);
            void parseGeomObjectMeshFaceList(Logger& logger, std::vector<MeshFace>& faces);
            void parseGeomObjectMeshFace(Logger& logger, std::vector<MeshFace>& faces);
            void parseGeomObjectMeshNumTVertex(Logger& logger, std::vector<vm::vec2f>& uv);
            void parseGeomObjectMeshTVertexList(Logger& logger, std::vector<vm::vec2f>& uv);
            void parseGeomObjectMeshTVertex(Logger& logger, std::vector<vm::vec2f>& uv);
            void parseGeomObjectMeshTFaceList(Logger& logger, std::vector<MeshFace>& faces);
            void parseGeomObjectMeshTFace(Logger& logger, std::vector<MeshFace>& faces);

            void parseBlock(const std::map<std::string, std::function<void(void)>>& handlers);

            void expectDirective(const std::string& name);
            void skipDirective(const std::string& name);
            void skipDirective();

            void expectArgumentName(const std::string& expected);

            void expectSizeArgument(size_t expected);
            size_t parseSizeArgument();
            vm::vec3f parseVecArgument();

            TokenNameMap tokenNames() const override;
        private: // model construction
            std::unique_ptr<Assets::EntityModel> buildModel(Logger& logger, const Scene& scene) const;
            std::unique_ptr<Assets::Texture> loadTexture(Logger& logger, const Path& path) const;
            Path fixTexturePath(Logger& logger, Path path) const;
        };
    }
}


#endif //TRENCHBROOM_ASEPARSER_H
