/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef TrenchBroom_TexturedPolygonSorter_h
#define TrenchBroom_TexturedPolygonSorter_h

namespace TrenchBroom {
    namespace Renderer {
        template <typename T>
        class TexturedPolygonSorter {
        private:
            class CompareTexturesById {
            public:
                inline bool operator() (const Model::Texture* left, const Model::Texture* right) const {
                    return left->uniqueId() < right->uniqueId();
                }
            };
        public:
            typedef std::vector<T> PolygonList;
            class PolygonCollection {
            private:
                PolygonList m_polygons;
                size_t m_vertexCount;
            public:
                PolygonCollection() :
                m_vertexCount(0) {}
                
                inline const PolygonList& polygons() const {
                    return m_polygons;
                }
                
                inline size_t vertexCount() const {
                    return m_vertexCount;
                }
                
                inline void addPolygon(T polygon, size_t vertexCount) {
                    m_polygons.push_back(polygon);
                    m_vertexCount += vertexCount;
                }
            };

            typedef std::map<Model::Texture*, PolygonCollection, CompareTexturesById> PolygonCollectionMap;
        private:
            
            PolygonCollectionMap m_polygonCollections;
            typename PolygonCollectionMap::iterator m_lastPolygonCollection;
        public:
            TexturedPolygonSorter() :
            m_lastPolygonCollection(m_polygonCollections.end()) {}
            
            inline void addPolygon(Model::Texture* texture, T polygon, size_t vertexCount) {
                if (m_lastPolygonCollection == m_polygonCollections.end() || m_lastPolygonCollection->first != texture)
                    m_lastPolygonCollection = m_polygonCollections.insert(std::pair<Model::Texture*, PolygonCollection>(texture, PolygonCollection())).first;
                
                m_lastPolygonCollection->second.addPolygon(polygon, vertexCount);
            }
            
            inline bool empty() const {
                return m_polygonCollections.empty();
            }
            
            inline const PolygonCollectionMap& collections() const {
                return m_polygonCollections;
            }
        };
    }
}

#endif
