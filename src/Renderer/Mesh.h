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
        public:
            Mesh() :
            m_currentSet(m_triangleSets.end()),
            m_currentFan(m_triangleFans.end()),
            m_currentStrip(m_triangleStrips.end()),
            m_currentType(Unset) {}
            
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
                typename TriangleSetMap::const_iterator it, end;
                for (it = m_triangleSets.begin(), end = m_triangleSets.end(); it != end; ++it) {
                    const Key& key = it->first;
                    const typename VertexSpec::Vertex::List& vertices = it->second;
                    VertexArray array(vbo, GL_TRIANGLES, vertices);
                    result.insert(std::make_pair(key, array));
                }
                return result;
            }
            
            inline VertexArrayMap triangleFanArrays(Vbo& vbo) const {
                return triangleSeriesRenderers(vbo, GL_TRIANGLE_FAN, m_triangleFans);
            }
            
            inline VertexArrayMap triangleStripArrays(Vbo& vbo) const {
                return triangleSeriesRenderers(vbo, GL_TRIANGLE_STRIP, m_triangleStrips);
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
            }

            inline void endTriangleSet() {
                assert(m_currentType == Set);
                m_currentType = Unset;
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
            }
            
            inline void endTriangleFan() {
                assert(m_currentType == Fan);
                m_currentType = Unset;
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
            }

            inline void endTriangleStrip() {
                assert(m_currentType == Strip);
                m_currentType = Unset;
            }

        private:
            inline VertexArrayMap triangleSeriesArrays(Vbo& vbo, const GLenum primType, const TriangleSeriesMap seriesMap) const {
                VertexArrayMap result;
                typename TriangleSeriesMap::const_iterator mIt, mEnd;
                for (mIt = seriesMap.begin(), mEnd = seriesMap.end(); mIt != mEnd; ++mIt) {
                    const Key& key = mIt->first;
                    const TriangleSeries& series = mIt->second;
                    
                    IndexedVertexList<VertexSpec> indexList;
                    typename TriangleSeries::const_iterator sIt, sEnd;
                    for (sIt = series.begin(), sEnd = series.end(); sIt != sEnd; ++sIt) {
                        const typename VertexSpec::Vertex::List& vertices = *sIt;
                        indexList.addVertices(vertices);
                        indexList.endPrimitive();
                    }
                    
                    result.insert(std::make_pair(key, VertexArray(vbo, primType, indexList.vertices(), indexList.indices(), indexList.counts())));
                }
                
                return result;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Mesh__) */
