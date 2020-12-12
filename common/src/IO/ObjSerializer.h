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

#pragma once

#include "FloatType.h"
#include "IO/NodeSerializer.h"
#include "IO/IOUtils.h"
#include "IO/Path.h"

#include <vecmath/forward.h>

#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class BrushNode;
        class BrushFace;
        class EntityProperty;
        class Node;
    }

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
                    const auto it = m_map.insert({ v, m_list.size() }).first;
                    const size_t index = it->second;
                    if (index == m_list.size()) {
                        m_list.push_back(v);
                    }
                    return index;
                }

                /**
                 * Values inserted after this is called will not reuse indices from before this
                 * is called.
                 */
                void clearIndices() {
                    m_map.clear();
                }
            };

            struct IndexedVertex {
                size_t vertex;
                size_t texCoords;
                size_t normal;

                IndexedVertex(size_t i_vertex, size_t i_texCoords, size_t i_normal);
            };

            using IndexedVertexList = std::vector<IndexedVertex>;

            struct Face {
                IndexedVertexList verts;
                std::string textureName;
                const Assets::Texture* texture;

                Face(IndexedVertexList i_verts, std::string i_textureName, const Assets::Texture* i_texture);
            };

            using FaceList = std::vector<Face>;

            struct Object {
                size_t entityNo;
                size_t brushNo;
                FaceList faces;
            };

            using ObjectList = std::vector<Object>;

            Path m_objPath;
            Path m_mtlPath;

            IO::OpenFile m_objFile;
            IO::OpenFile m_mtlFile;

            FILE* m_stream;
            FILE* m_mtlStream;

            IndexMap<vm::vec3> m_vertices;
            IndexMap<vm::vec2f> m_texCoords;
            IndexMap<vm::vec3> m_normals;

            Object m_currentObject;
            ObjectList m_objects;
        public:
            explicit ObjFileSerializer(const Path& path);
        private:
            void doBeginFile(const std::vector<const Model::Node*>& rootNodes) override;
            void doEndFile() override;

            void writeMtlFile();

            void writeVertices();
            void writeTexCoords();
            void writeNormals();
            void writeObjects();
            void writeFaces(const FaceList& faces);

            void doBeginEntity(const Model::Node* node) override;
            void doEndEntity(const Model::Node* node) override;
            void doEntityAttribute(const Model::EntityProperty& attribute) override;

            void doBrush(const Model::BrushNode* brush) override;
            void doBrushFace(const Model::BrushFace& face) override;
        };
    }
}

