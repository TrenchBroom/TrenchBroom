/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Mesh__
#define __TrenchBroom__Mesh__

#include "CollectionUtils.h"
#include "Renderer/IndexedVertexList.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"

#include <cassert>
#include <map>

// disable warnings about truncated names in MSVC:
#ifdef _WIN32
#pragma warning(disable:4503)
#endif

namespace TrenchBroom {
    namespace Renderer {
        template <typename Key, class VertexSpec>
        class Mesh {
        public:
            typedef std::vector<typename VertexSpec::Vertex::List> TriangleSeries;
            typedef std::map<Key, typename VertexSpec::Vertex::List> TriangleSetMap;
            typedef std::map<Key, TriangleSeries> TriangleSeriesMap;
            typedef std::map<Key, VertexArray> VertexArrayMap;
        private:
            typedef enum {
                Set,
                Fan,
                Strip,
                Unset
            } CurrentTriangleType;
            
            TriangleSetMap m_triangleSets;
            TriangleSeriesMap m_triangleFans;
            TriangleSeriesMap m_triangleStrips;
            
            typename TriangleSetMap::iterator m_currentSet;
            typename TriangleSeriesMap::iterator m_currentFan;
            typename TriangleSeriesMap::iterator m_currentStrip;
            
            CurrentTriangleType m_currentType;
            size_t m_vertexCount;
        public:
            Mesh() :
            m_currentSet(m_triangleSets.end()),
            m_currentFan(m_triangleFans.end()),
            m_currentStrip(m_triangleStrips.end()),
            m_currentType(Unset),
            m_vertexCount(0) {}
            
            inline const size_t size() const {
                return m_vertexCount * VertexSpec::Size;
            }
            
            inline const TriangleSetMap& triangleSets() const {
                assert(m_currentType == Unset);
                return m_triangleSets;
            }
            
            inline const TriangleSeriesMap& triangleFans() const {
                assert(m_currentType == Unset);
                return m_triangleFans();
            }
            
            inline const TriangleSeriesMap& triangleStrips() const {
                assert(m_currentType == Unset);
                return m_triangleStrips;
            }
            
            inline VertexArrayMap triangleSetArrays(Vbo& vbo) const {
                VertexArrayMap result;
                triangleSetArrays(vbo, m_triangleSets, result);
                return result;
            }
            
            inline VertexArrayMap triangleFanArrays(Vbo& vbo) const {
                VertexArrayMap result;
                triangleSeriesArrays(vbo, GL_TRIANGLE_FAN, m_triangleFans, result);
                return result;
            }
            
            inline VertexArrayMap triangleStripArrays(Vbo& vbo) const {
                VertexArrayMap result;
                triangleSeriesArrays(vbo, GL_TRIANGLE_STRIP, m_triangleStrips, result);
                return result;
            }
            
            inline VertexArrayMap triangleArrays(Vbo& vbo) const {
                VertexArrayMap result;
                triangleSetArrays(vbo, m_triangleSets, result);
                triangleSeriesArrays(vbo, GL_TRIANGLE_FAN, m_triangleFans, result);
                triangleSeriesArrays(vbo, GL_TRIANGLE_STRIP, m_triangleStrips, result);
                return result;
            }
            
            inline void beginTriangleSet(Key key) {
                assert(m_currentType == Unset);
                m_currentType = Set;
                
                if (m_currentSet == m_triangleSets.end() || m_currentSet->first != key)
                    m_currentSet = MapUtils::findOrInsert(m_triangleSets, key);
            }
            
            inline void addTriangleToSet(const typename VertexSpec::Vertex& v1,
                                         const typename VertexSpec::Vertex& v2,
                                         const typename VertexSpec::Vertex& v3) {
                assert(m_currentType == Set);
                m_currentSet->second.push_back(v1);
                m_currentSet->second.push_back(v2);
                m_currentSet->second.push_back(v3);
                m_vertexCount += 3;
            }
            
            inline void addTrianglesToSet(const typename VertexSpec::Vertex::List& vertices) {
                assert(m_currentType == Set);
                typename VertexSpec::Vertex::List& setVertices = m_currentSet->second;
                setVertices.insert(setVertices.end(), vertices.begin(), vertices.end());
                m_vertexCount += vertices.size();
            }

