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

#ifndef __TrenchBroom__Mesh__
#define __TrenchBroom__Mesh__

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
        struct MeshRenderData {
            Key key;
            VertexArray triangles;
            VertexArray triangleFans;
            VertexArray triangleStrips;
            
            MeshRenderData(const Key& i_key) :
            key(i_key) {}
            
            typedef std::vector<MeshRenderData<Key> > List;
        };

        template <typename Key, class VertexSpec>
        class Mesh {
        public:
            
            typedef std::vector<typename VertexSpec::Vertex::List> TriangleSeries;
            typedef std::map<Key, typename VertexSpec::Vertex::List> TriangleSetMap;
            typedef std::map<Key, TriangleSeries> TriangleSeriesMap;
        private:
            typedef enum {
                TriangleType_Set,
                TriangleType_Fan,
                TriangleType_Strip,
                TriangleType_Unset
            } TriangleType;

            typedef std::set<Key> KeySet;
            
            KeySet m_keys;
            TriangleSetMap m_triangleSets;
            TriangleSeriesMap m_triangleFans;
            TriangleSeriesMap m_triangleStrips;
            
            typename TriangleSetMap::iterator m_currentSet;
            typename TriangleSeriesMap::iterator m_currentFan;
            typename TriangleSeriesMap::iterator m_currentStrip;
            
            TriangleType m_currentType;
            size_t m_vertexCount;
        public:
            Mesh() :
            m_currentSet(m_triangleSets.end()),
            m_currentFan(m_triangleFans.end()),
            m_currentStrip(m_triangleStrips.end()),
            m_currentType(TriangleType_Unset),
            m_vertexCount(0) {}
            
            size_t size() const {
                return m_vertexCount * VertexSpec::Size;
            }
            
            const TriangleSetMap& triangleSets() const {
                assert(m_currentType == TriangleType_Unset);
                return m_triangleSets;
            }
            
            const TriangleSeriesMap& triangleFans() const {
                assert(m_currentType == TriangleType_Unset);
                return m_triangleFans();
            }
            
            const TriangleSeriesMap& triangleStrips() const {
                assert(m_currentType == TriangleType_Unset);
                return m_triangleStrips;
            }
            
            typename MeshRenderData<Key>::List renderData() {
                typename MeshRenderData<Key>::List result;
                typename KeySet::const_iterator keyIt, keyEnd;
                for (keyIt = m_keys.begin(), keyEnd = m_keys.end(); keyIt != keyEnd; ++keyIt) {
                    const Key& key = *keyIt;
                    typename TriangleSetMap::iterator setIt = m_triangleSets.find(key);
                    typename TriangleSeriesMap::iterator fanIt = m_triangleFans.find(key);
                    typename TriangleSeriesMap::iterator stripIt = m_triangleStrips.find(key);
                    
                    if (setIt == m_triangleSets.end() &&
                        fanIt == m_triangleFans.end() &&
                        stripIt == m_triangleStrips.end())
                        continue;
                    
                    MeshRenderData<Key> renderData(key);
                    
                    if (setIt != m_triangleSets.end()) {
                        typename VertexSpec::Vertex::List& vertices = setIt->second;
                        renderData.triangles = VertexArray::swap(GL_TRIANGLES, vertices);
                    }
                    
                    if (fanIt != m_triangleFans.end()) {
                        TriangleSeries& series = fanIt->second;
                        renderData.triangleFans = triangleSeriesArray(GL_TRIANGLE_FAN, series);
                    }
                    
                    if (stripIt != m_triangleStrips.end()) {
                        TriangleSeries& series = stripIt->second;
                        renderData.triangleStrips = triangleSeriesArray(GL_TRIANGLE_STRIP, series);
                    }
                    
                    result.push_back(renderData);
                }
                return result;
            }
            
            void beginTriangleSet(Key key) {
                assert(m_currentType == TriangleType_Unset);
                m_currentType = TriangleType_Set;
                
                if (m_currentSet == m_triangleSets.end() || m_currentSet->first != key)
                    m_currentSet = MapUtils::findOrInsert(m_triangleSets, key);
                m_keys.insert(key);
            }
            
            void addTriangleToSet(const typename VertexSpec::Vertex& v1,
                                         const typename VertexSpec::Vertex& v2,
                                         const typename VertexSpec::Vertex& v3) {
                assert(m_currentType == TriangleType_Set);
                m_currentSet->second.push_back(v1);
                m_currentSet->second.push_back(v2);
                m_currentSet->second.push_back(v3);
                m_vertexCount += 3;
            }
            
            void addTrianglesToSet(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == TriangleType_Set);
                typename VertexSpec::Vertex::List& setVertices = m_currentSet->second;
                setVertices.insert(setVertices.end(), vertices.begin(), vertices.end());
                m_vertexCount += vertices.size();
            }

            void endTriangleSet() {
                assert(m_currentType == TriangleType_Set);
                m_currentType = TriangleType_Unset;
            }
            
            void addTriangleFans(Key key, const TriangleSeries& fans) {
                if (!fans.empty()) {
                    TriangleSeries& fansForKey = m_triangleFans[key];
                    typename TriangleSeries::const_iterator it, end;
                    for (it = fans.begin(), end = fans.end(); it != end; ++it) {
                        const typename VertexSpec::Vertex::List& vertices = *it;
                        fansForKey.push_back(vertices);
                        m_vertexCount += vertices.size();
                    }
                    m_keys.insert(key);
                }
            }
            
            void beginTriangleFan(Key key) {
                assert(m_currentType == TriangleType_Unset);
                m_currentType = TriangleType_Fan;
                
                if (m_currentFan == m_triangleFans.end() || m_currentFan->first != key)
                    m_currentFan = MapUtils::findOrInsert(m_triangleFans, key);
                m_currentFan->second.push_back(VertexSpec::Vertex::List());
                m_keys.insert(key);
            }
            
            void addVertexToFan(const typename VertexSpec::Vertex& v) {
                assert(m_currentType == TriangleType_Fan);
                m_currentFan->second.back().push_back(v);
                ++m_vertexCount;
            }
            
            void endTriangleFan() {
                assert(m_currentType == TriangleType_Fan);
                m_currentType = TriangleType_Unset;
            }
            
            void addTriangleStrips(Key key, const TriangleSeries& strips) {
                if (!strips.empty()) {
                    TriangleSeries& stripsForKey = m_triangleStrips[key];
                    typename TriangleSeries::const_iterator it, end;
                    for (it = strips.begin(), end = strips.end(); it != end; ++it) {
                        const typename VertexSpec::Vertex::List& vertices = *it;
                        stripsForKey.push_back(vertices);
                        m_vertexCount += vertices.size();
                    }
                    m_keys.insert(key);
                }
            }
            
            void beginTriangleStrip(Key key) {
                assert(m_currentType == TriangleType_Unset);
                m_currentType = TriangleType_Strip;
                
                if (m_currentStrip == m_triangleStrips.end() || m_currentStrip->first != key)
                    m_currentStrip = MapUtils::findOrInsert(m_currentStrip, key);
                m_currentStrip->second.push_back(VertexSpec::Vertex::List());
                m_keys.insert(key);
            }

            void addVertexToStrip(const typename VertexSpec::Vertex& v) {
                assert(m_currentType == TriangleType_Strip);
                m_currentStrip->second.back().push_back(v);
                ++m_vertexCount;
            }

            void endTriangleStrip() {
                assert(m_currentType == TriangleType_Strip);
                m_currentType = TriangleType_Unset;
            }

        private:
            VertexArray triangleSeriesArray(const GLenum primType, TriangleSeries& series) const {
                IndexedVertexList<VertexSpec> indexList;
                typename TriangleSeries::const_iterator sIt, sEnd;
                for (sIt = series.begin(), sEnd = series.end(); sIt != sEnd; ++sIt) {
                    const typename VertexSpec::Vertex::List& vertices = *sIt;
                    indexList.addPrimitive(vertices);
                }
                
                return VertexArray::swap(primType, indexList.vertices(), indexList.indices(), indexList.counts());
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Mesh__) */
