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
            writeNormals();
            std::fprintf(m_stream, "\n");
            writeObjects();
        }

        void ObjFileSerializer::writeVertices() {
            std::fprintf(m_stream, "# vertices\n");
            const IndexMap<Vec3>::List& elements = m_vertices.list();
            IndexMap<Vec3>::List::const_iterator it, end;
            for (it = elements.begin(), end = elements.end(); it != end; ++it) {
                const Vec3& elem = *it;
                std::fprintf(m_stream, "v %.17g %.17g %.17g\n", elem.x(), elem.z(), -elem.y()); // no idea why I have to switch Y and Z
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
                std::fprintf(m_stream, "v %.17g %.17g %.17g\n", elem.x(), elem.z(), -elem.y()); // no idea why I have to switch Y and Z
            }
        }
        
        void ObjFileSerializer::writeObjects() {
            std::fprintf(m_stream, "# objects\n");
            ObjectList::const_iterator fIt, fEnd;
            for (fIt = m_objects.begin(), fEnd = m_objects.end(); fIt != fEnd; ++fIt) {
                const Object& object = *fIt;
                std::fprintf(m_stream, "o entity%u_brush%u\n",
                             static_cast<unsigned long>(object.entityNo),
                             static_cast<unsigned long>(object.brushNo));
                
                writeFaces(object.faces);
                std::fprintf(m_stream, "\n");
            }
        }

        void ObjFileSerializer::writeFaces(const FaceList& faces) {
            FaceList::const_iterator fIt, fEnd;
            for (fIt = faces.begin(), fEnd = faces.end(); fIt != fEnd; ++fIt) {
                const IndexedVertexList& face = *fIt;
                std::fprintf(m_stream, "f");
                
                IndexedVertexList::const_iterator vIt, vEnd;
                for (vIt = face.begin(), vEnd = face.end(); vIt != vEnd; ++vIt) {
                    const IndexedVertex& vertex = *vIt;
                    std::fprintf(m_stream, " %u/%u/%u",
                                 static_cast<unsigned long>(vertex.vertex) + 1,
                                 static_cast<unsigned long>(vertex.texCoords) + 1,
                                 static_cast<unsigned long>(vertex.normal) + 1);
                }
                std::fprintf(m_stream, "\n");
            }
        }

        void ObjFileSerializer::doBeginEntity(const Model::Node* node) {}
        void ObjFileSerializer::doEndEntity(Model::Node* node) {}
        void ObjFileSerializer::doEntityAttribute(const Model::EntityAttribute& attribute) {}
        
        void ObjFileSerializer::doBeginBrush(const Model::Brush* brush) {
            m_currentObject.entityNo = entityNo();
            m_currentObject.brushNo = brushNo();
        }
        
        void ObjFileSerializer::doEndBrush(Model::Brush* brush) {
            m_objects.push_back(m_currentObject);
            m_currentObject.faces.clear();
        }
        
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
            
            m_currentObject.faces.push_back(indexedVertices);
        }
    }
}