            inline void endTriangleSet() {
                assert(m_currentType == Set);
                m_currentType = Unset;
            }
            
            inline void addTriangleFans(Key key, const TriangleSeries& fans) {
                if (!fans.empty()) {
                    TriangleSeries& fansForKey = m_triangleFans[key];
                    typename TriangleSeries::const_iterator it, end;
                    for (it = fans.begin(), end = fans.end(); it != end; ++it) {
                        const typename VertexSpec::Vertex::List& vertices = *it;
                        fansForKey.push_back(vertices);
                        m_vertexCount += vertices.size();
                    }
                }
            }
            
            inline void beginTriangleFan(Key key) {
                assert(m_currentType == Unset);
                m_currentType = Fan;
                
                if (m_currentFan == m_triangleFans.end() || m_currentFan->first != key)
                    m_currentFan = MapUtils::findOrInsert(m_triangleFans, key);
                m_currentFan->second.push_back(VertexSpec::Vertex::List());
            }
            
            inline void addVertexToFan(const typename VertexSpec::Vertex& v) {
                assert(m_currentType == Fan);
                m_currentFan->second.back().push_back(v);
                ++m_vertexCount;
            }
            
            inline void endTriangleFan() {
                assert(m_currentType == Fan);
                m_currentType = Unset;
            }
            
            inline void addTriangleStrips(Key key, const TriangleSeries& strips) {
                if (!strips.empty()) {
                    TriangleSeries& stripsForKey = m_triangleStrips[key];
                    typename TriangleSeries::const_iterator it, end;
                    for (it = strips.begin(), end = strips.end(); it != end; ++it) {
                        const typename VertexSpec::Vertex::List& vertices = *it;
                        stripsForKey.push_back(vertices);
                        m_vertexCount += vertices.size();
                    }
                }
            }
            
            inline void beginTriangleStrip(Key key) {
                assert(m_currentType == Unset);
                m_currentType = Strip;
                
                if (m_currentStrip == m_triangleStrips.end() || m_currentStrip->first != key)
                    m_currentStrip = MapUtils::findOrInsert(m_currentStrip, key);
                m_currentStrip->second.push_back(VertexSpec::Vertex::List());
            }

            inline void addVertexToStrip(const typename VertexSpec::Vertex& v) {
                assert(m_currentType == Strip);
                m_currentStrip->second.back().push_back(v);
                ++m_vertexCount;
            }

            inline void endTriangleStrip() {
                assert(m_currentType == Strip);
                m_currentType = Unset;
            }

        private:
            inline void triangleSetArrays(Vbo& vbo, const TriangleSetMap& setMap, VertexArrayMap& result) const {
                typename TriangleSetMap::const_iterator it, end;
                for (it = setMap.begin(), end = setMap.end(); it != end; ++it) {
                    const Key& key = it->first;
                    const typename VertexSpec::Vertex::List& vertices = it->second;
                    VertexArray array(vbo, GL_TRIANGLES, vertices);
                    result.insert(std::make_pair(key, array));
                }
            }
            
            inline void triangleSeriesArrays(Vbo& vbo, const GLenum primType, const TriangleSeriesMap seriesMap, VertexArrayMap& result) const {
                typename TriangleSeriesMap::const_iterator mIt, mEnd;
                for (mIt = seriesMap.begin(), mEnd = seriesMap.end(); mIt != mEnd; ++mIt) {
                    const Key& key = mIt->first;
                    const TriangleSeries& series = mIt->second;
                    
                    IndexedVertexList<VertexSpec> indexList;
                    typename TriangleSeries::const_iterator sIt, sEnd;
                    for (sIt = series.begin(), sEnd = series.end(); sIt != sEnd; ++sIt) {
                        const typename VertexSpec::Vertex::List& vertices = *sIt;
                        indexList.addPrimitive(vertices);
                    }
                    
                    result.insert(std::make_pair(key, VertexArray(vbo, primType, indexList.vertices(), indexList.indices(), indexList.counts())));
                }
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Mesh__) */
