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

#ifndef ObjSerializer_h
#define ObjSerializer_h

#include "IO/NodeSerializer.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"

#include <cstdio>
#include <list>
#include <map>
#include <vector>

namespace TrenchBroom {
    namespace IO {
        class ObjFileSerializer : public NodeSerializer {
        private:
            template <typename V>
            class IndexMap {
            public:
                typedef std::vector<V> Array;
            private:
                typedef std::map<V, size_t> Map;
                Map m_map;
                Array m_array;
            public:
                const Array& array() const {
                    return m_array;
                }
                
                size_t index(const V& v) {
                    typename Map::iterator indexIt = MapUtils::findOrInsert(m_map, v, m_array.size());
                    const size_t index = indexIt->second;
                    if (index == m_array.size())
                        m_array.push_back(v);
                    return index;
                }
            };

            struct IndexedVertex {
                size_t vertex;
                size_t texCoords;
                size_t normal;

                IndexedVertex(size_t i_vertex, size_t i_texCoords, size_t i_normal);
            };
            
            typedef std::vector<IndexedVertex> IndexedVertexArray;
            typedef std::list<IndexedVertexArray> FaceList;

            struct Object {
                size_t entityNo;
                size_t brushNo;
                FaceList faces;
            };
            
            typedef std::list<Object> ObjectList;
            
            FILE* m_stream;

            IndexMap<Vec3> m_vertices;
            IndexMap<Vec2f> m_texCoords;
            IndexMap<Vec3> m_normals;

            Object m_currentObject;
            ObjectList m_objects;
        public:
            ObjFileSerializer(FILE* stream);
        private:
            void doBeginFile();
            void doEndFile();
            
            void writeVertices();
            void writeTexCoords();
            void writeNormals();
            void writeObjects();
            void writeFaces(const FaceList& faces);
            
            void doBeginEntity(const Model::Node* node);
            void doEndEntity(Model::Node* node);
            void doEntityAttribute(const Model::EntityAttribute& attribute);
            
            void doBeginBrush(const Model::Brush* brush);
            void doEndBrush(Model::Brush* brush);
            void doBrushFace(Model::BrushFace* face);
        };
    }
}

#endif /* ObjSerializer_h */
