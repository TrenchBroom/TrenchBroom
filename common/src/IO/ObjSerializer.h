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

#ifndef ObjSerializer_h
#define ObjSerializer_h

#include "IO/NodeSerializer.h"
#include "Model/ModelTypes.h"

#include <vecmath/forward.h>

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
                using List = std::vector<V>;
            private:
                using Map = std::map<V, size_t>;
                Map m_map;
                List m_list;
            public:
                const List& list() const {
                    return m_list;
                }

                size_t index(const V& v) {
                    typename Map::iterator indexIt = MapUtils::findOrInsert(m_map, v, m_list.size());
                    const size_t index = indexIt->second;
                    if (index == m_list.size())
                        m_list.push_back(v);
                    return index;
                }
            };

            struct IndexedVertex {
                size_t vertex;
                size_t texCoords;
                size_t normal;

                IndexedVertex(size_t i_vertex, size_t i_texCoords, size_t i_normal);
            };

            using IndexedVertexList = std::vector<IndexedVertex>;
            using FaceList = std::list<IndexedVertexList>;

            struct Object {
                size_t entityNo;
                size_t brushNo;
                FaceList faces;
            };

            using ObjectList = std::list<Object>;

            FILE* m_stream;

            IndexMap<vm::vec3> m_vertices;
            IndexMap<vm::vec2f> m_texCoords;
            IndexMap<vm::vec3> m_normals;

            Object m_currentObject;
            ObjectList m_objects;
        public:
            ObjFileSerializer(FILE* stream);
        private:
            void doBeginFile() override;
            void doEndFile() override;

            void writeVertices();
            void writeTexCoords();
            void writeNormals();
            void writeObjects();
            void writeFaces(const FaceList& faces);

            void doBeginEntity(const Model::Node* node) override;
            void doEndEntity(Model::Node* node) override;
            void doEntityAttribute(const Model::EntityAttribute& attribute) override;

            void doBeginBrush(const Model::Brush* brush) override;
            void doEndBrush(Model::Brush* brush) override;
            void doBrushFace(Model::BrushFace* face) override;
        };
    }
}

#endif /* ObjSerializer_h */
