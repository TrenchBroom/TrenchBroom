/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_Mesh
#define TrenchBroom_Mesh

#include "CollectionUtils.h"
#include "Renderer/IndexedVertexList.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"

#include <cassert>
#include <map>
#include <vector>

// disable warnings about truncated names in MSVC:
#ifdef _MSC_VER
#pragma warning(disable:4503)
#endif

namespace TrenchBroom {
    namespace Renderer {
        template <typename Key>
        struct TriangleMeshRenderData {
            Key key;
            VertexArray triangles;
            VertexArray triangleFans;
            VertexArray triangleStrips;
            
            TriangleMeshRenderData(const Key& i_key) :
            key(i_key) {}
            
            typedef std::vector<TriangleMeshRenderData<Key> > List;
        };

        template <class VertexSpec, typename Key = int>
        class TriangleMesh {
        public:
            class MeshSize {
            public:
                struct Size {
                    size_t setVertexCount;
                    size_t fanPrimitiveCount;
                    size_t fanVertexCount;
                    size_t stripPrimitiveCount;
                    size_t stripVertexCount;
                    Size() :
                    setVertexCount(0),
                    fanPrimitiveCount(0),
                    fanVertexCount(0),
                    stripPrimitiveCount(0),
                    stripVertexCount(0) {}
                };
                
                typedef std::map<Key, Size> KeySizeMap;
                KeySizeMap sizes;
            public:
                void addSet(const Key& key, const size_t vertexCount) {
                    sizes[key].setVertexCount += vertexCount;
                }
                void addFan(const Key& key, const size_t vertexCount) {
                    addFans(key, vertexCount, 1);
                }
                void addFans(const Key& key, const size_t vertexCount, const size_t primCount) {
                    Size& size = sizes[key];
                    size.fanVertexCount += vertexCount;
                    size.fanPrimitiveCount += primCount;
                }
                void addStrip(const Key& key, const size_t vertexCount) {
                    addStrips(key, vertexCount, 1);
                }
                void addStrips(const Key& key, const size_t vertexCount, const size_t primCount) {
                    Size& size = sizes[key];
                    size.stripVertexCount += vertexCount;
                    size.stripPrimitiveCount += primCount;;
                }
            };

            typedef typename VertexSpec::Vertex::List VertexList;
            typedef IndexedVertexList<VertexSpec> IndexedList;
        private:
            struct MeshData {
                VertexList triangleSet;
                IndexedList triangleFans;
                IndexedList triangleStrips;
                
                MeshData() {}
                MeshData(const typename MeshSize::Size& size) :
                triangleSet(0),
                triangleFans(0, 0),
                triangleStrips(0, 0) {
                    triangleSet.reserve(size.setVertexCount);
                    triangleFans.reserve(size.fanVertexCount, size.fanPrimitiveCount);
                    triangleStrips.reserve(size.stripVertexCount, size.stripPrimitiveCount);
                }
            };
            typedef std::map<Key, MeshData> MeshDataMap;
        private:
            typedef enum {
                TriangleType_Set,
                TriangleType_Fan,
                TriangleType_Strip,
                TriangleType_Unset
            } TriangleType;

            MeshDataMap m_meshData;
            typename MeshDataMap::iterator m_currentData;
            TriangleType m_currentType;
            size_t m_vertexCount;
        public:
            TriangleMesh() :
            m_currentData(m_meshData.end()),
            m_currentType(TriangleType_Unset),
            m_vertexCount(0) {}

            TriangleMesh(const MeshSize& meshSize) :
            m_currentData(m_meshData.end()),
            m_currentType(TriangleType_Unset),
            m_vertexCount(0) {
                init(meshSize);
            }

            size_t size() const {
                return m_vertexCount * VertexSpec::Size;
            }
            
            typename TriangleMeshRenderData<Key>::List renderData() {
                typename TriangleMeshRenderData<Key>::List result;
                typename MeshDataMap::iterator it, end;
                for (it = m_meshData.begin(), end = m_meshData.end(); it != end; ++it) {
                    const Key& key = it->first;
                    MeshData& meshData = it->second;
                    
                    VertexList& set = meshData.triangleSet;
                    IndexedList& fans = meshData.triangleFans;
                    IndexedList& strips = meshData.triangleStrips;
                    
                    TriangleMeshRenderData<Key> renderData(key);
                    if (!set.empty())
                        renderData.triangles = VertexArray::swap(GL_TRIANGLES, meshData.triangleSet);
                    if (!fans.empty())
                        renderData.triangleFans = VertexArray::swap(GL_TRIANGLE_FAN, fans.vertices(), fans.indices(), fans.counts());
                    if (!strips.empty())
                        renderData.triangleStrips = VertexArray::swap(GL_TRIANGLE_STRIP, strips.vertices(), strips.indices(), strips.counts());
                    result.push_back(renderData);
                }
                return result;
            }
            
