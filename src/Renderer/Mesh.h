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
#include "Renderer/Vertex.h"

#include <cassert>
#include <map>

namespace TrenchBroom {
    namespace Renderer {
        template <typename Key, class Vertex>
        class Mesh {
        public:
            typedef std::vector<typename Vertex::List> TriangleSeries;
            typedef std::map<Key, typename Vertex::List> TriangleSetMap;
            typedef std::map<Key, TriangleSeries> TriangleSeriesMap;
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
            
            inline void beginTriangleSet(Key texture) {
                assert(m_currentType == Unset);
                m_currentType = Set;
                
                if (m_currentSet == m_triangleSets.end() || m_currentSet->first != texture)
                    m_currentSet = MapUtils::findOrInsert(m_triangleSets, texture);
            }
            
            inline void addTriangleToSet(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
                assert(m_currentType == Set);
                m_currentSet->second.push_back(v1);
                m_currentSet->second.push_back(v2);
                m_currentSet->second.push_back(v3);
            }

            inline void endTriangleSet() {
                assert(m_currentType == Set);
                m_currentType = Unset;
            }
            
            inline void beginTriangleFan(Key texture) {
                assert(m_currentType == Unset);
                m_currentType = Fan;
                
                if (m_currentFan == m_triangleFans.end() || m_currentFan->first != texture)
                    m_currentFan = MapUtils::findOrInsert(m_triangleFans, texture);
                m_currentFan->second.push_back(Vertex::List());
            }
            
            inline void addVertexToFan(const Vertex& v) {
                assert(m_currentType == Fan);
                m_currentFan->second.back().push_back(v);
            }
            
            inline void endTriangleFan() {
                assert(m_currentType == Fan);
                m_currentType = Unset;
            }
            
            inline void beginTriangleStrip(Key texture) {
                assert(m_currentType == Unset);
                m_currentType = Strip;
                
                if (m_currentStrip == m_triangleStrips.end() || m_currentStrip->first != texture)
                    m_currentStrip = MapUtils::findOrInsert(m_currentStrip, texture);
                m_currentStrip->second.push_back(Vertex::List());
            }

            inline void addVertexToStrip(const Vertex& v) {
                assert(m_currentType == Strip);
                m_currentStrip->second.back().push_back(v);
            }

            inline void endTriangleStrip() {
                assert(m_currentType == Strip);
                m_currentType = Unset;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Mesh__) */
