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
#include "Assets/Texture.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/PatchNode.h"
#include "Model/Polyhedron.h"

#include <kdl/overload.h>

#include <fmt/format.h>

#include <iostream>

namespace TrenchBroom {
    namespace IO {
        std::ostream& operator<<(std::ostream& str, const ObjSerializer::IndexedVertex& vertex) {
            str << " " << (vertex.vertex + 1u) << "/" << (vertex.texCoords + 1u) << "/" << (vertex.normal + 1u);
            return str;
        }

        std::ostream& operator<<(std::ostream& str, const ObjSerializer::BrushObject& object) {
            str << "o entity" << object.entityNo << "_brush" << object.brushNo << "\n";
            for (const auto& face : object.faces) {
                str << "usemtl " << face.textureName << "\n";
                str << "f";
                for (const auto& vertex : face.verts) {
                    str << " " << vertex;
                }
                str << "\n";
            }
            return str;
        }

        std::ostream& operator<<(std::ostream& str, const ObjSerializer::PatchObject& object) {
            str << "o entity" << object.entityNo << "_patch" << object.patchNo << "\n";
            str << "usemtl " << object.textureName << "\n";
            for (const auto& quad : object.quads) {
                str << "f";
                for (const auto& vertex : quad.verts) {
                    str << " " << vertex;
                }
                str << "\n";
            }
            return str;
        }

        std::ostream& operator<<(std::ostream& str, const ObjSerializer::Object& object) {
            std::visit(kdl::overload(
                [&](const ObjSerializer::BrushObject& brushObject) {
                    str << brushObject;
                },
                [&](const ObjSerializer::PatchObject& patchObject) {
                    str << patchObject;
                }
            ), object);
            return str;
        }

        ObjSerializer::ObjSerializer(std::ostream& objStream, std::ostream& mtlStream, std::string mtlFilename) :
        m_objStream{objStream},
        m_mtlStream{mtlStream},
        m_mtlFilename{std::move(mtlFilename)} {
            ensure(m_objStream.good(), "obj stream is good");
            ensure(m_mtlStream.good(), "mtl stream is good");
        }

        void ObjSerializer::doBeginFile(const std::vector<const Model::Node*>& /* rootNodes */) {}

        static void writeMtlFile(std::ostream& str, const std::vector<ObjSerializer::Object>& objects) {
            auto usedTextures = std::map<std::string, const Assets::Texture*>{};

            for (const auto& object : objects) {
                std::visit(kdl::overload(
                    [&](const ObjSerializer::BrushObject& brushObject) {
                        for (const auto& face : brushObject.faces) {
                            usedTextures[face.textureName] = face.texture;
                        }
                    },
                    [&](const ObjSerializer::PatchObject& patchObject) {
                        usedTextures[patchObject.textureName] = patchObject.texture;
                    }
                ), object);
            }

            for (const auto& [textureName, texture] : usedTextures) {
                str << "newmtl " << textureName << "\n";
                if (texture != nullptr && !texture->relativePath().isEmpty()) {
                    str << "map_Kd " << texture->relativePath().asString() << "\n\n";
                }
            }
        }

        static void writeVertices(std::ostream& str, const std::vector<vm::vec3>& vertices) {
            str << "# vertices\n";
            for (const vm::vec3& elem : vertices) {
                // no idea why I have to switch Y and Z
                fmt::format_to(std::ostreambuf_iterator<char>(str), "v {} {} {}\n", elem.x(), elem.z(), -elem.y());
            }
        }

        static void writeTexCoords(std::ostream& str, const std::vector<vm::vec2f>& texCoords) {
            str << "# texture coordinates\n";
            for (const vm::vec2f& elem : texCoords) {
                // multiplying Y by -1 needed to get the UV's to appear correct in Blender and UE4
                // (see: https://github.com/TrenchBroom/TrenchBroom/issues/2851 )
                fmt::format_to(std::ostreambuf_iterator<char>(str), "vt {} {}\n", elem.x(), -elem.y());
            }
        }

        static void writeNormals(std::ostream& str, const std::vector<vm::vec3>& normals) {
            str << "# normals\n";
            for (const vm::vec3& elem : normals) {
                // no idea why I have to switch Y and Z
                fmt::format_to(std::ostreambuf_iterator<char>(str), "vn {} {} {}\n", elem.x(), elem.z(), -elem.y());
            }
        }