            void beginTriangleSet(const Key& key) {
                begin(TriangleType_Set, key);
            }
            
            void addTriangleToSet(const typename VertexSpec::Vertex& v1,
                                  const typename VertexSpec::Vertex& v2,
                                  const typename VertexSpec::Vertex& v3) {
                assert(m_currentType == TriangleType_Set);
                MeshData& meshData = m_currentData->second;
                meshData.triangleSet.push_back(v1);
                meshData.triangleSet.push_back(v2);
                meshData.triangleSet.push_back(v3);
                m_vertexCount += 3;
            }
            
            void addTrianglesToSet(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == TriangleType_Set);
                assert(vertices.size() % 3 == 0);
                MeshData& meshData = m_currentData->second;
                VectorUtils::append(meshData.triangleSet, vertices);
                m_vertexCount += vertices.size();
            }

            void endTriangleSet() {
                assert(m_currentType == TriangleType_Set);
                end();
            }
            
            void addTriangleFans(const IndexedList& fans, const Key& key = 0) {
                beginTriangleFan(key);
                MeshData& meshData = m_currentData->second;
                meshData.triangleFans.addPrimitives(fans);
                endTriangleFan();
            }
            
            void beginTriangleFan(const Key& key = 0) {
                begin(TriangleType_Fan, key);
            }
            
            void addVertexToFan(const typename VertexSpec::Vertex& v) {
                assert(m_currentType == TriangleType_Fan);
                MeshData& meshData = m_currentData->second;
                meshData.triangleFans.addVertex(v);
                ++m_vertexCount;
            }
            
            void addVerticesToFan(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == TriangleType_Fan);
                MeshData& meshData = m_currentData->second;
                meshData.triangleFans.addVertices(vertices);
                m_vertexCount += vertices.size();;
            }
            
            void addTriangleFan(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == TriangleType_Fan);
                MeshData& meshData = m_currentData->second;
                meshData.triangleFans.addPrimitive(vertices);
                m_vertexCount += vertices.size();;
            }

            void endTriangleFan() {
                assert(m_currentType == TriangleType_Fan);
                MeshData& meshData = m_currentData->second;
                meshData.triangleFans.endPrimitive();
                end();
            }
            
            void addTriangleStrips(const IndexedList& strips, const Key& key = 0) {
                beginTriangleStrip(key);
                MeshData& meshData = m_currentData->second;
                meshData.triangleStrips.addPrimitives(strips);
                endTriangleStrip();
            }
            
            void beginTriangleStrip(const Key& key = 0) {
                begin(TriangleType_Strip, key);
            }
            
            void addVertexToStrip(const typename VertexSpec::Vertex& v) {
                assert(m_currentType == TriangleType_Strip);
                MeshData& meshData = m_currentData->second;
                meshData.triangleStrips.addVertex(v);
                ++m_vertexCount;
            }
            
            void addVerticesToStrip(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == TriangleType_Strip);
                MeshData& meshData = m_currentData->second;
                meshData.triangleStrips.addVertices(vertices);
                m_vertexCount += vertices.size();;
            }
            
            void addTriangleStrip(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == TriangleType_Strip);
                MeshData& meshData = m_currentData->second;
                meshData.triangleStrips.addPrimitive(vertices);
                m_vertexCount += vertices.size();;
            }
            
            void endTriangleStrip() {
                assert(m_currentType == TriangleType_Strip);
                MeshData& meshData = m_currentData->second;
                meshData.triangleStrips.endPrimitive();
                end();
            }
            
            void clear() {
                m_meshData.clear();
                m_currentData = m_meshData.end();
                m_vertexCount = 0;
            }
        private:
            void init(const MeshSize& meshSize) {
                typename MeshSize::KeySizeMap::const_iterator it, end;
                for (it = meshSize.sizes.begin(), end = meshSize.sizes.end(); it != end; ++it) {
                    const Key& key = it->first;
                    const typename MeshSize::Size& size = it->second;
                    MapUtils::insertOrFail(m_meshData, key, MeshData(size));
                }
            }
            
            void begin(const TriangleType type, const Key& key) {
                assert(m_currentType == TriangleType_Unset);
                assert(type != TriangleType_Unset);
                m_currentType = type;
                updateCurrentData(key);
            }
            
            void end() {
                assert(m_currentType != TriangleType_Unset);
                m_currentType = TriangleType_Unset;
            }
            
            void updateCurrentData(const Key& key) {
                if (m_currentData == m_meshData.end() || m_currentData->first != key)
                    m_currentData = MapUtils::findOrInsert(m_meshData, key);
            }
        };
    }
}

#endif /* defined(TrenchBroom_Mesh) */
