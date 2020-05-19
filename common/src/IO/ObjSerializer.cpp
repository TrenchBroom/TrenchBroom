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

#include "ObjSerializer.h"

#include "Ensure.h"
#include "IO/Path.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/Polyhedron.h"

#include <set>

namespace TrenchBroom {
    namespace IO {
        ObjFileSerializer::IndexedVertex::IndexedVertex(const size_t i_vertex, const size_t i_texCoords, const size_t i_normal) :
        vertex(i_vertex),
        texCoords(i_texCoords),
        normal(i_normal) {}

        ObjFileSerializer::Face::Face(IndexedVertexList i_verts, std::string i_texture) :
        verts(std::move(i_verts)),
        texture(std::move(i_texture)) {}

        ObjFileSerializer::ObjFileSerializer(const Path& path) :
        m_objPath(path),
        m_mtlPath(path.replaceExtension("mtl")),
        m_objFile(m_objPath, true),
        m_mtlFile(m_mtlPath, true),
        m_stream(m_objFile.file),
        m_mtlStream(m_mtlFile.file),
        m_currentObject({ 0, 0, {} }) {
            ensure(m_stream != nullptr, "stream is null");
            ensure(m_mtlStream != nullptr, "mtl stream is null");
        }

        void ObjFileSerializer::doBeginFile() {}

        void ObjFileSerializer::doEndFile() {
            writeMtlFile();

            std::fprintf(m_stream, "mtllib %s\n", m_mtlPath.filename().c_str());
            writeVertices();
            std::fprintf(m_stream, "\n");
            writeTexCoords();
            std::fprintf(m_stream, "\n");
            writeNormals();
            std::fprintf(m_stream, "\n");
            writeObjects();
        }

        void ObjFileSerializer::writeMtlFile() {
            std::set<std::string> textureNames;

            for (const Object& object : m_objects) {
                for (const Face& face : object.faces) {
                    textureNames.insert(face.texture);
                }
            }

            for (const std::string& texture : textureNames) {
                std::fprintf(m_mtlStream, "newmtl %s\n", texture.c_str());
            }
        }

        void ObjFileSerializer::writeVertices() {
            std::fprintf(m_stream, "# vertices\n");
            for (const vm::vec3& elem : m_vertices.list())
                std::fprintf(m_stream, "v %.17g %.17g %.17g\n", elem.x(), elem.z(), -elem.y()); // no idea why I have to switch Y and Z
        }

        void ObjFileSerializer::writeTexCoords() {
            std::fprintf(m_stream, "# texture coordinates\n");
            for (const vm::vec2f& elem : m_texCoords.list()) {
                // multiplying Y by -1 needed to get the UV's to appear correct in Blender and UE4
                // (see: https://github.com/kduske/TrenchBroom/issues/2851 )
                std::fprintf(m_stream, "vt %.17g %.17g\n", static_cast<double>(elem.x()), static_cast<double>(-elem.y()));
            }
        }

        void ObjFileSerializer::writeNormals() {
            std::fprintf(m_stream, "# face normals\n");
            for (const vm::vec3& elem : m_normals.list()) {
                std::fprintf(m_stream, "vn %.17g %.17g %.17g\n", elem.x(), elem.z(), -elem.y()); // no idea why I have to switch Y and Z
            }
        }

        void ObjFileSerializer::writeObjects() {
            std::fprintf(m_stream, "# objects\n");
            for (const Object& object : m_objects) {
                std::fprintf(m_stream, "o entity%lu_brush%lu\n",
                             static_cast<unsigned long>(object.entityNo),
                             static_cast<unsigned long>(object.brushNo));

                writeFaces(object.faces);
                std::fprintf(m_stream, "\n");
            }
        }

        void ObjFileSerializer::writeFaces(const FaceList& faces) {
            for (const Face& face : faces) {
                std::fprintf(m_stream, "usemtl %s\n", face.texture.c_str());
                std::fprintf(m_stream, "f");
                for (const IndexedVertex& vertex : face.verts) {
                    std::fprintf(m_stream, " %lu/%lu/%lu",
                                 static_cast<unsigned long>(vertex.vertex) + 1,
                                 static_cast<unsigned long>(vertex.texCoords) + 1,
                                 static_cast<unsigned long>(vertex.normal) + 1);
                }
                std::fprintf(m_stream, "\n");
            }
        }

        void ObjFileSerializer::doBeginEntity(const Model::Node* /* node */) {}
        void ObjFileSerializer::doEndEntity(const Model::Node* /* node */) {}
        void ObjFileSerializer::doEntityAttribute(const Model::EntityAttribute& /* attribute */) {}

        void ObjFileSerializer::doBeginBrush(const Model::BrushNode* /* brush */) {
            m_currentObject.entityNo = entityNo();
            m_currentObject.brushNo = brushNo();
            // Vertex positions inserted from now on should get new indices
            m_vertices.clearIndices();
        }

        void ObjFileSerializer::doEndBrush(const Model::BrushNode* /* brush */) {
            m_objects.push_back(m_currentObject);
            m_currentObject.faces.clear();
        }

        void ObjFileSerializer::doBrushFace(const Model::BrushFace& face) {
            const vm::vec3& normal = face.boundary().normal;
            const size_t normalIndex = m_normals.index(normal);

            IndexedVertexList indexedVertices;
            indexedVertices.reserve(face.vertexCount());

            for (const Model::BrushVertex* vertex : face.vertices()) {
                const vm::vec3& position = vertex->position();
                const vm::vec2f texCoords = face.textureCoords(position);

                const size_t vertexIndex = m_vertices.index(position);
                const size_t texCoordsIndex = m_texCoords.index(texCoords);

                indexedVertices.push_back(IndexedVertex(vertexIndex, texCoordsIndex, normalIndex));
            }

            m_currentObject.faces.push_back(Face(indexedVertices, face.attributes().textureName()));
        }
    }
}