        static void writeObjFile(
            std::ostream& str, const std::string mtlFilename, 
            const std::vector<vm::vec3>& vertices, const std::vector<vm::vec2f>& texCoords, const std::vector<vm::vec3>& normals, 
            const std::vector<ObjSerializer::Object>& objects) {
            
            str << "mtllib " << mtlFilename << "\n";
            writeVertices(str, vertices);
            str << "\n";
            writeTexCoords(str, texCoords);
            str << "\n";
            writeNormals(str, normals);
            str << "\n";
            
            for (const auto& object : objects) {
                str << object;
                str << "\n";
            }
        }

        void ObjSerializer::doEndFile() {
            writeMtlFile(m_mtlStream, m_objects);
            writeObjFile(m_objStream, m_mtlFilename, m_vertices.list(), m_texCoords.list(), m_normals.list(), m_objects);
        }

        void ObjSerializer::doBeginEntity(const Model::Node* /* node */) {}
        void ObjSerializer::doEndEntity(const Model::Node* /* node */) {}
        void ObjSerializer::doEntityProperty(const Model::EntityProperty& /* property */) {}

        void ObjSerializer::doBrush(const Model::BrushNode* brush) {
            m_currentBrush = BrushObject{entityNo(), brushNo(), {}};
            m_currentBrush->faces.reserve(brush->brush().faceCount());

            // Vertex positions inserted from now on should get new indices
            m_vertices.clearIndices();

            for (const Model::BrushFace& face : brush->brush().faces()) {
                doBrushFace(face);
            }

            m_objects.push_back(std::move(*m_currentBrush));
            m_currentBrush = std::nullopt;
        }

        void ObjSerializer::doBrushFace(const Model::BrushFace& face) {
            const vm::vec3& normal = face.boundary().normal;
            const size_t normalIndex = m_normals.index(normal);

            auto indexedVertices = std::vector<IndexedVertex>{};
            indexedVertices.reserve(face.vertexCount());

            for (const Model::BrushVertex* vertex : face.vertices()) {
                const vm::vec3& position = vertex->position();
                const vm::vec2f texCoords = face.textureCoords(position);

                const size_t vertexIndex = m_vertices.index(position);
                const size_t texCoordsIndex = m_texCoords.index(texCoords);

                indexedVertices.push_back(IndexedVertex{vertexIndex, texCoordsIndex, normalIndex});
            }

            m_currentBrush->faces.push_back(BrushFace{std::move(indexedVertices), face.attributes().textureName(), face.texture()});
        }

        void ObjSerializer::doPatch(const Model::PatchNode* patchNode) {
            const auto& patch = patchNode->patch();
            auto patchObject = PatchObject{entityNo(), brushNo(), {}, patch.textureName(), patch.texture()};

            const auto& patchGrid = patchNode->grid();
            patchObject.quads.reserve(patchGrid.quadRowCount() * patchGrid.quadColumnCount());

            // Vertex positions inserted from now on should get new indices
            m_vertices.clearIndices();

            const auto makeIndexedVertex = [&](const auto& p) {
                const size_t positionIndex = m_vertices.index(p.position);
                const size_t texCoordsIndex = m_texCoords.index(vm::vec2f{p.texCoords});
                const size_t normalIndex = m_normals.index(p.normal);

                return IndexedVertex{positionIndex, texCoordsIndex, normalIndex};
            };

            for (size_t row = 0u; row < patchGrid.pointRowCount - 1u; ++row) {
                for (size_t col = 0u; col < patchGrid.pointColumnCount - 1u; ++col) {
                        // counter clockwise order
                        patchObject.quads.push_back(PatchQuad{{
                            makeIndexedVertex(patchGrid.point(row, col)),
                            makeIndexedVertex(patchGrid.point(row + 1u, col)),
                            makeIndexedVertex(patchGrid.point(row + 1u, col + 1u)),
                            makeIndexedVertex(patchGrid.point(row, col + 1u)),
                        }});
                }
            }

            m_objects.push_back(std::move(patchObject));
        }
    }
}

