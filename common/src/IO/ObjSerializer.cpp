/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"

#include <cassert>

namespace TrenchBroom {
    namespace IO {
        ObjFileSerializer::IndexedVertex::IndexedVertex(const size_t i_vertex, const size_t i_texCoords, const size_t i_normal) :
        vertex(i_vertex),
        texCoords(i_texCoords),
        normal(i_normal) {}

        ObjFileSerializer::ObjFileSerializer(FILE* stream) :
        m_stream(stream) {
            assert(m_stream != NULL);
        }

        void ObjFileSerializer::doBeginFile() {}
        
        void ObjFileSerializer::doEndFile() {
            writeVertices();
            std::fprintf(m_stream, "\n");
            writeTexCoords();
            std::fprintf(m_stream, "\n");
            writeFaces();
        }

        void ObjFileSerializer::writeVertices() {
            std::fprintf(m_stream, "# vertices\n");
            const IndexMap<Vec3>::List& elements = m_vertices.list();
            IndexMap<Vec3>::List::const_iterator it, end;
            for (it = elements.begin(), end = elements.end(); it != end; ++it) {
                const Vec3& elem = *it;
                std::fprintf(m_stream, "v %.17g %.17g %.17g\n", elem.x(), elem.y(), elem.z());
            }
        }
        
        void ObjFileSerializer::writeTexCoords() {
            std::fprintf(m_stream, "# texture coordinates\n");
            const IndexMap<Vec2f>::List& elements = m_texCoords.list();
            IndexMap<Vec2f>::List::const_iterator it, end;
            for (it = elements.begin(), end = elements.end(); it != end; ++it) {
                const Vec3& elem = *it;
                std::fprintf(m_stream, "vt %.17g %.17g\n", elem.x(), elem.y());
            }
        }
        
        void ObjFileSerializer::writeNormals() {
            std::fprintf(m_stream, "# face normals\n");
            const IndexMap<Vec3>::List& elements = m_normals.list();
            IndexMap<Vec3>::List::const_iterator it, end;
            for (it = elements.begin(), end = elements.end(); it != end; ++it) {
                const Vec3& elem = *it;
                std::fprintf(m_stream, "vn %.17g %.17g %.17g\n", elem.x(), elem.y(), elem.z());
            }
        }
        
        void ObjFileSerializer::writeFaces() {
            std::fprintf(m_stream, "# faces\n");
            FaceList::const_iterator fIt, fEnd;
            for (fIt = m_faces.begin(), fEnd = m_faces.end(); fIt != fEnd; ++fIt) {
                const IndexedVertexList& face = *fIt;
                std::fprintf(m_stream, "f");
                
                IndexedVertexList::const_iterator vIt, vEnd;
                for (vIt = face.begin(), vEnd = face.end(); vIt != vEnd; ++vIt) {
                    const IndexedVertex& vertex = *vIt;
                    std::fprintf(m_stream, " %u/%u/%u",
                                static_cast<unsigned long>(vertex.vertex),
                                static_cast<unsigned long>(vertex.texCoords),
                                static_cast<unsigned long>(vertex.normal));
                }
                std::fprintf(m_stream, "\n");
            }
        }

        void ObjFileSerializer::doBeginEntity(const Model::Node* node) {}
        void ObjFileSerializer::doEndEntity(Model::Node* node) {}
        void ObjFileSerializer::doEntityAttribute(const Model::EntityAttribute& attribute) {}
        
        void ObjFileSerializer::doBeginBrush(const Model::Brush* brush) {}
        void ObjFileSerializer::doEndBrush(Model::Brush* brush) {}
        
        void ObjFileSerializer::doBrushFace(Model::BrushFace* face) {
            const Vec3& normal = face->boundary().normal;
            const size_t normalIndex = m_normals.index(normal);
            
            const Model::BrushFace::VertexList vertices = face->vertices();
            IndexedVertexList indexedVertices;
            indexedVertices.reserve(vertices.size());
            
            typename Model::BrushFace::VertexList::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                const Vec3& vertex = (*it)->position();
                const Vec2f texCoords = face->textureCoords(vertex);
                
                const size_t vertexIndex = m_vertices.index(vertex);
                const size_t texCoordsIndex = m_texCoords.index(texCoords);
                
                indexedVertices.push_back(IndexedVertex(vertexIndex, texCoordsIndex, normalIndex));
            }
            
            m_faces.push_back(indexedVertices);
        }
    }
}
